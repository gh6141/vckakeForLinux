#pragma once
#include "BaseTableWidget.h"

class HimokuWidget : public BaseTableWidget
{
    Q_OBJECT
public:
    explicit HimokuWidget(QWidget* parent = nullptr);

protected slots:
    void addRow() override; // 空行追加＋列固有初期値
};
