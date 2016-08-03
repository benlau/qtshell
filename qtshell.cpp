#include <QtCore>
#include <QDir>
#include <QQueue>

#ifdef WIN32
#include <sys/utime.h>
#else
#include <utime.h>
#endif

#include "qtshell.h"

static bool match(const QString&fileName,const QStringList &nameFilters) {
    bool res = false;

    for (int i = 0 ; i < nameFilters.size() ; i++) {
        const QString& filter = nameFilters.at(i);
        QRegExp rx(filter,Qt::CaseInsensitive,QRegExp::Wildcard);
        if (rx.exactMatch(fileName))  {
            res = true;
            break;
        }
    }

    return res;
}

/// Take out "." and ".." files
static QStringList filterLocalFiles(const QStringList& files) {
    QStringList result;
    for (int i = 0 ; i < files.size() ; i++) {
        QString file = files[i];
        if (file == "." || file == "..") {
            continue;
        }
        result << file;
    }

    return result;
}

static QList<QFileInfo> filterLocalFiles(const QList<QFileInfo>& files) {
    QList<QFileInfo> result;
    for (int i = 0 ; i < files.size() ; i++) {
        QFileInfo file = files[i];
        if (file.fileName() == "." || file.fileName() == "..") {
            continue;
        }
        result << file;
    }

    return result;
}

static QStringList preservedPaths() {
    QStringList preservePaths;
    preservePaths << "/";

    for (int i = QStandardPaths::DesktopLocation ;
         i <= QStandardPaths::AppConfigLocation; i++) {
        QStringList paths =  QStandardPaths::standardLocations((QStandardPaths::StandardLocation) i);
        preservePaths.append(paths);
    }

    return preservePaths;
}

QStringList QtShell::find(const QString &root, const QStringList &nameFilters)
{
    QDir dir(root);
    QString absRoot = dir.absolutePath();

    QQueue<QString> queue;
    queue.enqueue(absRoot);
    QStringList result;

    while (queue.size() > 0) {
        QString current = queue.dequeue();
        QDir dir(current);
        QFileInfoList infos = dir.entryInfoList();

        for (int i = 0 ; i < infos.size() ; i++) {
            QFileInfo info = infos.at(i);

            if (info.fileName() == "." || info.fileName() == "..") {
                continue;
            }

            QString absPath = info.absoluteFilePath();

            if (info.isDir()) {
                queue.enqueue(absPath);
                continue;
            }

            QString fileName = info.fileName();

            if (nameFilters.size() > 0 && !match(fileName, nameFilters)) {
                continue;
            }

            if (root != absRoot) {
                absPath.replace(absRoot, root);
            }

            result << absPath;
        }
    }

    return result;
}

QString QtShell::dirname(const QString &path)
{
    QFileInfo info(path);
    QString parent = info.absolutePath();
    if (parent.isEmpty()) {
       parent = "/";
    }

    // No tailing "/" for dirname command

    return parent;
}

QString QtShell::basename(const QString &path)
{
    QString result = "/";
    QStringList token = path.split("/");

    qDebug() << token;

    int i = token.size() -1;
    while (i >=0) {
        QString name = token.at(i--);
        qDebug() << name << i;
        if (!name.isEmpty()) {
            result = name;
            break;
        }
    }

    return result;
}

bool QtShell::rmdir(const QString &path)
{
    QDir dir(path);

    QStringList entry = filterLocalFiles(dir.entryList());
    if (entry.size() > 0) {
        qWarning() << QString("rmdir: %1: Directory not empty").arg(path);
        return false;
    }

    return dir.removeRecursively();
}

bool QtShell::touch(const QString &path)
{
    bool res = true;
    QFileInfo info(path);

    if (!info.exists()) {

        QFile file(path);
        if (!file.open(QIODevice::WriteOnly)) {
            qWarning() << "Failed to create file:" << path;
            res = false;
        }
        file.close();

    } else {

        if (utime(path.toLocal8Bit().constData(), 0) == -1) {
            qWarning() << "utimes failed:" << path;
            res = false;
        }
    }

    return res;
}

bool QtShell::rm(const QString &file, bool recursive)
{
    if (file.isEmpty()) {
        qWarning() << "rm: it do not accept empty argument";
        return false;
    }

    bool res = true;
    QString folder = dirname(file);
    QString filter = basename(file);

    QDir dir(folder);

    QStringList preservePaths = preservedPaths();

    QList<QFileInfo> files = dir.entryInfoList(QStringList() << filter);
    files = filterLocalFiles(files);

    if (files.size() == 0) {
        qWarning() << QString("rm: %1: No such file or directory").arg(filter);
        return false;
    }

    foreach (QFileInfo file, files) {
        if (preservePaths.indexOf(file.absoluteFilePath()) >= 0) {
            qWarning() << QString("rm: %1: is a preserved directory").arg(file.absoluteFilePath());
            continue;
        }
        if (file.isDir()) {

            if (!recursive) {
                qWarning() << QString("rm: %1: is a directory").arg(file.fileName());
                res = false;
            } else {
                QDir dir(file.absoluteFilePath());
                if (!dir.removeRecursively()) {
                    res = false;
                    qWarning() << QString("rm: %1: can not remove the directory").arg(file.absoluteFilePath());
                }
            }
            continue;
        }

        if (!QFile::remove(file.absoluteFilePath()) ) {
            qWarning() << QString("rm: %1: can not remove the file").arg(file.fileName());
            res = false;
        }
    }

    return res;
}

bool QtShell::mkdir(const QString &path)
{
    QDir dir(path);

    if (dir.exists()) {
        qWarning() << QString("mkdir: %1: File exists").arg(path);
        return false;
    }

    return dir.mkpath(path);
}

bool QtShell::cp(const QString &source, const QString &target)
{
    if (source.isEmpty() || target.isEmpty()) {
        qWarning() << "cp(const QString &source, const QString &target)";
        return false;
    }

    bool res = true;
    QString folder = dirname(source);
    QString filter = basename(source);

    QDir dir(folder);

    QList<QFileInfo> files = dir.entryInfoList(QStringList() << filter);
    files = filterLocalFiles(files);

    if (files.size() == 0) {
        qWarning() << QString("cp: %1: No such file or directory").arg(filter);
        return false;
    }

    QFileInfo targetInfo(target);

    foreach (QFileInfo file, files) {
        if (file.isDir()) {
            qWarning() << QString("cp: %1 is a directory (not copied)").arg(file.fileName());
            continue;
        }

        QString targetFile = target;

        if (targetInfo.isDir()) {
            targetFile = target + "/" + file.fileName();
        }

        if (!QFile::copy(file.absoluteFilePath(), targetFile)) {
            qWarning() << QString("cp: %1: Failed to copy to %2").arg(file.fileName()).arg(target);
            res = false;
        }
    }

    return res;
}
