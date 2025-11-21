// BalanceListWidget.h
#pragma once
#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>

class BalanceListWidget : public QWidget {
    Q_OBJECT
public:
    BalanceListWidget(QWidget *parent = nullptr);

    void setData(const QList<QPair<QString,double>> &balances, const QStringList &headers,double total);
    static int calculateBalance(int accountNum, const QDate& date);
private:
    QTableView *view;
    QStandardItemModel *model;
};
