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
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVector>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QCloseEvent>
#include <QTimer>


QPair<QDate,QDate> getDateRangeFromUser(QWidget* parent = nullptr,QDate fromDate=QDate(),QDate toDate=QDate()) {
    QDialog dialog(parent);
    dialog.setWindowTitle("日付範囲を選択");
    QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);
    QDateEdit* fromEdit = new QDateEdit(fromDate);
    fromEdit->setCalendarPopup(true);
    QHBoxLayout* fromLayout = new QHBoxLayout;
    fromLayout->addWidget(new QLabel("開始日:"));
    fromLayout->addWidget(fromEdit);
    mainLayout->addLayout(fromLayout);
    QHBoxLayout* toLayout = new QHBoxLayout;
    toLayout->addWidget(new QLabel("終了日:"));

    QDateEdit* toEdit = new QDateEdit(toDate);
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

    // MainWindow コンストラクタなど
    balanceLabel = new QLabel("残高 0円", this);
    statusBar()->addPermanentWidget(balanceLabel);

    table = new KakeiboTable(); // 親は addWidget() で設定されるので不要
    QVBoxLayout *vbox = qobject_cast<QVBoxLayout*>(ui->centralwidget->layout());

    updateButton = new QPushButton("再計算", this);
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

        connect(deleteButton, &QPushButton::clicked, this, [this]() {
            QSortFilterProxyModel* proxy = table->getProxyModel();
            QSqlTableModel* model = table->getmodel();
            QItemSelectionModel *sel = table->selectionModel();
            if (!proxy || !model || !sel->hasSelection()) return;
            if (proxy->sourceModel() != model) {
                QMessageBox::warning(this, "削除エラー",
                                     "表示中のデータと削除対象が一致しません。再表示してからお試しください。");
                return;
            }

            QModelIndexList selectedProxy = sel->selectedRows();

            struct SelectedRecord {
                int num;
                QString himoku;
                int idosaki;
            };
            QVector<SelectedRecord> selectedRecords;
            selectedRecords.reserve(selectedProxy.size());
            for (const QModelIndex &proxyIndex : selectedProxy) {
                const QModelIndex sourceIndex = proxy->mapToSource(proxyIndex);
                if (!sourceIndex.isValid()) continue;
                SelectedRecord record;
                record.num = model->data(model->index(sourceIndex.row(), 0)).toInt();
                record.himoku = model->data(model->index(sourceIndex.row(), 5)).toString();
                record.idosaki = model->data(model->index(sourceIndex.row(), 7)).toInt();
                selectedRecords.append(record);
            }
            if (selectedRecords.isEmpty()) return;

            QSqlDatabase db = model->database();
            if (!db.transaction()) {
                QMessageBox::warning(this, "削除エラー",
                                     "削除を開始できませんでした: " + db.lastError().text());
                return;
            }

            const auto rollbackAndRefresh = [&]() {
                db.rollback();
                if (!model->select())
                    qWarning() << "Failed to refresh model after delete rollback:"
                               << model->lastError().text();
            };

            const QString currentTable = model->tableName();
            QSqlQuery deleteCurrentQuery(db);
            if (!deleteCurrentQuery.prepare(
                    QString("DELETE FROM %1 WHERE num = :num").arg(currentTable))) {
                const QString error = deleteCurrentQuery.lastError().text();
                rollbackAndRefresh();
                QMessageBox::warning(this, "削除エラー",
                                     "削除クエリを準備できませんでした: " + error);
                return;
            }

            for (const SelectedRecord &record : selectedRecords) {
                const QString &himoku = record.himoku;
                bool isNumber = !himoku.isEmpty() && record.idosaki > 0;
                for (auto c : himoku) {
                    if (!c.isDigit()) { isNumber = false; break; }
                }
                if (isNumber) {
                    // 移動データの場合、idosaki を使って他テーブルも削除
                    // 移動データの場合、idosaki を使って他テーブルも削除
                    const QString otherTableName =
                        QString("shishutunyu%1").arg(record.idosaki);

                    QSqlQuery query(db);

                    // SQLで直接削除
                    query.prepare(QString("DELETE FROM %1 WHERE himoku = :himoku").arg(otherTableName));
                    query.bindValue(":himoku", himoku);

                    if (!query.exec()) {
                        const QString error = query.lastError().text();
                        rollbackAndRefresh();
                        QMessageBox::warning(
                            this, "削除エラー",
                            QString("%1 の関連データを削除できませんでした: %2")
                                .arg(otherTableName, error));
                        return;
                    }
                }

                deleteCurrentQuery.bindValue(":num", record.num);
                if (!deleteCurrentQuery.exec() ||
                    deleteCurrentQuery.numRowsAffected() != 1) {
                    const QString error = deleteCurrentQuery.lastError().text().isEmpty()
                                              ? QString("番号 %1 のデータが見つかりません")
                                                    .arg(record.num)
                                              : deleteCurrentQuery.lastError().text();
                    rollbackAndRefresh();
                    QMessageBox::warning(this, "削除エラー",
                                         "選択したデータを削除できませんでした: " + error);
                    return;
                }
            }

            if (!db.commit()) {
                const QString error = db.lastError().text();
                rollbackAndRefresh();
                QMessageBox::warning(this, "削除エラー",
                                     "削除を確定できませんでした: " + error);
                return;
            }

            gotoLast();
        });


        connect(updateButton, &QPushButton::clicked, this, [this]() {
            gotoLast();
        });
        connect(table, &KakeiboTable::balanceChanged,
                this, &MainWindow::updateBalance);


        //編集後、残高再計算スクロール
        connect(table->getModel(), &QAbstractItemModel::dataChanged,
                this, [this](const QModelIndex&, const QModelIndex&, const QVector<int>&){
                    // 少し遅らせるのがコツ（編集中の確定待ち）
                    QTimer::singleShot(200, this, [this](){
                        gotoLast();
                       // recalcAndGoLastSafe();
                    });
                });
        //追加後（インポートなど）、残高再計算スクロール
        connect(table->getModel(), &QAbstractItemModel::rowsInserted,
                this, [this](const QModelIndex&, int, int){
                    QTimer::singleShot(200, this, [this](){
                        gotoLast();
              //  recalcAndGoLastSafe();
                    });
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

    ui->dateEdit->setDate(QDate::currentDate());
    ui->dateEdit_2->setDate(QDate::currentDate());
    ui->dateEdit_3->setDate(QDate::currentDate());

    cbh1_payee=new ComboRegisterHelper(ui->comboBox_5,ui->cb5Area,ui->hLayoutCb5, "shiharaisakimoto",  QSqlDatabase::database(),this);
    cbh2_payee=new ComboRegisterHelper(ui->comboBox_3,ui->cb3Area,  ui->hLayoutCb3, "shiharaisakimoto",  QSqlDatabase::database(),this);

    cbh1_biko=new ComboRegisterHelper(ui->comboBox_7,ui->cb7Area,ui->hLayoutCb7, "Biko",  QSqlDatabase::database(),this);
    cbh2_biko=new ComboRegisterHelper(ui->comboBox_4,ui->cb4Area,ui->hLayoutCb4, "Biko",  QSqlDatabase::database(),this);
    new ComboRegisterHelper(ui->comboBox_11, ui->cb11Area,ui->hLayoutCb11, "Biko",  QSqlDatabase::database(),this);

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

     manager = new QNetworkAccessManager(this);

      m_backupManager = new BackupManager("backups", 10, this);
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

    table->addRowForCurrentAccount(data, true, ckozanum);
    table->loadTable(ckozanum);   // ここで model / proxy へ反映

    gotoLast();

    bool flg= m_trw->checkExist(ui->comboBox_6->currentText(),
                                 ui->comboBox_5->currentText(),
                                 ui->comboBox_7->currentText());
    ui->pushButton_4->setEnabled(!flg);
    ui->pushButton_4->setStyleSheet("background-color: red;");
}


void MainWindow::gotoLast()
{
            table->loadTable(ckozanum);
    QSqlTableModel *baseModel = table->getModel();
    QSortFilterProxyModel *proxy = table->getProxyModel();

    if (!baseModel || !proxy) {
        return;
    }
    // ★★★ ここが重要：常に「現在の sourceModel」を使う ★★★
    if (proxy->sourceModel() != baseModel) {

        return;   // 安全側に倒す
    }

    // ---- ここから下は今までどおり ----
    if (baseModel) {
        baseModel->submitAll();

        table->recalculateBalances(ckozanum);
        baseModel->select();
    }

    int lastRow = baseModel->rowCount() - 1;
    if (lastRow >= 0) {

        QModelIndex sourceIndex = baseModel->index(lastRow, 0);

        // ★★★ 追加チェック（超重要）★★★
        if (!sourceIndex.isValid())
            return;

        QModelIndex proxyIndex = proxy->mapFromSource(sourceIndex);
        if (proxyIndex.isValid()) {
            table->getView()->scrollTo(proxyIndex,
                                       QAbstractItemView::PositionAtBottom);
        }
    }

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

    gotoLast();

    bool flg= m_trw->checkExist(ui->comboBox_2->currentText(),ui->comboBox_3->currentText(),ui->comboBox_4->currentText());
    // 存在しなければボタン有効、存在すれば無効
    ui->pushButton_5->setEnabled(!flg);
    ui->pushButton_5->setStyleSheet("background-color: red;");

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
    table->loadTable(dst_ckozanum);

    KakeiboRowData data2;
    data2.date = ui->dateEdit_3->date();
    data2.kingaku = kg;
    data2.himoku =cid;
    data2.shiharaisaki = "移動元:"+ui->comboBox_12->currentText();
    data2.biko = ui->comboBox_11->currentText();
    data2.idosaki=ckozanum;
   table->addRowForCurrentAccount(data2,false,dst_ckozanum);//true=sishutu false=shunyu
   table->loadTable(ckozanum);
   gotoLast();
}


void MainWindow::updateBalance()
{
    QDate today = QDate::currentDate();
    int balance = static_cast<int>(BalanceListWidget::calculateBalance(ckozanum, today));
    balanceLabel->setText(QString("残高 %1円").arg(balance));
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

    ui->pushButton_5->setStyleSheet("background-color: white;");
    ui->pushButton_5->setEnabled(false);

}


void MainWindow::on_pushButton_4_clicked()
{
    //関連保存　支出
    //ThreeRelationShipsWidget* trw=new ThreeRelationShipsWidget();
    m_trw->addRow(ui->comboBox_7->currentText(),ui->comboBox_6->currentText(),ui->comboBox_5->currentText());

    ui->pushButton_4->setStyleSheet("background-color: white;");
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
    QSettings settings("MyCompany", "QtKakeibo");
    QString dbPath = settings.value("Database/Path").toString();
    QString kname=koza::kozaNameFromNum(dbPath,ckozanum);
    if(!kname.contains("オリコ")){
            QMessageBox::warning(
                this,
                "口座選択の確認","注意！！口座選択が"+
                    QString::number(ckozanum)+":"+kname+" になっています！！オリコを選択してください。"
                );
        return;
        }

    // 前月1日を計算
    QDate today = QDate::currentDate();
    QDate firstOfLastMonth = QDate(today.year(), today.month(), 1).addMonths(-1);
    // From

    QDate sndOfLastMonth= QDate(today.year(), today.month(), 2).addMonths(-1);

    // 前月末日
    QDate lastOfLastMonth = firstOfLastMonth.addMonths(1).addDays(-1);


    auto dates = getDateRangeFromUser(this,sndOfLastMonth,lastOfLastMonth);

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
    QVector<OricoRowData> oricoRows = loadOricoCSV(filePath,total);//totalはcsvにある合計請求額

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
    // 上部ラベル部分
    auto *labelLayout = new QHBoxLayout;
    QLabel *label1=new QLabel("削除（左列）");
    label1->setAlignment(Qt::AlignCenter);
    labelLayout->addWidget(label1);
    QLabel *label2=new QLabel("適用（右列）");
    label2->setAlignment(Qt::AlignCenter);
    labelLayout->addWidget(label2);

    // ラベル + grid をまとめるコンテナ
    auto *container = new QWidget;
    auto *containerLayout = new QVBoxLayout(container);
    containerLayout->addLayout(labelLayout);
    containerLayout->addWidget(grid);

    // ScrollArea に container をセット
    scrollArea->setWidget(container);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    QVBoxLayout *vbox = new QVBoxLayout(&dlg); // ✅ dlg にレイアウトを設定
    vbox->addWidget(scrollArea);
    QPushButton* kosinBtn = new QPushButton("更新");
    vbox->addWidget(kosinBtn);


    QString rep=populateOricoGrid(grid, kRows, oricoRows,total);
    kosinBtn->setToolTip(rep);


    connect(kosinBtn, &QPushButton::clicked,
           this,
           [this, grid, kRows, oricoRows, &total,rep]() {
               onKosinClicked(grid, kRows, oricoRows, total,rep);
           });

    dlg.exec();

}



QString MainWindow::populateOricoGrid(DraggableGridWidget* grid,
                                   const QVector<KakeiboRowData>& kRows,
                                   const QVector<OricoRowData>& oricoRows,
                                   int& total)
{
    grid->clear(); // 既存ボタンを消す

    // まず右列にOricoボタンを配置
    for (int r = 0; r < oricoRows.size(); ++r) {
        const auto& o = oricoRows[r];
        auto btnOrico = new DraggableButton(
            QString::number(o.kingaku) + "(" + o.date.toString("MM/dd") + "):" + o.usePlace.left(10),
            grid
            );

        btnOrico->setToolTip(
            QString("%1\n%2\n%3")
                .arg("金額: " + QString::number(o.kingaku))
                .arg("日付: " + o.date.toString("yyyy/MM/dd"))
                .arg("店名: " + o.usePlace)
            );


        btnOrico->setOricoData(o);
        btnOrico->matchFlg = false;
        btnOrico->obtnX=1;
        btnOrico->kbtnX=-1;
        grid->addButton(btnOrico, r, 1); // 右列

        tmpDtForMap tmp;
        tmp.dtKind=1;
        //QDate date;int kingaku=0; QString himoku; QString shiharaisaki; QString biko;int idosaki; //koza number
        tmp.krdata=KakeiboRowData{ 1,o.date, o.kingaku, "","",o.usePlace,0};//id=1だが、新規だけなので使わない
        tmp.number=r;
        tmp.x=1;
        tmp.y=r;
        cButtonL.append(tmp);
    }

    // 下に未一致の家計簿ボタンを配置（左列）
    int offset = oricoRows.size() + 1;
    for (int kIdx = 0; kIdx < kRows.size(); ++kIdx) {
        const auto& k = kRows[kIdx];
        auto btnKakeibo = new DraggableButton(
            QString::number(k.kingaku) + "(" + k.date.toString("MM/dd") + "):" + k.biko.left(6),
            grid
            );

        btnKakeibo->setToolTip(
            QString("%1\n%2\n%3")
                .arg("金額: " + QString::number(k.kingaku))
                .arg("日付: " + k.date.toString("yyyy/MM/dd"))
                .arg("備考: " + k.biko)
            );

        btnKakeibo->setKakeiboData(k);
        btnKakeibo->matchFlg = false;
        btnKakeibo->obtnX=-1;
        btnKakeibo->kbtnX=0;
        grid->addButton(btnKakeibo, offset + kIdx, 0); // 左列

        tmpDtForMap tmp;
        tmp.dtKind=0;
        //QDate date;int kingaku=0; QString himoku; QString shiharaisaki; QString biko;int idosaki; //koza number
        tmp.krdata=k;
        tmp.number=kIdx;
        tmp.x=0;
        tmp.y=offset + kIdx;
        cButtonL.append(tmp);

    }

    // 二重ループで一致判定
    for (int r = 0; r < grid->rowCount(); ++r) {
        auto left = grid->buttonAt(r, 0);
        if (!left) continue;

        for (int c = 0; c < grid->rowCount(); ++c) {
            auto right = grid->buttonAt(c, 1);
            if (!right) continue;

            // 両方まだ未一致かつ金額が一致
            if (!left->matchFlg && !right->matchFlg &&
                left->kakeiboData().kingaku == right->oricoData().kingaku)
            {
                left->setOricoDataKingaku(left->kakeiboData().kingaku);  //なくてもいいが冗長性もたせた
                right->setKakeiboDataKingaku(right->oricoData().kingaku);  //〃

                updateTmpL(right->row(), right->col(), c, 0);
                updateTmpL(left->row(), left->col(), c, 1);

                //Oricoを左列に 家計簿を上右に　left_row:r   right_row:c
                grid->moveButton(right->row(), right->col(), c, 0);
                grid->moveButton(left->row(), left->col(), c, 1);

                left->obtnX=-1 ; //oricoでないとき-1
                left->kbtnX=1 ; //kakeibo なら 1か0
                right->obtnX= 0 ;
                right->kbtnX= -1 ;

                left->matchFlg = true;
                right->matchFlg = true;


                break; // 一致したら右ボタンは次の家計簿には使わない
            }
        }
    }

    // 集計やDB反映も右列ボタン優先、左列ボタンは削除対象など
    int kei = 0;
    int addCount=0;
    int matchCount=0;
    int allCount=0;

    QString del = "";

    for (int r = 0; r < grid->rowCount(); ++r) {

        auto left = grid->buttonAt(r, 0);
        auto right = grid->buttonAt(r, 1);

        if (right ) {
            if(right->obtnX==1){
                kei += right->oricoData().kingaku;

                addCount++;
                     allCount++;
            }else if(right->kbtnX==1)   {
                kei += right->kakeiboData().kingaku;

                 matchCount++;
                     allCount++;
            }

        }else if(left){
            if(left->obtnX==1){
                kei += left->oricoData().kingaku;

                     allCount++;
            } else if(left->kbtnX==1){
                kei += left->kakeiboData().kingaku;

     allCount++;
            }
        }

        if (left && !left->matchFlg && left->kakeiboData().kingaku > 0) {
            del += " " + QString::number(left->kakeiboData().kingaku) + ":" + left->kakeiboData().biko;
             allCount++;
        }
    }

    QString rep="";
    // 最終的なメッセージ
    if(!del.isEmpty()){
            rep=rep+del+"の日付を範囲外（後で削除必要）にします。\n";
    }

    if (total == kei&&addCount>0) {
       rep=rep+QString::number(matchCount)+"件が一致  右データの"+QString::number(addCount)+"件を追加（日付変更する場合あり）します。\nこの操作で 合計"+ QString::number(kei) + "に一致します！　" ;
    }else if(total == kei&&addCount==0&&allCount!=matchCount){
       rep=rep+QString::number(matchCount)+"件が一致　合計"+ QString::number(kei) + "に一致します！　";
    }else if(total == kei&&addCount==0&&allCount==matchCount){
        rep=rep+QString::number(matchCount)+"件がすべて一致(更新不要)　合計"+ QString::number(kei) + "に一致します！　";

    }else    {
       rep=rep+ "注意！！ここでの集計は"+ QString::number(kei) + "で、Orico請求の" + QString::number(total)+"と一致しません。CSVデータが正しいかチェック必要です。";
    }

/*
    for (int r=0; r<grid->rowCount(); ++r){
        auto left = grid->buttonAt(r,0);
        auto right = grid->buttonAt(r,1);
        qDebug() << "Row" << r << "Left:" << (left?left->text():"null") << "Right:" << (right?right->text():"null");
    }
*/
    return rep;
}

QString MainWindow::populateBankGrid(DraggableGridWidget* grid,
                                      const QVector<KakeiboRowData>& kRows,
                                      const QVector<importRecord>& records,
                                      int& total)
{
    grid->clear(); // 既存ボタンを消す

    int balBeforeFromDate = 0;
    QDate dateBeforeFromDate = fromDate.addDays(-1);
     balBeforeFromDate =
        BalanceListWidget::calculateBalance(ckozanum, dateBeforeFromDate);


    // まず右列にImportボタンを配置
    for (int r = 0; r < records.size(); ++r) {
        const auto& o = records[r];
        auto btnBank = new DraggableButton(
            "金額:"+QString::number(o.payment-o.deposit) + "(" + o.date.toString("yy/MM/dd") + "):" + o.summary.left(10),
            grid
            );

        btnBank->setToolTip(
            QString("%1\n%2\n%3")
                .arg("金額: " + QString::number(o.payment-o.deposit))
                .arg("日付: " + o.date.toString("yyyy/MM/dd"))
                .arg("適用: " + o.summary)
            );


        btnBank->setBankData(o);
        btnBank->matchFlg = false;
        btnBank->obtnX=1;
        btnBank->kbtnX=-1;
        grid->addButton(btnBank, r, 1); // 右列

        tmpDtForMap tmp;
        tmp.dtKind=1;
        //QDate date;int kingaku=0; QString himoku; QString shiharaisaki; QString biko;int idosaki; //koza number
        tmp.krdata=KakeiboRowData{ 1,o.date, o.payment-o.deposit, "","",o.summary,0};//id=1だが、新規だけなので使わない
        tmp.number=r;
        tmp.x=1;
        tmp.y=r;
        cButtonL.append(tmp);
    }

    // 下に未一致の家計簿ボタンを配置（左列）
    int offset = records.size() + 1;
    for (int kIdx = 0; kIdx < kRows.size(); ++kIdx) {
        const auto& k = kRows[kIdx];
        auto btnKakeibo = new DraggableButton(
            QString::number(k.kingaku) + "(" + k.date.toString("yy/MM/dd") + "):" + k.biko.left(6),
            grid
            );

        btnKakeibo->setToolTip(
            QString("%1\n%2\n%3")
                .arg("金額: " + QString::number(k.kingaku))
                .arg("日付: " + k.date.toString("yyyy/MM/dd"))
                .arg("備考: " + k.biko)
            );

        btnKakeibo->setKakeiboData(k);
        btnKakeibo->matchFlg = false;
        btnKakeibo->obtnX=-1;
        btnKakeibo->kbtnX=0;
        grid->addButton(btnKakeibo, offset + kIdx, 0); // 左列

        tmpDtForMap tmp;
        tmp.dtKind=0;
        //QDate date;int kingaku=0; QString himoku; QString shiharaisaki; QString biko;int idosaki; //koza number
        tmp.krdata=k;
        tmp.number=kIdx;
        tmp.x=0;
        tmp.y=offset + kIdx;
        cButtonL.append(tmp);

    }

    // 二重ループで一致判定
    for (int r = 0; r < grid->rowCount(); ++r) {
        auto left = grid->buttonAt(r, 0);
        if (!left) continue;

        for (int c = 0; c < grid->rowCount(); ++c) {
            auto right = grid->buttonAt(c, 1);
            if (!right) continue;

            // 両方まだ未一致かつ金額が一致
            if (!left->matchFlg && !right->matchFlg &&
                left->kakeiboData().kingaku == right->bankData().payment-right->bankData().deposit)
            {
                left->setOricoDataKingaku(left->kakeiboData().kingaku);  //なくてもいいが冗長性もたせた
                right->setKakeiboDataKingaku(right->bankData().payment-right->bankData().deposit);  //〃

                updateTmpL(right->row(), right->col(), c, 0);
                updateTmpL(left->row(), left->col(), c, 1);

                //Importを左列に 家計簿を上右に　left_row:r   right_row:c
                grid->moveButton(right->row(), right->col(), c, 0);
                grid->moveButton(left->row(), left->col(), c, 1);

                left->obtnX=-1 ; //bankでないとき-1
                left->kbtnX=1 ; //kakeibo なら 1か0
                right->obtnX= 0 ;
                right->kbtnX= -1 ;

                left->matchFlg = true;
                right->matchFlg = true;


                break; // 一致したら右ボタンは次の家計簿には使わない
            }
        }
    }

    // 集計やDB反映も右列ボタン優先、左列ボタンは削除対象など
    int kei = 0;
    int addCount=0;
    int matchCount=0;
    int allCount=0;

    QString del = "";

    for (int r = 0; r < grid->rowCount(); ++r) {

        auto left = grid->buttonAt(r, 0);
        auto right = grid->buttonAt(r, 1);

        if (right ) {
            if(right->obtnX==1){
                kei += right->bankData().payment - right->bankData().deposit;

                addCount++;
                allCount++;
            }else if(right->kbtnX==1)   {
                kei += right->kakeiboData().kingaku;

                matchCount++;
                allCount++;
            }

        }else if(left){
            if(left->obtnX==1){
                kei += left->bankData().payment - left->bankData().deposit;

                allCount++;
            } else if(left->kbtnX==1){
                kei += left->kakeiboData().kingaku;

                allCount++;
            }
        }

        if (left && !left->matchFlg && left->kakeiboData().kingaku > 0) {
            del += " " + QString::number(left->kakeiboData().kingaku) + ":" + left->kakeiboData().biko;
            allCount++;
        }
    }

    QString rep="";
    // 最終的なメッセージ
    if(!del.isEmpty()){
        rep=rep+del+"の日付を範囲外（後で削除必要）にします。\n";
    }

    if (total == -kei+balBeforeFromDate&&addCount>0) {
        rep=rep+QString::number(matchCount)+"件が一致  右データの"+QString::number(addCount)+"件を追加（日付変更する場合あり）します。\nこの操作で 最終残高"+ QString::number(-kei+balBeforeFromDate) + "に一致します！　" ;
    }else if(total == -kei+balBeforeFromDate&&addCount==0&&allCount!=matchCount){
        rep=rep+QString::number(matchCount)+"件が一致　最終残高"+ QString::number(-kei+balBeforeFromDate) + "に一致します！　";
    }else if(total == -kei+balBeforeFromDate&&addCount==0&&allCount==matchCount){
        rep=rep+QString::number(matchCount)+"件がすべて一致(更新不要)　最終残高"+ QString::number(-kei+balBeforeFromDate) + "に一致します！　";

    }else    {
        rep=rep+ "！！ここでの集計は"+ QString::number(balBeforeFromDate-kei)
              + "{"+QString::number(balBeforeFromDate)
              +"(処理前の残高)   "+QString::number(-kei)
              +"(増減)}"+"で、インポートデータの最終残高"
              + QString::number(total)+"と一致しません。CSVデータが正しいかチェック必要です。今の家計簿データは、"
              +QString::number(balBeforeFromDate)+"が処理範囲前日("+fromDate.addDays(-1).toString("yyyy/MM/dd")+")までの残高ですが、これが"+QString::number(records[0].balance+records[0].payment-records[0].deposit)+"になるように家計簿データの修正必要です。（引落日のずれ等が原因の場合もあり）";
    }


    return rep;
}


void MainWindow::updateTmpL(int fromRow, int fromCol, int toRow, int toCol)
{
    for (auto& item : cButtonL) {   // ポインタなら * をつける
        if (item.y == fromRow && item.x == fromCol) {  // x:行, y:列に合わせる
            item.y = toRow;
            item.x = toCol;

            break;
        }
    }
}

void MainWindow::onKosinClicked(DraggableGridWidget* grid,
                                const QList<KakeiboRowData>& kRows,
                                const QList<OricoRowData>& oRows,
                                int& mode,
                                QString rep)
{

    if (rep.left(2) == "注意") {
        QMessageBox::information(this, "確認", "更新取り消します(" + rep + ")");
        return;
    }



    //cButtonLによるチェックは不要になる。下のButtonによる処理がOkなったため。
    for (auto& tmp : cButtonL) {
        if (tmp.dtKind == 0) { // Kakeibo
            // tmp.krdata や tmp.x, tmp.y を使った処理

            if(tmp.x==0){ //del kake
                qDebug()<<"kake del"<< tmp.y << "  \\" << tmp.krdata.kingaku<<" num:"<<tmp.number;
            }else if(tmp.x==1){ //一致
                 qDebug()<<"kake 一致"<< tmp.y << "  \\" << tmp.krdata.kingaku<<" num:"<<tmp.number;
            }
        } else if (tmp.dtKind == 1) { // Orico
            // tmp.krdata や tmp.x, tmp.y を使った処理
            if(tmp.x==0){  //一致
                qDebug()<<"orico 一致"<< tmp.y << "  \"" << tmp.krdata.kingaku<<" num:"<<tmp.number;
            }else if(tmp.x==1){ //add
                qDebug()<<"ocrico add"<< tmp.y << "  \"" << tmp.krdata.kingaku<<" num:"<<tmp.number;
            }

        }
    }


    //button widgetでも確認*******************************************
    qDebug()<<"by buttonWidget";

    int rows = grid->rowCount();
    int cols = grid->colCount();

    // 例：格子上のボタン配置を走査して、対応を取得する
    for (int r = 0; r < rows; ++r)
    {
        DraggableButton* left  = grid->buttonAt(r, 0); // Orico
        DraggableButton* right = grid->buttonAt(r, 1); // Kakeibo

        if (!left && !right) continue;

        // 両方ある → マッチしている
        if (left && right)
        {
            auto odata = left->oricoData();
            auto kdata = right->kakeiboData();
       qDebug() << "MATCH row" << r << "金額" << odata.kingaku  << "日付" << odata.date << "備考" << kdata.biko+"("+odata.usePlace+")";

            if (!table->appendMemoToRow(kdata.id, "(Orico:",
                                        "(Orico:" + odata.usePlace + ")", ckozanum)) {
                QMessageBox::warning(
                    this, "照合結果の更新エラー",
                    QString("家計簿の明細（番号 %1）にオリコ情報を記録できませんでした。")
                        .arg(kdata.id));
                return;
            }
         }
        else if (left && !right)
        {
            KakeiboRowData datak=left->kakeiboData();
            datak.date = QDate(toDate.year(), toDate.month(), 1).addMonths(1).addDays(1);
            if (!datak.biko.contains("(削除の可能性あり)"))
                datak.biko += "(削除の可能性あり)";
            if (!table->updateRow(datak, datak.kingaku > 0, ckozanum)) {
                QMessageBox::warning(this, "照合結果の更新エラー",
                                     QString("家計簿の明細（番号 %1）を更新できませんでした。")
                                         .arg(datak.id));
                return;
            }

          //  qDebug() << "kakeibo only row(dateUpdate for del)" << r << left->kakeiboData().kingaku<< "del:"+left->kakeiboData().biko;

            // 必要に応じてDB処理
        }
        else if (!left && right)
        {
            KakeiboRowData data;
            QDate odate= right->oricoData().date; //check
            if(odate<=toDate&&odate>=fromDate){
                 data.date = odate;
            }else{
                 data.date = fromDate; //指定範囲でないときは2日に
            }
            data.kingaku = right->oricoData().kingaku;
            data.himoku = "";
            data.shiharaisaki = "";
            data.biko = right->oricoData().usePlace+"(Orico)";
            data.idosaki=0;
            table->addRowForCurrentAccount(data,true,ckozanum);//true=sishutu false=shunyu


            qDebug() << "orico only row(add or dateUpdate)" << r << right->oricoData().kingaku<<right->oricoData().usePlace;
        }
    }

    table->loadTable(ckozanum);


    qDebug() << "onKosinClicked completed.";
}


void MainWindow::onKosinClicked_Bank(DraggableGridWidget* grid,
                                const QList<KakeiboRowData>& kRows,
                                const QList<importRecord>& oRows,
                                int& mode,
                                QString rep)
{

    if (rep.left(2) == "注意") {
        QMessageBox::information(this, "確認", "更新取り消します(" + rep + ")");
        return;
    }

    //cButtonLによるチェックは不要になる。下のButtonによる処理がOkなったため。
    for (auto& tmp : cButtonL) {
        if (tmp.dtKind == 0) { // Kakeibo
            // tmp.krdata や tmp.x, tmp.y を使った処理
            if(tmp.x==0){ //del kake
                qDebug()<<"kake del"<< tmp.y << "  \\" << tmp.krdata.kingaku<<" num:"<<tmp.number;
            }else if(tmp.x==1){ //一致
                qDebug()<<"kake 一致"<< tmp.y << "  \\" << tmp.krdata.kingaku<<" num:"<<tmp.number;
            }
        } else if (tmp.dtKind == 1) { // Orico
            // tmp.krdata や tmp.x, tmp.y を使った処理
            if(tmp.x==0){  //一致
                qDebug()<<"bank 一致"<< tmp.y << "  \"" << tmp.krdata.kingaku<<" num:"<<tmp.number;
            }else if(tmp.x==1){ //add
                qDebug()<<"bank add"<< tmp.y << "  \"" << tmp.krdata.kingaku<<" num:"<<tmp.number;
            }
        }
    }


    //button widgetでも確認*******************************************
    qDebug()<<"by buttonWidget";

    int rows = grid->rowCount();
    int cols = grid->colCount();

    // 例：格子上のボタン配置を走査して、対応を取得する
    for (int r = 0; r < rows; ++r)
    {
        DraggableButton* left  = grid->buttonAt(r, 0); // Import?
        DraggableButton* right = grid->buttonAt(r, 1); // Kakeibo?
        if (!left && !right) continue;

        // 両方ある → マッチしている
        if (left && right)
        {
            auto odata = left->bankData();
            auto kdata = right->kakeiboData();
            qDebug() << "MATCH row" << r << "金額" << odata.payment-odata.deposit  << "日付" << odata.date << "備考" << kdata.biko+"("+odata.summary+")";
            if (!table->appendMemoToRow(kdata.id, "<imp",
                                        "<imp" + odata.summary, ckozanum)) {
                QMessageBox::warning(
                    this, "照合結果の更新エラー",
                    QString("家計簿の明細（番号 %1）に銀行CSV情報を記録できませんでした。")
                        .arg(kdata.id));
                return;
            }
        }
        else if (left && !right) //家計簿のみにある
        {
            KakeiboRowData datak=left->kakeiboData();
            datak.date = QDate(toDate.year(), toDate.month(), 1).addMonths(1).addDays(1);
            if (!datak.biko.contains("(削除の可能性あり)"))
                datak.biko += "(削除の可能性あり)";
            if (!table->updateRow(datak, datak.kingaku > 0, ckozanum)) {
                QMessageBox::warning(this, "照合結果の更新エラー",
                                     QString("家計簿の明細（番号 %1）を更新できませんでした。")
                                         .arg(datak.id));
                return;
            }
        }
        else if (!left && right) //インポートのみにある
        {
            KakeiboRowData data; //新規データのために作成
            QDate odate= right->bankData().date; //インポートデータの日付

           // if(odate<=toDate&&odate>=fromDate){ //指定範囲内ならそのまま、通常はこちら
                data.date = odate;
          //  }else{
          //      data.date = fromDate; //指定範囲でないときは2日に..通常ありえないが
          //  }

            data.kingaku = right->bankData().payment - right->bankData().deposit;
            data.himoku = "";
            data.shiharaisaki = "";
            if(!data.biko.contains("<imp")){
             data.biko = right->bankData().summary+"<imp";
             }
            data.idosaki=0;
            if(right->bankData().payment>0){
                table->addRowForCurrentAccount(data,true,ckozanum);//true=sishutu false=shunyu
            }else{
                data.kingaku=right->bankData().deposit;
                table->addRowForCurrentAccount(data,false,ckozanum);//true=sishutu false=shunyu
            }
            qDebug() << "orico only row(add or dateUpdate)" << r << right->oricoData().kingaku<<right->oricoData().usePlace;
        }
    }
    table->loadTable(ckozanum);
    qDebug() << "onKosinClicked_bank completed.";
}


void MainWindow::loadExpenses()
{
    QSettings settings("MyCompany", "QtKakeibo");
    QString currentUrl =
        settings.value("webapi/url", "https://my.address/api/items").toString();

    ExpenseLoader* loader = new ExpenseLoader(this);

    loader->loadFromApi(currentUrl, [this, loader](QVector<Expense> expenses){

        if (expenses.isEmpty()) {
            qDebug() << "APIからのデータは空です";
        } else {
            QString dbpath = MainWindow::getDatabasePath();

            int knum = -1;

            for (const auto& e : expenses) {
                KakeiboRowData data;

                knum = koza::numFromName(dbpath, e.method);
                data.date = QDate::fromString(e.date, "yyyy-MM-dd");
                data.kingaku = (int)e.amount;
                data.himoku = e.category;
                data.shiharaisaki = e.payee;
                data.biko = e.memo;
                data.idosaki = 0;

                table->addRowForCurrentAccount(data, true, knum);
                delCloud(e);  // ユーザーが Yes を選んだ場合に削除
            }

            // ✔ 全件追加後に 1 回だけ読み込み
            if (knum != -1)
                table->loadTable(knum);
        }

        loader->deleteLater(); // ✔ メモリリーク防止
        ui->pushButton_6->setStyleSheet("background-color: gray; color: white;");
    });
}




void MainWindow::netInFlg(std::function<void(bool)> callback) {
    QSettings settings("MyCompany", "QtKakeibo");
    QString currentUrl =
        settings.value("webapi/url", "https://my.address/api/items").toString();

    ExpenseLoader* loader = new ExpenseLoader(this);
    loader->loadFromApi(currentUrl, [callback](QVector<Expense> expenses){
        bool flag = (expenses.size() > 0);
        callback(flag); // 非同期に結果を返す
    });
}

// 例えばフォーム表示直後
void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);

    static bool first = true;
    if (first) {
        first = false;

        netInFlg([this](bool available){
            if(available) {
              //  qDebug() << "データあり";
                 ui->pushButton_6->setStyleSheet("background-color: red; color: white;");
            } else {
               // qDebug() << "データなしorアクセス不可";
            }
        });
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


void MainWindow::on_actionWebAPI_triggered()
{

    // ① 設定ロード
    QSettings settings("MyCompany", "QtKakeibo");
    QString currentUrl = settings.value("webapi/url",
                                        "https://my.address/api/items").toString();

    // ② ダイアログに現在値を渡して開く
    WebApiDialog dlg(this);
    dlg.setUrl(currentUrl);   // setUrl() はダイアログ側に実装しておく


    // ③ OK を押したときだけ保存
    if (dlg.exec() == QDialog::Accepted)
    {
        QString url = dlg.getUrl();
        settings.setValue("webapi/url", url);

        QMessageBox::information(this, tr("設定完了"),
                                 tr("URLを設定しました:\n%1").arg(url));
    }
}


void MainWindow::on_pushButton_6_clicked()
{
    loadExpenses();
}

void MainWindow::delCloud(const Expense& exp)
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "確認",
                                  QString("クラウド上のID %1 を削除しますか？").arg(exp.id),
                                  QMessageBox::Yes | QMessageBox::No);

    QSettings settings("MyCompany", "QtKakeibo");
    QString currentUrl = settings.value("webapi/url",
                                        "https://my.address/api/items").toString();


    if (reply == QMessageBox::Yes) {
        QNetworkRequest request(QUrl(QString("%1/%2").arg(currentUrl).arg(exp.id)));
        QNetworkReply* networkReply = manager->deleteResource(request);

        connect(networkReply, &QNetworkReply::finished, this, [networkReply, this]() {
            if (networkReply->error() == QNetworkReply::NoError) {
                QMessageBox::information(this, "情報", "削除しました。");
            } else {
                QMessageBox::critical(this, "エラー", networkReply->errorString());
            }
            networkReply->deleteLater();
        });
    }
}



