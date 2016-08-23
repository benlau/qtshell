#include <QQmlApplicationEngine>
#include <QTest>
#include <Automator>
#include <QDir>
#include "qtshelltests.h"
#include "qtshell.h"

using namespace QtShell;

QtShellTests::QtShellTests(QObject *parent) : QObject(parent)
{

}

void QtShellTests::test_basename()
{
    QVERIFY(QtShell::basename("/tmp.txt") == "tmp.txt");
    QVERIFY(QtShell::basename("/tmp") == "tmp");
    QVERIFY(QtShell::basename("/tmp/") == "");
}

void QtShellTests::test_dirname()
{
    QVERIFY(dirname("/tmp.txt") == "/");
    QVERIFY(dirname("/") == "/");
    QVERIFY(dirname("tmp.txt") == QDir::currentPath());
    QVERIFY(dirname("A file not existed") == QDir::currentPath());
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

void QtShellTests::test_pwd()
{

    QDir dir;
    QCOMPARE(QtShell::pwd(), dir.absolutePath());

}

