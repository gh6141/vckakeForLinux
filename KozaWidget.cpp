#include "KozaWidget.h"

KozaWidget::KozaWidget(QWidget* parent)
    : BaseTableWidget(parent)
{



    QString sql = "CREATE TABLE IF NOT EXISTS koza ("
                  "num INTEGER PRIMARY KEY AUTOINCREMENT, "
                  "kozaName TEXT, "
                  "tabColor TEXT)";
    initDB(sql);  // テーブル作成は派生クラスが決める

    QStringList headers = {"ID", "口座名", "インポート設定"};
    setTableName("koza",sql,headers);

}

// 必要なら派生クラスで addRow を上書き
void KozaWidget::addRow()
{
    BaseTableWidget::addRow();  // 空行追加
    int row = model->rowCount() - 1;
    if (row >= 0) {
       // model->setData(model->index(row, 0), "");
        model->setData(model->index(row, 1), "○銀行");
        model->setData(model->index(row, 2), "");
    }
}
