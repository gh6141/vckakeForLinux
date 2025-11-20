#include "WebApiDialog.h"
#include <QLabel>
#include <QDialogButtonBox>
#include <QString>
#include <QVBoxLayout>



WebApiDialog::WebApiDialog(QWidget* parent)
    : QDialog(parent)
{
    urlEdit = new QLineEdit(this);
    auto layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("API URL:", this));
    layout->addWidget(urlEdit);

    auto buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void WebApiDialog::setUrl(const QString& url) {
    urlEdit->setText(url);
}

QString WebApiDialog::getUrl() const {
    return urlEdit->text();
}
