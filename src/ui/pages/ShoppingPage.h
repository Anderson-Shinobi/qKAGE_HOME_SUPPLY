#pragma once

#include <QWidget>

class InfoCard;
class FeedbackBanner;
class QLabel;
class StatusBadge;
class QTableWidget;

class ShoppingPage : public QWidget {
public:
    explicit ShoppingPage(QWidget *parent = nullptr);
    void refreshData();

private:
    InfoCard *itemsCard_ = nullptr;
    InfoCard *suggestionCard_ = nullptr;
    StatusBadge *statusBadge_ = nullptr;
    FeedbackBanner *feedbackBanner_ = nullptr;
    QLabel *emptyStateLabel_ = nullptr;
    QTableWidget *shoppingTable_ = nullptr;

    void showActionDialog(const QString &title);
};
