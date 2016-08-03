#pragma once
#include <QObject>

class QtShellTests : public QObject
{
    Q_OBJECT
public:
    explicit QtShellTests(QObject *parent = 0);

private slots:
    void test_basename();

    void test_dirname();

    void test_find();

    void test_rmdir();

    void test_touch();

    void test_rm();

    void test_mkdir();

    void test_cp();
};

