#include "qtshell.h"
#include "priv/qtshellpriv.h"

using namespace QtShell::Private;

static bool _mv(const QString &source, const QString &target) {
    if (source.isEmpty() || target.isEmpty()) {
        qWarning() << "usage: mv(source, target)";
        return false;
    }

    return QtShell::Private::bulk(source, target, [&](const QString& from , const QString& to, const QFileInfo& fromInfo){
        Q_UNUSED(fromInfo);

        QDir dir;
        return dir.rename(from, to);
    });
}

bool QtShell::mv(const QString &source, const QString &target) {
    return _mv(source, target) == NO_ERROR;
}
