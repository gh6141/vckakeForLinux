#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "KozaWidget.h"
#include "HimokuWidget.h"
#include "BikoWidget.h"
#include "ShiharaisakimotoWidget.h"
#include "ThreeRelationShipsWidget.h"
#include "kakeibotable.h"
#include "KozaComboWidget.h"
#include "HimokuComboWidget.h"
#include "ShiharaisakiComboWidget.h"
#include "BikoComboWidget.h"
#include "KakeiboRowData.h"
#include <QWidget>
#include <QShowEvent>

#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>
#include <QPushButton>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    table = new KakeiboTable(); // 親は addWidget() で設定されるので不要
    //ui->centralwidget->layout()->addWidget(table);
    QVBoxLayout *vbox = qobject_cast<QVBoxLayout*>(ui->centralwidget->layout());
    // 削除ボタン作成
    deleteButton = new QPushButton("削除", this);
    deleteButton->setFixedWidth(80);
    deleteButton->setVisible(false);
    // 横レイアウト作成（右端に寄せる）
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addStretch();                  // 左側を伸ばす
    hbox->addWidget(deleteButton);       // 右端にボタン
    hbox->setAlignment(deleteButton, Qt::AlignRight);
    hbox->setContentsMargins(0, 0, 10, 0);
    if (vbox) {
        vbox->insertWidget(0, table);  // 👈 一番上（index=0）に挿入
        deleteButton->setVisible(true);
        vbox->insertLayout(1, hbox);  // table の下に配置

        // 削除ボタンクリックで選択行削除
        connect(deleteButton, &QPushButton::clicked, this, [this]() {
            QItemSelectionModel *sel = table->selectionModel();
            if (!sel->hasSelection()) return;

            QModelIndexList selected = sel->selectedRows();
            std::sort(selected.begin(), selected.end(),
                      [](const QModelIndex &a, const QModelIndex &b) { return a.row() > b.row(); });

            for (const QModelIndex &idx : selected) {
                table->getmodel()->removeRow(idx.row());
            }
             table->loadTable(ckozanum);

            // deleteButton->setVisible(false);
        });
    }


    KozaComboWidget kozaWidget;
    kozaWidget.fillComboBox(ui->comboBox_8);
    kozaWidget.fillComboBox(ui->comboBox);
    kozaWidget.fillComboBox(ui->comboBox_12);
    kozaWidget.fillComboBox(ui->comboBox_13);

    HimokuComboWidget himokuWidget;
    himokuWidget.fillComboBox(ui->comboBox_6,"yomi");
    himokuWidget.fillComboBox(ui->comboBox_2,"yomi");


    ShiharaisakiComboWidget shiharaisakiWidget;
    shiharaisakiWidget.fillComboBox(ui->comboBox_5,"yomi");
    shiharaisakiWidget.fillComboBox(ui->comboBox_3,"yomi");

    BikoComboWidget bikoWidget;
    bikoWidget.fillComboBox(ui->comboBox_7,"yomi");
    bikoWidget.fillComboBox(ui->comboBox_4,"yomi");
    bikoWidget.fillComboBox(ui->comboBox_11,"yomi");

    ui->dateEdit->setDate(QDate::currentDate());
    ui->dateEdit_2->setDate(QDate::currentDate());
    ui->dateEdit_3->setDate(QDate::currentDate());


    ckozanum=1;
    dst_ckozanum=1;

    // ComboBox 選択が変わったらテーブルを切り替える
    // ３つのコンボボックスを QList にまとめる
    QList<QComboBox*> combos = { ui->comboBox_8, ui->comboBox, ui->comboBox_12 };

    // 同期処理ラムダ
    auto syncComboBoxes = [=](int index, QComboBox* senderCombo){
        if (index < 0) return;
        int value = senderCombo->itemData(index).toInt();
        ckozanum = value;

        // 他のコンボボックスの選択を合わせる
        for (auto combo : combos) {
            if (combo != senderCombo) {
                // 無限ループ防止のためシグナルブロック
                QSignalBlocker blocker(combo);
                combo->setCurrentIndex(index);
            }
        }

        table->loadTable(ckozanum);
    };

    // ３つとも同じラムダで接続
    for (auto combo : combos) {
        connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, [=](int index){ syncComboBoxes(index, combo); });
    }


    connect(ui->comboBox_13, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, [=](int index){
       dst_ckozanum=ui->comboBox_13->itemData(index).toInt();

      });




    // 「追加」ボタンを押したら行追加

    connect(ui->pushButton_2, &QPushButton::clicked, this, [=](){


    });

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionkozaEdit_triggered()
{
    KozaWidget *w = new KozaWidget(this);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->show();
}


