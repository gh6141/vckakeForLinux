#include "kakeibotable.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDate>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include "mainwindow.h"
#include <QTimer>
#include <QHeaderView>
#include "MyKakeiboModel.h"
#include "MultiSortProxy.h"




class RightAlignDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override
    {
        QStyledItemDelegate::initStyleOption(option, index);
        option->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
    }
};

KakeiboTable::KakeiboTable(QWidget *parent)
    : QWidget(parent),
    model(nullptr),
    view(new QTableView(this)),
    currentAccountNum(1)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(view);
        view->setSortingEnabled(true);



    if (initDB()) {
        loadTable(currentAccountNum);

    }
}

bool KakeiboTable::initDB()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
   // db.setDatabaseName("vckake.db");
     db.setDatabaseName(MainWindow::getDatabasePath());

    if (!db.open()) {
        qDebug() << "DB open error:" << db.lastError().text();
        return false;
    }

    return true;
}

bool KakeiboTable::ensureTableExists(const QString &tableName)
{
    QSqlQuery query(db);
    QString createSQL = QString(
                            "CREATE TABLE IF NOT EXISTS %1 ("
                            "num INTEGER PRIMARY KEY AUTOINCREMENT,"
                            "date TEXT,"
                            "sishutu INTEGER,"
                            "shunyu INTEGER,"
                            "zandaka INTEGER,"
                            "himoku TEXT,"
                            "biko TEXT,"
                            "idosaki TEXT)"
                            ).arg(tableName);

    if (!query.exec(createSQL)) {
        qDebug() << "Create table error:" << query.lastError().text();
        return false;
    }

    return true;
}

QSqlTableModel* KakeiboTable::loadModel(int accountNum)
 {
    currentAccountNum = accountNum;
    QString tableName = QString("shishutunyu%1").arg(accountNum);

    ensureTableExists(tableName);

    if (model) delete model;

   // model = new QSqlTableModel(this, db);
    model = new MyKakeiboModel(this, db); // ← 継承型を使用
    model->setTable(tableName);
    model->setEditStrategy(QSqlTableModel::OnFieldChange);
    model->select();



    model->setHeaderData(0, Qt::Horizontal, "番号");
    model->setHeaderData(1, Qt::Horizontal, "日付");
    model->setHeaderData(2, Qt::Horizontal, "支出");
    model->setHeaderData(3, Qt::Horizontal, "収入");
    model->setHeaderData(4, Qt::Horizontal, "残高");
    model->setHeaderData(5, Qt::Horizontal, "費目/移動ID");
    model->setHeaderData(6, Qt::Horizontal, "支払/備考");




    return model;
}



void KakeiboTable::loadTable(int accountNum)
{



    view->setModel(loadModel(accountNum));
  //  view->verticalHeader()->setVisible(false); // 左端の行番号を非表示
    //
    // ▼ ここから外観設定
    //
    // --- 文字サイズ変更 ---
    QFont font = view->font();
    font.setPointSize(12);  // ← フォントサイズを指定（例: 12pt）
    view->setFont(font);

    QFont headerFont = view->horizontalHeader()->font();
    headerFont.setPointSize(12);        // 好きなサイズに変更
    view->horizontalHeader()->setFont(headerFont);

    // --- セル幅を個別指定 ---
    view->setColumnWidth(0, 10);   // 番号
    view->setColumnWidth(1, 100);  // 日付
    view->setColumnWidth(2, 80);   // 支出
    view->setColumnWidth(3, 80);   // 収入
    view->setColumnWidth(4, 80);   // 残高
    view->setColumnWidth(5, 120);  // 費目
    view->setColumnWidth(6, 200);  // 備考

    // --- 数値列を右揃え ---
    view->setItemDelegateForColumn(0, new RightAlignDelegate(view)); // num
    view->setItemDelegateForColumn(2, new RightAlignDelegate(view)); // 支出
    view->setItemDelegateForColumn(3, new RightAlignDelegate(view)); // 収入
    view->setItemDelegateForColumn(4, new RightAlignDelegate(view)); // 残高

    // --- 行の高さ自動調整 ---
    view->resizeRowsToContents();

    // --- 列幅を全体にフィットさせる場合（手動指定の代替） ---
     view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // --- テーブルのスタイルを少し見やすくする例 ---
    view->setAlternatingRowColors(true); // 交互色
    view->setSelectionBehavior(QAbstractItemView::SelectRows); // 行単位選択



    KakeiboTable::recalculateBalances(accountNum);
    //view->resizeColumnsToContents();






    view->setColumnHidden(7, true);       // idosaki列を非表示

    if (!proxy) {
        proxy = new MultiSortProxy(this);   // ← ここ重要
    }

    proxy->setSourceModel(model);   // これが必須
    view->setModel(proxy);

    proxy->setDynamicSortFilter(true);

    view->setSortingEnabled(true);
    int dateColumn=1;
    view->sortByColumn(dateColumn, Qt::AscendingOrder);//sort



    QTimer::singleShot(0, view, [this]() {
        view->scrollToBottom();
    });

}

