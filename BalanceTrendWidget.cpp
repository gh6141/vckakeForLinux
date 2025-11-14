#include "BalanceListWidget.h"
#include "BalanceTrendWidget.h"
#include "mainwindow.h"
#include "koza.h"

#include <QVBoxLayout>
#include <QHeaderView>

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
//using namespace QtCharts;>s
#include <QDateTimeAxis>
#include <QDateTime>

BalanceTrendWidget::BalanceTrendWidget(QDate start, QDate end, QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("残高推移リスト");

    view  = new QTableView(this);
    model = new QStandardItemModel(this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(view);

    view->setModel(model);
    view->setAlternatingRowColors(true);

    // データ読み込み
    loadData(start, end);

    // 表示調整
    QFont font = view->font();
    font.setPointSize(14);
    view->setFont(font);

    QFont headerFont = view->horizontalHeader()->font();
    headerFont.setPointSize(14);
    view->horizontalHeader()->setFont(headerFont);

    view->setColumnWidth(0,120);
    view->setColumnWidth(1,120);
    view->horizontalHeader()->setStretchLastSection(true);

     setMinimumSize(800, 600);
}

void BalanceTrendWidget::loadData(QDate start, QDate end)
{
    model->clear();
    model->setHorizontalHeaderLabels({"年月","総残高"});

    QString dbpath = MainWindow::getDatabasePath();
    QList<koza> accounts = koza::selectAll(dbpath);

    QDate date = start;

    while (date <= end)
    {
        double total = 0.0;

        for (const koza &k : accounts) {
            double bal = BalanceListWidget::calculateBalance(k.num, date);
            total += bal;
        }

        QList<QStandardItem*> row;

        QString ym = QString("%1-%2")
                         .arg(date.year())
                         .arg(date.month(),2,10,QChar('0'));

        QStandardItem *itemYm = new QStandardItem(ym);
        QStandardItem *itemTotal = new QStandardItem(QString::number(int(total)));

        itemTotal->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        row << itemYm << itemTotal;

        model->appendRow(row);

        balances.append(qMakePair(date,total));

        date = date.addMonths(1);
    }
}

void BalanceTrendWidget::chart() {
    QLineSeries *series = new QLineSeries();

    for(auto &pair : balances){
        //series->append(pair.first.toJulianDay(), pair.second);
       // QDateTime dt(pair.first);  // QDate → QDateTime
        QDateTime dt(pair.first, QTime(0, 0));  // QDate + QTime で QDateTime を作成
        series->append(dt.toMSecsSinceEpoch(), pair.second); // ミリ秒で追加
    }




    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("残高推移");

    QDateTimeAxis *axisX = new QDateTimeAxis();
    axisX->setFormat("yyyy-MM");
    axisX->setTitleText("年月");
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("残高");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // 既存のレイアウトに追加する
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(this->layout());
    if(layout)
        layout->addWidget(chartView);
}
