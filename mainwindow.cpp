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
#include "BikoSearchDialog.h"
#include "DraggableGridWidget.h"
#include <QScrollArea>
#include "DraggableButton.h"
#include "OricoRowData.h"


QPair<QDate,QDate> getDateRangeFromUser(QWidget* parent = nullptr) {
    QDialog dialog(parent);
    dialog.setWindowTitle("日付範囲を選択");

    QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);

    // 前月1日を計算
    QDate today = QDate::currentDate();
    QDate firstOfLastMonth = QDate(today.year(), today.month(), 1).addMonths(-1);

    // From
    QHBoxLayout* fromLayout = new QHBoxLayout;
    fromLayout->addWidget(new QLabel("開始日:"));
    QDate sndOfLastMonth= QDate(today.year(), today.month(), 2).addMonths(-1);
    QDateEdit* fromEdit = new QDateEdit(sndOfLastMonth);
    fromEdit->setCalendarPopup(true);
    fromLayout->addWidget(fromEdit);
    mainLayout->addLayout(fromLayout);

    // To
    QHBoxLayout* toLayout = new QHBoxLayout;
    toLayout->addWidget(new QLabel("終了日:"));

    // 前月末日
    QDate lastOfLastMonth = firstOfLastMonth.addMonths(1).addDays(-1);

    QDateEdit* toEdit = new QDateEdit(lastOfLastMonth);
    toEdit->setCalendarPopup(true);
    toLayout->addWidget(toEdit);
    mainLayout->addLayout(toLayout);

    // OK / Cancel ボタン
    QHBoxLayout* btnLayout = new QHBoxLayout;
    QPushButton* okBtn = new QPushButton("OK");
    QPushButton* cancelBtn = new QPushButton("キャンセル");
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    mainLayout->addLayout(btnLayout);

    QObject::connect(okBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    QObject::connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    QPair<QDate,QDate> result;
    if (dialog.exec() == QDialog::Accepted) {
        result.first = fromEdit->date();
        result.second = toEdit->date();
    } else {
        result.first = QDate();
        result.second = QDate();
    }
    return result;
}





MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)

