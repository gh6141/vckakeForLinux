#include "koza.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>


QList<koza> koza::selectAll(const QString &dbPath)
{
    QList<koza> list;

    QSqlDatabase db;
    if (QSqlDatabase::contains("koza_conn")) {  // 名前付き接続で確認
        db = QSqlDatabase::database("koza_conn");
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE", "koza_conn"); // 名前付き接続を作る
        db.setDatabaseName(dbPath);
        if (!db.open()) return list;
    }

    QSqlQuery query(db);
    if (query.exec("SELECT num, kozaName, tabColor FROM koza")) {
        while (query.next()) {
            koza k;
            k.num = query.value("num").toInt();
            k.kozaName = query.value("kozaName").toString();
            k.tabColor = query.value("tabColor").toString();
            list.append(k);
        }
    }

    // db.close() は呼ばない
    // removeDatabase() も呼ばない
    return list;
}

// koza.cpp に実装
int koza::numFromName(const QString &dbPath, const QString &name)
{
    int result = -1; // 見つからなかった場合は -1

    QSqlDatabase db;
    if (QSqlDatabase::contains("koza_conn")) {
        db = QSqlDatabase::database("koza_conn");
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE", "koza_conn");
        db.setDatabaseName(dbPath);
        if (!db.open()) return result;
    }

    QSqlQuery query(db);
    query.prepare("SELECT num FROM koza WHERE kozaName = :name LIMIT 1");
    query.bindValue(":name", name);
    if (query.exec() && query.next()) {
        result = query.value("num").toInt();
    }

    return result;
}

QString koza::kozaNameFromNum(const QString &dbPath, int num)
{
    QSqlDatabase db;
    if (QSqlDatabase::contains("koza_conn2")) {
        db = QSqlDatabase::database("koza_conn2");
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE", "koza_conn2");
        db.setDatabaseName(dbPath);
        if (!db.open()) return QString();
    }

    QSqlQuery query(db);
    query.prepare("SELECT kozaName FROM koza WHERE num = :num");
    query.bindValue(":num", num);

    if (!query.exec()) {
        qDebug() << "SQL error:" << query.lastError();
        return QString();
    }

    if (query.next()) {
        return query.value(0).toString();
    }

    return QString(); // 見つからない場合
}


//CsvMapping mapping {2,  // 日付 4,// 支払  5, // 預かり  8, // 残高  7,  // 取引区分  9   // 摘要};

CsvMapping koza::kozaImportMapFromNum(const QString &dbPath, int num)
{
    CsvMapping cmp;
    QSqlDatabase db;
    if (QSqlDatabase::contains("koza_conn3")) {
        db = QSqlDatabase::database("koza_conn3");
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE", "koza_conn3");
        db.setDatabaseName(dbPath);
        if (!db.open()) return cmp;
    }

    QSqlQuery query(db);
    query.prepare("SELECT tabColor FROM koza WHERE num = :num");
    query.bindValue(":num", num);

    if (!query.exec()) {
        qDebug() << "SQL error:" << query.lastError();
        return cmp;
    }

    if (query.next()) {
        QString s = query.value(0).toString();  // "2,4, , , , "
        QStringList list = s.split(',', Qt::KeepEmptyParts);

        if (list.size() < 6){
            qDebug() << "csvmapping error:koza db colortab check" ;
            return cmp;  // データ不正
        }

        CsvMapping m;
        m.date    = list[0].trimmed().toInt();
        m.payment = list[1].trimmed().toInt();
        m.deposit = list[2].trimmed().toInt();
        m.balance = list[3].trimmed().toInt();
        m.type    = list[4].trimmed().toInt();
        m.summary = list[5].trimmed().toInt();

        return m;
    }




    //return QString(); // 見つからない場合
    return cmp;
}