void MainWindow::backupDatabase(const QString& dbPath)
{
    QDir dir(QDir::currentPath() + "/backups");
    if (!dir.exists()) {
        dir.mkpath(".");  // backups フォルダ作成
    }

    // バックアップファイル名
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString backupFileName = QString("backup_%1.db").arg(timestamp);
    QString backupFilePath = dir.filePath(backupFileName);

    // DBファイルコピー
    if (!QFile::copy(dbPath, backupFilePath)) {
        qDebug() << "バックアップ作成失敗:" << backupFilePath;
        return;
    }

    qDebug() << "バックアップ作成:" << backupFilePath;

    // 既存バックアップファイルを取得して古いものから削除
    QFileInfoList files = dir.entryInfoList(QStringList() << "backup_*.db", QDir::Files, QDir::Time);
    while (files.size() > 5) {
        QFileInfo oldest = files.last();
        if (QFile::remove(oldest.absoluteFilePath())) {
            qDebug() << "古いバックアップ削除:" << oldest.fileName();
        }
        files.removeLast();
    }
}



void MainWindow::closeEvent(QCloseEvent *event)
{
    // 終了時にバックアップ（BackupManager を使用）
    QString dbPath = MainWindow::getDatabasePath();
    if (m_backupManager)
        m_backupManager->backupDatabase(dbPath);

    // そのまま終了
    event->accept();
}


