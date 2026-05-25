#include "StockPage.h"

#include <QGridLayout>
#include <QComboBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QByteArray>
#include <QRegularExpression>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include <algorithm>
#include <QBrush>
#include <QColor>
#include <QFont>
#include <utility>

#include "../../controllers/StockController.h"
#include "../UiDataPaths.h"
#include "../dialogs/AddStockItemDialog.h"
#include "../dialogs/ConsumeStockItemDialog.h"
#include "../dialogs/EditStockItemDialog.h"
#include "../widgets/FeedbackBanner.h"
#include "../widgets/InfoCard.h"
#include "../widgets/StatusBadge.h"

namespace {
QString valueAfter(const QString &line, const QString &prefix)
{
    return line.startsWith(prefix) ? line.mid(prefix.size()).trimmed() : QString();
}

QString firstToken(const QString &value)
{
    return value.section(' ', 0, 0).trimmed();
}

QString restAfterFirstToken(const QString &value)
{
    const int firstSpace = value.indexOf(' ');
    return firstSpace < 0 ? QString() : value.mid(firstSpace + 1).trimmed();
}

QList<StockPageItemData> parseStockPayload(const QString &payload)
{
    QList<StockPageItemData> rows;
    StockPageItemData current;
    bool hasCurrent = false;

    const QStringList lines = payload.split('\n');
    for (const QString &line : lines) {
        if (line.startsWith("Item: ")) {
            if (hasCurrent) {
                rows.append(current);
                current = StockPageItemData();
            }
            current.item = valueAfter(line, "Item: ");
            hasCurrent = true;
            continue;
        }

        if (!hasCurrent) {
            continue;
        }

        if (line.startsWith("Categoria: ")) {
            current.category = valueAfter(line, "Categoria: ");
        } else if (line.startsWith("Quantidade: ")) {
            const QString value = valueAfter(line, "Quantidade: ");
            current.quantity = firstToken(value);
            current.unit = restAfterFirstToken(value);
        } else if (line.startsWith("Consumo mensal: ")) {
            current.monthlyConsumption = firstToken(valueAfter(line, "Consumo mensal: "));
        } else if (line.startsWith("Preco unitario: R$ ")) {
            current.unitPrice = valueAfter(line, "Preco unitario: R$ ");
        } else if (line.startsWith("Validade: ")) {
            current.expiration = valueAfter(line, "Validade: ");
        } else if (line.startsWith("Estoque minimo: ")) {
            current.minimumStock = firstToken(valueAfter(line, "Estoque minimo: "));
        } else if (line.startsWith("Autonomia: ")) {
            current.autonomy = valueAfter(line, "Autonomia: ").remove(" mes(es)").trimmed();
        } else if (line.startsWith("Status: ")) {
            current.status = valueAfter(line, "Status: ");
        }
    }

    if (hasCurrent) {
        rows.append(current);
    }

    return rows;
}

QString normalizedStatus(QString status)
{
    status = status.trimmed().toUpper();
    if (status == "CRÍTICO") {
        return "CRITICO";
    }
    return status;
}

QString displayStatus(const QString &status)
{
    const QString normalized = normalizedStatus(status);
    return normalized == "CRITICO" ? "CRÍTICO" : status;
}

double numberValue(const QString &value)
{
    bool ok = false;
    const double parsed = value.toDouble(&ok);
    return ok ? parsed : 0.0;
}

QString sortValue(const StockPageItemData &item, int column)
{
    switch (column) {
    case 0: return item.item;
    case 1: return item.category;
    case 6: return normalizedStatus(item.status);
    case 7: return item.expiration;
    default: return QString();
    }
}

bool isSortableColumn(int column)
{
    return column == 0 || column == 1 || column == 2 || column == 5 || column == 6 || column == 7;
}

void applyStatusStyle(QTableWidgetItem *cell, const QString &status)
{
    const QString normalized = normalizedStatus(status);
    QColor background("#e8f5e9");
    QColor foreground("#1b5e20");

    if (normalized == "ATENÇÃO") {
        background = QColor("#fff3cd");
        foreground = QColor("#7a4f00");
    } else if (normalized == "CRITICO") {
        background = QColor("#fdecea");
        foreground = QColor("#b71c1c");
    } else if (normalized == "VENCIDO") {
        background = QColor("#ffcdd2");
        foreground = QColor("#7f0000");
    } else if (normalized == "SEM_VALIDADE") {
        background = QColor("#e8eef7");
        foreground = QColor("#34506f");
    }

    QFont font = cell->font();
    font.setBold(true);
    cell->setFont(font);
    cell->setTextAlignment(Qt::AlignCenter);
    cell->setBackground(QBrush(background));
    cell->setForeground(QBrush(foreground));
}
}

