#include "kakeibotable.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

KakeiboTable::KakeiboTable(QWidget *parent)
    : QWidget(parent),
    model(nullptr),
    view(new QTableView(this)),
    currentAccountNum(1)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(view);

    if (initDB()) {
        loadTable(currentAccountNum);
    }
}

bool KakeiboTable::initDB()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("vckake.db");

    if (!db.open()) {
        qDebug() << "DB open error:" << db.lastError().text();
        return false;
    }

    return true;
}

bool KakeiboTable::ensureTableExists(const QString &tableName)
{
    QSqlQuery query(db);
    QString createSQL = QString(
                            "CREATE TABLE IF NOT EXISTS %1 ("
                            "num INTEGER PRIMARY KEY AUTOINCREMENT,"
                            "date TEXT,"
                            "sishutu INTEGER,"
                            "shunyu INTEGER,"
                            "zandaka INTEGER,"
                            "himoku TEXT,"
                            "biko TEXT)"
                            ).arg(tableName);

    if (!query.exec(createSQL)) {
        qDebug() << "Create table error:" << query.lastError().text();
        return false;
    }

    return true;
}

void KakeiboTable::loadTable(int accountNum)
{
    currentAccountNum = accountNum;
    QString tableName = QString("shishutunyu%1").arg(accountNum);

    ensureTableExists(tableName);

    if (model) delete model;

    model = new QSqlTableModel(this, db);
    model->setTable(tableName);
    model->setEditStrategy(QSqlTableModel::OnFieldChange);
    model->select();

    view->setModel(model);
    view->resizeColumnsToContents();
}
