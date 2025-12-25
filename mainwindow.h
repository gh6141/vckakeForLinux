
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
#include "DraggableGridWidget.h"
#include "OricoRowData.h"
#include "ExpenseLoader.h"
#include "WebApiDialog.h"

namespace Ui {
class MainWindow;
}


struct tmpDtForMap{
    bool matchFlg = false;  // デフォルト値 false
    KakeiboRowData krdata; //OricoDataも最終的にDBに入れるのでこの形で統一
    int x;//x座標
    int y;//y座標
    int dtKind;//kakeiboData:0  oricoData:1
    int number;// original number 0~
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QList<tmpDtForMap> cButtonL;
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    static QString getDatabasePath();
    int ckozanum;
    int dst_ckozanum;
    int calculateBalance(int accountNum, const QDate& date);
    QList<int> getAccountList();

    void comboTwoUpdate();
    Ui::MainWindow* getUi() { return ui; }
    QList<ThreeRelationShip> loadThreeRelationList();
    QList<ThreeRelationShip> ssnLtrs;

    void loadExpenses();
    void netInFlg(std::function<void(bool)> callback) ;
    void onFormShown();
    void delCloud(const Expense& exp);
    void updateBalance();
    QLabel* balanceLabel ;
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

    void on_checkBox_2_checkStateChanged(const Qt::CheckState &arg1);

    void on_actionimport_triggered();

    void on_comboBox_8_currentIndexChanged(int index);

    void on_comboBox_8_activated(int index);

    void on_comboBox_8_currentTextChanged(const QString &arg1);

    void on_comboBox_5_currentTextChanged(const QString &arg1);

    void on_comboBox_6_editTextChanged(const QString &arg1);

    void on_comboBox_6_currentTextChanged(const QString &arg1);
    void onKosinClicked(DraggableGridWidget* grid,
                        const QList<KakeiboRowData>& kRows,
                        const QList<OricoRowData>& oRows,
                        int& mode,QString rep);

    void on_actionWebAPI_triggered();

    void on_pushButton_6_clicked();

    void on_actionimportKake_triggered();

   // void on_pushButton_7_clicked();

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
       Ui::MainWindow *ui;
    KakeiboTable *table;  // メンバとして保持
    KakeiboTable *dst_table;
    QPushButton *deleteButton = nullptr;
    QPushButton *updateButton=nullptr;
    ComboRegisterHelper *cbh1_payee,*cbh1_biko;
    ComboRegisterHelper *cbh2_payee,*cbh2_biko;

    ThreeRelationShipsWidget *m_trw = nullptr;
    PopularCbx *pcbx,*pcbx2;
    bool comboInitializing = true; // 初期化中は true
    QDialog *dlg;
    QString populateOricoGrid(DraggableGridWidget* grid,
                                       const QVector<KakeiboRowData>& kRows,
                           const QVector<OricoRowData>& oricoRows,int& total);

    KakeiboRowData *krdata;
    OricoRowData *ordata;
    QDate fromDate;
    QDate toDate;
    void updateTmpL(int fromRow, int fromCol, int toRow, int toCol);
    QNetworkAccessManager* manager;
    void backupDatabase(const QString& dbPath);


};

#endif // MAINWINDOW_H
