#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QTableView>
#include <QStandardItemModel>

class BikoSearchDialog : public QDialog
{
    Q_OBJECT
public:
    explicit BikoSearchDialog(QWidget *parent = nullptr);

private slots:
    void onSearchClicked();

private:
    QLineEdit *editKeyword;
    QPushButton *btnSearch;
    QTableView *view;
    QStandardItemModel *model;

    void searchKeyword(const QString &keyword);
};
