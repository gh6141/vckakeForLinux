#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "KozaWidget.h"
#include "HimokuWidget.h"
#include "BikoWidget.h"
#include "ShiharaisakimotoWidget.h"
#include "ThreeRelationShipsWidget.h"
#include "kakeibotable.h"
#include "KozaComboWidget.h"
#include "HimokuComboWidget.h"
#include "ShiharaisakiComboWidget.h"
#include "BikoComboWidget.h"
#include "KakeiboRowData.h"
#include <QWidget>
#include <QShowEvent>

#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>
#include <QPushButton>
#include "koza.h"
#include "BalanceListWidget.h"
#include "PeriodSelectDialog.h"
#include "BalanceTrendWidget.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    table = new KakeiboTable(); // 親は addWidget() で設定されるので不要
    //ui->centralwidget->layout()->addWidget(table);
    QVBoxLayout *vbox = qobject_cast<QVBoxLayout*>(ui->centralwidget->layout());
    // 削除ボタン作成
    deleteButton = new QPushButton("削除", this);
    deleteButton->setFixedWidth(80);
    deleteButton->setVisible(false);
    // 横レイアウト作成（右端に寄せる）
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addStretch();                  // 左側を伸ばす
    hbox->addWidget(deleteButton);       // 右端にボタン
    hbox->setAlignment(deleteButton, Qt::AlignRight);
    hbox->setContentsMargins(0, 0, 10, 0);
    if (vbox) {
        vbox->insertWidget(0, table);  // 👈 一番上（index=0）に挿入
        deleteButton->setVisible(true);
        vbox->insertLayout(1, hbox);  // table の下に配置

        // 削除ボタンクリックで選択行削除
        connect(deleteButton, &QPushButton::clicked, this, [this]() {
            QItemSelectionModel *sel = table->selectionModel();
            if (!sel->hasSelection()) return;

            QModelIndexList selected = sel->selectedRows();
            std::sort(selected.begin(), selected.end(),
                      [](const QModelIndex &a, const QModelIndex &b) { return a.row() > b.row(); });

            const int himokuColumn = 5;  // 例: 5列目が himoku
            const int idosakiColumn = 7; // 例: 7列目が idosaki

            for (const QModelIndex &idx : selected) {
                QSqlTableModel *model = table->getmodel();

                // --- himoku と idosaki を取得 ---
                QString himoku = model->data(model->index(idx.row(), himokuColumn)).toString();
                int idosaki = model->data(model->index(idx.row(), idosakiColumn)).toInt();

                // --- idosaki 番の別テーブルを操作 ---
                QSqlDatabase db = QSqlDatabase::database(); // デフォルト接続を取得
                QSqlTableModel otherModel(nullptr, db);  // どこかで db を保持している前提
                QString otherTableName = QString("shishutunyu%1").arg(idosaki); // 例: idosaki の番号でテーブル名
                otherModel.setTable(otherTableName);
                otherModel.select();

                // himoku が一致する行を削除
                for (int r = otherModel.rowCount() - 1; r >= 0; --r) {
                    if (otherModel.data(otherModel.index(r, himokuColumn)).toString() == himoku) {
                        otherModel.removeRow(r);
                    }
                }
                otherModel.submitAll();  // 変更を確定
                // --- 元のテーブルの行を削除 ---
                model->removeRow(idx.row());
            }
            table->getmodel()->submitAll(); // 元テーブルの変更確定
            table->loadTable(ckozanum);


        });
    }


    KozaComboWidget kozaWidget;
    kozaWidget.fillComboBox(ui->comboBox_8);
    kozaWidget.fillComboBox(ui->comboBox);
    kozaWidget.fillComboBox(ui->comboBox_12);
    kozaWidget.fillComboBox(ui->comboBox_13);

    HimokuComboWidget himokuWidget;
    himokuWidget.fillComboBox(ui->comboBox_6,"yomi");
    himokuWidget.fillComboBox(ui->comboBox_2,"yomi");


    ShiharaisakiComboWidget shiharaisakiWidget;
    shiharaisakiWidget.fillComboBox(ui->comboBox_5,"yomi");
    shiharaisakiWidget.fillComboBox(ui->comboBox_3,"yomi");

    BikoComboWidget bikoWidget;
    bikoWidget.fillComboBox(ui->comboBox_7,"yomi");
    bikoWidget.fillComboBox(ui->comboBox_4,"yomi");
    bikoWidget.fillComboBox(ui->comboBox_11,"yomi");

    ui->dateEdit->setDate(QDate::currentDate());
    ui->dateEdit_2->setDate(QDate::currentDate());
    ui->dateEdit_3->setDate(QDate::currentDate());


    ckozanum=1;
    dst_ckozanum=1;

    // ComboBox 選択が変わったらテーブルを切り替える
    // ３つのコンボボックスを QList にまとめる
    QList<QComboBox*> combos = { ui->comboBox_8, ui->comboBox, ui->comboBox_12 };

    // 同期処理ラムダ
    auto syncComboBoxes = [=](int index, QComboBox* senderCombo){
        if (index < 0) return;
        int value = senderCombo->itemData(index).toInt();
        ckozanum = value;

        // 他のコンボボックスの選択を合わせる
        for (auto combo : combos) {
            if (combo != senderCombo) {
                // 無限ループ防止のためシグナルブロック
                QSignalBlocker blocker(combo);
                combo->setCurrentIndex(index);
            }
        }

        table->loadTable(ckozanum);
    };

    // ３つとも同じラムダで接続
    for (auto combo : combos) {
        connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, [=](int index){ syncComboBoxes(index, combo); });
    }


    connect(ui->comboBox_13, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, [=](int index){
       dst_ckozanum=ui->comboBox_13->itemData(index).toInt();

      });




    // 「追加」ボタンを押したら行追加

    connect(ui->pushButton_2, &QPushButton::clicked, this, [=](){


    });

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionkozaEdit_triggered()
{
    KozaWidget *w = new KozaWidget(this);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->show();
}