void MainWindow::on_actionimportKake_triggered()
{

    QString filePath = QFileDialog::getOpenFileName(
        this,
        "VcKake CSV を選択",
        "",                 // 初期フォルダ
        "CSV ファイル (*.csv);;すべてのファイル (*)"
        );

    if (filePath.isEmpty()) {
        return;
    }
    int total;

    QVector<KakeiboRowData> kRows = table->getAllRows(fromDate, toDate,ckozanum);



}




void MainWindow::on_actionimport_2_triggered()
{


    QSettings settings("MyCompany", "QtKakeibo");
    QString dbPath = settings.value("Database/Path").toString();

    if (!m_backupManager) {
        qCritical() << "★ m_backupManager が nullptr";
        return;
    }

    if (dbPath.isEmpty()) {
        qCritical() << "★ DBパスが空";
        return;
    }

    if (!QFile::exists(dbPath)) {
        qCritical() << "★ DBファイルが存在しない:" << dbPath;
        return;
    }

    bool ok = m_backupManager->createUndoBackup(dbPath);
    qDebug() << "createUndoBackup 結果 =" << ok;




    QString kname=koza::kozaNameFromNum(dbPath,ckozanum);

    if(kname.contains("オリコ")){
        QMessageBox::warning(this,"口座選択について","オリコが選択されています。銀行口座を選択してください。");
        return;
    }

    CsvMapping mapping=koza::kozaImportMapFromNum(dbPath,ckozanum);
    if(mapping.date<0){
        QMessageBox::warning(this,"インポート設定について","口座登録で、インポート設定（日付,支払,預かり,残高,取引区分, 摘要の順に列番号を並べる）必要です。");
        return;
    }

    auto ret = QMessageBox::question(
        this,
        tr("口座選択の確認"),
        QString("%1:%2 の口座のインポートを実行します。\n"
                "よろしければ OK ボタンをクリックしてください。")
            .arg(ckozanum)
            .arg(kname),
        QMessageBox::Ok | QMessageBox::Cancel
        );

    QString wildCard="*.csv";
    if (ret != QMessageBox::Ok) {
        return;
    }

    if(kname.contains("労働")|kname.contains("労金")){
        wildCard="ny*.csv";
    }else if(kname.contains("SBI")||kname.contains("ＳＢＩ")){
         wildCard="nyushukinmeisai*.csv";
    }else if(kname.contains("荘内")||kname.contains("荘銀")){
        wildCard="ny*.csv";
    }


    QString filePath = QFileDialog::getOpenFileName(
        this,
        kname+"CSV を選択",
        "",                 // 初期フォルダ
        "CSV ファイル ("+wildCard+");;すべてのファイル (*)"
        );

    if (filePath.isEmpty()) {
        return;
    }

/*
    CsvMapping mapping { 2,  // 日付   4,  // 支払 5,  // 預かり   8,  // 残高 7,  // 取引区分 9   // 摘要 };  以下のようにこれをDBから取得
*/


    CsvImporter CsvImporter(mapping);
    QVector<importRecord> records;
    if (!filePath.isEmpty()) {
        records = CsvImporter.importFile(filePath);
    }

     if (records.isEmpty()) {
         QMessageBox::warning(this, "CSV Error", filePath+": CSV ファイルが存在しないか読み込めません");
        return;
     }

     //total（インポートデータの最終行の残高）取得  あとで合計額or残高照合で利用
     importRecord lastObj = records.last();  // 最後のオブジェクトを取得
     int total= lastObj.balance; //


    // qDebug() << records;
     QDate startDay=records[0].date;
     QDate today = QDate::currentDate();
     auto dates = getDateRangeFromUser(this,startDay,today);
     if (!dates.first.isValid() || !dates.second.isValid()) {
         return;
     }
     fromDate = dates.first;
     toDate   = dates.second;



    QVector<KakeiboRowData> kRows = table->getAllRows(fromDate, toDate,ckozanum);

    QDialog dlg(this);
    dlg.setWindowTitle("Draggable Grid");
    dlg.resize(800, 400);
    int rows = records.length()+kRows.length()+1;
    int cols = 2;

    DraggableGridWidget *grid = new DraggableGridWidget(rows, cols);
    QScrollArea *scrollArea = new QScrollArea(&dlg);
    // 上部ラベル部分
    auto *labelLayout = new QHBoxLayout;
    QLabel *label1=new QLabel("削除（左列）");
    label1->setAlignment(Qt::AlignCenter);
    labelLayout->addWidget(label1);
    QLabel *label2=new QLabel("適用（右列）");
    label2->setAlignment(Qt::AlignCenter);
    labelLayout->addWidget(label2);

    // ラベル + grid をまとめるコンテナ
    auto *container = new QWidget;
    auto *containerLayout = new QVBoxLayout(container);
    containerLayout->addLayout(labelLayout);
    containerLayout->addWidget(grid);

    // ScrollArea に container をセット
    scrollArea->setWidget(container);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    QVBoxLayout *vbox = new QVBoxLayout(&dlg); // ✅ dlg にレイアウトを設定
    vbox->addWidget(scrollArea);
    QPushButton* kosinBtn = new QPushButton("更新");
    vbox->addWidget(kosinBtn);


    QString rep=populateBankGrid(grid, kRows, records,total);
    kosinBtn->setToolTip(rep);


    connect(kosinBtn, &QPushButton::clicked,
            this,
            [this, grid, kRows, records, &total,rep]() {
                onKosinClicked_Bank(grid, kRows, records, total,rep);
            });

    dlg.exec();

}


