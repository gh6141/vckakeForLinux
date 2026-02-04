#pragma once
#include <QObject>
#include <QString>

class BackupManager : public QObject
{
    Q_OBJECT
public:
    explicit BackupManager(const QString &backupDir = "backups", int retainDays = 10, QObject *parent = nullptr);

    // DBバックアップ作成
    bool backupDatabase(const QString &dbPath);

    // 古いバックアップ削除
    void cleanupOldBackups();

    bool createUndoBackup(const QString &dbPath);   // 直前バックアップ作成
    bool restoreLatestUndo(const QString &dbPath);  // 直前バックアップ復元

private:
    QString m_backupDir;
    int m_retainDays;  // 最大保持日数（例：10日）
};
