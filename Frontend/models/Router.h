#pragma once

#include <QString>
#include <QList>

struct Router {
    int id = 0;
    QString name;
    QString ip;
    bool active = true;
    int degree = 0;
    QList<int> neighbors;
};
