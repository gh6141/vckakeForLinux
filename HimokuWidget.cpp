#include "HimokuWidget.h"

HimokuWidget::HimokuWidget(QWidget* parent)
    : BaseTableWidget(parent)
{

    QString sql = "CREATE TABLE IF NOT EXISTS Himoku ("
                  "num INTEGER PRIMARY KEY AUTOINCREMENT, "
                  "himoku TEXT, "
                  "yomi TEXT)";
    initDB(sql);  // テーブル作成は派生クラスが決める

    QStringList headers = {"ID", "費目", "読み"};
    setTableName("Himoku",sql,headers);

}

// 必要なら派生クラスで addRow を上書き
void HimokuWidget::addRow()
{
    BaseTableWidget::addRow();  // 空行追加
    int row = model->rowCount() - 1;
    if (row >= 0) {
        model->setData(model->index(row, 1), "〇費");
        model->setData(model->index(row, 2), "〇ひ");
    }
}
