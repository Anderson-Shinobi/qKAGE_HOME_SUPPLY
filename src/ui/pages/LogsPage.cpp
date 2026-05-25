#include "LogsPage.h"

#include <QByteArray>
#include <QColor>
#include <QComboBox>
#include <QDesktopServices>
#include <QFont>
#include <QGridLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QUrl>
#include <QVBoxLayout>

#include <filesystem>
#include <utility>

#include "../../controllers/SystemController.h"
#include "../widgets/FeedbackBanner.h"
#include "../widgets/InfoCard.h"
#include "../widgets/StatusBadge.h"

namespace {
struct LogRow {
    QString timestamp;
    QString level;
    QString module;
    QString message;
};

QList<LogRow> parseLogPayload(const QString &payload)
{
    QList<LogRow> rows;
    const QRegularExpression pattern(R"(^\[(.+)\]\s+\[(INFO|WARNING|ERROR|DEBUG)\]\s+\[([^\]]+)\]\s+(.*)$)");
    const QStringList lines = payload.split('\n');
    for (const QString &line : lines) {
        const QRegularExpressionMatch match = pattern.match(line.trimmed());
        if (!match.hasMatch()) {
            continue;
        }

        rows.append({
            match.captured(1),
            match.captured(2),
            match.captured(3),
            match.captured(4)
        });
    }

    return rows;
}

void applyLevelStyle(QTableWidgetItem *cell, const QString &level)
{
    QColor background("#e8f5e9");
    QColor foreground("#1b5e20");
    if (level == "WARNING") {
        background = QColor("#fff3cd");
        foreground = QColor("#7a4f00");
    } else if (level == "ERROR") {
        background = QColor("#fdecea");
        foreground = QColor("#b71c1c");
    } else if (level == "DEBUG") {
        background = QColor("#e8eef7");
        foreground = QColor("#34506f");
    }

    QFont font = cell->font();
    font.setBold(true);
    cell->setFont(font);
    cell->setTextAlignment(Qt::AlignCenter);
    cell->setBackground(background);
    cell->setForeground(foreground);
}
}

LogsPage::LogsPage(QWidget *parent)
    : QWidget(parent)
{
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

    openLogsFolderHandler_ = []() {
        const QUrl logsUrl = QUrl::fromLocalFile(QString::fromStdString(std::filesystem::absolute("logs").string()));
        return QDesktopServices::openUrl(logsUrl);
    };

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(28, 28, 28, 28);
    root->setSpacing(20);

    QLabel *title = new QLabel("Logs", this);
    title->setObjectName("PageTitle");

    QPushButton *refreshButton = new QPushButton("Atualizar logs", this);
    refreshButton->setObjectName("PrimaryActionButton");
    QPushButton *clearButton = new QPushButton("Limpar visualização", this);
    QPushButton *openFolderButton = new QPushButton("Abrir pasta de logs", this);

    QHBoxLayout *header = new QHBoxLayout();
    header->addWidget(title);
    header->addStretch(1);
    header->addWidget(refreshButton);
    header->addWidget(clearButton);
    header->addWidget(openFolderButton);
    root->addLayout(header);

    QHBoxLayout *filters = new QHBoxLayout();
    filters->setSpacing(12);
    QLabel *filterLabel = new QLabel("Nível", this);
    levelFilter_ = new QComboBox(this);
    levelFilter_->setObjectName("logsLevelFilter");
    levelFilter_->addItems({"Todos", "INFO", "WARNING", "ERROR", "DEBUG"});
    filters->addWidget(filterLabel);
    filters->addWidget(levelFilter_);
    filters->addStretch(1);
    root->addLayout(filters);

    statusBadge_ = new StatusBadge("INFO", this);
    statusBadge_->setObjectName("LogsStatusBadge");
    feedbackBanner_ = new FeedbackBanner(this);
    root->addWidget(statusBadge_);
    root->addWidget(feedbackBanner_);

    QHBoxLayout *summary = new QHBoxLayout();
    summary->setSpacing(16);
    entriesCard_ = new InfoCard("Entradas", "0", "Nenhum log carregado", this);
    levelCard_ = new InfoCard("Filtro", "Todos", "Níveis operacionais", this);
    summary->addWidget(entriesCard_);
    summary->addWidget(levelCard_);
    root->addLayout(summary);

    QGridLayout *grid = new QGridLayout();
    logsTable_ = new QTableWidget(this);
    logsTable_->setObjectName("LogsTable");
    logsTable_->setColumnCount(4);
    logsTable_->setHorizontalHeaderLabels({"timestamp", "nível", "módulo", "mensagem"});
    logsTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    logsTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    logsTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    logsTable_->setAlternatingRowColors(true);
    logsTable_->verticalHeader()->setVisible(false);
    logsTable_->verticalHeader()->setDefaultSectionSize(36);
    logsTable_->horizontalHeader()->setStretchLastSection(true);

    messageLabel_ = new QLabel("Nenhum log encontrado.", this);
    messageLabel_->setObjectName("PanelPlaceholder");
    messageLabel_->setAlignment(Qt::AlignCenter);

    grid->addWidget(logsTable_, 0, 0);
    grid->addWidget(messageLabel_, 1, 0);
    root->addLayout(grid, 1);

    connect(refreshButton, &QPushButton::clicked, this, [this]() {
        refreshData();
    });
    connect(clearButton, &QPushButton::clicked, this, [this]() {
        clearView();
    });
    connect(openFolderButton, &QPushButton::clicked, this, [this]() {
        openLogsFolder();
    });
    connect(levelFilter_, &QComboBox::currentTextChanged, this, [this]() {
        refreshData();
    });

    refreshData();
}

