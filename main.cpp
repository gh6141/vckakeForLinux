//#include "KozaWidget.h"
#include "mainwindow.h"
#include "kakeibotable.h"

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

    //KozaWidget w;
    //w.show();
    MainWindow w;
    w.show();

    KakeiboTable table;
    table.resize(800, 400);
    table.show();


    return a.exec();
}
