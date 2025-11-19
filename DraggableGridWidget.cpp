#include "DraggableGridWidget.h"
#include "DraggableButton.h"
#include <QPainter>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QDataStream>

DraggableGridWidget::DraggableGridWidget(int rows, int cols, QWidget *parent)
    : QWidget(parent), m_rows(rows), m_cols(cols)
{
    setAcceptDrops(true);

    // 縦方向固定サイズ
    setMinimumHeight(m_rows * cellH);
    setMaximumHeight(m_rows * cellH);
}

void DraggableGridWidget::addButton(DraggableButton *btn, int row, int col)
{
    btn->setParent(this);
    btn->setCell(row, col);
    btn->resize(width()/m_cols, cellH);
    btn->move(cellTopLeft(row, col));
    btn->show();

    btnMap[qMakePair(row, col)] = btn;

}

void DraggableGridWidget::addButton(QString text, int row, int col)
{ //実際はボタンでなく固定のテキストのみ配置　位置は調整必要
    QLabel* lbl = new QLabel(text, this);

    // テキストが隠れないように
    lbl->setAlignment(Qt::AlignCenter);     // 中央寄せ
    lbl->setWordWrap(false);                // 折り返し禁止
    lbl->setMargin(0);                      // 余計な余白削除
    lbl->setFrameStyle(QFrame::NoFrame);    // 余計な枠無し

    int minW = lbl->fontMetrics().horizontalAdvance(text) + 8; // 余裕を見て+8
    int cellW = width() / m_cols;
    int finalW = qMax(minW, cellW);

    lbl->resize(finalW, cellH);
    lbl->setMinimumWidth(finalW);
    lbl->setAlignment(Qt::AlignCenter);
    lbl->move(cellTopLeft(row, 8*col));
    lbl->show();

}

QPoint DraggableGridWidget::cellTopLeft(int row, int col) const
{
    int w = width() / m_cols;
    return QPoint(col * w, row * cellH);
}

DraggableButton* DraggableGridWidget::buttonAtCell(int row, int col) const
{
    for (auto btn : findChildren<DraggableButton*>()) {
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

    for (int r = 1; r < m_rows; ++r)
        painter.drawLine(0, r*cellH, width(), r*cellH);
    for (int c = 1; c < m_cols; ++c)
        painter.drawLine(c*w, 0, c*w, height());
}

void DraggableGridWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    int cellW = width() / m_cols;

    for (auto btn : findChildren<DraggableButton*>()) {
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
    if (!event->mimeData()->hasFormat("application/x-griditem"))
        return;

    QByteArray data = event->mimeData()->data("application/x-griditem");
    QDataStream stream(&data, QIODevice::ReadOnly);

    int fromRow, fromCol;
    stream >> fromRow >> fromCol;

    int cellW = width()  / m_cols;
    int cellH = height() / m_rows;

    int toCol = event->position().x() / cellW;
    int toRow = event->position().y() / cellH;

    moveButton(fromRow, fromCol, toRow, toCol);

    event->acceptProposedAction();



}


DraggableButton* DraggableGridWidget::buttonAt(int row, int col) const
{
    auto key = qMakePair(row, col);
    if (btnMap.contains(key)) {
        return btnMap[key];
    }
    return nullptr;
}

bool DraggableGridWidget::moveButton(int fromRow, int fromCol,
                                     int toRow,   int toCol)
{
    auto *fromBtn = buttonAtCell(fromRow, fromCol);
    if (!fromBtn) return false;

    auto *toBtn = buttonAtCell(toRow, toCol);

    // スワップ
    if (toBtn) {
        QPoint posFrom = cellTopLeft(fromRow, fromCol);
        QPoint posTo   = cellTopLeft(toRow,   toCol);

        fromBtn->move(posTo);
        toBtn->move(posFrom);

        // cell 情報を更新
        fromBtn->setCell(toRow, toCol);
        toBtn->setCell(fromRow, fromCol);
    }
    // 空セルへの移動
    else {
        QPoint posTo = cellTopLeft(toRow, toCol);
        fromBtn->move(posTo);
        fromBtn->setCell(toRow, toCol);
    }

    return true;
}

/*
bool DraggableGridWidget::moveButton(int fromRow, int fromCol, int toRow, int toCol)
{
    auto* btn = buttonAtCell(fromRow, fromCol);
    if (!btn) return false;

    btn->setCell(toRow, toCol);
    btn->move(cellTopLeft(toRow, toCol));
    return true;
}
*/


void DraggableGridWidget::clear()
{
    // すべての DraggableButton を削除
    auto buttons = findChildren<DraggableButton*>();
    for (auto btn : buttons) {
        btn->hide();    // 非表示
        btn->setParent(nullptr); // QWidget から切り離す
        delete btn;     // メモリ解放
    }
}
