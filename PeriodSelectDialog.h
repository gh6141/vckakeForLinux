#include <QDialog>
#include <QDate>
#include <QComboBox>



class PeriodSelectDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PeriodSelectDialog(QWidget *parent = nullptr);

    QDate startDate() const;
    QDate endDate() const;
    void disp();

private:
    QComboBox *cmbStartYear;
    QComboBox *cmbStartMonth;
    QComboBox *cmbEndYear;
    QComboBox *cmbEndMonth;
};