StockPage::StockPage(QWidget *parent)
    : QWidget(parent)
{
    listHandler_ = []() {
        StockController controller;
        QByteArray programName("qkage-ui");
        QByteArray commandName("list");
        QByteArray stockPath = qkageDataPath("estoque.csv").toUtf8();
        char *argv[] = {programName.data(), commandName.data(), stockPath.data()};
        return controller.list(3, argv);
    };

    removeHandler_ = [](const QString &itemName) {
        StockController controller;
        QByteArray programName("qkage-ui");
        QByteArray commandName("remove");
        QByteArray itemNameBytes = itemName.toUtf8();
        QByteArray stockPath = qkageDataPath("estoque.csv").toUtf8();
        char *argv[] = {programName.data(), commandName.data(), itemNameBytes.data(), stockPath.data()};
        return controller.remove(4, argv);
    };

    removeConfirmationHandler_ = [this](const QString &) {
        return QMessageBox::question(
                   this,
                   "Remover item",
                   "Tem certeza que deseja remover este item?",
                   QMessageBox::Yes | QMessageBox::No,
                   QMessageBox::No) == QMessageBox::Yes;
    };

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(28, 28, 28, 28);
    root->setSpacing(20);

    QLabel *title = new QLabel("Estoque", this);
    title->setObjectName("PageTitle");

    QPushButton *refreshButton = new QPushButton("Atualizar", this);
    refreshButton->setObjectName("PrimaryActionButton");
    QPushButton *addButton = new QPushButton("Adicionar item", this);
    QPushButton *editButton = new QPushButton("Editar item", this);
    QPushButton *removeButton = new QPushButton("Remover item", this);
    QPushButton *consumeButton = new QPushButton("Consumir item", this);

    QHBoxLayout *header = new QHBoxLayout();
    header->addWidget(title);
    header->addStretch(1);
    header->addWidget(refreshButton);
    header->addWidget(addButton);
    header->addWidget(editButton);
    header->addWidget(removeButton);
    header->addWidget(consumeButton);
    root->addLayout(header);

    QHBoxLayout *filters = new QHBoxLayout();
    filters->setSpacing(12);
    searchEdit_ = new QLineEdit(this);
    searchEdit_->setObjectName("stockSearchEdit");
    searchEdit_->setPlaceholderText("Buscar por item ou categoria");
    searchEdit_->setClearButtonEnabled(true);

    statusFilter_ = new QComboBox(this);
    statusFilter_->setObjectName("stockStatusFilter");
    statusFilter_->addItems({"Todos", "OK", "ATENÇÃO", "CRÍTICO", "VENCIDO", "SEM_VALIDADE"});

    counterLabel_ = new QLabel("Total de itens: 0 | Itens filtrados: 0", this);
    counterLabel_->setObjectName("StockCounterLabel");
    counterLabel_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    filters->addWidget(searchEdit_, 2);
    filters->addWidget(statusFilter_);
    filters->addStretch(1);
    filters->addWidget(counterLabel_);
    root->addLayout(filters);

    statusBadge_ = new StatusBadge("INFO", this);
    statusBadge_->setObjectName("StockStatusBadge");
    feedbackBanner_ = new FeedbackBanner(this);
    root->addWidget(statusBadge_);
    root->addWidget(feedbackBanner_);

    QHBoxLayout *summary = new QHBoxLayout();
    summary->setSpacing(16);
    itemsCard_ = new InfoCard("Itens", "0", "Cadastro", this);
    autonomyCard_ = new InfoCard("Autonomia", "Sem dados", "Análise", this);
    summary->addWidget(itemsCard_);
    summary->addWidget(autonomyCard_);
    root->addLayout(summary);

    QGridLayout *grid = new QGridLayout();
    grid->setSpacing(16);

    stockTable_ = new QTableWidget(this);
    stockTable_->setObjectName("DataTable");
    stockTable_->setColumnCount(8);
    stockTable_->setHorizontalHeaderLabels({
        "item",
        "categoria",
        "quantidade",
        "unidade",
        "consumo mensal",
        "autonomia",
        "status",
        "validade"
    });
    stockTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    stockTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    stockTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    stockTable_->setAlternatingRowColors(true);
    stockTable_->verticalHeader()->setDefaultSectionSize(38);
    stockTable_->verticalHeader()->setVisible(false);
    stockTable_->horizontalHeader()->setHighlightSections(false);
    stockTable_->horizontalHeader()->setStretchLastSection(true);
    stockTable_->horizontalHeader()->setSectionsClickable(true);
    stockTable_->setSortingEnabled(false);

    emptyStateLabel_ = new QLabel("Nenhum item cadastrado.", this);
    emptyStateLabel_->setObjectName("PanelPlaceholder");
    emptyStateLabel_->setAlignment(Qt::AlignCenter);

    grid->addWidget(stockTable_, 0, 0);
    grid->addWidget(emptyStateLabel_, 1, 0);
    root->addLayout(grid, 1);

    connect(refreshButton, &QPushButton::clicked, this, [this]() {
        refreshData();
    });
    connect(addButton, &QPushButton::clicked, this, [this]() {
        AddStockItemDialog dialog(this);
        if (dialog.exec() == QDialog::Accepted) {
            refreshData();
        }
    });
    connect(editButton, &QPushButton::clicked, this, [this]() {
        editSelectedItem();
    });
    connect(removeButton, &QPushButton::clicked, this, [this]() {
        removeSelectedItem();
    });
    connect(consumeButton, &QPushButton::clicked, this, [this]() {
        ConsumeStockItemDialog dialog(loadedItemNames_, this);
        if (dialog.exec() == QDialog::Accepted) {
            refreshData();
        }
    });
    connect(searchEdit_, &QLineEdit::textChanged, this, [this]() {
        applyFiltersAndRender();
    });
    connect(statusFilter_, &QComboBox::currentTextChanged, this, [this]() {
        applyFiltersAndRender();
    });
    connect(stockTable_->horizontalHeader(), &QHeaderView::sectionClicked, this, [this](int column) {
        if (!isSortableColumn(column)) {
            return;
        }

        if (sortColumn_ == column) {
            sortOrder_ = sortOrder_ == Qt::AscendingOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
        } else {
            sortColumn_ = column;
            sortOrder_ = Qt::AscendingOrder;
        }

        applyFiltersAndRender();
    });

    refreshData();
}

