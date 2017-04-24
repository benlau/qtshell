#include "qtshell.h"
#include "priv/qtshellpriv.h"

using namespace QtShell::Private;

QString QtShell::realpath(const QString &file) {
    QString input = file;

    QUrl url(input);

    if (url.scheme() == "file") {
        input = url.path();
    } else if (url.scheme() == "qrc") {
        input = QString(":") + url.path();
    }

    QFileInfo info(input);

    if (info.isAbsolute()) {
        return normalize(input);
    }

    return normalize(info.absoluteFilePath());
}

QString QtShell::realpath(const QString &basePath, const QString &subPath)
{
    QString res;
    QString p = realpath(basePath); // No trailing "/"

    if (subPath.size() == 0) {
        return p;
    }

    if (subPath[0] == QChar('/')) {
        res = realpath(basePath + subPath);
    } else {
        res = realpath(basePath + "/" + subPath);
    }

    return res;
}
