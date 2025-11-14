#include <QDate>
#include "BalanceListWidget.h"
#include <QVBoxLayout>
#include <QStandardItem>
#include <QList>
#include <QString>
#include <QPair>
#include "mainwindow.h"
#include "koza.h"
#include <QHeaderView>
#include <QStyledItemDelegate>
#include "PeriodSelectDialog.h"
#include <QDialogButtonBox>
#include <QLabel>
#include <QMessageBox>


PeriodSelectDialog::PeriodSelectDialog(QWidget *parent)
    : QDialog(parent)
{
   // QDialog dlg(this);
    setWindowTitle("期間選択");

    QVBoxLayout *layout = new QVBoxLayout(this);

    cmbStartYear = new QComboBox();
    cmbStartMonth = new QComboBox();
    cmbEndYear = new QComboBox();
    cmbEndMonth = new QComboBox();

    QDate now = QDate::currentDate();
    int currentYear = now.year();

    for (int y = currentYear - 2; y <= currentYear; ++y) {
        cmbStartYear->addItem(QString::number(y));
        cmbEndYear->addItem(QString::number(y));
    }
    for (int m = 1; m <= 12; ++m) {
        cmbStartMonth->addItem(QString::number(m));
        cmbEndMonth->addItem(QString::number(m));
    }

    cmbStartYear->setCurrentText(QString::number(currentYear));
    cmbStartMonth->setCurrentText(QString::number(now.month()));
    cmbEndYear->setCurrentText(QString::number(currentYear));
    cmbEndMonth->setCurrentText(QString::number(now.month()));

    // ▼ ここを追加 ▼
    QHBoxLayout *row1 = new QHBoxLayout();
    row1->addWidget(new QLabel("開始："));
    row1->addWidget(cmbStartYear);
    row1->addWidget(new QLabel("年"));
    row1->addWidget(cmbStartMonth);
    row1->addWidget(new QLabel("月"));

    QHBoxLayout *row2 = new QHBoxLayout();
    row2->addWidget(new QLabel("終了："));
    row2->addWidget(cmbEndYear);
    row2->addWidget(new QLabel("年"));
    row2->addWidget(cmbEndMonth);
    row2->addWidget(new QLabel("月"));

    layout->addLayout(row1);
    layout->addLayout(row2);
    // ▲ ここまで追加 ▲

    QDialogButtonBox *buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    layout->addWidget(buttonBox);

    // 最低サイズを設定
    setMinimumSize(300, 150);

}

void PeriodSelectDialog::disp(){
    int startYear = cmbStartYear->currentText().toInt();
    int startMonth = cmbStartMonth->currentText().toInt();
    int endYear   = cmbEndYear->currentText().toInt();
    int endMonth  = cmbEndMonth->currentText().toInt();

    QDate startDate(startYear, startMonth, 1);
    QDate endDate(endYear, endMonth, QDate(endYear, endMonth, 1).daysInMonth());
    QString dbpath=MainWindow::getDatabasePath();
    QList<koza> accounts = koza::selectAll(dbpath);

    QString result;
    result += "月ごとの残高推移:\n";

    QDate date = startDate;
    while (date <= endDate) {
        double total = 0.0;

        for (const koza &k : accounts) {
            double bal = BalanceListWidget::calculateBalance(k.num, date);
            total += bal;

        }

        result += QString("%1/%2 : %3 円\n").arg(date.year()).arg(date.month()).arg(total);

        // 次の月へ
        date = date.addMonths(1);
    }

    QMessageBox::information(this, "残高推移", result);
}

QDate PeriodSelectDialog::startDate() const
{
    int y = cmbStartYear->currentText().toInt();
    int m = cmbStartMonth->currentText().toInt();
    return QDate(y, m, 1);
}

QDate PeriodSelectDialog::endDate() const
{
    int y = cmbEndYear->currentText().toInt();
    int m = cmbEndMonth->currentText().toInt();
    return QDate(y, m, QDate(y, m, 1).daysInMonth());
}
