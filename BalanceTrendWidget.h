#ifndef BALANCETRENDWIDGET_H
#define BALANCETRENDWIDGET_H

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QDate>

class BalanceTrendWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BalanceTrendWidget(QDate start, QDate end, QWidget *parent = nullptr);
    void chart();
    QList<QPair<QDate,double>> balances;
private:
    QTableView *view;
    QStandardItemModel *model;

    void loadData(QDate start, QDate end);

};

#endif // BALANCETRENDWIDGET_H
