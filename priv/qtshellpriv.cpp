#include <QRegExp>
#include <QtShell>
#include "qtshellpriv.h"

using namespace QtShell::Private;

QString QtShell::Private::normalize(QString path)
{
    if (path.count("/") == path.size()) {
        return "/";
    }

    path.remove(QRegExp("/*$"));
    return path;
}

int QtShell::Private::bulk(const QString &source, const QString &target, std::function<bool (const QString &, const QString &, const QFileInfo &)> predicate)
{
    QString s = normalize(source);
    QString t = normalize(target);

    QString folder = QtShell::dirname(s);
    QString filter = QtShell::basename(s);

    QDir sourceDir(folder);
    QList<QFileInfo> files = sourceDir.entryInfoList(QStringList() << filter,
                                                     QDir::AllEntries | QDir::NoDot | QDir::NoDotDot);

    QFileInfo targetInfo(t);

    bool unexceptedError = false;

    if (files.size() > 1 && !targetInfo.isDir()) {
        return INVALID_TARGET;
    }

    if (files.size() == 0) {
        return NO_SUCH_FILE_OR_DIR;
    }

    foreach (QFileInfo file, files) {
        QString to = t;
        QString from = file.fileName();

        if (!folder.isEmpty()) {
            from = folder + "/" + from;
        }

        if (targetInfo.isDir()) {
            to = t + "/" + file.fileName();
        }

        unexceptedError = !predicate(from, to, file) || unexceptedError;
    }

    if (unexceptedError) {
        return UNEXCEPTED_ERROR;
    }

    return NO_ERROR;
}
