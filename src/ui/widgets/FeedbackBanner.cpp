#include "FeedbackBanner.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QSizePolicy>

#include "StatusBadge.h"

FeedbackBanner::FeedbackBanner(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("FeedbackBanner");
    setVisible(false);
    setMinimumHeight(52);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(12);
    layout->setSizeConstraint(QLayout::SetMinimumSize);

    badge_ = new StatusBadge("INFO", this);
    messageLabel_ = new QLabel(this);
    messageLabel_->setObjectName("FeedbackBannerMessage");
    messageLabel_->setWordWrap(true);
    messageLabel_->setMinimumHeight(messageLabel_->sizeHint().height() + 4);
    messageLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    layout->addWidget(badge_);
    layout->addWidget(messageLabel_, 1);

    setStyleSheet(
        "QWidget#FeedbackBanner { background: #111922; border: 1px solid #2d3743; border-radius: 10px; }"
        "QLabel#FeedbackBannerMessage { color: #f2f5f7; font-weight: 600; }");
}

void FeedbackBanner::showSuccess(const QString &message)
{
    setFeedback("OK", message);
}

void FeedbackBanner::showWarning(const QString &message)
{
    setFeedback("ATENÇÃO", message);
}

void FeedbackBanner::showError(const QString &message)
{
    setFeedback("ERROR", message);
}

void FeedbackBanner::showNoData(const QString &message)
{
    setFeedback("INFO", message);
}

void FeedbackBanner::clear()
{
    messageLabel_->clear();
    setVisible(false);
}

void FeedbackBanner::setFeedback(const QString &status, const QString &message)
{
    badge_->setStatus(status);
    messageLabel_->setText(message);
    messageLabel_->setMinimumHeight(messageLabel_->sizeHint().height() + 4);
    setVisible(true);
}