void StockPage::refreshData()
{
    const ControllerResult result = listHandler_();
    if (!result.success) {
        itemsCard_->setValue("Erro");
        autonomyCard_->setValue("Erro");
        autonomyCard_->setStatus(QString::fromStdString(result.error_code));
        emptyStateLabel_->setText("Não foi possível carregar o estoque.");
        emptyStateLabel_->setVisible(true);
        stockTable_->setRowCount(0);
        displayedItems_.clear();
        loadedItemNames_.clear();
        loadedItems_.clear();
        counterLabel_->setText("Total de itens: 0 | Itens filtrados: 0");
        statusBadge_->setStatus("ERROR");
        feedbackBanner_->showError("Erro ao carregar estoque via controller.");
        return;
    }

    const QList<StockPageItemData> rows = parseStockPayload(QString::fromStdString(result.payload));
    loadedItemNames_.clear();
    loadedItems_.clear();

    int criticalCount = 0;
    for (int row = 0; row < rows.size(); ++row) {
        const StockPageItemData &item = rows.at(row);
        loadedItems_.append(item);
        if (!item.item.isEmpty()) {
            loadedItemNames_.append(item.item);
        }

        if (normalizedStatus(item.status) == "CRITICO") {
            ++criticalCount;
        }
    }

    itemsCard_->setValue(QString::number(rows.size()));
    itemsCard_->setStatus(rows.isEmpty() ? "Nenhum cadastro" : "Itens carregados");
    autonomyCard_->setValue(criticalCount > 0 ? QString::number(criticalCount) + " crítico(s)" : "OK");
    autonomyCard_->setStatus("Status do estoque");
    statusBadge_->setStatus(criticalCount > 0 ? "CRÍTICO" : "OK");
    feedbackBanner_->showSuccess("Estoque atualizado via controller.");

    applyFiltersAndRender();
}

void StockPage::applyFiltersAndRender()
{
    displayedItems_.clear();
    for (const StockPageItemData &item : loadedItems_) {
        if (matchesCurrentFilters(item)) {
            displayedItems_.append(item);
        }
    }

    sortDisplayedItems();
    renderTable(displayedItems_);

    counterLabel_->setText(
        QString("Total de itens: %1 | Itens filtrados: %2")
            .arg(loadedItems_.size())
            .arg(displayedItems_.size()));

    emptyStateLabel_->setText(loadedItems_.isEmpty() ? "Nenhum item cadastrado." : "Nenhum item encontrado.");
    emptyStateLabel_->setVisible(displayedItems_.isEmpty());
    stockTable_->setVisible(!displayedItems_.isEmpty());
    if (displayedItems_.isEmpty()) {
        feedbackBanner_->showNoData(loadedItems_.isEmpty() ? "Nenhum item encontrado." : "Nenhum item encontrado para os filtros atuais.");
    }
}

