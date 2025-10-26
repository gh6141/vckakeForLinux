// KozaComboWidget.h
#include "BaseComboWidget.h"


class HimokuComboWidget : public BaseComboWidget
{
    Q_OBJECT
public:
    QString tableName() const override { return "Himoku"; }
    QString displayColumn() const override { return "himoku"; }
};
