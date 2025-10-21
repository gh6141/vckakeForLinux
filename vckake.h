#ifndef VCKAKE_H
#define VCKAKE_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class vckake;
}
QT_END_NAMESPACE

class vckake : public QMainWindow
{
    Q_OBJECT

public:
    vckake(QWidget *parent = nullptr);
    ~vckake();

private:
    Ui::vckake *ui;
};
#endif // VCKAKE_H
