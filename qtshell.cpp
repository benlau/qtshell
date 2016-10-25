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

    auto match = [](const QString&fileName,const QStringList &nameFilters) {
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
    };

    auto append = [&](const QString& absPath, const QString& fileName) {
        if (nameFilters.size() > 0 && !match(fileName, nameFilters)) {
            return;
        }

        result << resolve(absPath);
    };

    QQueue<QString> queue;
    queue.enqueue(absRoot);
    append(absRoot, "");

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
                append(absPath, info.fileName());
                continue;
            }

            append(absPath, info.fileName());
        }
    }

    return result;
}

QString QtShell::dirname(const QString &path)
{
    // Don't use QFileInfo.absolutePath() since it return absolute path.
    // The behaviour is different Unix's dirname command
    QStringList token = path.split("/");
    QString result = "/";

    if (token.size() == 1) {
        result = ".";
    } else {
        token.takeLast();
        result = token.join("/");
    }

    if (result.isEmpty()) {
        result = "/";
    }

    return result;
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
        QByteArray bytes = path.toUtf8();

        if (utime(bytes.constData(), 0) == -1) {
            qWarning() << "utimes failed:" << path;
            res = false;
        }
    }

    return res;
}

static bool _rm(const QString &file,
                 bool recursive = false,
                 bool verbose = false,
                 bool force = false)
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
        if (!force) {
            qWarning() << QString("rm: %1: No such file or directory").arg(filter);
            return false;
        }

        // If force is true, remove a non existed files is not a problem
        return true;
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
    bool force = parser.isSet("f");

    return _rm(file, recursive, verbose, force);
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

static bool _cp(QString source, QString target,
                bool recursive = false,
                bool verbose = false) {

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

        QString targetFile = target;

        if (targetInfo.isDir()) {
            targetFile = target + "/" + file.fileName();
        }

        if (file.isDir()) {

            if (!recursive) {
                qWarning() << QString("cp: %1 is a directory (not copied)").arg(file.fileName());
                res = false;
            } else {

                QtShell::mkdir(targetFile);
                QDir nextDir(file.absoluteFilePath());

                if (nextDir.entryList().size() > 2) { // except "." && ".."
                    if (!_cp(file.absoluteFilePath() + "/*", targetFile, recursive, verbose)) {
                        res = false;
                    }
                }
            }
            continue;
        }

        if (verbose) {
            qDebug().noquote() << QString("%1 -> %2").arg(file.absoluteFilePath()).arg(targetFile);
        }

        if (QFile::exists(targetFile)) {
            if (!QFile::remove(targetFile)) {
                qWarning() << QString("cp: %1: Failed to overwrite to %2").arg(file.fileName()).arg(target);
                res = false;
                continue;
            }
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
    parser.addOption(QCommandLineOption("R"));
    parser.addOption(QCommandLineOption("a"));

    if (!parser.parse(QStringList() << "cp" << options)) {
        qWarning() << QString("cp: %1").arg(parser.errorText());
        return false;
    }

    bool recursive = parser.isSet("R") || parser.isSet("a");
    bool verbose = parser.isSet("v");

    return _cp(source, target, recursive, verbose);

}


QStringList QtShell::find(const QString &path, const QString &nameFilter)
{
    QStringList nameFilters;
    if (!nameFilter.isEmpty()) {
        nameFilters << nameFilter;
    }

    return find(path, nameFilters);
}

QString QtShell::pwd()
{
    return QDir::currentPath();
}

QString QtShell::cat(const QString &file)
{
    QFileInfo info(file);

    if (!info.exists()) {
        qWarning() << QString("cat: %1: No such file or directory").arg(file);
        return "";
    }

    QFile f(file);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << QString("cat: %1: %2").arg(f.errorString());
        return "";
    }

    QByteArray content = f.readAll();

    return content;
}

QString QtShell::cat(const QStringList &files)
{
    QString content;

    for (int i = 0 ; i < files.size() ; i++) {
        if (i != 0) {
            content = content + "\n";
        }
        content = content + cat(files[i]);
    }

    return content;
}
