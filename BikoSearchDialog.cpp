#include "BikoSearchDialog.h"
#include "koza.h"
#include "mainwindow.h" // getDatabasePath()
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSqlTableModel>
#include <QSqlQuery>
#include "BikoSearchDialog.h"

BikoSearchDialog::BikoSearchDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("備考検索");
    setMinimumSize(600, 400);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 検索フォーム
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(new QLabel("キーワード:"));
    editKeyword = new QLineEdit();
    hLayout->addWidget(editKeyword);
    btnSearch = new QPushButton("検索");
    hLayout->addWidget(btnSearch);
    mainLayout->addLayout(hLayout);

    // 結果表示
    view = new QTableView(this);
    model = new QStandardItemModel(this);
    view->setModel(model);
    view->setAlternatingRowColors(true);
    mainLayout->addWidget(view);

    connect(btnSearch, &QPushButton::clicked, this, &BikoSearchDialog::onSearchClicked);
}


void BikoSearchDialog::onSearchClicked()
{
    QString keyword = editKeyword->text().trimmed();
    if (keyword.isEmpty()) return;

    searchKeyword(keyword);
}

void BikoSearchDialog::searchKeyword(const QString &keyword)
{
    model->clear();
    model->setHorizontalHeaderLabels({"口座名", "日付", "支出", "収入","費目", "備考"});

    QString dbpath = MainWindow::getDatabasePath();
    QList<koza> accounts = koza::selectAll(dbpath);

    for (const koza &k : accounts) {
        QString tableName = QString("shishutunyu%1").arg(k.num);
        QSqlTableModel tModel(nullptr, QSqlDatabase::database());
        tModel.setTable(tableName);

        bool ok = false;
        int keywordNum = keyword.toInt(&ok);

        QString filter;
        if (ok) {
            filter = QString(
                         "biko LIKE '%%1%' OR "
                         "sishutu = %2 OR "
                         "shunyu = %2"
                         ).arg(keyword).arg(keywordNum);
        } else {
            filter = QString("biko LIKE '%%1%'").arg(keyword);
        }


        tModel.setFilter(filter);
        //tModel.setFilter(QString("biko LIKE '%%1%'").arg(keyword));
        tModel.select();

        for (int r = 0; r < tModel.rowCount(); ++r) {
            QList<QStandardItem*> rowItems;
            rowItems << new QStandardItem(k.kozaName);
            rowItems << new QStandardItem(tModel.data(tModel.index(r, 1)).toString()); // date
            rowItems << new QStandardItem(tModel.data(tModel.index(r, 2)).toString()); // income
            rowItems << new QStandardItem(tModel.data(tModel.index(r, 3)).toString()); // expense
            rowItems << new QStandardItem(tModel.data(tModel.index(r, 5)).toString());
            rowItems << new QStandardItem(tModel.data(tModel.index(r, 6)).toString()); // biko
            model->appendRow(rowItems);
        }
    }

    view->resizeColumnsToContents();
}
