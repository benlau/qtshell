QtShell - Manipulate files by a shell command style API
===============================================

Installation
------------

    qpm install com.github.benlau.qtshell

API
---

    QString dirname(const QString& path);

return directory portion of pathname

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


