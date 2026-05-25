#include "InfoCard.h"

#include <QColor>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QSizePolicy>
#include <QVBoxLayout>

InfoCard::InfoCard(const QString &title, const QString &value, const QString &status, QWidget *parent)
    : InfoCard(title, value, status, QString(), parent)
{
}

InfoCard::InfoCard(const QString &title, const QString &value, const QString &status, const QString &iconText, QWidget *parent)
    : QWidget(parent)
{
    setObjectName("InfoCard");
    setMinimumHeight(156);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(18);
    shadow->setOffset(0, 6);
    shadow->setColor(QColor(0, 0, 0, 90));
    setGraphicsEffect(shadow);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 18, 20, 18);
    layout->setSpacing(10);
    layout->setSizeConstraint(QLayout::SetMinimumSize);

    QHBoxLayout *titleRow = new QHBoxLayout();
    titleRow->setContentsMargins(0, 0, 0, 0);
    titleRow->setSpacing(8);

    titleLabel_ = new QLabel(title, this);
    titleLabel_->setObjectName("InfoCardTitle");
    titleLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    titleLabel_->setMinimumHeight(titleLabel_->sizeHint().height() + 2);
    titleRow->addWidget(titleLabel_, 1);

    iconLabel_ = new QLabel(iconText, this);
    iconLabel_->setObjectName("InfoCardIcon");
    iconLabel_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    iconLabel_->setMinimumHeight(iconLabel_->sizeHint().height() + 2);
    iconLabel_->setVisible(!iconText.isEmpty());
    titleRow->addWidget(iconLabel_);

    valueLabel_ = new QLabel(value, this);
    valueLabel_->setObjectName("InfoCardValue");
    valueLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    valueLabel_->setWordWrap(true);
    valueLabel_->setMinimumHeight(38);
    valueLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    statusLabel_ = new QLabel(status, this);
    statusLabel_->setObjectName("InfoCardStatus");
    statusLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    statusLabel_->setWordWrap(true);
    statusLabel_->setMinimumHeight(statusLabel_->sizeHint().height() + 4);
    statusLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    statusLabel_->setVisible(!status.isEmpty());

    layout->addLayout(titleRow);
    layout->addWidget(valueLabel_, 1);
    layout->addWidget(statusLabel_);
}

void InfoCard::setValue(const QString &value)
{
    valueLabel_->setText(value);
}

void InfoCard::setStatus(const QString &status)
{
    statusLabel_->setText(status);
    statusLabel_->setMinimumHeight(statusLabel_->sizeHint().height() + 4);
    statusLabel_->setVisible(!status.isEmpty());
}
