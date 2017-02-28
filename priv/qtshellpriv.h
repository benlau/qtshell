#ifndef QTSHELLPRIV_H
#define QTSHELLPRIV_H

#include <QString>

namespace QtShell {

    namespace Private {

        /// Remove tailing "/" from a path.
        QString normalize(QString path);

    }
}

#endif // QTSHELLPRIV_H
