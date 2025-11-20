#pragma once
#include <QDialog>
#include <QWidget>
#include <QString>
#include <QLineEdit>

class WebApiDialog : public QDialog {
    Q_OBJECT
public:
    explicit WebApiDialog(QWidget* parent = nullptr);
    QString getUrl() const;
    void setUrl(const QString& url);

private:
    QLineEdit* urlEdit;
};