void StockPage::renderTable(const QVector<StockPageItemData> &rows)
{
    stockTable_->setRowCount(rows.size());
    for (int row = 0; row < rows.size(); ++row) {
        const StockPageItemData &item = rows.at(row);
        const QStringList values = {
            item.item,
            item.category,
            item.quantity,
            item.unit,
            item.monthlyConsumption,
            item.autonomy,
            displayStatus(item.status),
            item.expiration
        };

        for (int column = 0; column < values.size(); ++column) {
            QTableWidgetItem *cell = new QTableWidgetItem(values.at(column));
            cell->setTextAlignment(column == 0 || column == 1 ? Qt::AlignLeft | Qt::AlignVCenter : Qt::AlignCenter);
            if (column == 6) {
                applyStatusStyle(cell, item.status);
            }
            stockTable_->setItem(row, column, cell);
        }
    }

    stockTable_->resizeColumnsToContents();
}

void StockPage::sortDisplayedItems()
{
    std::sort(displayedItems_.begin(), displayedItems_.end(), [this](const StockPageItemData &left, const StockPageItemData &right) {
        int comparison = 0;
        if (sortColumn_ == 2) {
            const double diff = numberValue(left.quantity) - numberValue(right.quantity);
            comparison = diff < 0 ? -1 : (diff > 0 ? 1 : 0);
        } else if (sortColumn_ == 5) {
            const double diff = numberValue(left.autonomy) - numberValue(right.autonomy);
            comparison = diff < 0 ? -1 : (diff > 0 ? 1 : 0);
        } else {
            comparison = QString::localeAwareCompare(sortValue(left, sortColumn_), sortValue(right, sortColumn_));
        }

        return sortOrder_ == Qt::AscendingOrder ? comparison < 0 : comparison > 0;
    });
}

bool StockPage::matchesCurrentFilters(const StockPageItemData &item) const
{
    const QString query = searchEdit_->text().trimmed();
    if (!query.isEmpty() &&
        !item.item.contains(query, Qt::CaseInsensitive) &&
        !item.category.contains(query, Qt::CaseInsensitive)) {
        return false;
    }

    const QString filter = statusFilter_->currentText();
    return filter == "Todos" || normalizedStatus(item.status) == normalizedStatus(filter);
}

bool StockPage::removeSelectedItem()
{
    const int selectedRow = stockTable_->currentRow();
    if (selectedRow < 0 || selectedRow >= displayedItems_.size()) {
        QMessageBox::information(this, "Remover item", "Selecione um item para remover.");
        return false;
    }

    const QString itemName = displayedItems_.at(selectedRow).item;
    if (!removeConfirmationHandler_(itemName)) {
        return false;
    }

    const ControllerResult result = removeHandler_(itemName);
    if (!result.success) {
        const QString message = !result.message.empty()
            ? QString::fromStdString(result.message).trimmed()
            : "Não foi possível remover o item.";
        QMessageBox::warning(this, "Remover item", message);
        return false;
    }

    refreshData();
    return true;
}

void StockPage::setListHandler(std::function<ControllerResult()> handler)
{
    listHandler_ = std::move(handler);
}

void StockPage::setRemoveHandler(std::function<ControllerResult(const QString &)> handler)
{
    removeHandler_ = std::move(handler);
}

void StockPage::setRemoveConfirmationHandler(std::function<bool(const QString &)> handler)
{
    removeConfirmationHandler_ = std::move(handler);
}

bool StockPage::editSelectedItem()
{
    const int selectedRow = stockTable_->currentRow();
    if (selectedRow < 0 || selectedRow >= displayedItems_.size()) {
        QMessageBox::information(this, "Editar item", "Selecione um item para editar.");
        return false;
    }

    const StockPageItemData &item = displayedItems_.at(selectedRow);
    EditStockItemData dialogData;
    dialogData.originalItem = item.item;
    dialogData.item = item.item;
    dialogData.category = item.category;
    dialogData.quantity = item.quantity;
    dialogData.unit = item.unit;
    dialogData.monthlyConsumption = item.monthlyConsumption;
    dialogData.unitPrice = item.unitPrice;
    dialogData.expirationDate = item.expiration;
    dialogData.minimumStock = item.minimumStock;

    EditStockItemDialog dialog(dialogData, this);
    if (dialog.exec() == QDialog::Accepted) {
        refreshData();
        return true;
    }

    return false;
}

void StockPage::showPlaceholderDialog(const QString &title)
{
    QMessageBox::information(this, title, "Fluxo operacional disponível pela tela de estoque.");
}
