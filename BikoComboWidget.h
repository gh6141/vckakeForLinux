#pragma once
#include "BaseComboWidget.h"


class BikoComboWidget : public BaseComboWidget
{
    Q_OBJECT
public:
    QString tableName() const override { return "Biko"; }
    QString displayColumn() const override { return "biko"; }
};
