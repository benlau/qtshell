#include <QtCore>
#include <QDir>
#include <QQueue>
#include <QCommandLineParser>

#ifdef WIN32
#include <sys/utime.h>
#else
#include <utime.h>
#endif

#include "qtshell.h"

static QString normalize(QString path) {

    if (path.lastIndexOf("/") == path.size() - 1) {
        path.remove(path.size() - 1,1);
    }

    return path;
}

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

    QStringList result;

    auto resolve = [=](QString path) {
        return path.replace(absRoot, root);
    };

    auto append = [&](QString path) {
        if (nameFilters.size() > 0 && !match(path, nameFilters)) {
            return;
        }

        result << resolve(path);
    };

    QQueue<QString> queue;
    queue.enqueue(absRoot);
    append(absRoot);

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
                append(absPath);
                continue;
            }

            append(absPath);
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

    return token.last();
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

static bool _rm(const QString &file,
                 bool recursive = false,
                 bool verbose = false)
{
    QString path = file;

    if (path.isEmpty()) {
        qWarning() << "rm: it do not accept empty argument";
        return false;
    }

    path = normalize(path);

    bool res = true;
    QString folder = QtShell::dirname(path);
    QString filter = QtShell::basename(path);

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
                if (verbose) { qDebug().noquote() << file.absoluteFilePath();}
                if (!dir.removeRecursively()) {
                    res = false;
                    qWarning() << QString("rm: %1: can not remove the directory").arg(file.absoluteFilePath());
                }
            }
            continue;
        }

        if (verbose) { qDebug().noquote() << file.absoluteFilePath();}
        if (!QFile::remove(file.absoluteFilePath()) ) {
            qWarning() << QString("rm: %1: can not remove the file").arg(file.fileName());
            res = false;
        }
    }

    return res;
}

bool QtShell::rm(const QString &options, const QString &file)
{
    QCommandLineParser parser;
    parser.addOption(QCommandLineOption("v"));
    parser.addOption(QCommandLineOption("r"));
    parser.addOption(QCommandLineOption("R"));
    parser.addOption(QCommandLineOption("f")); // dummy

    if (!parser.parse(QStringList() << "rm" << options)) {
        qWarning() << QString("rm: %1").arg(parser.errorText());
        return false;
    }

    bool recursive = parser.isSet("r") || parser.isSet("R");
    bool verbose = parser.isSet("v");

    return _rm(file, recursive, verbose);
}

bool QtShell::rm(const QString &file)
{
    return _rm(file);
}


bool QtShell::mkdir(const QString &path)
{
    QDir dir(path);

    if (dir.exists()) {
        qWarning() << QString("mkdir: %1: File exists").arg(path);
        return false;
    }

    QString folder = dirname(path);
    QString file = basename(path);
    dir = QDir(folder);

    return dir.mkdir(file);
}

bool QtShell::mkdir(const QString &options, const QString &path)
{
    QCommandLineParser parser;
    parser.addOption(QCommandLineOption("p"));

    if (!parser.parse(QStringList() << "mkdir" << options)) {
        qWarning() << QString("mkdir: %1").arg(parser.errorText());
        return false;
    }

    bool p = parser.isSet("p");

    if (!p) {
        return mkdir(path);
    }

    QDir dir;
    return dir.mkpath(path);
}

static bool _cp(QString source, QString target, bool verbose = false) {
    if (source.isEmpty() || target.isEmpty()) {
        qWarning() << "cp(const QString &source, const QString &target)";
        return false;
    }

    source = normalize(source);

    target = normalize(target);

    bool res = true;
    QString folder = QtShell::dirname(source);
    QString filter = QtShell::basename(source);

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

        if (verbose) {
            qDebug().noquote() << QString("%1 -> %2").arg(file.absoluteFilePath()).arg(targetFile);
        }
        if (!QFile::copy(file.absoluteFilePath(), targetFile)) {
            qWarning() << QString("cp: %1: Failed to copy to %2").arg(file.fileName()).arg(target);
            res = false;
        }
    }

    return res;
}

bool QtShell::cp(const QString &source, const QString &target)
{
    return _cp(source, target);
}

bool QtShell::cp(const QString& options, const QString& source , const QString &target) {

    QCommandLineParser parser;
    parser.addOption(QCommandLineOption("v"));

    if (!parser.parse(QStringList() << "cp" << options)) {
        qWarning() << QString("cp: %1").arg(parser.errorText());
        return false;
    }

    bool verbose = parser.isSet("v");

    return _cp(source, target, verbose);

}


