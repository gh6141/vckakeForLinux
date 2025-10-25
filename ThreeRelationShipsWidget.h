#pragma once
#include "BaseTableWidget.h"

class ThreeRelationShipsWidget : public BaseTableWidget
{
    Q_OBJECT
public:
    explicit ThreeRelationShipsWidget(QWidget* parent = nullptr);

protected slots:
    void addRow() override; // 空行追加＋列固有初期値
};
