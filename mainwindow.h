#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "kakeibotable.h"
#include <QPushButton>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    static QString getDatabasePath();
    int ckozanum;
    int dst_ckozanum;
    double calculateBalance(int accountNum, const QDate& date);
    QList<int> getAccountList();

private slots:
    void on_actionkozaEdit_triggered();

    void on_actionactionhimokuEdit_triggered();

    void on_actionactionbikoEdit_triggered();

    void on_actionactionshiharaisaki_triggered();

    void on_actionaction3rsEdit_triggered();

    void on_actiondbSet_triggered();

    void on_tabWidget_currentChanged(int index);

    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

    void on_actionexit_triggered();

    void on_pushButton_3_clicked();

    void on_actionzandakaList_triggered();

    void on_actionsuii_triggered();

private:
    Ui::MainWindow *ui;
    KakeiboTable *table;  // メンバとして保持
    KakeiboTable *dst_table;
    QPushButton *deleteButton = nullptr;


};

#endif // MAINWINDOW_H
