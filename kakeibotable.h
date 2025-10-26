#ifndef KAKEIBOTABLE_H
#define KAKEIBOTABLE_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QTableView>
#include <QVBoxLayout>
#include <QFile>

class KakeiboTable : public QWidget
{
    Q_OBJECT

public:
    explicit KakeiboTable(QWidget *parent = nullptr);
    bool initDB();
    void loadTable(int accountNum);  // 口座番号でテーブル切替

private:
    QSqlDatabase db;
    QSqlTableModel *model;
    QTableView *view;
    int currentAccountNum;

    bool ensureTableExists(const QString &tableName);
};

#endif // KAKEIBOTABLE_H

