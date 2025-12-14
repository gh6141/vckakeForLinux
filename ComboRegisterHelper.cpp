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

   // m_labelFurigana = new QLabel("フリガナ", m_parent);
    m_editFurigana = new ClearOnClickLineEdit(m_parent);
    m_editFurigana->setPlaceholderText("ふりがなを入力");
    connect(m_combo->lineEdit(), &QLineEdit::textEdited,
            this, [this](const QString &text){
                 cFurigana=text;
            });


    m_buttonRegister = new QPushButton("登録", m_parent);
    m_buttonCancel = new QPushButton("キャンセル", m_parent);

    m_editFurigana->setVisible(false);
    m_buttonRegister->setVisible(false);

  //  m_buttonCancel->setFixedWidth(120); // 幅を揃えると見た目がきれい

    connect(m_buttonCancel, &QPushButton::clicked, this, [this]() {
        m_editFurigana->clear();
        m_editFurigana->setVisible(false);
        m_buttonRegister->setVisible(false);
        m_buttonCancel->setVisible(false);
    });

    m_buttonCancel->setVisible(false);

    QVBoxLayout* vLayout = qobject_cast<QVBoxLayout*>(m_parent->layout());
    if (!vLayout) {
        vLayout = new QVBoxLayout(m_parent);
        m_parent->setLayout(vLayout);
    }

   // vLayout->addWidget(m_labelFurigana);
    vLayout->addWidget(m_editFurigana);

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->addWidget(m_buttonRegister);
    hLayout->addWidget(m_buttonCancel);
    hLayout->addStretch();
    vLayout->addLayout(hLayout);

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
    if (comboPreventFlg) return;  // フラグで動作制御
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

    disp(!exists);
}


void ComboRegisterHelper::disp(bool noExists){
    //m_labelFurigana->setVisible(!exists);
    m_editFurigana->setVisible(noExists);
    m_buttonRegister->setVisible(noExists);
    m_buttonCancel->setVisible(noExists);

    if(noExists){
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
    //m_labelFurigana->setVisible(false);

    disp(false);
   // m_editFurigana->setVisible(false);
   // m_buttonRegister->setVisible(false);
   // m_buttonCancel->setVisible(false);


    MainWindow* mainWin = qobject_cast<MainWindow*>(this->parent());
   // qDebug() << "parent =" << this->parent();
    if(mainWin) {
        mainWin->comboTwoUpdate();
       // qDebug()<<"test";
    }

}


