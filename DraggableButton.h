#pragma once
#include <QPushButton>
#include <QDrag>
#include <QMouseEvent>
#include <QMimeData>
#include <QDataStream>

class DraggableButton : public QPushButton
{
    Q_OBJECT
public:
    explicit DraggableButton(const QString &text, QWidget *parent = nullptr)
        : QPushButton(text, parent), m_row(0), m_col(0)
    {
        setAcceptDrops(false);
    }

    int row() const { return m_row; }
    int col() const { return m_col; }
    void setCell(int r, int c) { m_row = r; m_col = c; }

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        dragStartPos = event->pos();
        QPushButton::mousePressEvent(event);
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
        if (!(event->buttons() & Qt::LeftButton)) return;
        if ((event->pos() - dragStartPos).manhattanLength() < 5) return;

        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream << m_row << m_col;

        auto *mime = new QMimeData;
        mime->setData("application/x-griditem", data);

        auto *drag = new QDrag(this);
        drag->setMimeData(mime);
        drag->exec(Qt::MoveAction);
    }

private:
    QPoint dragStartPos;
    int m_row;
    int m_col;
};

