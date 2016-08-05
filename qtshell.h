#ifndef QTSHELL_H
#define QTSHELL_H

#include <QStringList>

namespace QtShell {

    QString dirname(const QString& path);

    QString basename(const QString& path);

    QStringList find(const QString& path, const QStringList& nameFilters = QStringList());

    bool rmdir(const QString& path);

    bool touch(const QString &path);

    bool rm(const QString& file);

    bool rm(const QString& options,const QString& file);

    bool mkdir(const QString &path);

    bool mkdir(const QString &options, const QString &path);

    bool cp(const QString& source , const QString &target);
    bool cp(const QString& options, const QString& source , const QString &target);


}

#endif // QTSHELL_H
