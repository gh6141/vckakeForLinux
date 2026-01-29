#include "csvimporter.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>

//引数 m を、メンバ変数 map にコピーして初期化する  銀行毎のデータ対応順序を数値の順序であらわしたもの=mapをつくる
CsvImporter::CsvImporter(const CsvMapping& m) : map(m) {}

QVector<importRecord> CsvImporter::importFile(const QString& fileName)
{
    QVector<importRecord> records;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return records;

    QTextStream in(&file);
    //in.setEncoding(QStringConverter::Utf8);
    //in.setEncoding(QStringConverter::Encoding::ShiftJIS);
    in.setEncoding(QStringConverter::System);

    bool first = true;
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (first) { first = false; continue; }

        QStringList c = line.split(QRegularExpression("\",\""));
        for (QString& s : c) s.remove('"');

        importRecord r;
        r.date    = parseJapaneseDate(c[map.date]);
        r.payment = parseMoney(c[map.payment]);
        r.deposit = parseMoney(c[map.deposit]);
        r.balance = parseMoney(c[map.balance]);
        r.type    = c[map.type];
        r.summary = c[map.summary];

        records.push_back(r);
    }
    return records;
}

int CsvImporter::parseMoney(const QString& s) const
{
    QString t = s.trimmed();
    if (t.isEmpty())
        return 0;

    // 円記号、カンマ、空白を除去
    t.remove(QChar('\\'));   // ¥ が \ で入っている場合
    t.remove(QChar(0x00A5)); // 本物の ¥
    t.remove(',');

    bool ok = false;
    int value = t.toInt(&ok);
    return ok ? value : 0;
}

QDate CsvImporter::parseJapaneseDate(const QString& s) const
{
    QString t = s.trimmed();
    if (t.isEmpty())
        return QDate(); // invalid date

    static QRegularExpression re(R"((\d+)月(\d+)日)");
    QRegularExpressionMatch m = re.match(t);
    if (!m.hasMatch())
        return QDate();

    int month = m.captured(1).toInt();
    int day   = m.captured(2).toInt();
    int year  = QDate::currentDate().year();

    return QDate(year, month, day);
}


