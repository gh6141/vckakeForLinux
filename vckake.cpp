#include "vckake.h"
#include "./ui_vckake.h"

#include <QSqlDatabase>      // DB 接続用
#include <QSqlTableModel>    // テーブルモデル
#include <QSqlQuery>         // クエリ実行用（必要なら）
#include <QSqlError>         // エラー確認用
#include <QDebug>            // デバッグ出力用
#include <QFile>
#include <QPushButton>

vckake::vckake(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::vckake)
{
    ui->setupUi(this);


    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE"); // 例: SQLite
    db.setDatabaseName("vckake.db");

    bool create = !QFile::exists("vckake.db");
    if (!db.open()) {
        qDebug() << "DB接続失敗";
        return;
    }
    if (create) {
        QSqlQuery query;
        if (!query.exec("CREATE TABLE koza ("
                        "num INTEGER PRIMARY KEY AUTOINCREMENT, "
                        "kozaName TEXT, "
                        "tabColor TEXT)")) {
            qDebug() << "テーブル作成失敗:" << query.lastError().text();
        } else {
            qDebug() << "新規DBとテーブル作成完了";
        }
    }

    QSqlTableModel *model = new QSqlTableModel(parent, db);
    model->setTable("koza");   // 表示したいテーブル名
    model->setEditStrategy(QSqlTableModel::OnFieldChange); // 編集内容を即座に反映
    model->select();


    // QTableView にモデルをセット
    ui->tableView->setModel(model);

    // 必要ならカラム幅を自動調整
    ui->tableView->resizeColumnsToContents();


    // 行追加・削除ボタン
    connect(ui->btnAdd, &QPushButton::clicked, this, [=]() {
        int row = model->rowCount();
        model->insertRow(row);
    });

    connect(ui->btnDelete, &QPushButton::clicked, this, [=]() {
        int row = ui->tableView->currentIndex().row();
        if (row >= 0) model->removeRow(row);
        model->submitAll();
    });





}

vckake::~vckake()
{
    delete ui;
}
