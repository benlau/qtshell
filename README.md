QtShell - Manipulate files by a shell command style API
===============================================

Installation
------------

For user who are already using qpm from qpm.io

1) Run qpm install

    qpm install com.github.benlau.qtshell

2) Include vendor/vendor.pri in your .pro file

You may skip this step if you are already using qpm.

    include(vendor/vendor.pri)

3) Include header

    #include <QtShell>

API
---

**QString QtShell::dirname(const QString& pathname)**

Return directory portion of pathname

    QString basename(const QString& path);

return filename portion of pathname

    QStringList find(const QString& path, const QStringList& nameFilters = QStringList());

walk a file hierarchy

    bool rmdir(const QString& path);

The rmdir utility removes the directory entry specified by each directory argument, provided it is empty.

    bool touch(const QString &path);

The touch utility sets the modification and access times of files.  If any file does not exist, it is created with default permissions.

    bool rm(const QString& file);

Remove directory entries. (Recursive removal is not supported yet)

Examples:

    rm("/tmp/tmp.txt");

    rm("/tmp/*.txt");

    rm("*.txt"); // Remove all txt files in current path

**bool QtShell::mkdir(const QString &path)**
