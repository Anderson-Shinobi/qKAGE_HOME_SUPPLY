#include "ShoppingPage.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QByteArray>
#include <QHeaderView>
#include <QRegularExpression>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include <algorithm>
#include <exception>

#include "../../controllers/StockController.h"
#include "../../logging/Logger.h"
#include "../UiDataPaths.h"
#include "../widgets/FeedbackBanner.h"
#include "../widgets/InfoCard.h"
#include "../widgets/StatusBadge.h"

namespace {
struct ShoppingRow {
    QString item;
    QString category;
    QString currentQuantity;
    QString minimumStock;
    QString monthlyConsumption;
    QString estimatedAutonomy;
    QString suggestedPurchase;
    QString estimatedCost;
};

double numericText(QString value)
{
    value.remove("R$");
    value.remove("%");
    value.remove(QRegularExpression("[A-Za-zÀ-ÿ]+"));
    value = value.trimmed();
    bool ok = false;
    const double parsed = value.toDouble(&ok);
    return ok ? parsed : 0.0;
}

QString collapseSpaces(const QString &value)
{
    QString collapsed = value.trimmed();
    collapsed.replace(QRegularExpression("\\s+"), " ");
    return collapsed;
}

QString columnValue(const QString &line, int start, int end = -1)
{
    if (start < 0 || start >= line.size()) {
        return "";
    }

    const int length = end > start ? end - start : -1;
    return line.mid(start, length).trimmed();
}

QList<int> shoppingColumnStarts(const QString &header)
{
    const QStringList labels = {
        "item",
        "categoria",
        "quantidade atual",
        "estoque minimo",
        "consumo mensal",
        "autonomia",
        "sugestao compra",
        "custo estimado"
    };

    QList<int> starts;
    for (const QString &label : labels) {
        const int position = header.indexOf(label);
        if (position < 0) {
            return {};
        }
        starts.append(position);
    }
    return starts;
}

QList<ShoppingRow> parseShoppingPayload(const QString &payload)
{
    QList<ShoppingRow> rows;
    const QStringList lines = payload.split('\n');

    QList<int> starts;
    bool readingRows = false;
    for (const QString &line : lines) {
        if (starts.isEmpty() && line.contains("quantidade atual") && line.contains("sugestao compra")) {
            starts = shoppingColumnStarts(line);
            readingRows = !starts.isEmpty();
            continue;
        }

        if (line.trimmed().startsWith("---")) {
            readingRows = !starts.isEmpty();
            continue;
        }

        if (!readingRows || starts.size() < 7 || line.trimmed().isEmpty()) {
            continue;
        }

        ShoppingRow row;
        row.item = columnValue(line, starts.at(0), starts.at(1));
        row.category = columnValue(line, starts.at(1), starts.at(2));
        row.currentQuantity = columnValue(line, starts.at(2), starts.at(3));
        row.minimumStock = columnValue(line, starts.at(3), starts.at(4));
        row.monthlyConsumption = columnValue(line, starts.at(4), starts.at(5));
        row.estimatedAutonomy = columnValue(line, starts.at(5), starts.at(6));
        if (starts.size() >= 8) {
            row.suggestedPurchase = collapseSpaces(columnValue(line, starts.at(6), starts.at(7)));
            row.estimatedCost = collapseSpaces(columnValue(line, starts.at(7)));
        } else {
            row.suggestedPurchase = collapseSpaces(columnValue(line, starts.at(6)));
            row.estimatedCost = "R$ 0.00";
        }
        if (row.item.isEmpty()) {
            continue;
        }
        rows.append(row);
    }

    return rows;
}

void fillErrorState(InfoCard *itemsCard, InfoCard *suggestionCard, QLabel *emptyStateLabel, QTableWidget *shoppingTable, const QString &message, const QString &status)
{
    itemsCard->setValue("Erro");
    itemsCard->setStatus("Falha ao carregar");
    suggestionCard->setValue("Erro");
    suggestionCard->setStatus(status);
    emptyStateLabel->setText(message);
    emptyStateLabel->setVisible(true);
    shoppingTable->setRowCount(0);
    shoppingTable->setVisible(false);
}
}

