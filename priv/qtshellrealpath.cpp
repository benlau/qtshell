#include "qtshell.h"
#include "priv/qtshellpriv.h"

using namespace QtShell::Private;

QString QtShell::realpath_strip(const QString &file) {
    QString input = file;

    QUrl url(input);

    if (url.scheme() == "file") {
        input = url.path();
    } else if (url.scheme() == "qrc") {
        input = QString(":") + url.path();
    }

    QFileInfo info(input);

    if (info.isAbsolute()) {
        return canonicalPath(input);
    }

    return canonicalPath(info.absoluteFilePath());
}

QString QtShell::realpath_strip(const QString &basePath, const QString &subPath)
{
    QString res;
    QString p = realpath_strip(basePath); // No trailing "/"

    if (subPath.size() == 0) {
        return p;
    }

    if (subPath[0] == QChar('/')) {
        res = realpath_strip(basePath + subPath);
    } else {
        res = realpath_strip(basePath + "/" + subPath);
    }

    return res;
}
