#pragma once

#include <QWidget>

class QLabel;
class StatusBadge;

class FeedbackBanner : public QWidget {
public:
    explicit FeedbackBanner(QWidget *parent = nullptr);

    void showSuccess(const QString &message);
    void showWarning(const QString &message);
    void showError(const QString &message);
    void showNoData(const QString &message);
    void clear();

private:
    StatusBadge *badge_ = nullptr;
    QLabel *messageLabel_ = nullptr;

    void setFeedback(const QString &status, const QString &message);
};
