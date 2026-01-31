// BaseTableWidget.cpp
#include "BaseTableWidget.h"
#include "mainwindow.h"
#include <QStyledItemDelegate>
#include <QHeaderView>

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




BaseTableWidget::BaseTableWidget(QWidget *parent,QString createSQL)
    : QMainWindow(parent),
    model(nullptr),
    view(new QTableView(this))
{
    initUI();


}

void BaseTableWidget::initUI()
{
    QWidget* central = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(central);

    QPushButton* btnAdd = new QPushButton("追加", central);
    QPushButton* btnDelete = new QPushButton("削除", central);

    layout->addWidget(view);
    layout->addWidget(btnAdd);
    layout->addWidget(btnDelete);

    setCentralWidget(central);

    connect(btnAdd, &QPushButton::clicked, this, &BaseTableWidget::addRow);
    connect(btnDelete, &QPushButton::clicked, this, &BaseTableWidget::deleteRow);
}

void BaseTableWidget::setTableName(const QString &name,const QString createSQL,const QStringList& headers)
{
    tableName = name;
    initDB(createSQL);

    model = new QSqlTableModel(this, QSqlDatabase::database());
    model->setTable(tableName);
    model->setEditStrategy(QSqlTableModel::OnFieldChange);
    model->select();
    view->setModel(model);
    // num 列を非表示にする
  //  view->hideColumn(0); // 0 は num がモデルの最初の列の場合

    view->setMinimumWidth(600); // 表の最小幅
    view->setColumnWidth(1, 200); // 1列目の幅
    view->setColumnWidth(2, 400); // 2列目の幅
  //  view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//表全体の幅に合わせて列が均等に伸びる


    for (int i = 0; i < headers.size(); ++i) {
       model->setHeaderData(i, Qt::Horizontal, headers[i]);
    }

    view->setItemDelegate(new ToolTipDelegate(view)); //ツールチップ追加
}

void BaseTableWidget::initDB(const QString createSQL)
{
   // QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "mainConnection");
   // db.setDatabaseName(MainWindow::getDatabasePath());
QString connectionName = "mainConnection";
    QSqlDatabase db;
    if (QSqlDatabase::contains(connectionName)) {
        db = QSqlDatabase::database(connectionName);
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        db.setDatabaseName("mydatabase.db");
    }



    if (!db.open()) {
       // qDebug() << "DB接続失敗";
        return;
    }
        QSqlQuery query;
        if (!createSQL.isEmpty()) {
            if (!query.exec(createSQL))
                qDebug() << "テーブル作成失敗:" << query.lastError().text();
            //else
             //   qDebug() << "新規DBとテーブル作成完了";
        }
}


void BaseTableWidget::addRow()
{
    if (model)
        model->insertRow(model->rowCount()); // 空行追加
}

void BaseTableWidget::deleteRow()
{
    if (model) {
        int row = view->currentIndex().row();
        if (row >= 0) {
            model->removeRow(row);
            model->submitAll();
        }
    }
}

