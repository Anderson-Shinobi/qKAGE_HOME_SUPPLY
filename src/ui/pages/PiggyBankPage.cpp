#include "PiggyBankPage.h"

#include <QByteArray>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QProgressBar>
#include <QRegularExpression>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include <algorithm>

#include "../../controllers/FinanceController.h"
#include "../../logging/Logger.h"
#include "../UiDataPaths.h"
#include "../widgets/InfoCard.h"
#include "../widgets/StatusBadge.h"

namespace {
struct PiggyRow {
    QString name;
    double currentValue = 0.0;
    double goal = 0.0;
    double monthlyContribution = 0.0;
    double percentage = 0.0;
    QString status;
};

double decimalValue(QString value)
{
    value.remove("R$");
    value.remove("%");
    value = value.trimmed();
    bool ok = false;
    const double parsed = value.toDouble(&ok);
    return ok ? parsed : 0.0;
}

QString money(double value)
{
    return QString("R$ %1").arg(value, 0, 'f', 2);
}

QList<PiggyRow> parsePiggyPayload(const QString &payload)
{
    QList<PiggyRow> rows;
    bool readingRows = false;
    for (const QString &line : payload.split('\n')) {
        if (line.contains("valor atual") && line.contains("percentual meta")) {
            continue;
        }
        if (line.trimmed().startsWith("---")) {
            readingRows = true;
            continue;
        }
        if (!readingRows || line.trimmed().isEmpty()) {
            continue;
        }

        const QRegularExpression rowPattern(
            R"(^(.+?)\s+([0-9]+(?:\.[0-9]+)?)\s+([0-9]+(?:\.[0-9]+)?)\s+([0-9]+(?:\.[0-9]+)?)\s+([0-9]+(?:\.[0-9]+)?)%\s+(.+)$)");
        const QRegularExpressionMatch match = rowPattern.match(line.trimmed());
        if (!match.hasMatch()) {
            continue;
        }

        PiggyRow row;
        row.name = match.captured(1).trimmed();
        row.currentValue = decimalValue(match.captured(2));
        row.goal = decimalValue(match.captured(3));
        row.monthlyContribution = decimalValue(match.captured(4));
        row.percentage = row.goal > 0.0 ? (row.currentValue / row.goal) * 100.0 : decimalValue(match.captured(5));
        row.status = match.captured(6).trimmed();
        if (!row.name.isEmpty()) {
            rows.append(row);
        }
    }
    return rows;
}

QString progressColor(double percentage)
{
    if (percentage > 75.0) {
        return "#8ee6a3";
    }
    if (percentage > 40.0) {
        return "#f28c28";
    }
    return "#ff6b7a";
}
}

PiggyBankPage::PiggyBankPage(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(28, 28, 28, 28);
    root->setSpacing(20);

    QLabel *title = new QLabel("Cofrinhos", this);
    title->setObjectName("PageTitle");
    root->addWidget(title);

    FinanceController controller;
    QByteArray programName("qkage-ui");
    QByteArray commandName("piggy-report");
    QByteArray piggyPath = qkageDataPath("piggybanks.csv").toUtf8();
    char *argv[] = {programName.data(), commandName.data(), piggyPath.data()};
    const ControllerResult result = controller.piggyReport(3, argv);
    const QList<PiggyRow> rows = result.success ? parsePiggyPayload(QString::fromStdString(result.payload)) : QList<PiggyRow>{};
    if (!result.success) {
        Logger::log(LogLevel::Warning, "PiggyBankPage", "Falha ao sincronizar cofrinhos via FinanceController: " + result.message);
    } else if (rows.isEmpty()) {
        Logger::log(LogLevel::Warning, "PiggyBankPage", "Relatorio de cofrinhos carregado sem linhas renderizaveis.");
    } else {
        Logger::log(LogLevel::Info, "PiggyBankPage", "Cofrinhos sincronizados com piggybanks.csv.");
    }

    double totalCurrent = 0.0;
    double totalGoal = 0.0;
    for (const PiggyRow &row : rows) {
        totalCurrent += row.currentValue;
        totalGoal += row.goal;
    }
    const double averagePercentage = rows.isEmpty() ? 0.0 : (totalCurrent / std::max(1.0, totalGoal)) * 100.0;

    QGridLayout *cards = new QGridLayout();
    cards->setHorizontalSpacing(16);
    cards->setVerticalSpacing(18);
    cards->addWidget(new InfoCard("Total acumulado", money(totalCurrent), "Capital reservado", "$", this), 0, 0);
    cards->addWidget(new InfoCard("Meta consolidada", money(totalGoal), "Objetivo financeiro", "%", this), 0, 1);
    cards->addWidget(new InfoCard("Média percentual", QString("%1%").arg(averagePercentage, 0, 'f', 1), "Progresso geral", "~", this), 0, 2);
    root->addLayout(cards);

    StatusBadge *badge = new StatusBadge(rows.isEmpty() ? "INFO" : "OK", this);
    badge->setObjectName("PiggyBankStatusBadge");
    root->addWidget(badge);

    QTableWidget *table = new QTableWidget(this);
    table->setObjectName("DataTable");
    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({"nome", "valor atual", "meta", "percentual", "aporte mensal", "status"});
    table->setRowCount(rows.size());
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);
    table->horizontalHeader()->setStretchLastSection(true);

    for (int rowIndex = 0; rowIndex < rows.size(); ++rowIndex) {
        const PiggyRow &row = rows.at(rowIndex);
        table->setItem(rowIndex, 0, new QTableWidgetItem(row.name));
        table->setItem(rowIndex, 1, new QTableWidgetItem(money(row.currentValue)));
        table->setItem(rowIndex, 2, new QTableWidgetItem(money(row.goal)));

        QProgressBar *progress = new QProgressBar(table);
        progress->setRange(0, 100);
        progress->setValue(static_cast<int>(std::clamp(row.percentage, 0.0, 100.0)));
        progress->setFormat(QString("%1%").arg(row.percentage, 0, 'f', 1));
        progress->setStyleSheet(QString(
            "QProgressBar { background: #151c24; border: 1px solid #2d3743; border-radius: 6px; color: #f2f5f7; font-weight: 700; text-align: center; }"
            "QProgressBar::chunk { background: %1; border-radius: 5px; }")
            .arg(progressColor(row.percentage)));
        table->setCellWidget(rowIndex, 3, progress);

        table->setItem(rowIndex, 4, new QTableWidgetItem(money(row.monthlyContribution)));
        table->setItem(rowIndex, 5, new QTableWidgetItem(row.status));
    }

    table->resizeColumnsToContents();
    root->addWidget(table, 1);
}
