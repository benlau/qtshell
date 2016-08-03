#include <QQmlApplicationEngine>
#include <QTest>
#include <Automator>
#include "qtshelltests.h"
#include "qtshell.h"

using namespace QtShell;

QtShellTests::QtShellTests(QObject *parent) : QObject(parent)
{

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

