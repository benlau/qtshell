#ifndef QTSHELLPRIV_H
#define QTSHELLPRIV_H

#include <QDir>
#include <QString>
#include <QtShell>
#include <QtCore>

namespace QtShell {

    namespace Private {

        /// Remove tailing "/" from a path.
        QString normalize(QString path);

        typedef enum {
            NO_ERROR = 0,
            INVALID_TARGET = -1,
            NO_SUCH_FILE_OR_DIR = -2
        } BulkError ;

        // Process a bulk list of files from source to target
        template <typename P>
        int bulk(const QString& source, const QString& target, P predicate) {
            QString folder = QtShell::dirname(source);
            QString filter = QtShell::basename(source);

            QDir sourceDir(folder);
            QList<QFileInfo> files = sourceDir.entryInfoList(QStringList() << filter,
                                                       QDir::AllEntries | QDir::NoDot | QDir::NoDotDot);

            QFileInfo targetInfo(target);

            if (files.size() >= 1 && !targetInfo.isDir()) {
                return INVALID_TARGET;
            }

            if (files.size() == 0) {
                return NO_SUCH_FILE_OR_DIR;
            }

            foreach (QFileInfo file, files) {
                QString to = target;
                QString from = file.fileName();

                if (!folder.isEmpty()) {
                    from = folder + "/" + from;
                }

                if (targetInfo.isDir()) {
                    to = target + "/" + file.fileName();
                }

                bool cont = predicate(from, to, file);
                if (!cont) {
                    break;
                }
            }

            return NO_ERROR;
        }
    }
}

#endif // QTSHELLPRIV_H
