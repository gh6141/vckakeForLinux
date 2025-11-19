#pragma once
#include <QWidget>
#include <QLabel>
class DraggableButton;

class DraggableGridWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DraggableGridWidget(int rows, int cols, QWidget *parent = nullptr);

    void addButton(DraggableButton *btn, int row, int col);
    void addButton(QString text, int row, int col);
    DraggableButton* buttonAtCell(int row, int col) const;
    void clear();
    bool moveButton(int fromRow, int fromCol, int toRow, int toCol);
    DraggableButton* buttonAt(int row, int col) const;
    int rowCount() const { return m_rows; }
    int colCount() const { return m_cols; }

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    QPoint cellTopLeft(int row, int col) const;

    int m_rows;
    int m_cols;
    const int cellH = 50; // 固定セル高さ
    QMap<QPair<int,int>, DraggableButton*> btnMap;
};

