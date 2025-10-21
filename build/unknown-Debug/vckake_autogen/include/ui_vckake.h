/********************************************************************************
** Form generated from reading UI file 'vckake.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VCKAKE_H
#define UI_VCKAKE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableView>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_vckake
{
public:
    QWidget *centralwidget;
    QTableView *tableView;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QPushButton *btnAdd;
    QPushButton *btnDelete;
    QMenuBar *menubar;

    void setupUi(QMainWindow *vckake)
    {
        if (vckake->objectName().isEmpty())
            vckake->setObjectName("vckake");
        vckake->resize(947, 441);
        centralwidget = new QWidget(vckake);
        centralwidget->setObjectName("centralwidget");
        tableView = new QTableView(centralwidget);
        tableView->setObjectName("tableView");
        tableView->setGeometry(QRect(70, 50, 421, 271));
        horizontalLayoutWidget = new QWidget(centralwidget);
        horizontalLayoutWidget->setObjectName("horizontalLayoutWidget");
        horizontalLayoutWidget->setGeometry(QRect(90, 140, 160, 80));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        btnAdd = new QPushButton(centralwidget);
        btnAdd->setObjectName("btnAdd");
        btnAdd->setGeometry(QRect(110, 20, 84, 23));
        btnDelete = new QPushButton(centralwidget);
        btnDelete->setObjectName("btnDelete");
        btnDelete->setGeometry(QRect(240, 20, 84, 23));
        vckake->setCentralWidget(centralwidget);
        menubar = new QMenuBar(vckake);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 947, 20));
        vckake->setMenuBar(menubar);

        retranslateUi(vckake);

        QMetaObject::connectSlotsByName(vckake);
    } // setupUi

    void retranslateUi(QMainWindow *vckake)
    {
        vckake->setWindowTitle(QCoreApplication::translate("vckake", "vckake", nullptr));
        btnAdd->setText(QCoreApplication::translate("vckake", "\346\226\260\350\246\217", nullptr));
        btnDelete->setText(QCoreApplication::translate("vckake", "\345\211\212\351\231\244", nullptr));
    } // retranslateUi

};

namespace Ui {
    class vckake: public Ui_vckake {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VCKAKE_H
