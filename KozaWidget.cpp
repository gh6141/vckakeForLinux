#include "KozaWidget.h"

KozaWidget::KozaWidget(QWidget* parent)
    : BaseTableWidget(parent)
{



    QString sql = "CREATE TABLE koza ("
                  "num INTEGER PRIMARY KEY AUTOINCREMENT, "
                  "kozaName TEXT, "
                  "tabColor TEXT)";
    setTableName("koza",sql);
    initDB(sql);  // テーブル作成は派生クラスが決める
}

// 必要なら派生クラスで addRow を上書き
void KozaWidget::addRow()
{
    BaseTableWidget::addRow();  // 空行追加
    int row = model->rowCount() - 1;
    if (row >= 0) {
        model->setData(model->index(row, 1), "初期科目名");
        model->setData(model->index(row, 2), "赤");
    }
}
