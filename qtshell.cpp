#include <QDir>
#include <QQueue>
#include "qtshell.h"

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

QStringList QtShell::find(const QString &root, const QStringList &nameFilters)
{
    QDir dir(root);
    QString absRoot = dir.absolutePath();

    QQueue<QString> queue;
    queue.enqueue(absRoot);
    QStringList result;

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
                continue;
            }

            QString fileName = info.fileName();

            if (nameFilters.size() > 0 && !match(fileName, nameFilters)) {
                continue;
            }

            if (root != absRoot) {
                absPath.replace(absRoot, root);
            }

            result << absPath;
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

    int i = token.size() -1;
    while (i >=0) {
        QString name = token.at(i--);
        if (!name.isEmpty()) {
            result = name;
            break;
        }
    }

    return result;
}
