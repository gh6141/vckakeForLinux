// koza.cpp
#include "koza.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>

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