void LogsPage::refreshData()
{
    const QString level = levelFilter_->currentText() == "Todos" ? QString() : levelFilter_->currentText();
    const ControllerResult result = logsHandler_(level);
    levelCard_->setValue(level.isEmpty() ? "Todos" : level);

    if (!result.success) {
        const QString message = !result.message.empty()
            ? QString::fromStdString(result.message).trimmed()
            : "Não foi possível carregar os logs.";
        logsTable_->setRowCount(0);
        entriesCard_->setValue("Erro");
        entriesCard_->setStatus("Falha ao ler logs");
        statusBadge_->setStatus("ERROR");
        feedbackBanner_->showError(message);
        showMessage(message);
        return;
    }

    renderLogs(QString::fromStdString(result.payload));
}

void LogsPage::setLogsHandler(std::function<ControllerResult(const QString &)> handler)
{
    logsHandler_ = std::move(handler);
}

void LogsPage::setOpenLogsFolderHandler(std::function<bool()> handler)
{
    openLogsFolderHandler_ = std::move(handler);
}

void LogsPage::clearView()
{
    logsTable_->setRowCount(0);
    entriesCard_->setValue("0");
    entriesCard_->setStatus("Visualização limpa");
    statusBadge_->setStatus("INFO");
    feedbackBanner_->showNoData("Visualização limpa. Arquivo de log preservado.");
    showMessage("Nenhum log encontrado.");
}

void LogsPage::openLogsFolder()
{
    if (!openLogsFolderHandler_()) {
        QMessageBox::information(this, "Logs", "Não foi possível abrir a pasta de logs.");
    }
}

void LogsPage::renderLogs(const QString &payload)
{
    const QList<LogRow> rows = parseLogPayload(payload);
    logsTable_->setRowCount(rows.size());
    for (int row = 0; row < rows.size(); ++row) {
        const LogRow &entry = rows.at(row);
        const QStringList values = {entry.timestamp, entry.level, entry.module, entry.message};
        for (int column = 0; column < values.size(); ++column) {
            QTableWidgetItem *cell = new QTableWidgetItem(values.at(column));
            cell->setTextAlignment(column == 3 ? Qt::AlignLeft | Qt::AlignVCenter : Qt::AlignCenter);
            if (column == 1) {
                applyLevelStyle(cell, entry.level);
            }
            logsTable_->setItem(row, column, cell);
        }
    }

    logsTable_->resizeColumnsToContents();
    entriesCard_->setValue(QString::number(rows.size()));
    entriesCard_->setStatus(rows.isEmpty() ? "Nenhum log carregado" : "Últimas entradas");

    if (rows.isEmpty()) {
        statusBadge_->setStatus("INFO");
        feedbackBanner_->showNoData("Nenhum log encontrado.");
        showMessage("Nenhum log encontrado.");
        return;
    }

    statusBadge_->setStatus("OK");
    feedbackBanner_->showSuccess("Logs carregados via controller.");
    messageLabel_->setVisible(false);
    logsTable_->setVisible(true);
}

void LogsPage::showMessage(const QString &message)
{
    messageLabel_->setText(message);
    messageLabel_->setVisible(true);
    logsTable_->setVisible(false);
}
