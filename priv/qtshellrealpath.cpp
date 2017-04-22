#include "qtshell.h"
#include "priv/qtshellpriv.h"

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
        return input;
    }

    return info.absoluteFilePath();
}
