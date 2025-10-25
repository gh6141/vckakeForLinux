#include "BikoWidget.h"

BikoWidget::BikoWidget(QWidget* parent)
    : BaseTableWidget(parent)
{

    QString sql = "CREATE TABLE Biko ("
                  "num INTEGER PRIMARY KEY AUTOINCREMENT, "
                  "biko TEXT, "
                  "yomi TEXT)";
    initDB(sql);  // テーブル作成は派生クラスが決める

    QStringList headers = {"ID", "備考", "読み"};
    setTableName("Biko",sql,headers);

}

// 必要なら派生クラスで addRow を上書き
void BikoWidget::addRow()
{
    BaseTableWidget::addRow();  // 空行追加
    int row = model->rowCount() - 1;
    if (row >= 0) {
        model->setData(model->index(row, 1), "");
        model->setData(model->index(row, 2), "");
    }
}