void MainWindow::on_actionactionhimokuEdit_triggered()
{
    HimokuWidget *w = new HimokuWidget(this);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->show();
}


void MainWindow::on_actionactionbikoEdit_triggered()
{
    BikoWidget *w = new BikoWidget(this);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->show();
}


void MainWindow::on_actionactionshiharaisaki_triggered()
{
    ShiharaisakimotoWidget *w = new ShiharaisakimotoWidget(this);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->show();
}


void MainWindow::on_actionaction3rsEdit_triggered()
{
    ThreeRelationShipsWidget *w = new ThreeRelationShipsWidget(this);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->show();
}






void MainWindow::on_actiondbSet_triggered()
{
    QSettings settings("MyCompany", "QtKakeibo");  // ① 設定ファイル（自動で作成される）

    QString currentPath = settings.value("Database/Path", "qtkake.db").toString();

    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("データベースの保存場所を選択"),
        currentPath,
        tr("SQLite Database (*.db);;All Files (*.*)")
        );

    if (fileName.isEmpty())
        return; // キャンセル時

    // ② 選択されたパスを保存
    settings.setValue("Database/Path", fileName);

    QMessageBox::information(this, tr("設定完了"),
                             tr("データベースの保存場所を設定しました:\n%1").arg(fileName));
}


QString MainWindow::getDatabasePath()
{
    QSettings settings("MyCompany", "QtKakeibo");
    // ③ 設定値がなければ "qtkake.db" を返す
    return settings.value("Database/Path", "qtkake.db").toString();
}


void MainWindow::on_tabWidget_currentChanged(int index)
{

}


