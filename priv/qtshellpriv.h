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
            NO_SUCH_FILE_OR_DIR = -2,
            UNEXCEPTED_ERROR = -3
        } BulkError ;

        // Process a bulk list of files from source to target
        template <typename P>
        int bulk(const QString& source, const QString& target, P predicate) {

            QString s = normalize(source);
            QString t = normalize(target);

            QString folder = QtShell::dirname(s);
            QString filter = QtShell::basename(s);

            QDir sourceDir(folder);
            QList<QFileInfo> files = sourceDir.entryInfoList(QStringList() << filter,
                                                       QDir::AllEntries | QDir::NoDot | QDir::NoDotDot);

            QFileInfo targetInfo(t);

            bool unexceptedError = false;

            if (files.size() >= 1 && !targetInfo.isDir()) {
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
    }
}

#endif // QTSHELLPRIV_H
