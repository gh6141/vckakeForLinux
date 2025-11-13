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
  /*  void fillComboBox(QComboBox* combo)
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
*/


    void fillComboBox(QComboBox* combo, const QString& yomiColumn = QString())
    {
        combo->clear();

        QString sql;
        if (!yomiColumn.isEmpty()) {
            // yomi がある場合は列を取得
            sql = QString("SELECT %1, %2, %3 FROM %4")
                      .arg(idColumn(), displayColumn(), yomiColumn, tableName());
        } else {
            sql = QString("SELECT %1, %2 FROM %3")
            .arg(idColumn(), displayColumn(), tableName());
        }

        QSqlQuery query(sql);

        if (!yomiColumn.isEmpty()) {
            // yomi がある場合は一旦 QVector に集めてソート
            QVector<QPair<QString, QPair<int, QString>>> items; // <yomi, <id, name>>
            while(query.next()) {
                int id = query.value(0).toInt();
                QString name = query.value(1).toString();
                QString yomi = query.value(2).toString();
                items.append(qMakePair(yomi, qMakePair(id, name)));
            }

            // yomi で昇順ソート
            std::sort(items.begin(), items.end(),
                      [](const QPair<QString, QPair<int, QString>>& a,
                         const QPair<QString, QPair<int, QString>>& b) {
                          return a.first < b.first;
                      });

            // ComboBox に追加
            for (const auto& item : items) {
                combo->addItem(item.second.second, item.second.first);
            }
        } else {
            // yomi がない場合はそのまま追加
            while(query.next()) {
                int id = query.value(0).toInt();
                QString name = query.value(1).toString();
                combo->addItem(name, id);
            }
        }
    }






};
