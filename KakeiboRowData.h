#pragma once
#include <QString>
#include <QDate>

struct KakeiboRowData {
    QDate date;
    int kingaku;
    QString himoku;
    QString shiharaisaki;
    QString biko;
};
