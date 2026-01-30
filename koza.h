#pragma once
#include <QString>
#include <QList>
#include "csvimporter.h"

class koza {
public:
    int num;
    QString kozaName;
    QString tabColor;

    static QList<koza> selectAll(const QString &dbPath);
    static int numFromName(const QString &dbPath, const QString &name);
    static QString kozaNameFromNum(
        const QString &dbPath,
        int num
        );
    static CsvMapping kozaImportMapFromNum(const QString &dbPath, int num);
};
