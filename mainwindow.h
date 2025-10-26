#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "kakeibotable.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionkozaEdit_triggered();

    void on_actionactionhimokuEdit_triggered();

    void on_actionactionbikoEdit_triggered();

    void on_actionactionshiharaisaki_triggered();

    void on_actionaction3rsEdit_triggered();

private:
    Ui::MainWindow *ui;
    KakeiboTable *table;  // メンバとして保持

};

#endif // MAINWINDOW_H
