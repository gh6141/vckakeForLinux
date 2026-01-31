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
        QString line = in.readLine().trimmed();
        if (isSkippableLine(line))
            continue;
        if (first) {
            first = false;
            continue;
        }
        importRecord r;
        if (!parseRecordLine(line, map, r)) {
            QMessageBox::warning(nullptr, "警告", "データ読み取りエラー");
            break;
        }
        records.push_back(r);
    }

    // 2行だけ見て判定
    if (records.size() >= 2) {
        const auto& a = records[0];
        const auto& b = records[1];

        bool normal =
            qAbs(a.balance + b.deposit - b.payment - b.balance) < 0.01;

        bool reverse =
            qAbs(b.balance + a.deposit - a.payment - a.balance) < 0.01;

        if (normal == reverse) {
            QMessageBox::warning(nullptr,"警告","日付順を判定できません");
            return records;
        }

        if (reverse)
            std::reverse(records.begin(), records.end());
    }

    return records;
}

bool CsvImporter::isSkippableLine(const QString& line)
{
    if (line.isEmpty())
        return true;

    if (line.count(',') <= 2)
        return true;

    return false;
}


bool CsvImporter::parseRecordLine(
    const QString& line,
    const CsvMapping& map,
    importRecord& out
    ) {
    QStringList c = parseCsvLine(line);

    out.date = parseJapaneseDate(c[map.date]);
    if (!out.date.isValid())
        return false;

    out.payment = parseMoney(c[map.payment]);
    out.deposit = parseMoney(c[map.deposit]);
    out.balance = parseMoney(c[map.balance]);
    out.type    = c[map.type];
    out.summary = c[map.summary];

    return true;
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
    t.remove(' ');

    bool ok = false;
    int value = t.toInt(&ok);

   // if (!ok) {
   //     qDebug() << "金額変換失敗:" << s;
   //     return 0;
   // }

    return value;
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
   // static QRegularExpression reSlash(R"((\d{4})/(\d{1,2})/(\d{1,2}))");
    static QRegularExpression reSlash(
        R"((\d{4})[-\/\.](\d{1,2})[-\/\.](\d{1,2}))"
        );
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

QStringList CsvImporter::parseCsvLine(const QString& line)
{
    QStringList result;
    QString field;
    bool inQuotes = false;

    for (int i = 0; i < line.size(); ++i) {
        QChar ch = line[i];

        if (ch == '"') {
            inQuotes = !inQuotes;
        } else if (ch == ',' && !inQuotes) {
            result << field;
            field.clear();
        } else {
            field += ch;
        }
    }

    result << field; // 最後のフィールド

    // 前後の引用符を除去
    for (QString& s : result) {
        if (s.startsWith('"') && s.endsWith('"')) {
            s = s.mid(1, s.length() - 2);
        }
    }

    return result;
}

