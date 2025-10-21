// BaseTableWidget.h
#pragma once

#include <QMainWindow>
#include <QSqlTableModel>
#include <QTableView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QDebug>

class BaseTableWidget : public QMainWindow
{
    Q_OBJECT
public:
    explicit BaseTableWidget(QWidget *parent = nullptr,QString createSQL="");

    void setTableName(const QString& tableName,QString createSQL="");

protected slots:
    virtual void addRow();      // 空行追加
    void deleteRow();           // 選択行削除

protected:
    QSqlTableModel* model;
    QTableView* view;
    QString tableName;

    void initUI();
    void initDB(QString createSQL);
};
