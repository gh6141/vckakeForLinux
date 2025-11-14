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



double BalanceListWidget::calculateBalance(int accountNum, const QDate& date)
{
    // テーブル名を口座番号から生成
    QString tableName = QString("shishutunyu%1").arg(accountNum);

    QSqlTableModel model(nullptr, QSqlDatabase::database());
    model.setTable(tableName);
    model.setFilter(QString("date <= '%1'").arg(date.toString("yyyy-MM-dd"))); // 日付まで
    model.select();

    double balance = 0.0;
    int rows = model.rowCount();
    for (int r = 0; r < rows; ++r) {
        double income  = model.data(model.index(r, 3)).toDouble(); // 収入列
        double expense = model.data(model.index(r, 2)).toDouble(); // 支出列
        balance += (income - expense);
    }

    return balance;
}
