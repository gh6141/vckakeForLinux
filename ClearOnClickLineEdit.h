#ifndef CLEA_RONCLICKLINEEDIT_H
#define CLEA_RONCLICKLINEEDIT_H

#include <QLineEdit>
#include <QMouseEvent>

class ClearOnClickLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit ClearOnClickLineEdit(QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;
};

#endif // CLEA_RONCLICKLINEEDIT_H
