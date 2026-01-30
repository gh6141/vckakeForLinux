#pragma once
#include <QVector>
#include <QString>
#include "importRecord.h"

struct CsvMapping {
    int date;
    int payment;
    int deposit;
    int balance;
    int type;
    int summary;
};

class CsvImporter {
public:
    explicit CsvImporter(const CsvMapping& m);

    QVector<importRecord> importFile(const QString& fileName);

private:
    CsvMapping map;

    int   parseMoney(QString& s) ;
    QDate parseJapaneseDate(QString& s) ;

    int lastMonth;
    int yearCrossCount;
    QDate firstDate;
    QDate firstDate_before1yr;
};
