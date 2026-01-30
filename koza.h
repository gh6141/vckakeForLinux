#pragma once
#include <QString>
#include <QList>

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
};
