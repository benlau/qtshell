#include <QQmlApplicationEngine>
#include <QTest>
#include <Automator>
#include <QDir>
#include "qtshelltests.h"
#include "qtshell.h"
#include "priv/qtshellpriv.h"

using namespace QtShell;
using namespace QtShell::Private;

QtShellTests::QtShellTests(QObject *parent) : QObject(parent)
{
    auto ref = [=]() {
        QTest::qExec(this, 0, 0); // Autotest detect available test cases of a QObject by looking for "QTest::qExec" in source code
    };
    Q_UNUSED(ref);
}

#define PRINT(x) qDebug() << #x << (x);

void QtShellTests::system_info()
{
    PRINT(QFileInfo("C:/temp").isAbsolute());
    PRINT(QFileInfo(":/temp").isAbsolute());

    PRINT(QUrl("file:///C:/temp").path());
}

void QtShellTests::test_normalize()
{
    QVERIFY(normalize("/tmp") == "/tmp");
    QVERIFY(normalize("/tmp/") == "/tmp");

}

void QtShellTests::test_canonicalPath()
{
    QVERIFY(canonicalPath("/tmp", false) == "/tmp");
    QVERIFY(canonicalPath("/tmp/", false) == "/tmp");
    QVERIFY(canonicalPath("/tmp//subdir", false) == "/tmp/subdir");
    QVERIFY(canonicalPath("//tmp///subdir/", false) == "/tmp/subdir");

    QVERIFY(canonicalPath("//tmp/../subdir/", false) == "/subdir");

    QVERIFY(canonicalPath("//tmp/../../subdir/", false) == "/subdir");
    QVERIFY(canonicalPath("//tmp/./subdir/", false) == "/tmp/subdir");

    QCOMPARE(canonicalPath("C:/temp", true),  QString("C:/temp"));
    QCOMPARE(canonicalPath("/C:/temp", true),  QString("C:/temp"));

}

void QtShellTests::test_bulk()
{
    QList<QPair<QString,QString> > log;

    auto predicate = [&](const QString& from , const QString& to, const QFileInfo& fromInfo) {
        Q_UNUSED(fromInfo);
        log << QPair<QString,QString>(from, to);
        return true;
    };

    QtShell::rm("-rf", "src");
    mkdir("-p","src/1");
    mkdir("-p","src/2");
    mkdir("-p","target");
    touch("src/1/1.txt");
    touch("src/2/1.txt");
    touch("src/2/2.txt");

    QVERIFY(bulk("src/*", "target", predicate) == NO_ERROR);
    QCOMPARE(log.size(), 2);
    QVERIFY(log[0].first == "src/1");
    QVERIFY(log[0].second == "target/1");

    log.clear();
    QVERIFY(bulk("src/3", "target", predicate) == NO_SUCH_FILE_OR_DIR);

    QCOMPARE((int) bulk("src/*", "target/1.txt", predicate),(int) INVALID_TARGET);

    QCOMPARE((int) bulk("src/1/1.txt", "target/1.txt", predicate),(int) NO_ERROR);
}

void QtShellTests::test_basename()
{
    // Use QtShell namespace to avoid mix up with the basename in libgen.h
    QVERIFY(QtShell::basename("/tmp.txt") == "tmp.txt");
    QVERIFY(QtShell::basename("/tmp") == "tmp");
    QVERIFY(QtShell::basename("/tmp/") == "tmp");

    QVERIFY(QtShell::basename("/") == "/");
    QVERIFY(QtShell::basename("//") == "/");
    QVERIFY(QtShell::basename("tmp.txt") == "tmp.txt");
    QVERIFY(QtShell::basename("A file not existed") == "A file not existed");
    QVERIFY(QtShell::basename("//tmp/tmp.txt") == "tmp.txt");
}

void QtShellTests::test_dirname()
{
    QVERIFY(dirname("/tmp.txt") == "/");
    QVERIFY(dirname("/tmp") == "/");

    QVERIFY(dirname("/") == "/");
    QVERIFY(dirname("//") == "/");

    QVERIFY(dirname("tmp.txt") == ".");
    QVERIFY(dirname("A file not existed") == ".");
    QVERIFY(dirname("//tmp/tmp.txt") == "//tmp");
}

void QtShellTests::test_basenameAndDirname()
{
    QFETCH(QString, path);
    QFETCH(QString, d);
    QFETCH(QString, b);

    QVERIFY(QtShell::dirname(path) == d);
    QVERIFY(QtShell::basename(path) == b);
}

