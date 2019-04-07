QtShell - Manipulate files by a shell command style API
===============================================

[![Build Status](https://www.travis-ci.org/benlau/qtshell.svg?branch=master)](https://www.travis-ci.org/benlau/qtshell)
[![Build status](https://ci.appveyor.com/api/projects/status/2qry21dualt0t9xe?svg=true)](https://ci.appveyor.com/project/benlau/qtshell)


QtShell is a Qt library that provides a set of file manipulation API in shell command style.

Example Usages
---------

1) Copy files from qrc to a directory recursively

```
QtShell::cp("-a", ":/resources", targetPath);
```

2) Create a lock file  

```
QtShell::touch(lockFile);
```

3) Read all from a file

```
QString content = QtShell::cat(input);
```

4) Join multiple string into an absolute path

```
    realpath_strip("/tmp", "subdir1", "/subdir2/"); // "/tmp/subdir1/subdir2"
    realpath_strip("tmp", "subdir1", "..//subdir2/"); // "$PWD/tmp/subdir2"
    realpath_strip("file:///tmp1"); // "/tmp1"
    realpath_strip("qrc:///tmp1"); // ":/tmp1"
    
    // Windows    
   realpath_strip("file://networkdrive/tmp1"); // "\\networkdrive\tmp1"

```


Installation
------------

For user who are already using qpm from qpm.io

1) Run qpm install

    qpm install com.github.benlau.qtshell

2) Include vendor/vendor.pri in your .pro file

You may skip this step if you are already using qpm.

    include(vendor/vendor.pri)

3) Include header

```
    #include <QtShell>
```

API
===

dirname
-------

    QString QtShell::dirname(const QString& pathname);

Return directory portion of pathname.

Remarks: It is a non-blocking function

basename
--------

    QString QtShell::basename(const QString& path);

Return filename portion of pathname

Remarks: It is a non-blocking function

find
----

    QStringList QtShell::find(const QString& path, const QStringList& nameFilters = QStringList());
    QStringList QtShell::find(const QString& path, const QString& filter);

Walk a file hierarchy

Examples

    find("/tmp");

    find("/tmp", QStringList() << "*.jpg" << "*.png");
    
    find("/tmp", "*.txt");


rmdir
-----

    bool QtShell::rmdir(const QString& path);

The rmdir utility removes the directory entry specified by each directory argument, provided it is empty.

touch
-----

    bool QtShell::touch(const QString &path);

The touch utility sets the modification and access times of a file to current time.  If a file does not exist, it is created with default permissions.

rm
--

    bool QtShell::rm(const QString& file);
    bool QtShell::rm(const QString& options, const QString& file);

Remove directory entries. Preserved paths (all paths defined in QStandPaths will not be removed)

Examples:

    rm("/tmp/tmp.txt");

    rm("/tmp/*.txt");

    rm("*.txt"); // Remove all txt files in current path

    rm("-rf", "/tmp/dir"); // Remove a directory

Options:

    -v          Be verbose when deleting files, showing them as they are removed.

    -r          Equivalent to -R.

    -R          Attempt to remove the file hierarchy rooted in each file argument include directory

    -f          Dummy option. No different will be made.

mkdir
-----

    bool QtShell::mkdir(const QString &path)
    bool QtShell::mkdir(const QString &options,const QString &path)

Creates the directory.

Example

    mkdir("tmp");
    mkdir("-p", "/tmp/myapp/cache");

Options

    -p  Create intermediate directories as required.  If this option is not specified, the
    full path prefix of each operand must already exist.  On the other hand, with this
    option specified, no error will be reported if a directory given as an operand
    already exists.

cp
--

    bool QtShell::cp(const QString& source , const QString &target);
    bool QtShell::cp(const QString& options, const QString& source , const QString &target);
    bool QtShell::cp(const QString& source , const QString &target, QList<QPair<QString,QString> > &log);
    bool QtShell::cp(const QString& options, const QString& source , const QString &target, QList<QPair<QString,QString> > &log);

Copy files

Examples

    cp("-a", ":/*", "/target"); // copy all files from qrc resource to target path recursively

    cp("tmp.txt", "/tmp");

    cp("*.txt", "/tmp");

    cp("/tmp/123.txt", "456.txt");

    cp("-va","src/*", "/tmp");

    cp("-va","src/*", "/tmp", log); // copy files and save the result of successfully copied file to log

Options

     -a    Same as -R options.

     -R    If source_file designates a directory, cp copies the directory
           and the entire subtree connected at that point.

     -v    Cause cp to be verbose, showing files as they are copied.



cat
---

    QString QtShell::cat(const QString& file);
    QString QtShell::cat(const QStringList& files);

Concatenate and print files

Examples

    QString content = QtShell::cat("input1.txt"); 
    
    QString content = QtShell::cat(QStringList() << "input1.txt" << "input2.txt);

mv
--

    bool QtShell::mv(const QString& source , const QString &target);

move files

Example

    mv("src/*.txt","target");

    mv("src/1.txt","target/2,txt");
    
realpath_strip
--------------

    QString realpath_strip(...);

Implementation of `realpath --strip` which prints the canonicalized absolute path (remove "." & "..") of input path without expanding the symbolic link.

`realpath_strip` supports variadic arguments. It joins all the input path by "/"  then produces the output.

It is not a blocking API.

Example

    realpath_strip("tmp/subdir"); // pwd() + "/tmp/subdir";
    realpath_strip("/tmp", "subdir1", "/subdir2/"); // "/tmp/subdir1/subdir2"
    realpath_strip("/tmp", "subdir1", "..//subdir2/"); // "/tmp/subdir2"
    realpath_strip("file:///tmp1"); // "/tmp1"
    realpath_strip("qrc:///tmp1"); // ":/tmp1"


pwd
---

```
    QString pwd();
```

Implementation of `pwd` which return the current working directory name

Example

    pwd();

which
-----

```
    QString which(const QString& program)
```

Locate a program file in the user's path listed in the PATH environment variable.

Example:

```
    // Linux / Mac
    which("sh"); // "/bin/sh"

    // Windows
    which("ping"); // "c:\\Windows\\System32\\PING.EXE"
```
