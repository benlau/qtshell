#pragma once
#include <QObject>

class QtShellTests : public QObject
{
    Q_OBJECT
public:
    explicit QtShellTests(QObject *parent = 0);

private slots:
    void basename();

    void find();

    void rmdir();

    void touch();

    void rm();

    void mkdir();

    void cp();
};

