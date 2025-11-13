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


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    table = new KakeiboTable(); // 親は addWidget() で設定されるので不要
    //ui->centralwidget->layout()->addWidget(table);
    QVBoxLayout *vbox = qobject_cast<QVBoxLayout*>(ui->centralwidget->layout());
    if (vbox) {
        vbox->insertWidget(0, table);  // 👈 一番上（index=0）に挿入
    }

    KozaComboWidget kozaWidget;
    kozaWidget.fillComboBox(ui->comboBox_8);
    kozaWidget.fillComboBox(ui->comboBox);
    kozaWidget.fillComboBox(ui->comboBox_12);
    kozaWidget.fillComboBox(ui->comboBox_13);

    HimokuComboWidget himokuWidget;
    himokuWidget.fillComboBox(ui->comboBox_6);
    himokuWidget.fillComboBox(ui->comboBox_2);


    ShiharaisakiComboWidget shiharaisakiWidget;
    shiharaisakiWidget.fillComboBox(ui->comboBox_5);
    shiharaisakiWidget.fillComboBox(ui->comboBox_3);

    BikoComboWidget bikoWidget;
    bikoWidget.fillComboBox(ui->comboBox_7);
    bikoWidget.fillComboBox(ui->comboBox_4);
    bikoWidget.fillComboBox(ui->comboBox_11);

    ui->dateEdit->setDate(QDate::currentDate());
    ui->dateEdit_2->setDate(QDate::currentDate());
    ui->dateEdit_3->setDate(QDate::currentDate());



    // ComboBox 選択が変わったらテーブルを切り替える
    connect(ui->comboBox_8, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [=](int index){
                int num = ui->comboBox_8->itemData(index).toInt();
                table->loadTable(num);
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
    table->addRowForCurrentAccount(data,true);//true=sishutu false=shunyu
}


void MainWindow::on_pushButton_clicked()
{
    KakeiboRowData data;
    data.date = ui->dateEdit->date();
    data.kingaku = ui->lineEdit_2->text().toInt();
    data.himoku = ui->comboBox_2->currentText();
    data.shiharaisaki = ui->comboBox_3->currentText();
    data.biko = ui->comboBox_4->currentText();
    table->addRowForCurrentAccount(data,false);//true=sishutu false=shunyu
}