void MainWindow::on_actionactionhimokuEdit_triggered()
{
    HimokuWidget *w = new HimokuWidget(this);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->show();
}


void MainWindow::on_actionactionbikoEdit_triggered()
{
    BikoWidget *w = new BikoWidget(this);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->show();
}


void MainWindow::on_actionactionshiharaisaki_triggered()
{
    ShiharaisakimotoWidget *w = new ShiharaisakimotoWidget(this);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->show();
}


void MainWindow::on_actionaction3rsEdit_triggered()
{
    ThreeRelationShipsWidget *w = new ThreeRelationShipsWidget(this);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->show();
}






void MainWindow::on_actiondbSet_triggered()
{
    QSettings settings("MyCompany", "QtKakeibo");  // ① 設定ファイル（自動で作成される）

    QString currentPath = settings.value("Database/Path", "qtkake.db").toString();

    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("データベースの保存場所を選択"),
        currentPath,
        tr("SQLite Database (*.db);;All Files (*.*)")
        );

    if (fileName.isEmpty())
        return; // キャンセル時

    // ② 選択されたパスを保存
    settings.setValue("Database/Path", fileName);

    QMessageBox::information(this, tr("設定完了"),
                             tr("データベースの保存場所を設定しました:\n%1").arg(fileName));
}


QString MainWindow::getDatabasePath()
{
    QSettings settings("MyCompany", "QtKakeibo");
    // ③ 設定値がなければ "qtkake.db" を返す
    return settings.value("Database/Path", "qtkake.db").toString();
}


void MainWindow::on_tabWidget_currentChanged(int index)
{

}


void MainWindow::on_pushButton_2_clicked()
{
    KakeiboRowData data;
    data.date = ui->dateEdit_2->date();
    data.kingaku = ui->lineEdit->text().toInt();
    data.himoku = ui->comboBox_6->currentText();
    data.shiharaisaki = ui->comboBox_5->currentText();
    data.biko = ui->comboBox_7->currentText();
    table->addRowForCurrentAccount(data,true,ckozanum);//true=sishutu false=shunyu
        table->loadTable(ckozanum);
}


void MainWindow::on_pushButton_clicked()
{
    KakeiboRowData data;
    data.date = ui->dateEdit->date();
    data.kingaku = ui->lineEdit_2->text().toInt();
    data.himoku = ui->comboBox_2->currentText();
    data.shiharaisaki = ui->comboBox_3->currentText();
    data.biko = ui->comboBox_4->currentText();
    table->addRowForCurrentAccount(data,false,ckozanum);//true=sishutu false=shunyu
        table->loadTable(ckozanum);
}


void MainWindow::on_actionexit_triggered()
{
    QApplication::quit();
}





void MainWindow::on_pushButton_3_clicked()
{
    if(ckozanum==dst_ckozanum){
        QMessageBox::warning(this, "口座の一致", "送り先と元が同じになっています");
        return;
    }

    KakeiboRowData data;
    data.date = ui->dateEdit_3->date();
    int kg=ui->lineEdit_3->text().toInt();
    data.kingaku = kg;
    QString cid=QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
    data.himoku = cid;
    data.shiharaisaki = ui->comboBox_12->currentText()+">"+ui->comboBox_13->currentText();
    data.biko = ui->comboBox_11->currentText();
    table->addRowForCurrentAccount(data,true,ckozanum);//true=sishutu false=shunyu
  //  table->loadTable(ckozanum);

 // QMessageBox::warning(this, "次の処理", "送り先の処理をします");
    table->loadTable(dst_ckozanum);
   //QMessageBox::warning(this, "次の処理", "送り先の処理をします2");
    KakeiboRowData data2;
    data2.date = ui->dateEdit_3->date();
    data2.kingaku = kg;
    data2.himoku =cid;
    data2.shiharaisaki = ui->comboBox_12->currentText()+">"+ui->comboBox_13->currentText();
    data2.biko = ui->comboBox_11->currentText();


   table->addRowForCurrentAccount(data2,false,dst_ckozanum);//true=sishutu false=shunyu
  //  QMessageBox::warning(this, "次の処理", "送り先の処理をします3");
    table->loadTable(ckozanum);

}

