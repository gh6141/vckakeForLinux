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



    QFont font = a.font();
    font.setPointSize(12);
    a.setFont(font);

    MainWindow *w = new MainWindow();
    w->show();



    return a.exec();
}