ShoppingPage::ShoppingPage(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(28, 28, 28, 28);
    root->setSpacing(20);

    QLabel *title = new QLabel("Compras", this);
    title->setObjectName("PageTitle");

    QPushButton *refreshButton = new QPushButton("Atualizar lista", this);
    refreshButton->setObjectName("PrimaryActionButton");
    QPushButton *exportButton = new QPushButton("Exportar lista", this);
    QPushButton *copyButton = new QPushButton("Copiar para área de transferência", this);

    QHBoxLayout *header = new QHBoxLayout();
    header->addWidget(title);
    header->addStretch(1);
    header->addWidget(refreshButton);
    header->addWidget(exportButton);
    header->addWidget(copyButton);
    root->addLayout(header);

    QGridLayout *cards = new QGridLayout();
    cards->setSpacing(16);
    itemsCard_ = new InfoCard("Itens para compra", "0", "Lista automática", this);
    suggestionCard_ = new InfoCard("Sugestão", "Sem compras", "Reposição", this);
    cards->addWidget(itemsCard_, 0, 0);
    cards->addWidget(suggestionCard_, 0, 1);
    root->addLayout(cards);

    statusBadge_ = new StatusBadge("INFO", this);
    statusBadge_->setObjectName("ShoppingStatusBadge");
    feedbackBanner_ = new FeedbackBanner(this);
    root->addWidget(statusBadge_);
    root->addWidget(feedbackBanner_);

    QHBoxLayout *content = new QHBoxLayout();
    QVBoxLayout *tableArea = new QVBoxLayout();

    shoppingTable_ = new QTableWidget(this);
    shoppingTable_->setObjectName("DataTable");
    shoppingTable_->setColumnCount(5);
    shoppingTable_->setHorizontalHeaderLabels({
        "item",
        "categoria",
        "quantidade sugerida",
        "prioridade",
        "custo estimado"
    });
    shoppingTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    shoppingTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    shoppingTable_->setAlternatingRowColors(true);
    shoppingTable_->horizontalHeader()->setStretchLastSection(true);

    emptyStateLabel_ = new QLabel("Estoque saudável. Nenhuma compra necessária.", this);
    emptyStateLabel_->setObjectName("PanelPlaceholder");
    emptyStateLabel_->setAlignment(Qt::AlignCenter);

    tableArea->addWidget(shoppingTable_);
    tableArea->addWidget(emptyStateLabel_);
    content->addLayout(tableArea);
    root->addLayout(content, 1);

    connect(refreshButton, &QPushButton::clicked, this, [this]() {
        refreshData();
    });
    connect(exportButton, &QPushButton::clicked, this, [this]() {
        showActionDialog("Exportar lista");
    });
    connect(copyButton, &QPushButton::clicked, this, [this]() {
        showActionDialog("Copiar para área de transferência");
    });

    refreshData();
}

