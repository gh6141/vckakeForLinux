#include "DraggableGridWidget.h"
#include <QPainter>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QDataStream>

DraggableGridWidget::DraggableGridWidget(int rows, int cols, QWidget *parent)
    : QWidget(parent), m_rows(rows), m_cols(cols)
{
    setAcceptDrops(true);
}

void DraggableGridWidget::addButton(DraggableButton *btn, int row, int col)
{
    btn->setParent(this);
    btn->setCell(row, col);
    btn->resize(width()/m_cols, height()/m_rows);
    btn->move(cellTopLeft(row, col));
    btn->show();
}

QPoint DraggableGridWidget::cellTopLeft(int row, int col) const
{
    int w = width() / m_cols;
    int h = height() / m_rows;
    return QPoint(col * w, row * h);
}

DraggableButton* DraggableGridWidget::buttonAtCell(int row, int col) const
{
    for (auto btn : findChildren<DraggableButton*>())
    {
        if (btn->row() == row && btn->col() == col)
            return btn;
    }
    return nullptr;
}

void DraggableGridWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setPen(QPen(Qt::gray, 1));

    int w = width() / m_cols;
    int h = height() / m_rows;

    for (int r = 1; r < m_rows; ++r)
        painter.drawLine(0, r*h, width(), r*h);
    for (int c = 1; c < m_cols; ++c)
        painter.drawLine(c*w, 0, c*w, height());
}

void DraggableGridWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    int cellW = width() / m_cols;
    int cellH = height() / m_rows;

    for (auto btn : findChildren<DraggableButton*>())
    {
        int row = btn->row();
        int col = btn->col();
        btn->resize(cellW, cellH);
        btn->move(cellTopLeft(row, col));
    }
}

void DraggableGridWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-griditem"))
        event->acceptProposedAction();
}

void DraggableGridWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-griditem"))
        event->acceptProposedAction();
}

void DraggableGridWidget::dropEvent(QDropEvent *event)
{
    if (!event->mimeData()->hasFormat("application/x-griditem")) return;

    QByteArray data = event->mimeData()->data("application/x-griditem");
    QDataStream stream(&data, QIODevice::ReadOnly);

    int fromRow, fromCol;
    stream >> fromRow >> fromCol;

    int cellW = width()/m_cols;
    int cellH = height()/m_rows;
    int toCol = event->position().x() / cellW;
    int toRow = event->position().y() / cellH;

    auto *fromBtn = buttonAtCell(fromRow, fromCol);
    auto *toBtn   = buttonAtCell(toRow, toCol);

    if (!fromBtn) return;

    if (toBtn)
    {
        // 交換
        fromBtn->move(cellTopLeft(toRow, toCol));
        toBtn->move(cellTopLeft(fromRow, fromCol));

        fromBtn->setCell(toRow, toCol);
        toBtn->setCell(fromRow, fromCol);
    }
    else
    {
        fromBtn->move(cellTopLeft(toRow, toCol));
        fromBtn->setCell(toRow, toCol);
    }

    event->acceptProposedAction();
}
