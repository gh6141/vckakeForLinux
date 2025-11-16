#include "ShiharaisakimotoWidget.h"

ShiharaisakimotoWidget::ShiharaisakimotoWidget(QWidget* parent)
    : BaseTableWidget(parent)
{

    QString sql = "CREATE TABLE IF NOT EXISTS shiharaisakimoto ("
                  "num INTEGER PRIMARY KEY AUTOINCREMENT, "
                  "shiharaisaki TEXT, "
                  "yomi TEXT)";
    initDB(sql);  // テーブル作成は派生クラスが決める

    QStringList headers = {"ID", "支払先・元", "読み"};
    setTableName("Shiharaisakimoto",sql,headers);

}

// 必要なら派生クラスで addRow を上書き
void ShiharaisakimotoWidget::addRow()
{
    BaseTableWidget::addRow();  // 空行追加
    int row = model->rowCount() - 1;
    if (row >= 0) {
        model->setData(model->index(row, 1), "");
        model->setData(model->index(row, 2), "");
    }
}
