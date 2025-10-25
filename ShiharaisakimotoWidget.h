#pragma once
#include "BaseTableWidget.h"

class ShiharaisakimotoWidget : public BaseTableWidget
{
    Q_OBJECT
public:
    explicit ShiharaisakimotoWidget(QWidget* parent = nullptr);

protected slots:
    void addRow() override; // 空行追加＋列固有初期値
};
