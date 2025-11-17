#pragma once
#include <QString>
#include <QDate>
#include <QVector>
#include <QFile>
#include <QTextStream>

struct OricoRowData {
    QDate date;         // 利用日
    int kingaku;        // ご利用金額
    QString usePlace;   // ご利用先
    QString payType;    // 支払区分（任意）
    int rowId;          // CSV行番号など
};

// CSVを読み込んで QVector<OricoRowData> に変換
inline QVector<OricoRowData> loadOricoCSV(const QString& filePath)
{
    QVector<OricoRowData> list;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return list;

    QTextStream in(&file);
    bool inDetail = false;
    int rowCounter = 0;

    while (!in.atEnd()) {
        QString line = in.readLine();

        if (line.startsWith("<利用明細>")) {
            inDetail = true;
            continue;
        }
        if (!inDetail) continue;

        auto fields = line.split(',');
        if (fields.size() < 10) continue;

        OricoRowData item;
        item.date = QDate::fromString(fields[0], "yyyy/MM/dd");
        item.usePlace = fields[1];
        item.kingaku = fields[8].remove("\\\",").toInt(); // 金額を int に変換
        item.payType = fields[6];                          // 支払区分など
        item.rowId = rowCounter++;
        list.append(item);
    }

    return list;
}
