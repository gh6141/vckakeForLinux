#include "ClearOnClickLineEdit.h"

ClearOnClickLineEdit::ClearOnClickLineEdit(QWidget *parent)
    : QLineEdit(parent)
{
}

void ClearOnClickLineEdit::mousePressEvent(QMouseEvent *event)
{
    clear(); // クリックで内容をクリア
    QLineEdit::mousePressEvent(event); // 親の処理も呼ぶ（カーソル移動など）
}
