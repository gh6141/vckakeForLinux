#include "ExpenseLoader.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <QUrl>

ExpenseLoader::ExpenseLoader(QObject* parent)
    : QObject(parent)
{
    manager = new QNetworkAccessManager(this);
}

void ExpenseLoader::loadFromApi(const QString& apiUrl, std::function<void(QVector<Expense>)> callback)
{
    // QNetworkRequest apiRequest(QUrl(apiUrl));
    QNetworkRequest apiRequest{QUrl(apiUrl)};
    QNetworkReply* reply = manager->get(apiRequest);  // ← ここがポイント


    connect(reply, &QNetworkReply::finished, this, [reply, callback]() {
        QVector<Expense> expenses;

        if (reply->error() != QNetworkReply::NoError) {
            // QMessageBox::warning(nullptr, "API接続エラー", reply->errorString());
            reply->deleteLater();
            callback(expenses);
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        if (!doc.isArray()) {
            reply->deleteLater();
            callback(expenses);
            return;
        }

        QJsonArray arr = doc.array();
        for (const auto& v : arr) {
            if (!v.isObject()) continue;
            QJsonObject obj = v.toObject();
            Expense e;
            e.id = obj.value("id").toInt();
            e.amount = obj.value("amount").toDouble();
            e.memo = obj.value("memo").toString();
            e.date = obj.value("date").toString();
            e.category = obj.value("category").toString();
            e.payee = obj.value("").toString();
            e.method = obj.value("method").toString();
            expenses.append(e);
        }

        reply->deleteLater();
        callback(expenses);
    });
}

