#pragma once

#include <functional>

#include <QWidget>

#include "../../controllers/ControllerResult.h"

class InfoCard;
class FeedbackBanner;
class QLabel;
class StatusBadge;
class QTableWidget;

class SettingsPage : public QWidget {
public:
    explicit SettingsPage(QWidget *parent = nullptr);
    void refreshData();

    void setConfigHandler(std::function<ControllerResult()> handler);
    void setOpenFolderHandler(std::function<bool(const QString &)> handler);

private:
    InfoCard *statusCard_ = nullptr;
    InfoCard *systemCard_ = nullptr;
    InfoCard *pathsCard_ = nullptr;
    StatusBadge *statusBadge_ = nullptr;
    FeedbackBanner *feedbackBanner_ = nullptr;
    QLabel *messageLabel_ = nullptr;
    QTableWidget *settingsTable_ = nullptr;
    std::function<ControllerResult()> configHandler_;
    std::function<bool(const QString &)> openFolderHandler_;

    void renderConfig(const QString &payload);
    void showError(const QString &message);
    void openFolder(const QString &path);
    QString configValue(const QString &payload, const QString &key) const;
};
