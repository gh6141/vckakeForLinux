#pragma once
#include <QString>
#include <QDate>

struct KakeiboRowData {
    int id;
    QDate date;
    int kingaku=0;
    QString himoku;
    QString shiharaisaki;
    QString biko;
    int idosaki; //koza number
};
