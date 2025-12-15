#ifndef COMBOREGISTERHELPER_H
#define COMBOREGISTERHELPER_H

#include <QObject>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QSqlDatabase>
#include <QMap>
#include <QHBoxLayout>

class ComboRegisterHelper : public QObject {
    Q_OBJECT
public:
    ComboRegisterHelper(QComboBox* combo,
                        QWidget* cbArea,
                        QHBoxLayout* layout,
                        const QString& tableName,
                        QSqlDatabase db,
                        QObject* parent = nullptr);
    bool comboPreventFlg = false;  // ここにフラグを置く

    void setPrevent(bool val) { comboPreventFlg = val; }

    void disp(bool noExists);


private slots:
    void onComboTextChanged(const QString &text);
    void onRegisterClicked();


private:
    QComboBox* m_combo;
    QLineEdit* m_editFurigana;
    QPushButton* m_buttonRegister;
    QPushButton* m_buttonCancel;
    QLabel* m_labelFurigana;
    QWidget* m_cbarea;
    QLayout* m_layout;
    QWidget* m_parent;    
    QString m_tableName;
    QSqlDatabase m_db;

    void createUi();
    void connectSignals();

    static QString fldconv(const QString& tbname);
    QString cFurigana;
};

#endif // COMBOREGISTERHELPER_H