{
    ui->setupUi(this);
    table = new KakeiboTable(); // 親は addWidget() で設定されるので不要
    //ui->centralwidget->layout()->addWidget(table);
    QVBoxLayout *vbox = qobject_cast<QVBoxLayout*>(ui->centralwidget->layout());

    updateButton = new QPushButton("再表示", this);
    updateButton->setFixedWidth(100);

    // 削除ボタン作成
    deleteButton = new QPushButton("削除", this);
    deleteButton->setFixedWidth(80);
    deleteButton->setVisible(false);
    // 横レイアウト作成（右端に寄せる）
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addStretch();                  // 左側を伸ばす
    hbox->addWidget(updateButton);
    hbox->addWidget(deleteButton);       // 右端にボタン
    hbox->setAlignment(deleteButton, Qt::AlignRight);
    hbox->setAlignment(deleteButton, Qt::AlignLeft);
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

        connect(updateButton, &QPushButton::clicked, this, [this]() {

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


    comboTwoUpdate();

   // MainWindow::();

    ui->dateEdit->setDate(QDate::currentDate());
    ui->dateEdit_2->setDate(QDate::currentDate());
    ui->dateEdit_3->setDate(QDate::currentDate());



    cbh1_payee=new ComboRegisterHelper(ui->comboBox_5, ui->tab_3, "shiharaisakimoto",  QSqlDatabase::database(),this);
    cbh2_payee=new ComboRegisterHelper(ui->comboBox_3, ui->tab, "shiharaisakimoto",  QSqlDatabase::database(),this);

    cbh1_biko=new ComboRegisterHelper(ui->comboBox_7, ui->tab_3, "Biko",  QSqlDatabase::database(),this);
    cbh2_biko=new ComboRegisterHelper(ui->comboBox_4, ui->tab, "Biko",  QSqlDatabase::database(),this);
    new ComboRegisterHelper(ui->comboBox_11, ui->tab_2, "Biko",  QSqlDatabase::database(),this);

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


     m_trw = new ThreeRelationShipsWidget(this);

     ssnLtrs = loadThreeRelationList();

     ui->comboBox_6->setProperty("popularField", "himoku");
     ui->comboBox_2->setProperty("popularField", "himoku");

     ui->comboBox_5->setProperty("popularField", "payee");
     ui->comboBox_3->setProperty("popularField", "payee");

     ui->comboBox_7->setProperty("popularField", "biko");
     ui->comboBox_4->setProperty("popularField", "biko");

      pcbx=new PopularCbx(this);
      pcbx->setDataList(ssnLtrs);
      pcbx2=new PopularCbx(this);
      pcbx2->setDataList(ssnLtrs);
      comboInitializing = false;

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

    m_trw->show();
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

    bool flg= m_trw->checkExist(ui->comboBox_6->currentText(),ui->comboBox_5->currentText(),ui->comboBox_7->currentText());
    // 存在しなければボタン有効、存在すれば無効
    ui->pushButton_4->setEnabled(!flg);
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

    bool flg= m_trw->checkExist(ui->comboBox_2->currentText(),ui->comboBox_3->currentText(),ui->comboBox_4->currentText());
    // 存在しなければボタン有効、存在すれば無効
    ui->pushButton_5->setEnabled(!flg);

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


void MainWindow::on_actionsearch_triggered()
{
    BikoSearchDialog dlg(this);
    dlg.exec();
}

void MainWindow::comboTwoUpdate(){

    ShiharaisakiComboWidget shiharaisakiWidget;
    BikoComboWidget bikoWidget;

    shiharaisakiWidget.fillComboBox(ui->comboBox_5,"yomi");
    shiharaisakiWidget.fillComboBox(ui->comboBox_3,"yomi");

    bikoWidget.fillComboBox(ui->comboBox_7,"yomi");
    bikoWidget.fillComboBox(ui->comboBox_4,"yomi");
    bikoWidget.fillComboBox(ui->comboBox_11,"yomi");
}


void MainWindow::on_pushButton_5_clicked()
{
    //関連保存
    //ThreeRelationShipsWidget* trw=new ThreeRelationShipsWidget();
    m_trw->addRow(ui->comboBox_4->currentText(),ui->comboBox_2->currentText(),ui->comboBox_3->currentText());

    ui->pushButton_5->setEnabled(false);
}


void MainWindow::on_pushButton_4_clicked()
{
    //関連保存　支出
    //ThreeRelationShipsWidget* trw=new ThreeRelationShipsWidget();
    m_trw->addRow(ui->comboBox_7->currentText(),ui->comboBox_6->currentText(),ui->comboBox_5->currentText());
    ui->pushButton_4->setEnabled(false);

}

QList<ThreeRelationShip> MainWindow::loadThreeRelationList()
{
    QList<ThreeRelationShip> list;

    QSqlQuery query("SELECT himoku, shiharaiSakiMoto, biko FROM ThreeRelationShips");
    while (query.next()) {
        ThreeRelationShip t;
        t.himoku = query.value(0).toString();
        t.shiharaiSakiMoto = query.value(1).toString();
        t.biko = query.value(2).toString();
        list << t;
    }

    return list;
}



void MainWindow::on_comboBox_6_currentIndexChanged(int index)
{
    if (comboInitializing) return;  // 初期化中は無視
    pcbx->setDataList(ssnLtrs);
    QList<QComboBox*> targetCbs = { ui->comboBox_5, ui->comboBox_7 };
    pcbx->updateComboBoxes(ui->comboBox_6, targetCbs);
}


void MainWindow::on_comboBox_5_currentIndexChanged(int index)
{
    if (comboInitializing) return;
    pcbx->setDataList(ssnLtrs);
    QList<QComboBox*> targetCbs = {  ui->comboBox_7 };
    pcbx->updateComboBoxes(ui->comboBox_5, targetCbs);
}


void MainWindow::on_comboBox_2_currentIndexChanged(int index)
{
    if (comboInitializing) return;  // 初期化中は無視
    pcbx2->setDataList(ssnLtrs);
    QList<QComboBox*> targetCbs = { ui->comboBox_3, ui->comboBox_4 };
    pcbx2->updateComboBoxes(ui->comboBox_2, targetCbs);
}


void MainWindow::on_comboBox_3_currentIndexChanged(int index)
{
    if (comboInitializing) return;
    pcbx2->setDataList(ssnLtrs);
    QList<QComboBox*> targetCbs = {  ui->comboBox_4 };
    pcbx2->updateComboBoxes(ui->comboBox_3, targetCbs);
}


void MainWindow::on_checkBox_checkStateChanged(const Qt::CheckState &arg1)
{  
    pcbx->setDataList(ssnLtrs);
    pcbx->setFilterEnabled(ui->checkBox->isChecked());

    if(!ui->checkBox->isChecked()){
        cbh1_payee->setPrevent(true);   // コンボ更新の間は無効
        cbh1_biko->setPrevent(true);
          comboTwoUpdate();
        cbh1_payee->setPrevent(false);
        cbh1_biko->setPrevent(false);
    }

}


void MainWindow::on_checkBox_2_checkStateChanged(const Qt::CheckState &arg1)
{  
    pcbx2->setDataList(ssnLtrs);
    pcbx2->setFilterEnabled(ui->checkBox_2->isChecked());

    if(!ui->checkBox_2->isChecked()){
        cbh2_payee->setPrevent(true);
        cbh2_biko->setPrevent(true);
          comboTwoUpdate();
        cbh2_payee->setPrevent(false);
        cbh2_biko->setPrevent(false);
    }

}



void MainWindow::on_actionimport_triggered()
{
    auto dates = getDateRangeFromUser(this);

    if (!dates.first.isValid() || !dates.second.isValid()) {
         return;
    }

     fromDate = dates.first;
     toDate   = dates.second;

    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Orico CSV を選択",
        "",                 // 初期フォルダ
        "CSV ファイル (*.csv);;すべてのファイル (*)"
        );

    if (filePath.isEmpty()) {
        return;
    }
    int total;
    QVector<OricoRowData> oricoRows = loadOricoCSV(filePath,total);

    if (oricoRows.isEmpty()) {
        QMessageBox::warning(this, "CSV Error", filePath+":Orico CSV ファイルが存在しないか読み込めません");
        return;
    }

    QVector<KakeiboRowData> kRows = table->getAllRows(fromDate, toDate,ckozanum);

    QDialog dlg(this);
    dlg.setWindowTitle("Draggable Grid");
    dlg.resize(800, 400);
    int rows = oricoRows.length()+kRows.length()+1;
    int cols = 2;
    DraggableGridWidget *grid = new DraggableGridWidget(rows, cols);
    QScrollArea *scrollArea = new QScrollArea(&dlg);
    scrollArea->setWidget(grid);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    QVBoxLayout *vbox = new QVBoxLayout(&dlg); // ✅ dlg にレイアウトを設定
    vbox->addWidget(scrollArea);
    QPushButton* kosinBtn = new QPushButton("更新");
    vbox->addWidget(kosinBtn);

    populateOricoGrid(grid, kRows, oricoRows,total);
    dlg.exec();

}



void MainWindow::populateOricoGrid(DraggableGridWidget* grid,
                                   const QVector<KakeiboRowData>& kRows,
                                   const QVector<OricoRowData>& oricoRows,
                                   int& total
                                   )
{

    tmpOkV.resize(kRows.size() + oricoRows.size() + 1);
    grid->clear(); // 既存ボタンを消すメソッドを追加しておくと安全

    for (int kIdx = 0; kIdx < kRows.size(); ++kIdx) {
        tmpOkV[kIdx].krdata=kRows[kIdx];
    }

    for (int r = 0; r < oricoRows.size(); ++r)
    {
        const auto& o = oricoRows[r];
        auto btnOricoKingaku = new DraggableButton(QString::number(o.kingaku)+"("+o.date+"):"+o.usePlace.left(10), grid);
        grid->addButton(btnOricoKingaku, r, 1);
        tmpOkV[kRows.size()+r].ordata=o;
        tmpOkV[kRows.size()+r].obtnX=1;  //o button
        tmpOkV[kRows.size()+r].kbtnX=-1; //no apply


        // 2列目：kakeibo
        for (int kIdx = 0; kIdx < kRows.size(); ++kIdx) {
            const auto& k = kRows[kIdx];
            if (k.kingaku == o.kingaku&&!tmpOkV[kIdx].matchFlg) {
                QString text = QString::number(k.kingaku)+"("+k.date.toString("MM/dd")+"):"+ k.biko.left(6);
                auto btnKakeibo = new DraggableButton(text, grid);
                grid->addButton(btnKakeibo, r, 1);
                tmpOkV[kIdx].matchFlg=true;   //k button
                tmpOkV[kIdx].ordata=o;
                tmpOkV[kIdx].krdata=k;
                tmpOkV[kIdx].obtnX=-1;  //no apply
                tmpOkV[kIdx].kbtnX=1;  //k button

              //  一致ならボタンは左にある↓
                grid->moveButton(btnOricoKingaku->row(), btnOricoKingaku->col(), r, 0);
                tmpOkV[kRows.size()+r].obtnX=0; //o button
                tmpOkV[kRows.size()+r].kbtnX=-1; //no apply
                break;  // 最初の1件だけ
            }
        }

    }



     grid->addButton(QString::number(total), oricoRows.size(), 0);

    // 一致しなかった kRows を下にまとめて表示
    int offset = oricoRows.size()+1;
    for (int kIdx = 0; kIdx < kRows.size(); ++kIdx) {
        if (!tmpOkV[kIdx].matchFlg) {
            const auto& k = kRows[kIdx];
            QString text = QString::number(k.kingaku)+"("+k.date.toString("MM/dd")+"):"+ k.biko.left(6);
            auto btnKakeibo = new DraggableButton(text, grid);
            grid->addButton(btnKakeibo, offset, 0);
            offset++;  // 次の行にずらす

        }
    }


    //集計チェック
    int kei=0;
    QString sk=" 2日に変更:";
    QString del=" 削除必要：";
    for (int i = 0; i < kRows.size(); ++i)
    {
        if(tmpOkV[i].matchFlg){  //一致した分
            kei=kei+tmpOkV[i].krdata.kingaku;
        } else{
            if(tmpOkV[i].kbtnX==0){
                del=del+" "+QString::number(tmpOkV[i].krdata.kingaku)+":"+tmpOkV[i].krdata.biko;
            }
        }
    }

    for (int r = 0; r < oricoRows.size(); ++r)
    {
        if(tmpOkV[kRows.size()+r].obtnX==1){  //追加分
            kei=kei+tmpOkV[kRows.size()+r].ordata.kingaku;

            QDate ordDate = QDate::fromString(tmpOkV[kRows.size()+r].ordata.date, "yyyy-MM-dd");
            if(ordDate<=toDate && ordDate>=fromDate){
                tmpOkV[kRows.size()+r].krdata.date=ordDate;
            }else{ //日付変更したもの
                sk=sk+" "+QString::number(tmpOkV[kRows.size()+r].ordata.kingaku)+":"+tmpOkV[kRows.size()+r].ordata.usePlace;
                //tmpOkV[kRows.size()+r].ordata.date=fromDate.toString("yyyy-MM-dd");
                tmpOkV[kRows.size()+r].krdata.date=fromDate;  //QDate
            }
            tmpOkV[kRows.size()+r].krdata.biko=tmpOkV[kRows.size()+r].ordata.usePlace;
            tmpOkV[kRows.size()+r].krdata.kingaku=tmpOkV[kRows.size()+r].ordata.kingaku;
            table->add(tmpOkV[kRows.size()+r].krdata, true, ckozanum);  // knum は対象アカウント番号など
        }
    }


    if(total==kei){
        QMessageBox::information(this, "集計結果", fromDate.toString()+"~"+toDate.toString()+"の結果は"+QString::number(kei)+"で、一致しました！! "+sk+del);
    }else{
        QMessageBox::information(this, "集計結果", fromDate.toString()+"~"+toDate.toString()+"の結果は"+QString::number(kei)+"で一致しませんでした。"+QString::number(total)+"になるはずです。合計したいものが日付が範囲内にあるか、確認必要です。");
    }


}

void MainWindow::on_comboBox_8_currentIndexChanged(int index)
{

}


void MainWindow::on_comboBox_8_activated(int index)
{

}


void MainWindow::on_comboBox_8_currentTextChanged(const QString &arg1)
{

}


void MainWindow::on_comboBox_5_currentTextChanged(const QString &arg1)
{

}


void MainWindow::on_comboBox_6_editTextChanged(const QString &arg1)
{

}


void MainWindow::on_comboBox_6_currentTextChanged(const QString &arg1)
{

}

