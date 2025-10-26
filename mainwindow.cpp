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
#include <QWidget>

#include <QShowEvent>


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

    HimokuComboWidget himokuWidget;
    himokuWidget.fillComboBox(ui->comboBox_6);

    ShiharaisakiComboWidget shiharaisakiWidget;
    shiharaisakiWidget.fillComboBox(ui->comboBox_5);

    BikoComboWidget bikoWidget;
    bikoWidget.fillComboBox(ui->comboBox_7);



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