void ShoppingPage::refreshData()
{
    ControllerResult result;
    try {
        StockController controller;
        QByteArray programName("qkage-ui");
        QByteArray commandName("shopping-list");
        QByteArray stockPath = qkageDataPath("estoque.csv").toUtf8();
        char *argv[] = {programName.data(), commandName.data(), stockPath.data()};
        result = controller.shoppingList(3, argv);
    } catch (const std::exception &exception) {
        fillErrorState(itemsCard_, suggestionCard_, emptyStateLabel_, shoppingTable_,
                       QString("Não foi possível carregar a lista de compras. %1").arg(exception.what()),
                       "EXCEPTION");
        feedbackBanner_->showError("Erro ao carregar lista de compras via controller.");
        statusBadge_->setStatus("ERROR");
        return;
    } catch (...) {
        fillErrorState(itemsCard_, suggestionCard_, emptyStateLabel_, shoppingTable_,
                       "Não foi possível carregar a lista de compras.",
                       "EXCEPTION");
        feedbackBanner_->showError("Erro ao carregar lista de compras via controller.");
        statusBadge_->setStatus("ERROR");
        return;
    }

    if (!result.success) {
        const QString status = QString::fromStdString(result.error_code).isEmpty()
            ? "SERVICE_ERROR"
            : QString::fromStdString(result.error_code);
        fillErrorState(itemsCard_, suggestionCard_, emptyStateLabel_, shoppingTable_,
                       "Não foi possível carregar a lista de compras.",
                       status);
        feedbackBanner_->showError("Erro ao carregar lista de compras via controller.");
        statusBadge_->setStatus("ERROR");
        return;
    }

    const QString payload = QString::fromStdString(result.payload);
    const QList<ShoppingRow> rows = parseShoppingPayload(payload);
    if (rows.isEmpty() && payload.contains("Lista automatica de compras")) {
        Logger::log(LogLevel::Warning, "ShoppingPage", "Payload de compras recebido sem linhas renderizaveis.");
    } else {
        Logger::log(LogLevel::Info, "ShoppingPage", "Lista de compras sincronizada com estoque_controller.");
    }
    shoppingTable_->setRowCount(rows.size());

    double totalEstimatedCost = 0.0;
    int criticalCount = 0;
    for (int row = 0; row < rows.size(); ++row) {
        const ShoppingRow &item = rows.at(row);
        const double currentQuantity = numericText(item.currentQuantity);
        const double minimumStock = numericText(item.minimumStock);
        const double autonomy = numericText(item.estimatedAutonomy);
        const bool critical = currentQuantity <= minimumStock || autonomy <= 1.0;
        const QString priority = critical ? "CRÍTICO" : "ATENÇÃO";
        totalEstimatedCost += numericText(item.estimatedCost);
        if (critical) {
            ++criticalCount;
        }

        const QStringList values = {
            item.item,
            item.category,
            item.suggestedPurchase,
            priority,
            item.estimatedCost
        };

        for (int column = 0; column < values.size(); ++column) {
            if (column == 3) {
                StatusBadge *priorityBadge = new StatusBadge(priority, shoppingTable_);
                shoppingTable_->setCellWidget(row, column, priorityBadge);
                continue;
            }
            QTableWidgetItem *tableItem = new QTableWidgetItem(values.at(column));
            shoppingTable_->setItem(row, column, tableItem);
        }
    }

    shoppingTable_->resizeColumnsToContents();
    itemsCard_->setValue(QString::number(rows.size()));
    itemsCard_->setStatus(rows.isEmpty() ? "Estoque saudável" : "Reposição sugerida");
    suggestionCard_->setValue(rows.isEmpty() ? "R$ 0.00" : QString("R$ %1").arg(totalEstimatedCost, 0, 'f', 2));
    suggestionCard_->setStatus(rows.isEmpty() ? "Compras" : "Total estimado da compra");

    emptyStateLabel_->setText("Estoque saudável. Nenhuma compra necessária.");
    emptyStateLabel_->setVisible(rows.isEmpty());
    shoppingTable_->setVisible(!rows.isEmpty());
    if (rows.isEmpty()) {
        statusBadge_->setStatus("INFO");
        feedbackBanner_->showNoData("Nenhum item encontrado para compra.");
    } else {
        statusBadge_->setStatus(criticalCount > 0 ? "CRÍTICO" : "ATENÇÃO");
        feedbackBanner_->showWarning(QString("%1 itens precisam reposição imediata.").arg(rows.size()));
    }
}

void ShoppingPage::showActionDialog(const QString &title)
{
    QMessageBox::information(this, title, "Lista operacional preparada a partir dos dados atuais de estoque.");
}
