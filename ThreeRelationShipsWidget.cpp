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


// ThreeRelationShipsWidget.cpp に実装
bool ThreeRelationShipsWidget::checkExist(const QString& himoku, const QString& shiharai, const QString& biko)
{
    // model が QSqlTableModel の場合、その database() を利用
    QSqlQuery query(model->database());
    query.prepare("SELECT COUNT(*) FROM ThreeRelationShips WHERE himoku = :himoku AND shiharaiSakiMoto = :shiharai AND biko = :biko");
    query.bindValue(":himoku", himoku);
    query.bindValue(":shiharai", shiharai);
    query.bindValue(":biko", biko);

    if (!query.exec()) {
        qDebug() << "checkExist query failed:" << query.lastError();
        return false;
    }

    if (query.next()) {
        int count = query.value(0).toInt();
        return count > 0;  // 1件以上あれば存在する
    }

    return false;
}
