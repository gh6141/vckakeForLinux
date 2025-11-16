
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "kakeibotable.h"
#include <QPushButton>
#include "ShiharaisakiComboWidget.h"
#include "BikoComboWidget.h"
#include "ComboRegisterHelper.h"
#include "ThreeRelationShipsWidget.h"
#include "PopularCbx.h"

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

    void comboTwoUpdate();
    Ui::MainWindow* getUi() { return ui; }
    QList<ThreeRelationShip> loadThreeRelationList();
    QList<ThreeRelationShip> ssnLtrs;

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

    void on_actionsearch_triggered();

    void on_pushButton_5_clicked();

    void on_pushButton_4_clicked();

    void on_comboBox_6_currentIndexChanged(int index);

    void on_comboBox_5_currentIndexChanged(int index);

    void on_comboBox_2_currentIndexChanged(int index);

    void on_comboBox_3_currentIndexChanged(int index);

    void on_checkBox_checkStateChanged(const Qt::CheckState &arg1);

private:
       Ui::MainWindow *ui;
    KakeiboTable *table;  // メンバとして保持
    KakeiboTable *dst_table;
    QPushButton *deleteButton = nullptr;
    ComboRegisterHelper* comboHelper;  // ← ここが必要

    ThreeRelationShipsWidget *m_trw = nullptr;
    PopularCbx *pcbx,*pcbx2;
    bool comboInitializing = true; // 初期化中は true
};

#endif // MAINWINDOW_H
