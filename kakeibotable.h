#ifndef KAKEIBOTABLE_H
#define KAKEIBOTABLE_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QTableView>
#include <QVBoxLayout>
#include <QFile>
#include "KakeiboRowData.h"

class KakeiboTable : public QWidget
{
    Q_OBJECT

public:
    explicit KakeiboTable(QWidget *parent = nullptr);
    bool initDB();
    void loadTable(int accountNum);  // 口座番号でテーブル切替
    void recalculateBalances();//残高更新
    QSqlTableModel* loadModel(int accountNUm);
    void addRowForCurrentAccountModel(const KakeiboRowData& data,bool sishutuFlg,int cnum);
    QItemSelectionModel* selectionModel() const {
        return view->selectionModel();
    }
    QSqlTableModel* getmodel(){
        return model;
    }
    QVector<KakeiboRowData> getAllRows(const QDate& fromDate, const QDate& toDate,const int& tableNum) const;
    QVector<KakeiboRowData> getAllRows() const;



public slots:
   // void addRowForCurrentAccount();
    void addRowForCurrentAccount(const KakeiboRowData& data,bool flg,int knum);


private:
    QSqlDatabase db;
    QSqlTableModel *model;
    QTableView *view;
    int currentAccountNum;

    bool ensureTableExists(const QString &tableName);
};




#endif // KAKEIBOTABLE_H

