// BaseTableWidget.cpp
#include "BaseTableWidget.h"

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
    view->hideColumn(0); // 0 は num がモデルの最初の列の場合

     // QStringList headers = {"ID", "口座名", "タブ色"};
    for (int i = 0; i < headers.size(); ++i) {
       model->setHeaderData(i, Qt::Horizontal, headers[i]);
    }
}

void BaseTableWidget::initDB(const QString createSQL)
{

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("vckake.db");

    bool create = !QFile::exists("vckake.db");
    if (!db.open()) {
        qDebug() << "DB接続失敗";
        return;
    }

    if (create) {
        QSqlQuery query;


        if (!createSQL.isEmpty()) {
            if (!query.exec(createSQL))
                qDebug() << "テーブル作成失敗:" << query.lastError().text();
            else
                qDebug() << "新規DBとテーブル作成完了";

        }
       // query.exec("INSERT INTO koza (kozaName, tabColor) VALUES ('', '')");
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
