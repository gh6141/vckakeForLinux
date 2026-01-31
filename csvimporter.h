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
    static QStringList parseCsvLine(const QString& line);

private:
    CsvMapping map;

    int   parseMoney(QString& s) ;
    QDate parseJapaneseDate(QString& s) ;
    bool isSkippableLine(const QString& line);

    bool parseRecordLine(
        const QString& line,
        const CsvMapping& map,
        importRecord& out
        );


    int lastMonth;
    int yearCrossCount;
    QDate firstDate;
    QDate firstDate_before1yr;
};
