#pragma once
// BaseComboWidget.h
#include <QWidget>
#include <QComboBox>
#include <QSqlQuery>
#include <QList>
#include <QPair>
class BaseComboWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BaseComboWidget(QWidget *parent = nullptr) : QWidget(parent) {}

    // 派生クラスで実装
    virtual QString tableName() const = 0;
    virtual QString idColumn() const { return "num"; }
    virtual QString displayColumn() const = 0;

    // ComboBox にセットする共通処理
    void fillComboBox(QComboBox* combo)
    {
        combo->clear();
        QString sql = QString("SELECT %1, %2 FROM %3")
                          .arg(idColumn(), displayColumn(), tableName());
        QSqlQuery query(sql);
        while(query.next()) {
            int id = query.value(0).toInt();
            QString name = query.value(1).toString();
            combo->addItem(name, id);
        }
    }
};