void MainWindow::on_pushButton_2_clicked()
{
    KakeiboRowData data;
    data.date = ui->dateEdit_2->date();
    data.kingaku = ui->lineEdit->text().toInt();
    data.himoku = ui->comboBox_6->currentText();
    data.shiharaisaki = ui->comboBox_5->currentText();
    data.biko = ui->comboBox_7->currentText();
    table->addRowForCurrentAccount(data,true,ckozanum);//true=sishutu false=shunyu
        table->loadTable(ckozanum);
}


void MainWindow::on_pushButton_clicked()
{
    KakeiboRowData data;
    data.date = ui->dateEdit->date();
    data.kingaku = ui->lineEdit_2->text().toInt();
    data.himoku = ui->comboBox_2->currentText();
    data.shiharaisaki = ui->comboBox_3->currentText();
    data.biko = ui->comboBox_4->currentText();
    table->addRowForCurrentAccount(data,false,ckozanum);//true=sishutu false=shunyu
        table->loadTable(ckozanum);
}


void MainWindow::on_actionexit_triggered()
{
    QApplication::quit();
}





void MainWindow::on_pushButton_3_clicked()
{
    if(ckozanum==dst_ckozanum){
        QMessageBox::warning(this, "口座の一致", "送り先と元が同じになっています");
        return;
    }

    KakeiboRowData data;
    data.date = ui->dateEdit_3->date();
    int kg=ui->lineEdit_3->text().toInt();
    data.kingaku = kg;
    QString cid=QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
    data.himoku = cid;
    data.shiharaisaki = "移動先:"+ui->comboBox_13->currentText();
    data.biko = ui->comboBox_11->currentText();
    data.idosaki=dst_ckozanum;
    table->addRowForCurrentAccount(data,true,ckozanum);//true=sishutu false=shunyu
  //  table->loadTable(ckozanum);

 // QMessageBox::warning(this, "次の処理", "送り先の処理をします");
    table->loadTable(dst_ckozanum);
   //QMessageBox::warning(this, "次の処理", "送り先の処理をします2");
    KakeiboRowData data2;
    data2.date = ui->dateEdit_3->date();
    data2.kingaku = kg;
    data2.himoku =cid;
    data2.shiharaisaki = "移動元:"+ui->comboBox_12->currentText();
    data2.biko = ui->comboBox_11->currentText();
    data2.idosaki=ckozanum;


   table->addRowForCurrentAccount(data2,false,dst_ckozanum);//true=sishutu false=shunyu
  //  QMessageBox::warning(this, "次の処理", "送り先の処理をします3");
    table->loadTable(ckozanum);

}


void MainWindow::on_actionzandakaList_triggered()
{

    BalanceListWidget *blw = new BalanceListWidget(); // 親は nullptr にする
    blw->setAttribute(Qt::WA_DeleteOnClose);
    blw->resize(400, 400);
    blw->show();



}



QList<int> MainWindow::getAccountList()
{
    QList<int> accountList;
    QSqlDatabase db = QSqlDatabase::database(); // デフォルト接続

    QStringList tables = db.tables(); // DB 内の全テーブル名を取得
    QRegularExpression re("^shishutunyu(\\d+)$"); // shishutunyu + 数字 のパターン

    for (const QString &t : tables) {
        QRegularExpressionMatch match = re.match(t);
        if (match.hasMatch()) {
            int accountNum = match.captured(1).toInt(); // 数字部分だけ取得
            accountList.append(accountNum);
        }
    }

    std::sort(accountList.begin(), accountList.end()); // 必要なら番号順にソート
    return accountList;
}



void MainWindow::on_actionsuii_triggered()
{
    PeriodSelectDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        // ここで集計処理へ
        dlg.disp();
    }else{
        return;
    }

    QDate start = dlg.startDate();
    QDate end   = dlg.endDate();

    // 推移ウィジェット表示
    BalanceTrendWidget *trend = new BalanceTrendWidget(start, end, this);
    trend->setAttribute(Qt::WA_DeleteOnClose);
    trend->show();
    trend->chart();


}

