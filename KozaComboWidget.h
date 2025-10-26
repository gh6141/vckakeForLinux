// KozaComboWidget.h
#include "BaseComboWidget.h"


class KozaComboWidget : public BaseComboWidget
{
    Q_OBJECT
public:
    QString tableName() const override { return "koza"; }
    QString displayColumn() const override { return "kozaName"; }
};
