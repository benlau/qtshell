#include <QRegExp>
#include "qtshellpriv.h"

QString QtShell::Private::normalize(QString path)
{
    if (path.count("/") == path.size()) {
        return "/";
    }

    path.remove(QRegExp("/*$"));
    return path;
}
