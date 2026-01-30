#include "csvimporter.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QMessageBox>

//引数 m を、メンバ変数 map にコピーして初期化する  銀行毎のデータ対応順序を数値の順序であらわしたもの=mapをつくる
CsvImporter::CsvImporter(const CsvMapping& m) : map(m) {}

QVector<importRecord> CsvImporter::importFile(const QString& fileName)
{
    yearCrossCount=0;
    lastMonth=0;
    firstDate = QDate();   // ← import 開始時に必ずリセット
    firstDate_before1yr = QDate();   // ← import 開始時に必ずリセット
    QVector<importRecord> records;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return records;

    QTextStream in(&file);

    in.setEncoding(QStringConverter::System);

    bool first = true;
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (first) { first = false; continue; }

        QStringList c = line.split(QRegularExpression("\",\""));
        for (QString& s : c) s.remove('"');

        importRecord r;

        r.date    = parseJapaneseDate(c[map.date]);
        if(r.date==QDate()){
            QMessageBox::warning(
                nullptr,
                "警告",
                "データは１年以内にしてください"
                );
            break;
        }
        r.payment = parseMoney(c[map.payment]);
        r.deposit = parseMoney(c[map.deposit]);
        r.balance = parseMoney(c[map.balance]);
        r.type    = c[map.type];
        r.summary = c[map.summary];

        records.push_back(r);
    }
    return records;
}

int CsvImporter::parseMoney( QString& s)
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

QDate CsvImporter::parseJapaneseDate(QString& s)
{
    QString t = s.trimmed();
    if (t.isEmpty())
        return QDate(); // 無効日

    QDate today = QDate::currentDate();
    int currentYear = today.year();
    int currentMonth=today.month();

    // 年月日形式: 例 2026年1月29日
    static QRegularExpression reYMD(R"((\d{4})年(\d+)月(\d+)日)");
    QRegularExpressionMatch m = reYMD.match(t);
    if (m.hasMatch()) {
        int year  = m.captured(1).toInt();
        int month = m.captured(2).toInt();
        int day   = m.captured(3).toInt();  
        QDate d(year, month, day);
        return d;
    }

    // スラッシュ形式: 例 2026/01/29
    static QRegularExpression reSlash(R"((\d{4})/(\d{1,2})/(\d{1,2}))");
    m = reSlash.match(t);
    if (m.hasMatch()) {
        int year  = m.captured(1).toInt();
        int month = m.captured(2).toInt();
        int day   = m.captured(3).toInt();
        QDate d(year, month, day);
        return d;
    }

    // 月日形式: 例 1月29日
    static QRegularExpression reMD(R"((\d+)月(\d+)日)");
    m = reMD.match(t);
    if (m.hasMatch()) {
        int month = m.captured(1).toInt();
        int day   = m.captured(2).toInt();
        int year = currentYear;

        if(month>currentMonth){
            year -=1;
            if(!firstDate_before1yr.isValid()){
                QDate d(year, month, day);
                firstDate_before1yr=d;
            }
         }

        // 前回月との比較で年跨ぎカウント
        if (lastMonth > 0 && month < lastMonth) {
            yearCrossCount += 1;

            if (yearCrossCount >= 2) {
                qWarning() << "2年以上前の月日形式データがあります:t="<<t;
                return QDate(); // エラー扱い
            }
        }

        QDate d(year, month, day);
        if(!firstDate.isValid()){
            firstDate=d;
        }


        if(firstDate!=firstDate_before1yr&&firstDate_before1yr.isValid()){
           //qDebug()<<firstDate<<firstDate_before1yr<<"=fst  fst_bfr";
             qWarning() << "１年以上前のデータがあります:t="<<t;
            return QDate();
        }


        lastMonth = month;

        return d;
    }

    // どれにもマッチしない場合
    return QDate();
}


