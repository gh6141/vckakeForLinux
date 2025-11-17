#pragma once
#include <QWidget>
#include "DraggableButton.h"

class DraggableGridWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DraggableGridWidget(int rows, int cols, QWidget *parent = nullptr);
    void addButton(DraggableButton *btn, int row, int col);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    int m_rows;
    int m_cols;

    QPoint cellTopLeft(int row, int col) const;
    DraggableButton* buttonAtCell(int row, int col) const;
};