void QtShellTests::test_basenameAndDirname_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("d");
    QTest::addColumn<QString>("b");

    QTest::newRow("/tmp//") << "/tmp//" << "/" << "tmp";
    QTest::newRow("///") << "///" << "/" << "/";
    QTest::newRow(" ") << " " << "." << " ";
}

void QtShellTests::test_find()
{
    rm("-rf","tmp");
    mkdir("tmp");
    touch("tmp/1.txt");

    QStringList files = find("tmp");
    QCOMPARE(files.size() , 2);
    QVERIFY(files[0] == "tmp");

    files = find("tmp/");

    QCOMPARE(files.size() , 2);
    QVERIFY(files[0] == "tmp/");

    files = find("tmp",QStringList() << "*.txt");
    QCOMPARE(files.size() , 1);
    QVERIFY(files[0] == "tmp/1.txt");


    QVERIFY(find(".").size() > 0);
    QVERIFY(find(QDir::current().path()).size() > 0);

    QVERIFY(find("..").size() > 0);

    files = find("..", QStringList() << "*.h");

    QVERIFY(files.filter(QRegExp("*.h",Qt::CaseInsensitive,QRegExp::Wildcard)).size() == files.size());

    // find by file Url
    files = find(QUrl::fromLocalFile(QtShell::pwd() + "/tmp").toString());
    qDebug() << files;
    QCOMPARE(files.size() , 2);
    QVERIFY(files[0].indexOf("file") == 0);

}

void QtShellTests::test_find_verify_filters()
{
    mkdir("-p", "findTestCases");
    touch("findTestCases/a1.txt");
    touch("findTestCases/b2.txt");
    touch("findTestCases/c3.md");

    QCOMPARE(find("findTestCases", "*.txt").size(), 2);
    QCOMPARE(find("findTestCases", "a*.txt").size(), 1);
}

void QtShellTests::test_find_options()
{
    QString folder = realpath_strip(pwd(), QTest::currentTestFunction());
    mkdir("-p", folder +"/A");
    mkdir("-p", folder +"/A/A1");
    touch(folder + "/file1.txt");
    touch(folder + "/A/file2.txt");
    touch(folder + "/A/A1/file3.txt");

    {
        // Depth
        FindOptions options;

        QCOMPARE(find(folder).size(), 6);

        options.maxdepth = 0;
        QCOMPARE(find(options,folder).size(), 1);

        options.maxdepth = 1;
        QCOMPARE(find(options,folder).size(), 3);

        options.maxdepth = 2;
        QCOMPARE(find(options,folder).size(), 5);

        options.maxdepth = 3;
        QCOMPARE(find(options,folder).size(), 6);

    }

}

void QtShellTests::test_rmdir()
{
    QDir dir("tmp");
    QString tmpPath = dir.absolutePath();;

    if (dir.exists()) {
        dir.removeRecursively();
    }

    dir.mkpath(tmpPath);

    QVERIFY(dir.exists());
    QVERIFY(rmdir(tmpPath));
    QVERIFY(!dir.exists());

    dir.mkdir(tmpPath);
    touch(tmpPath + "/files.txt");

    QVERIFY(!rmdir(tmpPath));
    QVERIFY(dir.exists());
}

void QtShellTests::test_touch()
{
    QFileInfo info("tmp.txt");
    if (info.exists()) {
        QFile::remove("tmp.txt");
    }

    info = QFileInfo("tmp.txt");
    QVERIFY(!info.exists());

    touch("tmp.txt");

    info = QFileInfo("tmp.txt");
    QVERIFY(info.exists());

    QVERIFY(touch("tmp.txt"));

    /* Test UTF-8 file */
    QString fileName = "UTF8文字チャー.data";
    if (QFile::exists(fileName)) {
        rm("-f",fileName);
    }
    touch(fileName);
    QVERIFY(QFile::exists(fileName));
}

void QtShellTests::test_rm()
{
    touch("tmp.txt");

    QVERIFY(rm("-v","tmp.txt"));

    QFileInfo info("tmp.txt");
    QVERIFY(!info.exists());

    touch("tmp.txt");
    QVERIFY(rm("*.txt"));

    info = QFileInfo("tmp.txt");
    QVERIFY(!info.exists());

    mkdir("tmp");
    QVERIFY(!rm("tmp")); // rm: tmp: is a directory"
    QDir dir("tmp");
    QVERIFY(dir.exists());
    QVERIFY(rm("-vr", "tmp"));
    QVERIFY(!dir.exists());

    // Test trailing "/"
    mkdir("tmp");
    QVERIFY(rm("-vr","tmp/"));
    QVERIFY(!dir.exists());

    QVERIFY(rm("-f", "a-file-not-existed"));

    {
        // Remove directory with absolute path
        QString path = QtShell::pwd() + "/tmp";
        QDir dir(path);

        qDebug() << path;
        mkdir("-p", path);
        QVERIFY(dir.exists());
        QVERIFY(rm("-vr",path));
        QVERIFY(!dir.exists());
    }

}

