#pragma once
#include "BaseComboWidget.h"


class ShiharaisakiComboWidget : public BaseComboWidget
{
    Q_OBJECT
public:

    QString tableName() const override { return "shiharaisakimoto"; }
    QString displayColumn() const override { return "shiharaisaki"; }
};
