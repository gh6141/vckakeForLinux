// koza.cpp
#include "koza.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>

QList<koza> koza::selectAll(const QString &dbPath)
{
    QList<koza> list;
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "koza_conn");
    db.setDatabaseName(dbPath);
    if (!db.open()) return list;

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
    db.close();
    QSqlDatabase::removeDatabase("koza_conn");
    return list;
}
