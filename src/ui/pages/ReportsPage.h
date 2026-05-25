#pragma once

#include <functional>

#include <QWidget>

#include "../../controllers/ControllerResult.h"

class InfoCard;
class FeedbackBanner;
class QLabel;
class StatusBadge;
class QTextEdit;

class ReportsPage : public QWidget {
public:
    explicit ReportsPage(QWidget *parent = nullptr);
    void refreshData();

    void setMonthlyReportHandler(std::function<ControllerResult()> handler);
    void setExportReportHandler(std::function<ControllerResult()> handler);
    void setOpenReportsFolderHandler(std::function<bool()> handler);

private:
    InfoCard *statusCard_ = nullptr;
    InfoCard *sectionsCard_ = nullptr;
    InfoCard *exportCard_ = nullptr;
    StatusBadge *statusBadge_ = nullptr;
    FeedbackBanner *feedbackBanner_ = nullptr;
    QLabel *messageLabel_ = nullptr;
    QTextEdit *reportView_ = nullptr;
    std::function<ControllerResult()> monthlyReportHandler_;
    std::function<ControllerResult()> exportReportHandler_;
    std::function<bool()> openReportsFolderHandler_;

    void exportMarkdown();
    void openReportsFolder();
    void showError(const QString &message);
    void showNoDataMessage();
    bool hasCompleteReportSections(const QString &payload) const;
};
