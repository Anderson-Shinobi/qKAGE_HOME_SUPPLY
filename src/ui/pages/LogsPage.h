#pragma once

#include <functional>

#include <QWidget>

#include "../../controllers/ControllerResult.h"

class InfoCard;
class FeedbackBanner;
class QLabel;
class StatusBadge;
class QComboBox;
class QTableWidget;

class LogsPage : public QWidget {
public:
    explicit LogsPage(QWidget *parent = nullptr);
    void refreshData();

    void setLogsHandler(std::function<ControllerResult(const QString &)> handler);
    void setOpenLogsFolderHandler(std::function<bool()> handler);

private:
    InfoCard *entriesCard_ = nullptr;
    InfoCard *levelCard_ = nullptr;
    StatusBadge *statusBadge_ = nullptr;
    FeedbackBanner *feedbackBanner_ = nullptr;
    QLabel *messageLabel_ = nullptr;
    QComboBox *levelFilter_ = nullptr;
    QTableWidget *logsTable_ = nullptr;
    std::function<ControllerResult(const QString &)> logsHandler_;
    std::function<bool()> openLogsFolderHandler_;

    void clearView();
    void openLogsFolder();
    void renderLogs(const QString &payload);
    void showMessage(const QString &message);
};
