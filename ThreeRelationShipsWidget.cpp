#include "ThreeRelationShipsWidget.h"

ThreeRelationShipsWidget::ThreeRelationShipsWidget(QWidget* parent)
    : BaseTableWidget(parent)
{



    QString sql = "CREATE TABLE IF NOT EXISTS ThreeRelationShips ("
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
         model->setData(model->index(row, 3), "");
    }
}

void ThreeRelationShipsWidget::addRow(const QString &biko, const QString &himoku, const QString &shiharai)
{
    BaseTableWidget::addRow();
    int row = model->rowCount() - 1;
    if (row >= 0) {
        model->setData(model->index(row, 1), biko);
        model->setData(model->index(row, 2), himoku);
        model->setData(model->index(row, 3), shiharai);
    }
 model->submitAll();  // ★ これが必要！！

}
