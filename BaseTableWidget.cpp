// BaseTableWidget.cpp
#include "BaseTableWidget.h"
#include "mainwindow.h"

BaseTableWidget::BaseTableWidget(QWidget *parent,QString createSQL)
    : QMainWindow(parent),
    model(nullptr),
    view(new QTableView(this))
{
    initUI();
}

void BaseTableWidget::initUI()
{
    QWidget* central = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(central);

    QPushButton* btnAdd = new QPushButton("追加", central);
    QPushButton* btnDelete = new QPushButton("削除", central);

    layout->addWidget(view);
    layout->addWidget(btnAdd);
    layout->addWidget(btnDelete);

    setCentralWidget(central);

    connect(btnAdd, &QPushButton::clicked, this, &BaseTableWidget::addRow);
    connect(btnDelete, &QPushButton::clicked, this, &BaseTableWidget::deleteRow);
}

void BaseTableWidget::setTableName(const QString &name,const QString createSQL,const QStringList& headers)
{
    tableName = name;
    initDB(createSQL);

    model = new QSqlTableModel(this, QSqlDatabase::database());
    model->setTable(tableName);
    model->setEditStrategy(QSqlTableModel::OnFieldChange);
    model->select();
    view->setModel(model);
    // num 列を非表示にする
  //  view->hideColumn(0); // 0 は num がモデルの最初の列の場合


    for (int i = 0; i < headers.size(); ++i) {
       model->setHeaderData(i, Qt::Horizontal, headers[i]);
    }
}

void BaseTableWidget::initDB(const QString createSQL)
{
   // QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "mainConnection");
   // db.setDatabaseName(MainWindow::getDatabasePath());
QString connectionName = "mainConnection";
    QSqlDatabase db;
    if (QSqlDatabase::contains(connectionName)) {
        db = QSqlDatabase::database(connectionName);
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        db.setDatabaseName("mydatabase.db");
    }



    if (!db.open()) {
       // qDebug() << "DB接続失敗";
        return;
    }
        QSqlQuery query;
        if (!createSQL.isEmpty()) {
            if (!query.exec(createSQL))
                qDebug() << "テーブル作成失敗:" << query.lastError().text();
            //else
             //   qDebug() << "新規DBとテーブル作成完了";
        }
}


void BaseTableWidget::addRow()
{
    if (model)
        model->insertRow(model->rowCount()); // 空行追加
}

void BaseTableWidget::deleteRow()
{
    if (model) {
        int row = view->currentIndex().row();
        if (row >= 0) {
            model->removeRow(row);
            model->submitAll();
        }
    }
}
