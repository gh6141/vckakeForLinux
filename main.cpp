//#include "KozaWidget.h"
#include "mainwindow.h"


#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "vckake_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

   // MainWindow w;
   // w.show();

    MainWindow *w = new MainWindow();
    w->show();
    //KakeiboTable table;
    //table.resize(800, 400);
    //table.show();

    // mainwindow.cpp 内
  //  KakeiboTable *table = new KakeiboTable(this);  // this = MainWindow
 //   KakeiboTable *table=new KakeiboTable(w);
  //  ui->centralWidget->layout()->addWidget(table); // レイアウトに組み込むだけ


    return a.exec();
}
