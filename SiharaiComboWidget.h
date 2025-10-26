// KozaComboWidget.h
#include "BaseComboWidget.h"


class SiharaiComboWidget : public BaseComboWidget
{
    Q_OBJECT
public:
    QString tableName() const override { return "shiharaisakimoto"; }
    QString displayColumn() const override { return "shiharaisaki"; }
};
