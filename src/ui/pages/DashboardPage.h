#pragma once

#include <functional>

#include <QWidget>

#include "../../controllers/ControllerResult.h"

class InfoCard;
class FeedbackBanner;
class QLabel;
class OperationalChartWidget;
class StatusBadge;

class DashboardGauge;

class DashboardPage : public QWidget {
public:
    explicit DashboardPage(QWidget *parent = nullptr);
    void refreshData();
    void setReportHandler(std::function<ControllerResult()> handler);
    void setLogsHandler(std::function<ControllerResult(const QString &)> handler);

private:
    InfoCard *totalItemsCard_ = nullptr;
    InfoCard *criticalItemsCard_ = nullptr;
    InfoCard *attentionItemsCard_ = nullptr;
    InfoCard *estimatedSavingsCard_ = nullptr;
    InfoCard *piggyBanksCard_ = nullptr;
    InfoCard *pendingPurchasesCard_ = nullptr;
    InfoCard *accumulatedCapitalCard_ = nullptr;
    InfoCard *projectedCapitalCard_ = nullptr;
    InfoCard *lastBackupCard_ = nullptr;
    InfoCard *errorLogsCard_ = nullptr;
    InfoCard *operationalCriticalCard_ = nullptr;
    InfoCard *operationalStatusCard_ = nullptr;
    StatusBadge *systemStatusBadge_ = nullptr;
    DashboardGauge *stockHealthGauge_ = nullptr;
    DashboardGauge *operationalEfficiencyGauge_ = nullptr;
    DashboardGauge *freeCapitalGauge_ = nullptr;
    OperationalChartWidget *consumptionChart_ = nullptr;
    FeedbackBanner *feedbackBanner_ = nullptr;
    std::function<ControllerResult()> reportHandler_;
    std::function<ControllerResult(const QString &)> logsHandler_;
};
