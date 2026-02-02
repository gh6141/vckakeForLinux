#include "KozaWidget.h"

#include <QStyledItemDelegate>
#include <QHeaderView>

#include <QLineEdit>

// BaseTableWidget.cpp の中に
class ToolTipDelegate : public QStyledItemDelegate
{
public:
    ToolTipDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override
    {
        QLineEdit* editor = new QLineEdit(parent);

        switch (index.column()) {
        case 1: editor->setPlaceholderText("〇銀行、オリコ等"); break;
        case 2: editor->setPlaceholderText("インポート設定 例. 2(日),4(払),5(預),8(残),7(区分), 9(摘要）"); break;

        }
        return editor;
    }
};




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

    view->setItemDelegate(new ToolTipDelegate(view)); //ツールチップ追加

}

// 必要なら派生クラスで addRow を上書き
void KozaWidget::addRow()
{
    BaseTableWidget::addRow();  // 空行追加
    int row = model->rowCount() - 1;
    if (row >= 0) {
       // model->setData(model->index(row, 0), "");
        model->setData(model->index(row, 1), "");
        model->setData(model->index(row, 2), "");
    }
}