void QtShellTests::test_mkdir()
{
    QDir dir("tmp");
    rm("-rf", "tmp");

    QVERIFY(!dir.exists());
    QVERIFY(mkdir("tmp"));
    QVERIFY(dir.exists());
    QVERIFY(!mkdir("tmp"));

    QVERIFY(find("tmp").size() == 1);

    QVERIFY(!mkdir("tmp/a/b"));
    QVERIFY(mkdir("-p", "tmp/a/b"));
    QVERIFY(find("tmp").size() == 3);
    QVERIFY(mkdir("-p", "tmp/a/b"));
    QVERIFY(find("tmp").size() == 3);

}

void QtShellTests::test_cp()
{
    rm("-rf", "src");
    mkdir("src");
    touch("src/1.txt");
    touch("src/2.text");
    touch("src/3.txt");
    rm("-rf", "target");
    mkdir("target");

    QCOMPARE(find("src").size(), 4);
    QCOMPARE(find("target").size(), 1);

    QVERIFY(cp("src/*.txt","target"));
    QCOMPARE(find("target").size(), 3);

    rm("-rf", "target");
    mkdir("target");

    QVERIFY(cp("-v", "src/*","target"));
    QCOMPARE(find("target").size(), 4);

    QVERIFY(cp(":/*.cpp", "target"));
    QCOMPARE(find("target",QStringList() << "*.cpp").size(), 1);

    /* Test -r / -a */

    mkdir("src/a");
    mkdir("src/b");
    touch("src/b/b1.txt");
    touch("src/b/b2.txt");
    rm("-rf", "target");
    mkdir("target");

    QVERIFY(cp("-av","src/*","target"));

    QStringList files = find("target");
    QCOMPARE(files.size(), 8);
}

void QtShellTests::test_cp_overwrite()
{
    rm("-rf", "src");
    mkdir("src");
    touch("src/1.txt");
    touch("src/2.text");
    touch("src/3.txt");
    rm("-rf", "target");
    mkdir("target");

    QCOMPARE(find("src").size(), 4);
    QCOMPARE(find("target").size(), 1);

    QVERIFY(cp("src/*.txt","target"));
    QCOMPARE(find("target").size(), 3);

    QVERIFY(cp("src/*.txt","target"));


}

void QtShellTests::test_cp_recursive()
{
    rm("-rf", "src");
    mkdir("-p","src/1");
    mkdir("-p","src/2");
    touch("src/1/1.txt");
    touch("src/2/1.txt");
    touch("src/2/2.txt");

    rm("-rf", "target");
    mkdir("target");

    QVERIFY(cp("-a", "src/*","target"));
    QCOMPARE(find("target","*.txt").size(), 3);

}

void QtShellTests::test_cp_log()
{
    rm("-rf", "src");
    mkdir("-p","src/1");
    mkdir("-p","src/2");
    touch("src/1/1.txt");
    touch("src/2/1.txt");
    touch("src/2/2.txt");

    rm("-rf", "target");
    mkdir("target");

    QList<QPair<QString,QString> > log;
    QVERIFY(cp("-a", "src/*","target", log));
    QCOMPARE(log.size(), 3);

    QPair<QString,QString> record = log.first();
    QVERIFY(record.first == "src/1/1.txt");
    QVERIFY(record.second == "target/1/1.txt");
}

void QtShellTests::test_pwd()
{

    QDir dir;
    QCOMPARE(QtShell::pwd(), dir.absolutePath());

}

void QtShellTests::test_cat()
{
    QFile file("cat.txt");
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("0123456789");
    file.close();

    QString content = cat("cat.txt");
    QVERIFY(content == "0123456789");

    content = cat(QStringList() << "cat.txt" << "cat.txt");
    QVERIFY(content == "0123456789\n0123456789");

    QUrl url = QUrl::fromLocalFile(QtShell::pwd() + "/cat.txt");
    qDebug() << "cat" << url.toString();
    content = cat(url.toString());
    QVERIFY(content == "0123456789");

    content = cat(QUrl::fromLocalFile(QtShell::pwd() + "/cat.txt").toString());
    QVERIFY(content == "0123456789");


}

