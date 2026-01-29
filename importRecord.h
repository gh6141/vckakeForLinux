#pragma once
#include <QDate>
#include <QString>
#include <QDebug>

struct importRecord {
    QDate date;
    int payment;
    int deposit;
    int balance;
    QString type;
    QString summary;
};


inline QDebug operator<<(QDebug dbg, const importRecord& r)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace()
        << "importRecord("
        << "date=" << r.date.toString("yyyy-MM-dd") << ", "
        << "payment=" << r.payment << ", "
        << "deposit=" << r.deposit << ", "
        << "balance=" << r.balance << ", "
        << "type=" << r.type << ", "
        << "summary=" << r.summary
        << ")";
    return dbg;
}
