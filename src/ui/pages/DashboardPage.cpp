#include "DashboardPage.h"

#include <QByteArray>
#include <QColor>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QSizePolicy>
#include <QStringList>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include <utility>

#include "../../controllers/ReportController.h"
#include "../../controllers/SystemController.h"
#include "../UiDataPaths.h"
#include "../widgets/FeedbackBanner.h"
#include "../widgets/InfoCard.h"
#include "../widgets/OperationalChartWidget.h"
#include "../widgets/StatusBadge.h"

class DashboardGauge : public QWidget {
public:
    explicit DashboardGauge(const QString &title, QWidget *parent = nullptr)
        : QWidget(parent), title_(title)
    {
        setMinimumSize(210, 150);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setToolTip(title + ": indicador percentual calculado para acompanhamento operacional.");
    }

    void setValue(double value, const QString &detail)
    {
        value_ = std::clamp(value, 0.0, 100.0);
        detail_ = detail;
        update();
    }

    double value() const
    {
        return value_;
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        const QRectF bounds = rect().adjusted(18, 16, -18, -12);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#151c24"));
        painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 12, 12);

        painter.setPen(QPen(QColor("#2d3743"), 1));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 12, 12);

        painter.setPen(QColor("#f28c28"));
        QFont titleFont = painter.font();
        titleFont.setBold(true);
        titleFont.setPointSize(9);
        painter.setFont(titleFont);
        painter.drawText(bounds.left(), bounds.top(), bounds.width(), 20, Qt::AlignLeft | Qt::AlignVCenter, title_);

        const QRectF arcRect(bounds.left() + 22, bounds.top() + 34, bounds.width() - 44, 96);
        const int startAngle = 180 * 16;
        const int spanAngle = -180 * 16;

        QPen basePen(QColor("#283442"), 10, Qt::SolidLine, Qt::RoundCap);
        painter.setPen(basePen);
        painter.drawArc(arcRect, startAngle, spanAngle);

        QPen valuePen(QColor("#f28c28"), 10, Qt::SolidLine, Qt::RoundCap);
        painter.setPen(valuePen);
        painter.drawArc(arcRect, startAngle, static_cast<int>(spanAngle * (value_ / 100.0)));

        painter.setPen(QColor("#f2f5f7"));
        QFont valueFont = painter.font();
        valueFont.setBold(true);
        valueFont.setPointSize(18);
        painter.setFont(valueFont);
        painter.drawText(arcRect, Qt::AlignCenter, QString::number(std::round(value_)) + "%");

        painter.setPen(QColor("#c1cad4"));
        QFont detailFont = painter.font();
        detailFont.setBold(false);
        detailFont.setPointSize(8);
        painter.setFont(detailFont);
        painter.drawText(bounds.left(), bounds.bottom() - 18, bounds.width(), 18, Qt::AlignCenter, detail_);
    }

private:
    QString title_;
    QString detail_ = "Sem dados";
    double value_ = 0.0;
};

namespace {
QString extractValue(const QString &payload, const QString &label)
{
    const QString prefix = label + ": ";
    const QStringList lines = payload.split('\n');
    for (const QString &line : lines) {
        if (line.startsWith(prefix)) {
            return line.mid(prefix.size()).trimmed();
        }
    }

    return "0";
}

int numericValue(const QString &value)
{
    bool ok = false;
    const int parsed = value.toInt(&ok);
    return ok ? parsed : 0;
}

double decimalValue(QString value)
{
    value.remove("R$");
    value.remove("%");
    value = value.trimmed();
    bool ok = false;
    const double parsed = value.toDouble(&ok);
    return ok ? parsed : 0.0;
}

QString overallStatus(int totalItems, int criticalItems, int attentionItems)
{
    if (criticalItems > 0) {
        return "CRÍTICO";
    }
    if (attentionItems > 0) {
        return "ATENÇÃO";
    }
    return totalItems > 0 ? "OK" : "OK";
}

QString compactOperationalStatus(const QString &status)
{
    if (status == "CRÍTICO" || status == "ATENÇÃO") {
        return "WARNING";
    }
    return "OK";
}

QVector<double> accumulatedSeries(double total)
{
    QVector<double> values;
    values.reserve(30);
    for (int day = 0; day < 30; ++day) {
        const double progress = static_cast<double>(day + 1) / 30.0;
        values.append(total * progress);
    }
    return values;
}

int countLogRows(const QString &payload)
{
    int count = 0;
    for (const QString &line : payload.split('\n')) {
        if (line.startsWith('[') && line.contains("] [")) {
            ++count;
        }
    }
    return count;
}

QString lastBackupFromLogs(const QString &payload)
{
    const QRegularExpression timestampPattern(R"(^\[([^\]]+)\])");
    QString last = "Não identificado";
    for (const QString &line : payload.split('\n')) {
        if (!line.contains("Backup", Qt::CaseInsensitive)) {
            continue;
        }

        const QRegularExpressionMatch match = timestampPattern.match(line);
        last = match.hasMatch() ? match.captured(1) : "Registro encontrado";
    }
    return last;
}
}

