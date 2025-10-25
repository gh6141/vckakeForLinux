#include "ThreeRelationShipsWidget.h"

ThreeRelationShipsWidget::ThreeRelationShipsWidget(QWidget* parent)
    : BaseTableWidget(parent)
{

    QString sql = "CREATE TABLE ThreeRelationShips ("
                  "num INTEGER PRIMARY KEY AUTOINCREMENT, "
                  "biko TEXT, "
                  "himoku TEXT,"
                  "shiharaiSakiMoto TEXT)";
    initDB(sql);  // テーブル作成は派生クラスが決める

    QStringList headers = {"ID", "備考", "費目","支払先・元"};
    setTableName("ThreeRelationShips",sql,headers);

}

// 必要なら派生クラスで addRow を上書き
void ThreeRelationShipsWidget::addRow()
{
    BaseTableWidget::addRow();  // 空行追加
    int row = model->rowCount() - 1;
    if (row >= 0) {
        model->setData(model->index(row, 1), "");
        model->setData(model->index(row, 2), "");
    }
}
