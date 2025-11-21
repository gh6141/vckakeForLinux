#include "MultiSortProxy.h"
#include <QDate>    // ← これが必要

MultiSortProxy::MultiSortProxy(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool MultiSortProxy::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    int dateCol = 1;
    int numCol  = 0;


    QVariant leftDateVar  = sourceModel()->data(sourceModel()->index(left.row(), dateCol));
    QVariant rightDateVar = sourceModel()->data(sourceModel()->index(right.row(), dateCol));

    // "/" フォーマットで QDate に変換
    QDate leftDate  = QDate::fromString(leftDateVar.toString(), "yyyy/MM/dd");
    QDate rightDate = QDate::fromString(rightDateVar.toString(), "yyyy/MM/dd");

    if (leftDate != rightDate)
        return leftDate < rightDate;  // QDate なら正しく比較される


    int leftNum  = sourceModel()->data(sourceModel()->index(left.row(), numCol)).toInt();
    int rightNum = sourceModel()->data(sourceModel()->index(right.row(), numCol)).toInt();

    return leftNum < rightNum;
}