void MainWindow::recalcAndGoLastSafe()
{
     //QApplication::processEvents();

    auto m = table->getModel();
    if (!m) return;

    static bool inProgress = false;
    if (inProgress) return;
    inProgress = true;

    // ★ まず編集中のセルを確定
    if (QWidget *w = QApplication::focusWidget()) {
        w->clearFocus();
    }

    QTimer::singleShot(80, this, [this, m]() {

        if (!m->submitAll()) {
            qWarning() << "submitAll failed:" << m->lastError();
            inProgress = false;
            return;
        }

        table->recalculateBalances(ckozanum);

        // ★★★ ここが重要：select() の「完了後」にスクロールする ★★★
        connect(m, &QAbstractItemModel::modelReset,
                this, [this]() {

                    // さらに一拍だけ待つ（念のため）
                    QTimer::singleShot(10, this, [this]() {
                        table->getView()->scrollToBottom();
                    });
                },
                Qt::SingleShotConnection); // ← 1回だけ実行

        m->select();   // ← ここでモデルが作り直される

        inProgress = false;
    });
}


void MainWindow::on_actioninportBefore_triggered()
{
    auto ret = QMessageBox::question(
        this,
        tr("Undo（復元）"),
        tr("注意！！この操作で、インポート直前のデータに戻します。\n\n"
           "※ 復元はアプリ再起動後に行ってください。\n"
           "続行しますか？"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No   // ← デフォルトは No（安全側）
        );

    if (ret != QMessageBox::Yes)
        return;   // ★ キャンセル

    // ★ 1) いったんDBを閉じる
    QSqlDatabase::database().close();   // ← まずはこれ

    // ★ 2) Undo 復元
    bool ok = m_backupManager->restoreLatestUndo(getDatabasePath());

    if (ok) {
        // ★ 3) DBを開き直す
        QSqlDatabase::database().open();

        // ★ 4) モデルを再読み込み
        gotoLast();
    }



}
