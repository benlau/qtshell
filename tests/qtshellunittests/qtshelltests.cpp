#include <QQmlApplicationEngine>
#include <QTest>
#include <Automator>
#include "qtshelltests.h"
#include "qtshell.h"

using namespace QtShell;

QtShellTests::QtShellTests(QObject *parent) : QObject(parent)
{

}

void QtShellTests::basename()
{
    QVERIFY(QtShell::basename("/tmp.txt") == "tmp.txt");
    QVERIFY(QtShell::basename("/tmp") == "tmp");
    QVERIFY(QtShell::basename("/tmp/") == "tmp");

}

void QtShellTests::find()
{
    QVERIFY(QtShell::find(".").size() > 0);
    QVERIFY(QtShell::find(QDir::current().path()).size() > 0);

    QVERIFY(QtShell::find("..").size() > 0);

    QStringList files = QtShell::find("..", QStringList() << "*.h");

    QVERIFY(files.filter(QRegExp("*.h",Qt::CaseInsensitive,QRegExp::Wildcard)).size() == files.size());

}

void QtShellTests::rmdir()
{
    QDir dir("tmp");
    QString tmpPath = dir.absolutePath();;

    if (dir.exists()) {
        dir.removeRecursively();
    }

    dir.mkpath(tmpPath);

    QVERIFY(dir.exists());
    QVERIFY(QtShell::rmdir(tmpPath));
    QVERIFY(!dir.exists());

    dir.mkdir(tmpPath);
    QtShell::touch(tmpPath + "/files.txt");

    QVERIFY(!QtShell::rmdir(tmpPath));
    QVERIFY(dir.exists());
}

void QtShellTests::touch()
{
    QFileInfo info("tmp.txt");
    if (info.exists()) {
        QFile::remove("tmp.txt");
    }

    info = QFileInfo("tmp.txt");
    QVERIFY(!info.exists());

    QtShell::touch("tmp.txt");

    info = QFileInfo("tmp.txt");
    QVERIFY(info.exists());

    QVERIFY(QtShell::touch("tmp.txt"));
}

void QtShellTests::rm()
{
    QtShell::touch("tmp.txt");

    QVERIFY(QtShell::rm("tmp.txt"));

    QFileInfo info("tmp.txt");
    QVERIFY(!info.exists());

    QtShell::touch("tmp.txt");
    QVERIFY(QtShell::rm("*.txt"));

    info = QFileInfo("tmp.txt");
    QVERIFY(!info.exists());

    QtShell::mkdir("tmp");
    QVERIFY(!QtShell::rm("tmp")); // rm: tmp: is a directory"
    QDir dir("tmp");
    QVERIFY(dir.exists());
    QVERIFY(QtShell::rm("tmp", true));
    QVERIFY(!dir.exists());
}

void QtShellTests::mkdir()
{
    QDir dir("tmp");
    if (dir.exists()) {
        dir.removeRecursively();
    }

    QVERIFY(!dir.exists());
    QVERIFY(QtShell::mkdir("tmp"));
    QVERIFY(dir.exists());
    QVERIFY(!QtShell::mkdir("tmp"));
}

void QtShellTests::cp()
{
    QtShell::rm("src", true);
    QtShell::mkdir("src");
    QtShell::touch("src/1.txt");
    QtShell::touch("src/2.text");
    QtShell::touch("src/3.txt");
    QtShell::rm("target", true);
    QtShell::mkdir("target");

    QCOMPARE(QtShell::find("src").size(), 3);
    QCOMPARE(QtShell::find("target").size(), 0);

    QVERIFY(QtShell::cp("src/*.txt","target"));
    QCOMPARE(QtShell::find("target").size(), 2);

    QtShell::rm("target", true);
    QtShell::mkdir("target");

    QVERIFY(QtShell::cp("src/*","target"));
    QCOMPARE(QtShell::find("target").size(), 3);

    QVERIFY(QtShell::cp(":/*.cpp", "target"));
    QCOMPARE(QtShell::find("target",QStringList() << "*.cpp").size(), 1);

}

