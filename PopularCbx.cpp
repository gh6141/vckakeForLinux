#include "PopularCbx.h"
#include <QDebug>
#include <QSet>




PopularCbx::PopularCbx(QObject* parent)
    : QObject(parent), filterEnabled(true), updatingCombo(false)
{
}

// データリスト設定
void PopularCbx::setDataList(const QList<ThreeRelationShip>& list) {
    dataList = list;
}

// フィルタON/OFF切替
void PopularCbx::setFilterEnabled(bool enabled) {
    filterEnabled = enabled;
}

// 共通コンボ更新
void PopularCbx::updateComboBoxes(QComboBox* sender,const QList<QComboBox*>& targetCbs)
{
    if (!filterEnabled) return;

    QString field = sender->property("popularField").toString();
    QString value = sender->currentText().trimmed();

    if (field.isEmpty() || value.isEmpty())
        return;

    // ① sender の値でフィルタ
    QList<ThreeRelationShip> filtered;
    for (const auto& t : dataList)
    {
        if (field == "himoku" && t.himoku == value) filtered << t;
        else if (field == "payee" && t.shiharaiSakiMoto == value) filtered << t;
        else if (field == "biko" && t.biko == value) filtered << t;
    }

    // ② 下位コンボを順に更新
    for (QComboBox* cb : targetCbs)
    {
        if (cb == sender) continue; // 自分はスキップ

        QString currentText = cb->currentText();
        QStringList items;

        QString f = cb->property("popularField").toString();
        if (f.isEmpty()) continue;

        for (const auto& t : filtered) {
            if (f == "himoku") items << t.himoku;
            else if (f == "payee") items << t.shiharaiSakiMoto;
            else if (f == "biko") items << t.biko;
        }

        items.removeDuplicates();

      /*  QString oldText = cb->currentText();  // 現在の文字を保存
        cb->blockSignals(true);  // イベントを一時的に無効化
        cb->clear();
        cb->addItems(items);
        cb->setCurrentText(oldText);  // 元の文字に戻す
        cb->blockSignals(false);  // イベント再開*/
        QString oldText = cb->currentText();
        {
            QSignalBlocker blocker(cb);  // ブロック開始
            cb->clear();
            cb->addItems(items);
            cb->setCurrentText(oldText);
        } // ブロック終了と同時に自動復帰

        /*
        if (!currentText.isEmpty() && items.contains(currentText))
            cb->setCurrentText(currentText);
        else if (!items.isEmpty())
            cb->setCurrentIndex(0);
        */
        //cb->setCurrentIndex(-1);
    }
}




// フィルタOFF時の全件表示
void PopularCbx::populateAll(QComboBox* cb) {

     if (filterEnabled) return;

    QString field = cb->property("popularField").toString();
    if (field.isEmpty()) return;

    QStringList items;
    for (const auto& t : dataList) {
        if (field == "himoku") items << t.himoku;
        else if (field == "payee") items << t.shiharaiSakiMoto;
        else if (field == "biko") items << t.biko;
    }

    items.removeDuplicates();
    cb->clear();
    cb->addItems(items);
}
