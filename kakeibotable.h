#ifndef KAKEIBOTABLE_H
#define KAKEIBOTABLE_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QTableView>
#include <QVBoxLayout>
#include <QFile>
#include "KakeiboRowData.h"
#include <QSortFilterProxyModel>
#include "MultiSortProxy.h"

class KakeiboTable : public QWidget
{
    Q_OBJECT

public:
    explicit KakeiboTable(QWidget *parent = nullptr);
    bool initDB();
    void loadTable(int accountNum);  // 口座番号でテーブル切替
    void scrollDown();
    void recalculateBalances(int kznum);//残高更新
    QSqlTableModel* loadModel(int accountNUm);
    void addRowForCurrentAccountModel(const KakeiboRowData& data,bool sishutuFlg,int cnum);
    QItemSelectionModel* selectionModel() const {
        return view->selectionModel();
    }
    QSqlTableModel* getmodel(){
        return model;
    }
    QVector<KakeiboRowData> getAllRows(const QDate& fromDate, const QDate& toDate,const int& tableNum) const;


    bool add(const KakeiboRowData& data, bool sishutuFlg, int knum);
    bool updateRow(const KakeiboRowData& data,bool sishutuFlg, int tbnum);
    QSqlTableModel* getModel() const { return model; }
    QSortFilterProxyModel* getProxyModel() const { return proxy; }
    QTableView* getView() const { return view; }

public slots:
   // void addRowForCurrentAccount();
    void addRowForCurrentAccount(const KakeiboRowData& data,bool flg,int knum);
signals:
       void balanceChanged();  // signal を宣言

private:
    QSqlDatabase db;
    QSqlTableModel *model;
    QTableView *view;
    int currentAccountNum;

    bool ensureTableExists(const QString &tableName);
    //QSortFilterProxyModel* proxy;   // ★ これが必要
    MultiSortProxy* proxy;

};




#endif // KAKEIBOTABLE_H