void QtShellTests::test_mv()
{
    QList<QPair<QString,QString> > log;

    QtShell::rm("-rf", "src");
    QtShell::rm("-rf", "target");
    mkdir("-p","src/1");
    mkdir("-p","src/2");
    mkdir("-p","src/3");
    mkdir("-p","target");
    touch("src/1/1.txt");
    touch("src/1/2.txt");
    touch("src/2/1.txt");
    touch("src/2/2.txt");
    touch("src/3/1.txt");
    touch("src/3/2.txt");
    touch("src/3/3.txt");

    QVERIFY(QtShell::mv("src/1/1.txt","target/"));
    QVERIFY(QFile::exists("target/1.txt"));
    QVERIFY(QtShell::mv("src/1/2.txt","target/2.txt"));
    QVERIFY(QFile::exists("target/2.txt"));
    QVERIFY(QtShell::mv("src/2","target", log));

    QVERIFY(QFile::exists("target/2/1.txt"));
    QVERIFY(QFile::exists("target/2/2.txt"));
    QCOMPARE(log.size(), 1);
    mkdir("-p", "target/3");
    QVERIFY(QtShell::mv("src/3/*","target/3", log));
    QCOMPARE(log.size(), 4);


}

void QtShellTests::test_realpath_strip()
{
    QCOMPARE(QtShell::realpath_strip(QtShell::pwd()),  QtShell::pwd());

    QCOMPARE(QtShell::realpath_strip(":tmp"),  QString(":tmp"));
    QCOMPARE(QtShell::realpath_strip(":/tmp"), QString(":/tmp"));

    QCOMPARE(QtShell::realpath_strip("tmp"),  (QtShell::pwd() + "/tmp"));

    QCOMPARE(QtShell::realpath_strip("tmp/"),  (QtShell::pwd() + "/tmp"));

    QCOMPARE(QtShell::realpath_strip("tmp/../"),  (QtShell::pwd()));

    QCOMPARE(QtShell::realpath_strip("tmp","subdir1"),  (QtShell::pwd() + "/tmp/subdir1"));

    QCOMPARE(QtShell::realpath_strip("tmp","subdir1","subdir2"),  (QtShell::pwd() + "/tmp/subdir1/subdir2"));

    QCOMPARE(QtShell::realpath_strip("tmp","/subdir1","subdir2"),  (QtShell::pwd() + "/tmp/subdir1/subdir2"));

    QCOMPARE(QtShell::realpath_strip("tmp","/subdir1/","subdir2"),  (QtShell::pwd() + "/tmp/subdir1/subdir2"));

    QCOMPARE(QtShell::realpath_strip("tmp","/subdir1/","../subdir2"),  (QtShell::pwd() + "/tmp/subdir2"));

    QCOMPARE(QtShell::realpath_strip(QtShell::pwd()),  (QtShell::pwd()));

    /* Test URL */

    QUrl url = QUrl::fromLocalFile(QtShell::pwd());

    QCOMPARE(QtShell::realpath_strip(url.toString()), QtShell::pwd());

    QCOMPARE(QtShell::realpath_strip("qrc:/tmp1.txt"), QString(":/tmp1.txt"));


#ifdef Q_OS_WIN32
    qDebug().noquote() << QtShell::realpath_strip("file://networkdrive/tmp1.txt");
    QCOMPARE(QtShell::realpath_strip("file:///E:/dir/tmp.txt"), QString("E:/dir/tmp.txt"));
    QCOMPARE(QtShell::realpath_strip("file://networkdrive/tmp1.txt"), QString("//networkdrive/tmp1.txt"));
    QCOMPARE(QtShell::realpath_strip("\\\\networkdrive\\tmp1.txt"), QString("//networkdrive/tmp1.txt"));
    QCOMPARE(QtShell::realpath_strip("//networkdrive/tmp1.txt"), QString("//networkdrive/tmp1.txt"));

    // QFileDialog::getOpenFileName returns folder in a way like //networkdrive/tmp1.txt
#else
    /// The no. of "/" is critical for windows system. It help to determine is it a network drive
    QCOMPARE(QtShell::realpath_strip("file:///tmp1.txt"), QString("/tmp1.txt"));
    QCOMPARE(QtShell::realpath_strip("file://networkdrive/tmp1.txt"), QString("/tmp1.txt"));
#endif

}

void QtShellTests::test_which()
{
#ifdef Q_OS_UNIX
    QCOMPARE(QtShell::which("sh"), QString("/bin/sh"));
#endif

#ifdef Q_OS_WIN32
    QCOMPARE(QtShell::which("ping").toLower(), QString("c:\\Windows\\System32\\PING.EXE").toLower());
#endif
}

