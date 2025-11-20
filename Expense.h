#define EXPENSE_H

#include <QString>
#include <QVector>

// Expense データ構造
struct Expense
{
    int id;
    double amount;
    QString memo;
    QString date;
    QString category;
    QString payee;
    QString method;
};

#endif // EXPENSE_H

