#include "backupmanager.h"
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QDateTime>
#include <QFileInfoList>

BackupManager::BackupManager(const QString &backupDir, int retainDays, QObject *parent)
    : QObject(parent),
    m_backupDir(backupDir),
    m_retainDays(retainDays)
{
    QDir dir(m_backupDir);
    if (!dir.exists()) {
        dir.mkpath("."); // フォルダ作成
    }
}

bool BackupManager::backupDatabase(const QString &dbPath)
{
    QDir dir(m_backupDir);

    // バックアップファイル名
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString backupFileName = QString("backup_%1.db").arg(timestamp);
    QString backupFilePath = dir.filePath(backupFileName);

    // DBコピー
    if (!QFile::copy(dbPath, backupFilePath)) {
        qDebug() << "バックアップ作成失敗:" << backupFilePath;
        return false;
    }

    qDebug() << "バックアップ作成:" << backupFilePath;

    // 古いバックアップ削除
    cleanupOldBackups();

    return true;
}

void BackupManager::cleanupOldBackups()
{
    QDir dir(m_backupDir);
    QFileInfoList files = dir.entryInfoList(QStringList() << "backup_*.db", QDir::Files, QDir::Time);
    QDateTime now = QDateTime::currentDateTime();

    for (const QFileInfo &fi : files) {
        QDateTime fileTime = fi.lastModified(); // Qtバージョン依存回避
        if (fileTime.daysTo(now) > m_retainDays) {
            if (QFile::remove(fi.absoluteFilePath()))
                qDebug() << "古いバックアップ削除:" << fi.fileName();
            else
                qWarning() << "古いバックアップ削除失敗:" << fi.fileName();
        }
    }
}

bool BackupManager::createUndoBackup(const QString &dbPath)
{
    QDir dir(m_backupDir);

    QString undoPath = dir.filePath("undo_latest.db");

    // 既存のundoがあれば削除
    if (QFile::exists(undoPath))
        QFile::remove(undoPath);

    if (!QFile::copy(dbPath, undoPath)) {
        qWarning() << "Undoバックアップ作成失敗";
        return false;
    }

    qDebug() << "Undoバックアップ作成:" << undoPath;
    return true;
}


bool BackupManager::restoreLatestUndo(const QString &dbPath)
{
    QDir dir(m_backupDir);
    QString undoPath = dir.filePath("undo_latest.db");

    qDebug() << "=== Undo復元開始 ===";
    qDebug() << "undoPath =" << undoPath;
    qDebug() << "dbPath   =" << dbPath;

    if (!QFile::exists(undoPath)) {
        qWarning() << "★ undo_latest.db が無い";
        return false;
    }

    if (!QFile::exists(dbPath)) {
        qWarning() << "★ 現行DBが無い:" << dbPath;
        return false;
    }

    // いまのDBをBadとして退避（任意だが推奨）
    QString badPath = dir.filePath(
        "bad_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".db");

    if (!QFile::copy(dbPath, badPath)) {
        qWarning() << "★ Bad退避に失敗:" << badPath;
        // ここで止めるかは設計次第
    } else {
        qDebug() << "Bad退避:" << badPath;
    }

    // ★★★ ここがいちばん怪しいポイント ★★★
    if (!QFile::remove(dbPath)) {
        qWarning() << "★ 現行DBの削除に失敗:" << dbPath;
        qWarning() << "  → おそらくDBが開きっぱなし";
        return false;
    }

    if (!QFile::copy(undoPath, dbPath)) {
        qWarning() << "★ Undoからの復元コピーに失敗";
        return false;
    }

    qDebug() << "=== Undo復元成功 ===";
    return true;
}