void KakeiboTable::addRowForCurrentAccountModel(const KakeiboRowData& data,bool sishutuFlg,int knum){
    if (!model) return;


    QString tableName = QString("shishutunyu%1").arg(knum);
    ensureTableExists(tableName);
    model->setTable(tableName);


    int row = model->rowCount();
    model->insertRow(row);
    model->setData(model->index(row, 1), data.date.toString("yyyy/MM/dd"));
    if(sishutuFlg){
        model->setData(model->index(row, 2), data.kingaku);
        model->setData(model->index(row, 3), 0);
    }else{
        model->setData(model->index(row, 2), 0);
        model->setData(model->index(row, 3), data.kingaku);
    }


    model->setData(model->index(row, 4), 0);
    model->setData(model->index(row, 5), data.himoku);   // 費目名
    if(data.shiharaisaki==""){
          model->setData(model->index(row, 6), data.biko);   // ID
    }else{
          model->setData(model->index(row, 6), data.shiharaisaki+"/"+data.biko);   // ID
    }

    model->setData(model->index(row, 7), data.idosaki);

    model->submitAll();
}


void KakeiboTable::addRowForCurrentAccount(const KakeiboRowData& data,bool sishutuFlg,int knum)
{
    addRowForCurrentAccountModel(data,sishutuFlg,knum);

    KakeiboTable::recalculateBalances(knum);
}


void KakeiboTable::recalculateBalances(int kznum)
{
    if (!model) return;
    if (!db.isOpen())    qDebug() << "Database not open!";
    int currentBalance = 0;
    // DB を日付順に取得
    QSqlQuery query("SELECT num, sishutu, shunyu FROM shishutunyu"+QString::number(kznum)+" ORDER BY date ASC, num ASC");
    int totalRows = 0;
    while (query.next())
    {
        ++totalRows;
        int num = query.value("num").toInt();
        int shishutsu = query.value("sishutu").toInt();
        int shuunyuu  = query.value("shunyu").toInt();
        currentBalance += (shuunyuu - shishutsu);
     //   qDebug() << "num=" << num << "in=" << shuunyuu << "out=" << shishutsu << "currentBalance=" << currentBalance;
        QSqlQuery update;
        update.prepare("UPDATE shishutunyu"+QString::number(kznum)+" SET zandaka=? WHERE num=?");
        update.addBindValue(currentBalance);
        update.addBindValue(num);
        if (!update.exec())
            qDebug() << "UPDATE failed:" << update.lastError().text();
    }
  //  qDebug() << "Total rows processed:" << totalRows<<"SELECT num, sishutsu, shunyu FROM shishutunyu"+QString::number(kznum)+" ORDER BY date ASC";
  //  model->select();
    QSqlTableModel *sqlModel = new QSqlTableModel(this);
    sqlModel->setTable("shishutunyu" + QString::number(kznum));
    sqlModel->select();

    MultiSortProxy *proxy = new MultiSortProxy(this);
    proxy->setSourceModel(sqlModel);
    proxy->setDynamicSortFilter(true);     // ← これがないと動かない
    view->setModel(proxy);
    view->setSortingEnabled(true);
    view->sortByColumn(1, Qt::AscendingOrder);  // 日付列を昇順

}


