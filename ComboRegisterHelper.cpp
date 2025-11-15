#include "ComboRegisterHelper.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLayout>
#include <QEvent>
#include "ClearOnClickLineEdit.h"
#include "mainwindow.h"



ComboRegisterHelper::ComboRegisterHelper(QComboBox* combo,
                                         QWidget* parentContainer,
                                         const QString& tableName,
                                         QSqlDatabase db,
                                         QObject* parent)
    : QObject(parent),
    m_combo(combo),
    m_parent(parentContainer),
    m_tableName(tableName),
    m_db(db)
{
    createUi();
    connectSignals();
}

// 以下、onComboTextChanged, onRegisterClicked, fldconv などの実装


void ComboRegisterHelper::createUi()
{

    // m_parent は QWidget* で渡していると仮定
    m_labelFurigana = new QLabel("フリガナ", m_parent);
    //m_editFurigana = new QLineEdit(m_parent);
    m_editFurigana = new ClearOnClickLineEdit(m_parent);

    connect(m_combo->lineEdit(), &QLineEdit::textEdited,
            this, [this](const QString &text){
                 qDebug() << "入力中：" << text;
                 cFurigana=text;

            });
    m_buttonRegister = new QPushButton("登録", m_parent);

    m_labelFurigana->setVisible(false);
    m_editFurigana->setVisible(false);

    m_buttonRegister->setVisible(false);

    QVBoxLayout* vLayout = new QVBoxLayout(m_parent);
    vLayout->addWidget(m_labelFurigana);
    vLayout->addWidget(m_editFurigana);
    vLayout->addWidget(m_buttonRegister);
    m_parent->setLayout(vLayout);

    // 親コンテナ上に配置
    //this->setParent(m_parent);
}

void ComboRegisterHelper::connectSignals()
{
    connect(m_combo, &QComboBox::editTextChanged,
            this, &ComboRegisterHelper::onComboTextChanged);
    connect(m_buttonRegister, &QPushButton::clicked,
            this, &ComboRegisterHelper::onRegisterClicked);

}

void ComboRegisterHelper::onComboTextChanged(const QString &text)
{

    // 未登録なら表示
    QSqlQuery query(m_db);
    if(m_tableName=="Biko"){
        query.prepare(QString("SELECT COUNT(*) FROM %1 WHERE biko=:name").arg(m_tableName));

    }else{
        query.prepare(QString("SELECT COUNT(*) FROM %1 WHERE shiharaisaki=:name").arg(m_tableName));

    }

  query.bindValue(":name", text);
    query.exec();
    query.next();
    bool exists = query.value(0).toInt() > 0;

    m_labelFurigana->setVisible(!exists);
    m_editFurigana->setVisible(!exists);
    m_buttonRegister->setVisible(!exists);

    if(!exists){
        // 自動で読みを設定（仮に全角カタカナなどの簡易変換）
      //  combo->lineEdit()->setPlaceholderText("ここに入力");

        m_editFurigana->setText(cFurigana);
    }
}

void ComboRegisterHelper::onRegisterClicked()
{
    QString name = m_combo->currentText();
    QString yomi = m_editFurigana->text();

    QSqlQuery query(m_db);
    if(m_tableName=="Biko"){
        query.prepare(QString("INSERT INTO %1 (biko, yomi) VALUES (:name, :yomi)").arg(m_tableName));

    }else{
        query.prepare(QString("INSERT INTO %1 (shiharaisaki, yomi) VALUES (:name, :yomi)").arg(m_tableName));

    }
    query.bindValue(":name", name);
    query.bindValue(":yomi", yomi);

    if(!query.exec()){
       // QMessageBox::warning(this, "登録失敗", query.lastError().text());
        QMessageBox::warning(m_parent, tr("登録エラー"), query.lastError().text(), QMessageBox::Ok);

        return;
    }

    m_combo->addItem(name); // コンボに追加
    m_labelFurigana->setVisible(false);
    m_editFurigana->setVisible(false);
    m_buttonRegister->setVisible(false);

/*
    MainWindow* mainWin = qobject_cast<MainWindow*>(this->parent());
    if(mainWin) {
        mainWin->comboTwoUpdate();
    }
*/
}


