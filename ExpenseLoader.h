

#pragma once
#include <QObject>
#include <QVector>
#include <QString>
#include <functional>

struct Expense {
    int id;
    double amount;
    QString memo;
    QString date;
    QString category;
    QString payee;
    QString method;
};

class QNetworkAccessManager;

class ExpenseLoader : public QObject
{
    Q_OBJECT
public:
    explicit ExpenseLoader(QObject* parent = nullptr);
    void loadFromApi(const QString& apiUrl, std::function<void(QVector<Expense>)> callback);

private:
    QNetworkAccessManager* manager;
};