QVector<KakeiboRowData> KakeiboTable::getAllRows(const QDate& fromDate, const QDate& toDate,const int& tbnum) const
{
    //model = new QSqlTableModel(this, QSqlDatabase::database());
QSqlTableModel* model = new QSqlTableModel(nullptr, QSqlDatabase::database());
    model->setTable("shishutunyu"+QString::number(tbnum));
    model->select();

    QVector<KakeiboRowData> rows;
    if (!model) return rows;

    for (int r = 0; r < model->rowCount(); ++r) {
        QDate date = model->data(model->index(r, 1)).toDate();
        if (date < fromDate || date > toDate) continue;

        KakeiboRowData data;
        data.id= model->data(model->index(r, 0)).toInt();
        data.date = date;
        data.kingaku = model->data(model->index(r, 2)).toInt(); // 支出 or 収入に応じて分ける場合あり
        data.himoku = model->data(model->index(r, 5)).toString();
        data.shiharaisaki = model->data(model->index(r, 6)).toString();
        data.biko = model->data(model->index(r, 6)).toString();
        data.idosaki = model->data(model->index(r, 7)).toInt();

        rows.append(data);
    }
    return rows;
}

bool KakeiboTable::add(const KakeiboRowData& data, bool sishutuFlg, int knum)
{
    if (!model) return false;

    // テーブル名
    QString tableName = QString("shishutunyu%1").arg(knum);

    // 存在しなければ作成
    ensureTableExists(tableName);
    model->setTable(tableName);

    int row = model->rowCount();
    if (!model->insertRow(row)) return false;

    // 日付
    model->setData(model->index(row, 1), data.date.toString("yyyy/MM/dd"));

    // 支出 or 収入
    if (sishutuFlg) {
        model->setData(model->index(row, 2), data.kingaku);
        model->setData(model->index(row, 3), 0);
    } else {
        model->setData(model->index(row, 2), 0);
        model->setData(model->index(row, 3), data.kingaku);
    }

    // 残高（あとで再計算する場合は 0）
    model->setData(model->index(row, 4), 0);

    // 費目・備考
    model->setData(model->index(row, 5), data.himoku);
    model->setData(model->index(row, 6), data.shiharaisaki + "/" + data.biko);

    // 移動先（idosaki）
    model->setData(model->index(row, 7), data.idosaki);

    // DB に反映
    if (!model->submitAll()) {
        qDebug() << "Add failed:" << model->lastError();
        return false;
    }

    return true;
}

bool KakeiboTable::updateRow(const KakeiboRowData& data,bool sishutuFlg, int tbnum)
{
    QSqlTableModel model(nullptr, QSqlDatabase::database());
    model.setTable("shishutunyu" + QString::number(tbnum));
    model.select();
    // 行番号 r を探すなら r に対応する index(r,0) が主キー
    for (int r = 0; r < model.rowCount(); ++r) {
       //  qDebug()<<"update4 modelIndex="<<model.data(model.index(r,0)).toInt()<<" dataId="<<data.id;
        if (model.data(model.index(r,0)).toInt() == data.id) {

            model.setData(model.index(r, 1), data.date.toString("yyyy/MM/dd"));

            // ...他の列も更新
            // 支出 or 収入
            if (sishutuFlg) {
                model.setData(model.index(r, 2), data.kingaku);
                model.setData(model.index(r, 3), 0);
            } else {
                model.setData(model.index(r, 2), 0);
                model.setData(model.index(r, 3), data.kingaku);
            }

            // 残高（あとで再計算する場合は 0）
            model.setData(model.index(r, 4), 0);

            // 費目・備考
            model.setData(model.index(r, 5), data.himoku);
            model.setData(model.index(r, 6), data.biko);

            // 移動先（idosaki）
            model.setData(model.index(r, 7), data.idosaki);


            return model.submitAll();
        }
    }
    return false;
}
