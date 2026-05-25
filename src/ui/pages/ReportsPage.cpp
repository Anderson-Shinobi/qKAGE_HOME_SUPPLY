#include "ReportsPage.h"

#include <QByteArray>
#include <QDesktopServices>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QUrl>
#include <QVBoxLayout>

#include <filesystem>
#include <utility>

#include "../../controllers/ReportController.h"
#include "../widgets/FeedbackBanner.h"
#include "../widgets/InfoCard.h"
#include "../widgets/StatusBadge.h"

ReportsPage::ReportsPage(QWidget *parent)
    : QWidget(parent)
{
    monthlyReportHandler_ = []() {
        ReportController controller;
        QByteArray programName("qkage-ui");
        QByteArray commandName("monthly-report");
        char *argv[] = {programName.data(), commandName.data()};
        return controller.monthlyReport(2, argv);
    };

    exportReportHandler_ = []() {
        ReportController controller;
        QByteArray programName("qkage-ui");
        QByteArray commandName("export-report");
        char *argv[] = {programName.data(), commandName.data()};
        return controller.exportReport(2, argv);
    };

    openReportsFolderHandler_ = []() {
        const QUrl reportsUrl = QUrl::fromLocalFile(QString::fromStdString(std::filesystem::absolute("reports").string()));
        return QDesktopServices::openUrl(reportsUrl);
    };

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(28, 28, 28, 28);
    root->setSpacing(20);

    QLabel *title = new QLabel("Relatórios", this);
    title->setObjectName("PageTitle");

    QPushButton *refreshButton = new QPushButton("Atualizar relatório", this);
    refreshButton->setObjectName("PrimaryActionButton");
    QPushButton *exportButton = new QPushButton("Exportar Markdown", this);
    QPushButton *openFolderButton = new QPushButton("Abrir pasta de relatórios", this);

    QHBoxLayout *header = new QHBoxLayout();
    header->addWidget(title);
    header->addStretch(1);
    header->addWidget(refreshButton);
    header->addWidget(exportButton);
    header->addWidget(openFolderButton);
    root->addLayout(header);

    statusBadge_ = new StatusBadge("INFO", this);
    statusBadge_->setObjectName("ReportsStatusBadge");
    feedbackBanner_ = new FeedbackBanner(this);
    root->addWidget(statusBadge_);
    root->addWidget(feedbackBanner_);

    QGridLayout *cards = new QGridLayout();
    cards->setSpacing(16);
    statusCard_ = new InfoCard("Relatório mensal", "Sem dados", "Aguardando atualização", this);
    sectionsCard_ = new InfoCard("Seções", "0/5", "Estoque, economia, cofrinhos, investimento e resumo", this);
    exportCard_ = new InfoCard("Markdown", "Não exportado", "Use o fluxo do controller", this);
    cards->addWidget(statusCard_, 0, 0);
    cards->addWidget(sectionsCard_, 0, 1);
    cards->addWidget(exportCard_, 0, 2);
    root->addLayout(cards);

    messageLabel_ = new QLabel("Ainda não há dados suficientes para gerar um relatório completo.", this);
    messageLabel_->setObjectName("PanelPlaceholder");
    messageLabel_->setAlignment(Qt::AlignCenter);
    root->addWidget(messageLabel_);

    reportView_ = new QTextEdit(this);
    reportView_->setObjectName("MonthlyReportView");
    reportView_->setReadOnly(true);
    reportView_->setMinimumHeight(320);
    reportView_->setPlaceholderText("Relatório mensal consolidado");
    root->addWidget(reportView_, 1);

    connect(refreshButton, &QPushButton::clicked, this, [this]() {
        refreshData();
    });
    connect(exportButton, &QPushButton::clicked, this, [this]() {
        exportMarkdown();
    });
    connect(openFolderButton, &QPushButton::clicked, this, [this]() {
        openReportsFolder();
    });

    refreshData();
}

void ReportsPage::refreshData()
{
    const ControllerResult result = monthlyReportHandler_();
    if (!result.success) {
        const QString message = !result.message.empty()
            ? QString::fromStdString(result.message).trimmed()
            : "Não foi possível carregar o relatório.";
        showError(message);
        return;
    }

    const QString payload = QString::fromStdString(result.payload).trimmed();
    if (!hasCompleteReportSections(payload)) {
        showNoDataMessage();
        return;
    }

    reportView_->setPlainText(payload);
    reportView_->setVisible(true);
    messageLabel_->setVisible(false);
    statusCard_->setValue("Atualizado");
    statusCard_->setStatus("Relatório consolidado carregado");
    statusBadge_->setStatus("OK");
    feedbackBanner_->showSuccess("Relatório mensal consolidado carregado.");
    sectionsCard_->setValue("5/5");
    sectionsCard_->setStatus("Seções disponíveis");
}

void ReportsPage::setMonthlyReportHandler(std::function<ControllerResult()> handler)
{
    monthlyReportHandler_ = std::move(handler);
}

void ReportsPage::setExportReportHandler(std::function<ControllerResult()> handler)
{
    exportReportHandler_ = std::move(handler);
}

void ReportsPage::setOpenReportsFolderHandler(std::function<bool()> handler)
{
    openReportsFolderHandler_ = std::move(handler);
}

void ReportsPage::exportMarkdown()
{
    const ControllerResult result = exportReportHandler_();
    if (!result.success) {
        const QString message = !result.message.empty()
            ? QString::fromStdString(result.message).trimmed()
            : "Não foi possível exportar o relatório.";
        showError(message);
        return;
    }

    exportCard_->setValue("Exportado");
    exportCard_->setStatus(QString::fromStdString(result.payload).trimmed());
    feedbackBanner_->showSuccess("Exportação Markdown concluída via controller.");
}

void ReportsPage::openReportsFolder()
{
    if (!openReportsFolderHandler_()) {
        QMessageBox::information(this, "Relatórios", "Não foi possível abrir a pasta de relatórios.");
    }
}

void ReportsPage::showError(const QString &message)
{
    reportView_->clear();
    reportView_->setVisible(false);
    messageLabel_->setText(message);
    messageLabel_->setVisible(true);
    statusCard_->setValue("Erro");
    statusCard_->setStatus("Falha ao carregar");
    statusBadge_->setStatus("ERROR");
    feedbackBanner_->showError(message);
}

void ReportsPage::showNoDataMessage()
{
    reportView_->clear();
    reportView_->setVisible(false);
    messageLabel_->setText("Ainda não há dados suficientes para gerar um relatório completo.");
    messageLabel_->setVisible(true);
    statusCard_->setValue("Sem dados");
    statusCard_->setStatus("Aguardando dados");
    sectionsCard_->setValue("0/5");
    statusBadge_->setStatus("INFO");
    feedbackBanner_->showNoData("Ainda não há dados suficientes para gerar um relatório completo.");
}

bool ReportsPage::hasCompleteReportSections(const QString &payload) const
{
    return payload.contains("[ESTOQUE]") &&
           payload.contains("[ECONOMIA]") &&
           payload.contains("[COFRINHOS]") &&
           payload.contains("[INVESTIMENTO]") &&
           payload.contains("[RESUMO FINAL]") &&
           !payload.contains("Total de itens cadastrados: 0") &&
           !payload.contains("Cofrinhos cadastrados: 0");
}
