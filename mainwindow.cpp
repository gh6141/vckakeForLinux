#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "KozaWidget.h"
#include "HimokuWidget.h"
#include "BikoWidget.h"
#include "ShiharaisakimotoWidget.h"
#include "ThreeRelationShipsWidget.h"
#include "kakeibotable.h"
#include <QWidget>

#include <QShowEvent>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    table = new KakeiboTable(); // 親は addWidget() で設定されるので不要
    ui->centralwidget->layout()->addWidget(table);
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



