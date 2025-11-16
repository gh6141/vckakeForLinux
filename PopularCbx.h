#ifndef POPULARCBX_H
#define POPULARCBX_H

#include <QObject>
#include <QComboBox>
#include <QList>
#include <QString>
#include <QStringList>

// データ構造
struct ThreeRelationShip {
    QString himoku;
    QString shiharaiSakiMoto;
    QString biko;
};

class PopularCbx : public QObject
{
    Q_OBJECT
public:
    explicit PopularCbx(QObject* parent = nullptr);

    // データ設定
    void setDataList(const QList<ThreeRelationShip>& list);

    // フィルタON/OFF切替
    void setFilterEnabled(bool enabled);

    // 共通コンボ更新（上位コンボ選択→下位コンボ更新）
    //void updateComboBoxes(QComboBox* baseCb, const QList<QComboBox*>& targetCbs);
    void updateComboBoxes(QComboBox* sender,const QList<QComboBox*>& targetCbs);
    // フィルタOFF時の全件表示
    void populateAll(QComboBox* cb);

private:
    QList<ThreeRelationShip> dataList;  // 全データ
    bool filterEnabled;                  // フィルタON/OFF
    bool updatingCombo;                  // イベント再帰防止用フラグ
};

#endif // POPULARCBX_H