DashboardPage::DashboardPage(QWidget *parent)
    : QWidget(parent)
{
    reportHandler_ = []() {
        ReportController controller;
        QByteArray programName("qkage-ui");
        QByteArray commandName("monthly-report");
        QByteArray stockPath = qkageDataPath("estoque.csv").toUtf8();
        QByteArray piggyPath = qkageDataPath("piggybanks.csv").toUtf8();
        char *argv[] = {programName.data(), commandName.data(), stockPath.data(), piggyPath.data()};
        return controller.monthlyReport(4, argv);
    };

    logsHandler_ = [](const QString &level) {
        SystemController controller;
        QByteArray programName("qkage-ui");
        QByteArray commandName("logs-show");
        QByteArray levelBytes = level.toUtf8();
        if (level.isEmpty()) {
            char *argv[] = {programName.data(), commandName.data()};
            return controller.logsShow(2, argv);
        }

        char *argv[] = {programName.data(), commandName.data(), levelBytes.data()};
        return controller.logsShow(3, argv);
    };

    QVBoxLayout *pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(0);

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setObjectName("DashboardScrollArea");
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QWidget *content = new QWidget(scrollArea);
    content->setObjectName("DashboardContent");
    content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QVBoxLayout *root = new QVBoxLayout(content);
    root->setContentsMargins(30, 26, 30, 26);
    root->setSpacing(22);

    QLabel *title = new QLabel("Dashboard", content);
    title->setObjectName("PageTitle");

    QPushButton *refreshButton = new QPushButton("Atualizar Dashboard", content);
    refreshButton->setObjectName("PrimaryActionButton");
    refreshButton->setMinimumHeight(38);

    QHBoxLayout *header = new QHBoxLayout();
    header->addWidget(title);
    header->addStretch(1);
    QLabel *statusLabel = new QLabel("Status operacional", content);
    statusLabel->setObjectName("InfoCardStatus");
    statusLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    systemStatusBadge_ = new StatusBadge("OK", content);
    systemStatusBadge_->setObjectName("DashboardStatusBadge");
    systemStatusBadge_->setToolTip("Indicador operacional compacto calculado a partir dos dados do dashboard.");
    header->addWidget(statusLabel);
    header->addWidget(systemStatusBadge_);
    header->addWidget(refreshButton);
    root->addLayout(header);

    QGridLayout *cards = new QGridLayout();
    cards->setHorizontalSpacing(16);
    cards->setVerticalSpacing(18);
    cards->setRowMinimumHeight(0, 156);
    cards->setRowMinimumHeight(1, 156);
    cards->setColumnStretch(0, 1);
    cards->setColumnStretch(1, 1);
    cards->setColumnStretch(2, 1);

    totalItemsCard_ = new InfoCard("Total de itens", "0", "Estoque monitorado", "#", content);
    criticalItemsCard_ = new InfoCard("Itens críticos", "0", "Autonomia baixa", "!", content);
    attentionItemsCard_ = new InfoCard("Itens em atenção", "0", "Monitoramento", "~", content);
    estimatedSavingsCard_ = new InfoCard("Economia operacional", "R$ 0.00", "Mensal", "$", content);
    pendingPurchasesCard_ = new InfoCard("Compras pendentes", "0", "Reposição automática", "C", content);
    piggyBanksCard_ = new InfoCard("Cofrinhos ativos", "0", "Metas", "%", content);
    accumulatedCapitalCard_ = new InfoCard("Total acumulado", "0%", "Cofrinhos", "A", content);
    projectedCapitalCard_ = new InfoCard("Capital projetado", "R$ 0.00", "Investimentos", ">", content);

    totalItemsCard_->setToolTip("Quantidade total de itens cadastrados no estoque.");
    criticalItemsCard_->setToolTip("Itens com autonomia crítica no relatório consolidado.");
    estimatedSavingsCard_->setToolTip("Economia estimada informada pelo relatório mensal.");
    piggyBanksCard_->setToolTip("Quantidade de cofrinhos monitorados.");

    cards->addWidget(totalItemsCard_, 0, 0);
    cards->addWidget(criticalItemsCard_, 0, 1);
    cards->addWidget(attentionItemsCard_, 0, 2);
    cards->addWidget(estimatedSavingsCard_, 1, 0);
    cards->addWidget(pendingPurchasesCard_, 1, 1);
    cards->addWidget(piggyBanksCard_, 1, 2);
    cards->addWidget(accumulatedCapitalCard_, 2, 0);
    cards->addWidget(projectedCapitalCard_, 2, 1);
    root->addLayout(cards);

    feedbackBanner_ = new FeedbackBanner(content);
    feedbackBanner_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    root->addWidget(feedbackBanner_);

    QVBoxLayout *chartHeader = new QVBoxLayout();
    chartHeader->setContentsMargins(0, 0, 0, 0);
    chartHeader->setSpacing(2);
    QLabel *chartTitle = new QLabel("Consumo mensal", content);
    chartTitle->setObjectName("PageSectionTitle");
    QLabel *chartSubtitle = new QLabel("Últimos 30 dias", content);
    chartSubtitle->setObjectName("InfoCardStatus");
    chartHeader->addWidget(chartTitle);
    chartHeader->addWidget(chartSubtitle);
    root->addLayout(chartHeader);

    consumptionChart_ = new OperationalChartWidget(content);
    consumptionChart_->setObjectName("DashboardOperationalChart");
    consumptionChart_->setPlaceholder("Aguardando dados operacionais.");
    root->addWidget(consumptionChart_);

    QHBoxLayout *gauges = new QHBoxLayout();
    gauges->setSpacing(16);
    stockHealthGauge_ = new DashboardGauge("Saúde do estoque", content);
    stockHealthGauge_->setObjectName("stockHealthGauge");
    operationalEfficiencyGauge_ = new DashboardGauge("Eficiência operacional", content);
    operationalEfficiencyGauge_->setObjectName("operationalEfficiencyGauge");
    freeCapitalGauge_ = new DashboardGauge("Capital livre estimado", content);
    freeCapitalGauge_->setObjectName("freeCapitalGauge");
    gauges->addWidget(stockHealthGauge_);
    gauges->addWidget(operationalEfficiencyGauge_);
    gauges->addWidget(freeCapitalGauge_);
    root->addLayout(gauges);

    QLabel *summaryTitle = new QLabel("Resumo operacional", content);
    summaryTitle->setObjectName("PageSectionTitle");
    summaryTitle->setToolTip("Visão rápida de sinais operacionais vindos dos controllers.");
    root->addWidget(summaryTitle);

    QGridLayout *operationalSummary = new QGridLayout();
    operationalSummary->setHorizontalSpacing(16);
    operationalSummary->setVerticalSpacing(18);
    operationalSummary->setRowMinimumHeight(0, 156);
    operationalSummary->setColumnStretch(0, 1);
    operationalSummary->setColumnStretch(1, 1);
    operationalSummary->setColumnStretch(2, 1);
    operationalSummary->setColumnStretch(3, 1);
    lastBackupCard_ = new InfoCard("Último backup", "Não identificado", "Logs operacionais", "B", content);
    errorLogsCard_ = new InfoCard("Logs ERROR", "0", "Últimas entradas", "E", content);
    operationalCriticalCard_ = new InfoCard("Itens críticos", "0", "Resumo operacional", "!", content);
    operationalStatusCard_ = new InfoCard("Estado geral", "Sem dados", "Sistema", "*", content);
    lastBackupCard_->setToolTip("Último registro de backup encontrado nos logs operacionais.");
    errorLogsCard_->setToolTip("Quantidade de entradas ERROR retornadas pelo controller de logs.");
    operationalCriticalCard_->setToolTip("Itens críticos informados pelo relatório consolidado.");
    operationalStatusCard_->setToolTip("Estado geral do sistema no dashboard.");
    operationalSummary->addWidget(lastBackupCard_, 0, 0);
    operationalSummary->addWidget(errorLogsCard_, 0, 1);
    operationalSummary->addWidget(operationalCriticalCard_, 0, 2);
    operationalSummary->addWidget(operationalStatusCard_, 0, 3);
    root->addLayout(operationalSummary);

    connect(refreshButton, &QPushButton::clicked, this, [this]() {
        refreshData();
    });

    scrollArea->setWidget(content);
    pageLayout->addWidget(scrollArea);

    refreshData();
}

