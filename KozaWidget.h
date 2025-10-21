// KozaWidget.h
#pragma once
#include "BaseTableWidget.h"

class KozaWidget : public BaseTableWidget
{
    Q_OBJECT
public:
    explicit KozaWidget(QWidget* parent = nullptr);

protected slots:
    void addRow() override; // 空行追加＋列固有初期値
};
