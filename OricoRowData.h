#pragma once
#include <QString>
#include <QDate>
#include <QVector>
#include <QFile>
#include <QTextStream>
//#include <QTextCodec>
struct OricoRowData {
    QString date;         // 利用日
    int kingaku;        // ご利用金額
    QString usePlace;   // ご利用先
    QString payType;    // 支払区分（任意）
    int rowId;          // CSV行番号など
};

inline QString extractKingaku(const QString &line)
{
    QString result;
    bool accumulate = false;

    for (int i = 0; i < line.length(); ++i) {
        if (!accumulate) {
            // 累積モードに入る判定
            if (i + 1 < line.length() && line[i] == '"' && line[i+1] == '\\') {
                accumulate = true;
                ++i; // '\' はスキップ
            }
        } else {
            // 終了判定
            if (i + 1 < line.length() && line[i] == '"' && line[i+1] == ',') {
                break; // 累積終了
            }
            result += line[i];
        }
    }

    // カンマを除去して返す
    result.remove(',');
    return result;
}


// CSVを読み込んで QVector<OricoRowData> に変換
inline QVector<OricoRowData> loadOricoCSV(const QString& filePath, int& total)
{
    QVector<OricoRowData> list;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return list;

    QByteArray data = file.readAll();  // バイナリとして読み込む

    // Shift-JIS から QString に変換
    QString content = QString::fromLocal8Bit(data);

    QTextStream in(&content, QIODevice::ReadOnly);  // QTextStream の代わりに文字列を読む
    int rowCounter = 0;
    bool inDetail = false;

    while (!in.atEnd()) {
        QString line = in.readLine();

        if (line.startsWith("ご請求総額")) {
            QRegularExpression re("(\\d[\\d,]*)");
            QRegularExpressionMatch m = re.match(line);
            if (m.hasMatch()) {
                QString num = m.captured(1);
                num.remove(",");
                total = num.toInt();
                qDebug() << total;
            }
            continue;
        }

        if (line.startsWith("ご利用日")) {
            inDetail = true;
            continue;
        }
        if (!inDetail) continue;

        auto fields = line.split(',');
        if (fields.size() < 10) continue;

        OricoRowData item;
        item.date = fields[0];
        item.usePlace = fields[1];
        item.kingaku = extractKingaku(line).toInt();
        item.payType = fields[6];
        item.rowId = rowCounter++;
        list.append(item);
    }

    return list;
}

