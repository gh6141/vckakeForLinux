// BalanceListWidget.cpp
#include "BalanceListWidget.h"
#include <QVBoxLayout>
#include <QStandardItem>
#include <QDate>
#include <QList>
#include <QString>
#include <QPair>
#include "mainwindow.h"
#include "koza.h"
#include <QHeaderView>
#include <QStyledItemDelegate>

BalanceListWidget::BalanceListWidget(QWidget *parent) : QWidget(parent) {
    view = new QTableView(this);
    model = new QStandardItemModel(this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(view);

    view->setModel(model);
    view->setAlternatingRowColors(true);


    QDate targetDate = QDate::currentDate(); // 今日まで
    QString dbpath = MainWindow::getDatabasePath();
    QList<koza> accounts = koza::selectAll(dbpath);

    QList<QPair<QString,double>> result;
    double total = 0;
    for (const koza &k : accounts) {
        double bal = calculateBalance(k.num, targetDate);
        total += bal;
        result.append(qMakePair(k.kozaName, bal));
    }

    // ヘッダー
    QStringList headers = {"口座名","残高"};

    // データセット
    setData(result, headers,total);


}

void BalanceListWidget::setData(const QList<QPair<QString,double>> &balances, const QStringList &headers,double total) {
    model->clear();
    model->setHorizontalHeaderLabels(headers);


    for (const auto &pair : balances) {
        QList<QStandardItem*> rowItems;
        QStandardItem *itemName = new QStandardItem(pair.first);

        QStandardItem *itemBalance = new QStandardItem(QString::number(int(pair.second)));
        itemBalance->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter); // 右寄せ

        rowItems.append(itemName);
        rowItems.append(itemBalance);
        model->appendRow(rowItems);
    }

    QList<QStandardItem*> totalRow;
    totalRow.append(new QStandardItem("総計"));

    QStandardItem *totalItem = new QStandardItem(QString::number(int(total)));
    totalItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter); // 右寄せ
    totalRow.append(totalItem);

    model->appendRow(totalRow);


    view->resizeColumnsToContents();
    QFont font = view->font();
    font.setPointSize(14);  // 好きなサイズに調整
    view->setFont(font);

    // ヘッダーのフォントサイズ
    QFont headerFont = view->horizontalHeader()->font();
    headerFont.setPointSize(14); // ヘッダーを少し大きく
    view->horizontalHeader()->setFont(headerFont);

    // 列幅調整
    view->setColumnWidth(0, 150); // 口座名
    view->setColumnWidth(1, 100); // 残高
    view->horizontalHeader()->setStretchLastSection(true);
}



int BalanceListWidget::calculateBalance(int accountNum, const QDate& date)
{


    QSqlQuery query;
    query.exec("SELECT sishutu, shunyu FROM shishutunyu" + QString::number(accountNum) +
               (date == QDate::currentDate() ? "" : " WHERE date <= '" + date.toString("yyyy/MM/dd") + "'"));

    int balance = 0;
    int incomet = 0;
    int outcomet = 0;

    while(query.next()) {
        int expense = query.value(0).toInt();
        int income  = query.value(1).toInt();
        balance += (income - expense);
        incomet += income;
        outcomet += expense;
    }

    //qDebug() << "in out=" << incomet << outcomet;
    return balance;

}