void DashboardPage::refreshData()
{
    const ControllerResult result = reportHandler_();
    const ControllerResult allLogsResult = logsHandler_("");
    const ControllerResult errorLogsResult = logsHandler_("ERROR");
    if (!result.success) {
        systemStatusBadge_->setStatus("ERROR");
        operationalStatusCard_->setValue("Erro");
        operationalStatusCard_->setStatus("Falha ao carregar dados");
        stockHealthGauge_->setValue(0.0, "Falha ao carregar");
        operationalEfficiencyGauge_->setValue(0.0, "Falha ao carregar");
        freeCapitalGauge_->setValue(0.0, "Falha ao carregar");
        consumptionChart_->setPlaceholder("Aguardando dados operacionais.");
        feedbackBanner_->showError("Erro ao carregar indicadores via controller.");
        return;
    }

    const QString payload = QString::fromStdString(result.payload);
    const QString totalItems = extractValue(payload, "Total de itens cadastrados");
    const QString criticalItems = extractValue(payload, "Itens CRÍTICO");
    const QString attentionItems = extractValue(payload, "Itens ATENÇÃO");
    const QString estimatedSavings = extractValue(payload, "Economia total estimada");
    const QString piggyBanks = extractValue(payload, "Cofrinhos cadastrados");
    const QString freeCapital = extractValue(payload, "Capital liberado para investimento");
    const QString projectedCapital = extractValue(payload, "Valor final estimado");
    const QString piggyPercentage = extractValue(payload, "Percentual medio das metas");

    totalItemsCard_->setValue(totalItems);
    criticalItemsCard_->setValue(criticalItems);
    attentionItemsCard_->setValue(attentionItems);
    estimatedSavingsCard_->setValue(estimatedSavings);
    piggyBanksCard_->setValue(piggyBanks);

    const int totalCount = numericValue(totalItems);
    const int criticalCount = numericValue(criticalItems);
    const int attentionCount = numericValue(attentionItems);
    const int piggyCount = numericValue(piggyBanks);
    const double freeCapitalValue = decimalValue(freeCapital);
    const double estimatedSavingsValue = decimalValue(estimatedSavings);
    const double piggyGoalPercentage = decimalValue(piggyPercentage);
    const int pendingPurchases = criticalCount + attentionCount;
    const QString status = overallStatus(totalCount, criticalCount, attentionCount);
    const bool hasData = totalCount > 0 || piggyCount > 0;

    pendingPurchasesCard_->setValue(QString::number(pendingPurchases));
    pendingPurchasesCard_->setStatus(pendingPurchases == 0 ? "Sem compras pendentes" : "Reposição operacional");
    accumulatedCapitalCard_->setValue(piggyPercentage);
    accumulatedCapitalCard_->setStatus("Média dos cofrinhos");
    projectedCapitalCard_->setValue(projectedCapital);
    projectedCapitalCard_->setStatus("Juros compostos");
    operationalCriticalCard_->setValue(criticalItems);
    operationalStatusCard_->setValue(status);
    operationalStatusCard_->setStatus(hasData ? "Dados carregados" : "Aguardando cadastro");

    QString statusState = compactOperationalStatus(status);
    if (allLogsResult.success) {
        lastBackupCard_->setValue(lastBackupFromLogs(QString::fromStdString(allLogsResult.payload)));
        lastBackupCard_->setStatus("Controller logs-show");
    } else {
        lastBackupCard_->setValue("Indisponível");
        lastBackupCard_->setStatus("Falha ao consultar logs");
        statusState = "WARNING";
    }

    if (errorLogsResult.success) {
        errorLogsCard_->setValue(QString::number(countLogRows(QString::fromStdString(errorLogsResult.payload))));
        errorLogsCard_->setStatus("Filtro ERROR");
    } else {
        errorLogsCard_->setValue("Erro");
        errorLogsCard_->setStatus("Falha ao consultar logs");
        statusState = "WARNING";
    }

    const double stockHealth = totalCount == 0
        ? 0.0
        : std::max(0.0, 100.0 - ((criticalCount * 34.0 + attentionCount * 16.0) / static_cast<double>(totalCount)));
    const double operationalEfficiency = hasData ? std::clamp(72.0 + piggyGoalPercentage * 0.18 - criticalCount * 8.0, 0.0, 100.0) : 0.0;
    const double capitalGauge = std::clamp(freeCapitalValue, 0.0, 100.0);

    stockHealthGauge_->setValue(stockHealth, totalCount > 0 ? "Baseado em criticidade" : "Sem estoque");
    operationalEfficiencyGauge_->setValue(operationalEfficiency, piggyCount > 0 ? "Metas e estoque" : "Sem cofrinhos");
    freeCapitalGauge_->setValue(capitalGauge, freeCapital);
    if (hasData && estimatedSavingsValue > 0.0) {
        consumptionChart_->setValues(accumulatedSeries(estimatedSavingsValue), "Economia acumulada estimada via relatório mensal");
    } else {
        consumptionChart_->setPlaceholder("Aguardando dados operacionais.");
    }

    systemStatusBadge_->setStatus(statusState);
    if (statusState == "WARNING") {
        feedbackBanner_->showWarning("Indicadores atualizados com alertas operacionais.");
    } else if (hasData) {
        feedbackBanner_->showSuccess("Indicadores atualizados via controller.");
    } else {
        feedbackBanner_->showNoData("Nenhum dado cadastrado ainda.");
    }
}

void DashboardPage::setReportHandler(std::function<ControllerResult()> handler)
{
    reportHandler_ = std::move(handler);
}

void DashboardPage::setLogsHandler(std::function<ControllerResult(const QString &)> handler)
{
    logsHandler_ = std::move(handler);
}
