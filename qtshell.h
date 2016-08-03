#ifndef QTSHELL_H
#define QTSHELL_H

#include <QStringList>

namespace QtShell {

    QString dirname(const QString& path);

    QString basename(const QString& path);

    QStringList find(const QString& path, const QStringList& nameFilters = QStringList());

    bool rmdir(const QString& path);

    bool touch(const QString &path);

}

#endif // QTSHELL_H
