#pragma once
#include <QSortFilterProxyModel>
#include <QModelIndex>
#include <QVariant>

class MultiSortProxy : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit MultiSortProxy(QObject *parent = nullptr);

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    int dateCol = 1;
    int numCol  = 0;
};
