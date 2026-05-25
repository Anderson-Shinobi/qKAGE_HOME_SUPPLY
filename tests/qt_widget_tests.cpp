#include "ui/MainWindow.h"
#include "ui/dialogs/AddStockItemDialog.h"
#include "ui/dialogs/AboutDialog.h"
#include "ui/dialogs/ConsumeStockItemDialog.h"
#include "ui/dialogs/EditStockItemDialog.h"
#include "ui/pages/DashboardPage.h"
#include "ui/pages/InvestmentsPage.h"
#include "ui/pages/LogsPage.h"
#include "ui/pages/PiggyBankPage.h"
#include "ui/pages/ReportsPage.h"
#include "ui/pages/SettingsPage.h"
#include "ui/pages/ShoppingPage.h"
#include "ui/pages/StockPage.h"
#include "ui/theme/ThemeManager.h"
#include "ui/widgets/FeedbackBanner.h"
#include "ui/widgets/InfoCard.h"
#include "ui/widgets/OperationalChartWidget.h"
#include "ui/widgets/StatusBadge.h"

#include <QApplication>
#include <QByteArray>
#include <QComboBox>
#include <QFile>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMetaObject>
#include <QPixmap>
#include <QPushButton>
#include <QHeaderView>
#include <QStackedWidget>
#include <QStatusBar>
#include <QStringList>
#include <QTableWidget>
#include <QTimer>
#include <QTextEdit>
#include <QVector>
#include <QtGlobal>

#include <algorithm>
#include <iostream>

namespace {
QStringList capturedCriticalMessages;
QtMessageHandler previousMessageHandler = nullptr;

void captureCriticalMessages(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    if (type == QtCriticalMsg || type == QtFatalMsg) {
        capturedCriticalMessages.append(message);
    }

    if (previousMessageHandler != nullptr) {
        previousMessageHandler(type, context, message);
    }
}

int fail(const char *message)
{
    std::cerr << message << '\n';
    return 1;
}

bool labelHasVisibleText(const QList<QLabel *> &labels, const QString &text)
{
    for (QLabel *label : labels) {
        if (label->text() == text && label->isVisible()) {
            return true;
        }
    }

    return false;
}

bool labelHeightFits(QLabel *label)
{
    if (label == nullptr || !label->isVisible() || label->text().isEmpty()) {
        return true;
    }

    return label->height() + 2 >= label->sizeHint().height();
}

QPushButton *findButtonByText(QWidget &window, const QString &text)
{
    const QList<QPushButton *> buttons = window.findChildren<QPushButton *>();
    for (QPushButton *button : buttons) {
        if (button->text() == text) {
            return button;
        }
    }

    return nullptr;
}

void fillValidStockItem(AddStockItemDialog &dialog)
{
    dialog.findChild<QLineEdit *>("itemEdit")->setText("Teste visual");
    dialog.findChild<QLineEdit *>("categoryEdit")->setText("Testes");
    dialog.findChild<QLineEdit *>("quantityEdit")->setText("2");
    dialog.findChild<QLineEdit *>("unitEdit")->setText("un");
    dialog.findChild<QLineEdit *>("monthlyConsumptionEdit")->setText("1");
    dialog.findChild<QLineEdit *>("unitPriceEdit")->setText("3.50");
    dialog.findChild<QLineEdit *>("expirationEdit")->setText("2027-12-31");
    dialog.findChild<QLineEdit *>("minimumStockEdit")->setText("1");
}

QString dialogError(AddStockItemDialog &dialog)
{
    QLabel *label = dialog.findChild<QLabel *>("DialogErrorLabel");
    return label == nullptr ? QString() : label->text();
}

QString dialogError(ConsumeStockItemDialog &dialog)
{
    QLabel *label = dialog.findChild<QLabel *>("DialogErrorLabel");
    return label == nullptr ? QString() : label->text();
}

QString dialogError(EditStockItemDialog &dialog)
{
    QLabel *label = dialog.findChild<QLabel *>("DialogErrorLabel");
    return label == nullptr ? QString() : label->text();
}

QString stockPayloadFor(
    const QString &item,
    const QString &category,
    const QString &status,
    const QString &quantity,
    const QString &autonomy,
    const QString &expiration);

QString stockPayloadFor(const QString &item)
{
    return stockPayloadFor(item, "Testes", "OK", "2", "2", "2027-12-31");
}

QString stockPayloadFor(
    const QString &item,
    const QString &category,
    const QString &status,
    const QString &quantity,
    const QString &autonomy,
    const QString &expiration)
{
    return QString(
        "KAGE Home Supply - Sistema CSV inicial\n"
        "Arquivo: teste.csv\n\n"
        "Estoque atual\n"
        "-------------\n"
        "Item: %1\n"
        "Categoria: %2\n"
        "Quantidade: %3 un\n"
        "Consumo mensal: 1 un\n"
        "Preco unitario: R$ 3.50\n"
        "Validade: %6\n"
        "Estoque minimo: 1 un\n"
        "Autonomia: %4 mes(es)\n"
        "Status: %5\n\n").arg(item, category, quantity, autonomy, status, expiration);
}

QString stockPayloadRows(const QList<QStringList> &rows)
{
    QString payload =
        "KAGE Home Supply - Sistema CSV inicial\n"
        "Arquivo: teste.csv\n\n";

    for (const QStringList &row : rows) {
        payload += stockPayloadFor(row.at(0), row.at(1), row.at(2), row.at(3), row.at(4), row.at(5));
    }

    return payload;
}

QTableWidget *stockTable(StockPage &page)
{
    return page.findChild<QTableWidget *>("DataTable");
}

QLineEdit *stockSearch(StockPage &page)
{
    return page.findChild<QLineEdit *>("stockSearchEdit");
}

QComboBox *stockStatusFilter(StockPage &page)
{
    return page.findChild<QComboBox *>("stockStatusFilter");
}

QLabel *stockCounter(StockPage &page)
{
    return page.findChild<QLabel *>("StockCounterLabel");
}

QString sampleStockPayload()
{
    return stockPayloadRows({
        {"Arroz", "Alimentos", "OK", "5", "5", "2027-12-31"},
        {"Sabao", "Limpeza", "ATENÇÃO", "2", "1", "2027-08-20"},
        {"Leite", "Alimentos", "CRITICO", "1", "0.5", "2026-06-01"},
        {"Iogurte", "Alimentos", "VENCIDO", "3", "2", "2025-01-01"},
        {"Sal", "Alimentos", "SEM_VALIDADE", "4", "12", "sem_validade"}
    });
}

QString sampleDashboardPayload()
{
    return QString(
        "KAGE Home Supply - Relatorio mensal consolidado\n\n"
        "[ESTOQUE]\n"
        "Total de itens cadastrados: 5\n"
        "Itens CRÍTICO: 1\n"
        "Itens ATENÇÃO: 2\n"
        "Itens OK: 2\n\n"
        "[ECONOMIA]\n"
        "Economia total estimada: R$ 48.50\n\n"
        "[COFRINHOS]\n"
        "Cofrinhos cadastrados: 3\n"
        "Percentual medio das metas: 62.00%\n\n"
        "[INVESTIMENTO]\n"
        "Capital liberado para investimento: R$ 48.50\n\n"
        "[RESUMO FINAL]\n"
        "Estoque monitorado: ativo\n"
        "Economia mensal consolidada: R$ 48.50\n"
        "Cofrinhos monitorados: 3\n");
}

QString dashboardPayloadWithCounts(int total, int critical, int attention)
{
    return QString(
        "KAGE Home Supply - Relatorio mensal consolidado\n\n"
        "[ESTOQUE]\n"
        "Total de itens cadastrados: %1\n"
        "Itens CRÍTICO: %2\n"
        "Itens ATENÇÃO: %3\n"
        "Itens OK: %4\n\n"
        "[ECONOMIA]\n"
        "Economia total estimada: R$ 48.50\n\n"
        "[COFRINHOS]\n"
        "Cofrinhos cadastrados: 3\n"
        "Percentual medio das metas: 62.00%\n\n"
        "[INVESTIMENTO]\n"
        "Capital liberado para investimento: R$ 48.50\n\n"
        "[RESUMO FINAL]\n"
        "Estoque monitorado: ativo\n"
        "Economia mensal consolidada: R$ 48.50\n"
        "Cofrinhos monitorados: 3\n")
        .arg(total)
        .arg(critical)
        .arg(attention)
        .arg(std::max(0, total - critical - attention));
}

QString sampleReportsPayload()
{
    return QString(
        "KAGE Home Supply - Relatorio mensal consolidado\n\n"
        "[ESTOQUE]\n"
        "Total de itens cadastrados: 4\n"
        "Itens CRÍTICO: 1\n"
        "Itens ATENÇÃO: 1\n"
        "Itens OK: 2\n\n"
        "[ECONOMIA]\n"
        "Economia total estimada: R$ 20.00\n\n"
        "[COFRINHOS]\n"
        "Cofrinhos cadastrados: 2\n"
        "Percentual medio das metas: 55.00%\n\n"
        "[INVESTIMENTO]\n"
        "Capital liberado para investimento: R$ 20.00\n"
        "Valor final estimado: R$ 20.00\n\n"
        "[RESUMO FINAL]\n"
        "Estoque monitorado: ativo\n"
        "Economia mensal consolidada: R$ 20.00\n"
        "Cofrinhos monitorados: 2\n");
}

QString noDataReportsPayload()
{
    return QString(
        "KAGE Home Supply - Relatorio mensal consolidado\n\n"
        "[ESTOQUE]\n"
        "Total de itens cadastrados: 0\n"
        "Itens CRÍTICO: 0\n"
        "Itens ATENÇÃO: 0\n"
        "Itens OK: 0\n\n"
        "[ECONOMIA]\n"
        "Economia total estimada: R$ 0.00\n\n"
        "[COFRINHOS]\n"
        "Cofrinhos cadastrados: 0\n\n"
        "[INVESTIMENTO]\n"
        "Capital liberado para investimento: R$ 0.00\n\n"
        "[RESUMO FINAL]\n"
        "Estoque monitorado: sem itens cadastrados\n"
        "Cofrinhos monitorados: 0\n");
}

QString sampleLogsPayload()
{
    return QString(
        "KAGE Home Supply - Logs operacionais\n"
        "Arquivo: logs/qkage.log\n\n"
        "[2026-05-25 10:00:00] [INFO] [CsvInventoryWriter] Item adicionado\n"
        "[2026-05-25 10:01:00] [WARNING] [StockConsumptionManager] Estoque baixo\n"
        "[2026-05-25 10:02:00] [ERROR] [CsvInventoryWriter] Falha ao validar item\n"
        "[2026-05-25 10:03:00] [DEBUG] [ConfigManager] Debug ativo\n");
}

QString emptyLogsPayload()
{
    return QString(
        "KAGE Home Supply - Logs operacionais\n"
        "Arquivo: logs/qkage.log\n\n"
        "Nenhuma entrada de log encontrada.\n");
}

QString sampleConfigPayload()
{
    return QString(
        "KAGE Home Supply - Configuracao central\n"
        "Arquivo: data/config.ini\n\n"
        "Configuracao do sistema\n"
        "-----------------------\n"
        "[system]\n"
        "version=1.0.0\n"
        "currency=BRL\n"
        "logs_enabled=true\n"
        "backup_enabled=true\n"
        "reports_enabled=false\n"
        "debug_mode=false\n\n"
        "[paths]\n"
        "data_dir=data\n"
        "backup_dir=backups\n"
        "reports_dir=reports\n\n"
        "[limits]\n"
        "critical_autonomy_months=1\n"
        "warning_autonomy_months=2\n"
        "expiration_critical_days=7\n"
        "expiration_warning_days=30\n");
}

int runAddStockItemDialogTests()
{
    AddStockItemDialog instance;
    fillValidStockItem(instance);

    bool handlerCalled = false;
    AddStockItemDialog emptyItemDialog;
    fillValidStockItem(emptyItemDialog);
    emptyItemDialog.findChild<QLineEdit *>("itemEdit")->clear();
    emptyItemDialog.setSubmitHandler([&handlerCalled](const AddStockItemData &) {
        handlerCalled = true;
        return ControllerResult{true, "", "", ""};
    });
    if (emptyItemDialog.submit() || handlerCalled || !dialogError(emptyItemDialog).contains("item", Qt::CaseInsensitive)) {
        return fail("Validacao de item vazio falhou.");
    }

    AddStockItemDialog negativeQuantityDialog;
    fillValidStockItem(negativeQuantityDialog);
    negativeQuantityDialog.findChild<QLineEdit *>("quantityEdit")->setText("-1");
    if (negativeQuantityDialog.submit() || !dialogError(negativeQuantityDialog).contains("Quantidade")) {
        return fail("Validacao de quantidade negativa falhou.");
    }

    AddStockItemDialog invalidDateDialog;
    fillValidStockItem(invalidDateDialog);
    invalidDateDialog.findChild<QLineEdit *>("expirationEdit")->setText("2027-99-99");
    if (invalidDateDialog.submit() || !dialogError(invalidDateDialog).contains("Validade")) {
        return fail("Validacao de data invalida falhou.");
    }

    AddStockItemDialog validDialog;
    fillValidStockItem(validDialog);
    bool submitted = false;
    validDialog.setSubmitHandler([&submitted](const AddStockItemData &data) {
        submitted = data.item == "Teste visual" &&
                    data.category == "Testes" &&
                    data.quantity == "2" &&
                    data.expirationDate == "2027-12-31";
        return ControllerResult{true, "", "", ""};
    });

    if (!validDialog.submit() || !submitted || validDialog.result() != QDialog::Accepted) {
        return fail("Cadastro valido via handler mockado falhou.");
    }

    return 0;
}

void fillValidConsumption(ConsumeStockItemDialog &dialog)
{
    dialog.findChild<QComboBox *>("consumeItemCombo")->setCurrentText("Teste visual");
    dialog.findChild<QLineEdit *>("consumeQuantityEdit")->setText("1");
    dialog.findChild<QLineEdit *>("consumeObservationEdit")->setText("uso mensal");
}

int runConsumeStockItemDialogTests()
{
    ConsumeStockItemDialog instance({"Teste visual", "Outro item"});
    fillValidConsumption(instance);

    bool handlerCalled = false;
    ConsumeStockItemDialog emptyItemDialog({"Teste visual"});
    fillValidConsumption(emptyItemDialog);
    emptyItemDialog.findChild<QComboBox *>("consumeItemCombo")->setCurrentText("");
    emptyItemDialog.setSubmitHandler([&handlerCalled](const ConsumeStockItemData &) {
        handlerCalled = true;
        return ControllerResult{true, "", "", ""};
    });
    if (emptyItemDialog.submit() || handlerCalled || !dialogError(emptyItemDialog).contains("item", Qt::CaseInsensitive)) {
        return fail("Validacao de item vazio no consumo falhou.");
    }

    ConsumeStockItemDialog zeroQuantityDialog({"Teste visual"});
    fillValidConsumption(zeroQuantityDialog);
    zeroQuantityDialog.findChild<QLineEdit *>("consumeQuantityEdit")->setText("0");
    if (zeroQuantityDialog.submit() || !dialogError(zeroQuantityDialog).contains("Quantidade")) {
        return fail("Validacao de quantidade zero no consumo falhou.");
    }

    ConsumeStockItemDialog negativeQuantityDialog({"Teste visual"});
    fillValidConsumption(negativeQuantityDialog);
    negativeQuantityDialog.findChild<QLineEdit *>("consumeQuantityEdit")->setText("-1");
    if (negativeQuantityDialog.submit() || !dialogError(negativeQuantityDialog).contains("Quantidade")) {
        return fail("Validacao de quantidade negativa no consumo falhou.");
    }

    ConsumeStockItemDialog validDialog({"Teste visual"});
    fillValidConsumption(validDialog);
    bool submitted = false;
    validDialog.setSubmitHandler([&submitted](const ConsumeStockItemData &data) {
        submitted = data.item == "Teste visual" &&
                    data.quantity == "1" &&
                    data.observation == "uso mensal";
        return ControllerResult{true, "", "", ""};
    });

    if (!validDialog.submit() || !submitted || validDialog.result() != QDialog::Accepted) {
        return fail("Consumo valido via handler mockado falhou.");
    }

    return 0;
}

EditStockItemData validEditData()
{
    return {
        "Teste visual",
        "Teste visual",
        "Testes",
        "2",
        "un",
        "1",
        "3.50",
        "2027-12-31",
        "1"
    };
}

int runEditStockItemDialogTests()
{
    EditStockItemDialog instance(validEditData());

    EditStockItemDialog emptyItemDialog(validEditData());
    emptyItemDialog.findChild<QLineEdit *>("editItemEdit")->clear();
    if (emptyItemDialog.submit() || !dialogError(emptyItemDialog).contains("item", Qt::CaseInsensitive)) {
        return fail("Validacao de item vazio na edicao falhou.");
    }

    EditStockItemDialog negativeQuantityDialog(validEditData());
    negativeQuantityDialog.findChild<QLineEdit *>("editQuantityEdit")->setText("-1");
    if (negativeQuantityDialog.submit() || !dialogError(negativeQuantityDialog).contains("Quantidade")) {
        return fail("Validacao de quantidade negativa na edicao falhou.");
    }

    EditStockItemDialog invalidDateDialog(validEditData());
    invalidDateDialog.findChild<QLineEdit *>("editExpirationEdit")->setText("2027-99-99");
    if (invalidDateDialog.submit() || !dialogError(invalidDateDialog).contains("Validade")) {
        return fail("Validacao de data invalida na edicao falhou.");
    }

    EditStockItemDialog validDialog(validEditData());
    bool submitted = false;
    validDialog.findChild<QLineEdit *>("editQuantityEdit")->setText("3");
    validDialog.setSubmitHandler([&submitted](const EditStockItemData &data) {
        submitted = data.originalItem == "Teste visual" &&
                    data.item == "Teste visual" &&
                    data.quantity == "3" &&
                    data.expirationDate == "2027-12-31";
        return ControllerResult{true, "", "", ""};
    });

    if (!validDialog.submit() || !submitted || validDialog.result() != QDialog::Accepted) {
        return fail("Edicao valida via handler mockado falhou.");
    }

    return 0;
}

int runStockPageEditWithoutSelectionTest()
{
    StockPage page;
    page.show();
    QApplication::processEvents();

    QPushButton *editButton = findButtonByText(page, "Editar item");
    if (editButton == nullptr) {
        return fail("Botao Editar item nao foi encontrado.");
    }

    bool messageFound = false;
    QTimer::singleShot(0, [&messageFound]() {
        QMessageBox *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
        if (box != nullptr && box->text() == "Selecione um item para editar.") {
            messageFound = true;
            box->accept();
        }
    });

    editButton->click();
    QApplication::processEvents();
    page.close();

    if (!messageFound) {
        return fail("Mensagem de selecao obrigatoria para edicao nao foi exibida.");
    }

    return 0;
}

int runStockPageRemoveWithoutSelectionTest()
{
    StockPage page;
    page.setListHandler([]() {
        return ControllerResult{true, "", "", stockPayloadFor("Remocao teste").toStdString()};
    });
    page.refreshData();
    page.show();
    QApplication::processEvents();

    QTableWidget *table = stockTable(page);
    if (table == nullptr) {
        return fail("Tabela de estoque nao foi encontrada.");
    }
    table->clearSelection();
    table->setCurrentCell(-1, -1);

    QPushButton *removeButton = findButtonByText(page, "Remover item");
    if (removeButton == nullptr) {
        return fail("Botao Remover item nao foi encontrado.");
    }

    bool messageFound = false;
    QTimer::singleShot(0, [&messageFound]() {
        QMessageBox *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
        if (box != nullptr && box->text() == "Selecione um item para remover.") {
            messageFound = true;
            box->accept();
        }
    });

    removeButton->click();
    QApplication::processEvents();
    page.close();

    if (!messageFound) {
        return fail("Mensagem de selecao obrigatoria para remocao nao foi exibida.");
    }

    return 0;
}

int runStockPageValidRemovalTest()
{
    StockPage page;
    int refreshCalls = 0;
    bool itemRemoved = false;
    bool confirmationCalled = false;
    QString removedItem;

    page.setListHandler([&refreshCalls, &itemRemoved]() {
        ++refreshCalls;
        return ControllerResult{
            true,
            "",
            "",
            itemRemoved ? "" : stockPayloadFor("Remocao teste").toStdString()
        };
    });
    page.setRemoveConfirmationHandler([&confirmationCalled](const QString &item) {
        confirmationCalled = item == "Remocao teste";
        return true;
    });
    page.setRemoveHandler([&itemRemoved, &removedItem](const QString &item) {
        removedItem = item;
        itemRemoved = true;
        return ControllerResult{true, "", "", "Item removido do estoque: Remocao teste\n"};
    });

    page.refreshData();
    QTableWidget *table = stockTable(page);
    if (table == nullptr || table->rowCount() != 1) {
        return fail("StockPage nao carregou item para remocao valida.");
    }

    table->selectRow(0);
    table->setCurrentCell(0, 0);
    if (!page.removeSelectedItem()) {
        return fail("Remocao valida na StockPage falhou.");
    }

    if (!confirmationCalled || removedItem != "Remocao teste" || refreshCalls != 2 || table->rowCount() != 0) {
        return fail("Remocao valida nao confirmou, chamou controller ou atualizou tabela corretamente.");
    }

    return 0;
}

int runStockPageRemovalCancelTest()
{
    StockPage page;
    int refreshCalls = 0;
    bool removeCalled = false;

    page.setListHandler([&refreshCalls]() {
        ++refreshCalls;
        return ControllerResult{true, "", "", stockPayloadFor("Cancelamento teste").toStdString()};
    });
    page.setRemoveConfirmationHandler([](const QString &) {
        return false;
    });
    page.setRemoveHandler([&removeCalled](const QString &) {
        removeCalled = true;
        return ControllerResult{true, "", "", ""};
    });

    page.refreshData();
    QTableWidget *table = stockTable(page);
    if (table == nullptr || table->rowCount() != 1) {
        return fail("StockPage nao carregou item para cancelamento de remocao.");
    }

    table->selectRow(0);
    table->setCurrentCell(0, 0);
    if (page.removeSelectedItem() || removeCalled || refreshCalls != 1) {
        return fail("Cancelamento de remocao nao foi respeitado.");
    }

    return 0;
}

int runStockPageMissingItemRemovalTest()
{
    StockPage page;
    int refreshCalls = 0;

    page.setListHandler([&refreshCalls]() {
        ++refreshCalls;
        return ControllerResult{true, "", "", stockPayloadFor("Inexistente teste").toStdString()};
    });
    page.setRemoveConfirmationHandler([](const QString &) {
        return true;
    });
    page.setRemoveHandler([](const QString &) {
        return ControllerResult{false, "Erro ao remover item: item nao encontrado para remocao: Inexistente teste\n", "SERVICE_ERROR", ""};
    });

    page.refreshData();
    QTableWidget *table = stockTable(page);
    if (table == nullptr || table->rowCount() != 1) {
        return fail("StockPage nao carregou item para teste de inexistente.");
    }

    table->selectRow(0);
    table->setCurrentCell(0, 0);

    bool messageFound = false;
    QTimer::singleShot(0, [&messageFound]() {
        QMessageBox *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
        if (box != nullptr && box->text().contains("item nao encontrado")) {
            messageFound = true;
            box->accept();
        }
    });

    if (page.removeSelectedItem() || refreshCalls != 1) {
        return fail("StockPage aceitou remocao de item inexistente.");
    }

    QApplication::processEvents();
    page.close();

    if (!messageFound) {
        return fail("Erro do controller na remocao nao foi exibido.");
    }

    return 0;
}

void configureStockPageWithPayload(StockPage &page, const QString &payload)
{
    page.setListHandler([payload]() {
        return ControllerResult{true, "", "", payload.toStdString()};
    });
    page.refreshData();
}

int runStockPageSearchByItemTest()
{
    StockPage page;
    configureStockPageWithPayload(page, sampleStockPayload());
    QTableWidget *table = stockTable(page);
    QLineEdit *search = stockSearch(page);
    QLabel *counter = stockCounter(page);
    if (table == nullptr || search == nullptr || counter == nullptr) {
        return fail("Controles de busca da StockPage nao foram encontrados.");
    }

    search->setText("sab");
    QApplication::processEvents();
    if (table->rowCount() != 1 || table->item(0, 0)->text() != "Sabao" || !counter->text().contains("Itens filtrados: 1")) {
        return fail("Busca por item na StockPage falhou.");
    }

    return 0;
}

int runStockPageSearchByCategoryTest()
{
    StockPage page;
    configureStockPageWithPayload(page, sampleStockPayload());
    QTableWidget *table = stockTable(page);
    QLineEdit *search = stockSearch(page);
    if (table == nullptr || search == nullptr) {
        return fail("Controles de busca por categoria nao foram encontrados.");
    }

    search->setText("limpeza");
    QApplication::processEvents();
    if (table->rowCount() != 1 || table->item(0, 1)->text() != "Limpeza") {
        return fail("Busca por categoria na StockPage falhou.");
    }

    return 0;
}

int runStockPageStatusFilterTest()
{
    StockPage page;
    configureStockPageWithPayload(page, sampleStockPayload());
    QTableWidget *table = stockTable(page);
    QComboBox *filter = stockStatusFilter(page);
    if (table == nullptr || filter == nullptr) {
        return fail("Filtro de status da StockPage nao foi encontrado.");
    }

    filter->setCurrentText("CRÍTICO");
    QApplication::processEvents();
    if (table->rowCount() != 1 || table->item(0, 6)->text() != "CRÍTICO") {
        return fail("Filtro por status CRITICO na StockPage falhou.");
    }

    filter->setCurrentText("SEM_VALIDADE");
    QApplication::processEvents();
    if (table->rowCount() != 1 || table->item(0, 0)->text() != "Sal") {
        return fail("Filtro por status SEM_VALIDADE na StockPage falhou.");
    }

    return 0;
}

int runStockPageBasicSortingTest()
{
    StockPage page;
    configureStockPageWithPayload(page, sampleStockPayload());
    QTableWidget *table = stockTable(page);
    if (table == nullptr || table->horizontalHeader() == nullptr) {
        return fail("Tabela para ordenacao nao foi encontrada.");
    }

    QMetaObject::invokeMethod(table->horizontalHeader(), "sectionClicked", Q_ARG(int, 1));
    QApplication::processEvents();
    if (table->rowCount() < 2 || table->item(0, 1)->text() != "Alimentos") {
        return fail("Ordenacao por categoria na StockPage falhou.");
    }

    QMetaObject::invokeMethod(table->horizontalHeader(), "sectionClicked", Q_ARG(int, 2));
    QApplication::processEvents();
    if (table->item(0, 0)->text() != "Leite") {
        return fail("Ordenacao numerica por quantidade na StockPage falhou.");
    }

    return 0;
}

int runStockPageEmptyTableTest()
{
    StockPage page;
    configureStockPageWithPayload(page, "");
    QTableWidget *table = stockTable(page);
    QLabel *counter = stockCounter(page);
    if (table == nullptr || counter == nullptr) {
        return fail("Tabela vazia da StockPage nao foi encontrada.");
    }

    if (table->rowCount() != 0 || !counter->text().contains("Total de itens: 0") || !counter->text().contains("Itens filtrados: 0")) {
        return fail("Estado de tabela vazia da StockPage falhou.");
    }

    return 0;
}

int runStockPageButtonsPreservedTest()
{
    StockPage page;
    configureStockPageWithPayload(page, sampleStockPayload());
    const QStringList expectedButtons = {
        "Atualizar",
        "Adicionar item",
        "Consumir item",
        "Editar item",
        "Remover item"
    };

    for (const QString &text : expectedButtons) {
        if (findButtonByText(page, text) == nullptr) {
            return fail("Botao esperado da StockPage nao foi preservado.");
        }
    }

    return 0;
}

int runDashboardPageInstantiableTest()
{
    DashboardPage page;
    if (page.findChild<QPushButton *>() == nullptr) {
        return fail("DashboardPage instanciavel nao criou controles.");
    }

    return 0;
}

int runInvestmentsPageOperationalDemoTest()
{
    InvestmentsPage page;
    page.resize(900, 520);
    page.show();
    QApplication::processEvents();

    const QList<QLabel *> labels = page.findChildren<QLabel *>();
    const QStringList expectedTexts = {
        "Investimentos",
        "Capital livre estimado",
        "Projeção mensal",
        "Projeção anual",
        "Juros compostos simulados",
        "Projeção temporal de capital",
        "Rendimento estimado",
        "Taxa simulada",
        "Projeção futura"
    };

    for (const QString &text : expectedTexts) {
        if (!labelHasVisibleText(labels, text)) {
            return fail("InvestmentsPage nao renderizou demo operacional esperada.");
        }
    }

    OperationalChartWidget *chart = page.findChild<OperationalChartWidget *>("InvestmentsProjectionChart");
    if (chart == nullptr || !chart->hasSeries()) {
        return fail("InvestmentsPage nao renderizou grafico de projecao.");
    }

    return 0;
}

int runShoppingPageOperationalRowsTest()
{
    ShoppingPage page;
    page.refreshData();

    QTableWidget *table = page.findChild<QTableWidget *>("DataTable");
    if (table == nullptr || table->rowCount() < 3) {
        return fail("ShoppingPage nao renderizou itens criticos de compra.");
    }

    bool cafeFound = false;
    bool costFound = false;
    for (int row = 0; row < table->rowCount(); ++row) {
        QTableWidgetItem *item = table->item(row, 0);
        QTableWidgetItem *cost = table->item(row, 4);
        cafeFound = cafeFound || (item != nullptr && item->text().contains("Cafe Premium"));
        costFound = costFound || (cost != nullptr && cost->text().contains("R$"));
    }
    if (!cafeFound || !costFound) {
        return fail("ShoppingPage nao sincronizou item e custo estimado.");
    }

    if (page.findChild<StatusBadge *>() == nullptr) {
        return fail("ShoppingPage nao renderizou badges de prioridade.");
    }

    return 0;
}

int runPiggyBankPageOperationalRowsTest()
{
    PiggyBankPage page;
    page.show();
    QApplication::processEvents();

    QTableWidget *table = page.findChild<QTableWidget *>("DataTable");
    if (table == nullptr || table->rowCount() < 4) {
        return fail("PiggyBankPage nao renderizou cofrinhos persistidos.");
    }

    bool reserveFound = false;
    bool progressFound = false;
    for (int row = 0; row < table->rowCount(); ++row) {
        QTableWidgetItem *name = table->item(row, 0);
        reserveFound = reserveFound || (name != nullptr && name->text().contains("Reserva Mercado"));
        progressFound = progressFound || table->cellWidget(row, 3) != nullptr;
    }
    if (!reserveFound || !progressFound) {
        return fail("PiggyBankPage nao sincronizou tabela e barras de progresso.");
    }

    return 0;
}

int runStatusBadgeTest()
{
    StatusBadge badge("OK");
    if (badge.status() != "OK" || badge.category() != "success" || badge.toolTip().isEmpty()) {
        return fail("StatusBadge OK invalido.");
    }

    badge.setStatus("ATENÇÃO");
    if (badge.status() != "ATENÇÃO" || badge.category() != "warning" || !badge.styleSheet().contains(ThemeManager::accentColor())) {
        return fail("StatusBadge ATENCAO invalido.");
    }

    badge.setStatus("CRITICO");
    if (badge.status() != "CRÍTICO" || badge.category() != "error") {
        return fail("StatusBadge CRITICO invalido.");
    }

    badge.setStatus("VENCIDO");
    if (badge.status() != "VENCIDO" || badge.category() != "error") {
        return fail("StatusBadge VENCIDO invalido.");
    }

    badge.setStatus("SEM_VALIDADE");
    if (badge.status() != "SEM_VALIDADE" || badge.category() != "neutral") {
        return fail("StatusBadge SEM_VALIDADE invalido.");
    }

    badge.setStatus("WARNING");
    if (badge.status() != "WARNING" || badge.category() != "warning") {
        return fail("StatusBadge WARNING invalido.");
    }

    badge.setStatus("ERROR");
    if (badge.status() != "ERROR" || badge.category() != "error") {
        return fail("StatusBadge ERROR invalido.");
    }

    badge.setStatus("INFO");
    if (badge.status() != "INFO" || badge.category() != "neutral") {
        return fail("StatusBadge INFO invalido.");
    }

    return 0;
}

int runFeedbackBannerTest()
{
    FeedbackBanner banner;
    banner.resize(420, 80);
    banner.show();
    banner.showSuccess("Operação concluída");
    QApplication::processEvents();
    if (!banner.isVisible()) {
        return fail("FeedbackBanner nao exibiu sucesso.");
    }
    if (banner.height() < banner.minimumHeight()) {
        return fail("FeedbackBanner ficou menor que a altura minima.");
    }

    banner.showError("Erro ao carregar dados");
    QApplication::processEvents();
    if (!banner.isVisible()) {
        return fail("FeedbackBanner nao exibiu erro.");
    }

    banner.clear();
    if (banner.isVisible()) {
        return fail("FeedbackBanner nao limpou visualizacao.");
    }

    return 0;
}

int runOperationalChartWidgetTest()
{
    OperationalChartWidget chart;
    chart.resize(640, 280);
    chart.show();
    chart.setPlaceholder("Aguardando dados operacionais.");
    QApplication::processEvents();

    QPixmap placeholderPixmap(chart.size());
    placeholderPixmap.fill(Qt::transparent);
    chart.render(&placeholderPixmap);
    if (placeholderPixmap.isNull() || chart.hasSeries()) {
        return fail("OperationalChartWidget placeholder invalido.");
    }

    chart.setValues(QVector<double>{1.0, 2.0, 2.8, 4.2, 5.0}, "Serie teste");
    QApplication::processEvents();

    QPixmap seriesPixmap(chart.size());
    seriesPixmap.fill(Qt::transparent);
    chart.render(&seriesPixmap);
    if (seriesPixmap.isNull() || !chart.hasSeries()) {
        return fail("OperationalChartWidget nao renderizou serie valida.");
    }

    return 0;
}

int runInfoCardTextLayoutTest()
{
    InfoCard card("Economia estimada", "R$ 48.50", "Aguardando cadastro", "$", nullptr);
    card.resize(280, 156);
    card.show();
    QApplication::processEvents();

    const QList<QLabel *> labels = card.findChildren<QLabel *>();
    if (!labelHasVisibleText(labels, "Economia estimada") ||
        !labelHasVisibleText(labels, "R$ 48.50") ||
        !labelHasVisibleText(labels, "Aguardando cadastro")) {
        return fail("InfoCard nao renderizou titulo, valor e subtitulo esperados.");
    }

    for (QLabel *label : labels) {
        if (!labelHeightFits(label)) {
            return fail("InfoCard renderizou label com altura insuficiente.");
        }
    }

    return 0;
}

int runPagesUseBadgesTest()
{
    DashboardPage dashboard;
    StockPage stock;
    ShoppingPage shopping;
    ReportsPage reports;
    LogsPage logs;
    SettingsPage settings;

    if (dashboard.findChild<StatusBadge *>() == nullptr ||
        stock.findChild<StatusBadge *>() == nullptr ||
        shopping.findChild<StatusBadge *>() == nullptr ||
        reports.findChild<StatusBadge *>() == nullptr ||
        logs.findChild<StatusBadge *>() == nullptr ||
        settings.findChild<StatusBadge *>() == nullptr) {
        return fail("Paginas integradas nao possuem StatusBadge operacional.");
    }

    return 0;
}

int runThemeManagerTests()
{
    const QString qss = ThemeManager::applicationStyleSheet();
    if (qss.isEmpty() ||
        !qss.contains(ThemeManager::accentColor()) ||
        !qss.contains(ThemeManager::backgroundColor()) ||
        !qss.contains("QWidget#InfoCard") ||
        !qss.contains("QStatusBar")) {
        return fail("ThemeManager nao carregou QSS oficial.");
    }

    MainWindow window;
    if (!window.styleSheet().contains(ThemeManager::accentColor())) {
        return fail("MainWindow nao aplicou tema global.");
    }

    InfoCard card("Teste", "1", "Status");
    if (card.objectName() != "InfoCard" || !qss.contains("QLabel#InfoCardStatus")) {
        return fail("InfoCard nao esta preparado para tema global.");
    }

    return 0;
}

int runSidebarIconResourceTests()
{
    if (!QFile::exists(":/icons/dashboard.svg") ||
        !QFile::exists(":/icons/dashboard_active.svg") ||
        !QFile::exists(":/icons/investments.svg") ||
        !QFile::exists(":/icons/investments_active.svg") ||
        !QFile::exists(":/icons/about.svg") ||
        !QFile::exists(":/icons/about_active.svg") ||
        !QFile::exists(":/logo/qkage_logo.svg")) {
        return fail("Icones da sidebar nao foram carregados via Qt Resource System.");
    }

    MainWindow window;
    const QStringList expectedButtons = {
        "Dashboard",
        "Estoque",
        "Compras",
        "Cofrinhos",
        "Investimentos",
        "Relatórios",
        "Logs",
        "Configurações",
        "Sobre"
    };

    for (const QString &text : expectedButtons) {
        QPushButton *button = findButtonByText(window, text);
        if (button == nullptr || button->icon().isNull()) {
            return fail("Botao da sidebar sem icone carregado.");
        }
    }

    QPushButton *dashboardButton = findButtonByText(window, "Dashboard");
    QPushButton *stockButton = findButtonByText(window, "Estoque");
    const qint64 dashboardIconKey = dashboardButton->icon().cacheKey();
    stockButton->click();
    QApplication::processEvents();
    if (!stockButton->isChecked() || dashboardButton->isChecked() || dashboardButton->icon().cacheKey() == dashboardIconKey) {
        return fail("Destaque visual ativo da sidebar nao mudou com navegacao.");
    }

    return 0;
}

int runDashboardRefreshDataTest()
{
    DashboardPage page;
    bool handlerCalled = false;
    page.setReportHandler([&handlerCalled]() {
        handlerCalled = true;
        return ControllerResult{true, "", "", sampleDashboardPayload().toStdString()};
    });
    page.setLogsHandler([](const QString &level) {
        if (level == "ERROR") {
            return ControllerResult{true, "", "", QString(
                "KAGE Home Supply - Logs operacionais\n\n"
                "[2026-05-25 11:00:00] [ERROR] [BackupManager] Falha simulada\n").toStdString()};
        }
        return ControllerResult{true, "", "", QString(
            "KAGE Home Supply - Logs operacionais\n\n"
            "[2026-05-25 10:00:00] [INFO] [BackupManager] Backup concluido em: backups/test\n").toStdString()};
    });
    page.refreshData();

    const QList<QLabel *> labels = page.findChildren<QLabel *>();
    bool totalFound = false;
    bool savingsFound = false;
    for (QLabel *label : labels) {
        totalFound = totalFound || label->text() == "5";
        savingsFound = savingsFound || label->text() == "R$ 48.50";
    }

    StatusBadge *badge = page.findChild<StatusBadge *>("DashboardStatusBadge");
    OperationalChartWidget *chart = page.findChild<OperationalChartWidget *>("DashboardOperationalChart");
    bool summaryFound = false;
    for (QLabel *label : labels) {
        summaryFound = summaryFound || label->text() == "Resumo operacional";
    }

    if (!handlerCalled || !totalFound || !savingsFound || !summaryFound ||
        badge == nullptr || badge->status() != "WARNING" ||
        chart == nullptr || !chart->hasSeries()) {
        return fail("DashboardPage refreshData nao atualizou indicadores reais.");
    }

    return 0;
}

int runDashboardOperationalStatusSuccessTest()
{
    DashboardPage page;
    page.setReportHandler([]() {
        return ControllerResult{true, "", "", dashboardPayloadWithCounts(5, 0, 0).toStdString()};
    });
    page.setLogsHandler([](const QString &) {
        return ControllerResult{true, "", "", ""};
    });
    page.refreshData();

    StatusBadge *badge = page.findChild<StatusBadge *>("DashboardStatusBadge");
    if (badge == nullptr || badge->status() != "OK") {
        return fail("DashboardPage nao definiu Status operacional OK apos sucesso.");
    }

    bool successBannerFound = false;
    for (QLabel *label : page.findChildren<QLabel *>()) {
        successBannerFound = successBannerFound || label->text() == "Indicadores atualizados via controller.";
    }
    if (!successBannerFound) {
        return fail("DashboardPage nao sincronizou banner de sucesso com status OK.");
    }

    return 0;
}

int runDashboardOperationalStatusFailureTest()
{
    DashboardPage page;
    page.setReportHandler([]() {
        return ControllerResult{false, "", "REPORT_ERROR", ""};
    });
    page.setLogsHandler([](const QString &) {
        return ControllerResult{true, "", "", ""};
    });
    page.refreshData();

    StatusBadge *badge = page.findChild<StatusBadge *>("DashboardStatusBadge");
    if (badge == nullptr || badge->status() != "ERROR") {
        return fail("DashboardPage nao definiu Status operacional ERROR apos falha.");
    }

    page.setReportHandler([]() {
        return ControllerResult{true, "", "", dashboardPayloadWithCounts(5, 0, 0).toStdString()};
    });
    page.refreshData();

    if (badge->status() != "OK") {
        return fail("DashboardPage manteve ERROR residual apos refresh bem-sucedido.");
    }

    return 0;
}

int runDashboardOperationalStatusWarningTest()
{
    DashboardPage page;
    page.setReportHandler([]() {
        return ControllerResult{true, "", "", dashboardPayloadWithCounts(5, 0, 2).toStdString()};
    });
    page.setLogsHandler([](const QString &) {
        return ControllerResult{true, "", "", ""};
    });
    page.refreshData();

    StatusBadge *badge = page.findChild<StatusBadge *>("DashboardStatusBadge");
    if (badge == nullptr || badge->status() != "WARNING") {
        return fail("DashboardPage nao definiu Status operacional WARNING para alerta.");
    }

    bool warningBannerFound = false;
    for (QLabel *label : page.findChildren<QLabel *>()) {
        warningBannerFound = warningBannerFound || label->text() == "Indicadores atualizados com alertas operacionais.";
    }
    if (!warningBannerFound) {
        return fail("DashboardPage nao sincronizou banner de warning com status WARNING.");
    }

    return 0;
}

int runDashboardGaugesRenderTest()
{
    DashboardPage page;
    page.setReportHandler([]() {
        return ControllerResult{true, "", "", sampleDashboardPayload().toStdString()};
    });
    page.setLogsHandler([](const QString &) {
        return ControllerResult{true, "", "", ""};
    });
    page.refreshData();

    const QStringList gaugeNames = {
        "stockHealthGauge",
        "operationalEfficiencyGauge",
        "freeCapitalGauge",
        "DashboardOperationalChart"
    };

    for (const QString &name : gaugeNames) {
        QWidget *gauge = page.findChild<QWidget *>(name);
        if (gauge == nullptr || gauge->width() <= 0 || gauge->height() <= 0) {
            return fail("Gauge da DashboardPage nao foi criado.");
        }

        QPixmap pixmap(gauge->size());
        pixmap.fill(Qt::transparent);
        gauge->render(&pixmap);
        if (pixmap.isNull()) {
            return fail("Gauge da DashboardPage nao renderizou.");
        }
    }

    return 0;
}

int runDashboardResizeAndInfoCardRenderTest()
{
    DashboardPage page;
    page.resize(960, 640);
    page.show();
    QApplication::processEvents();
    page.resize(1280, 760);
    QApplication::processEvents();

    const QList<InfoCard *> cards = page.findChildren<InfoCard *>();
    if (cards.size() < 9) {
        return fail("DashboardPage nao renderizou InfoCards esperados.");
    }
    if (page.findChild<OperationalChartWidget *>("DashboardOperationalChart") == nullptr) {
        return fail("DashboardPage nao renderizou grafico operacional.");
    }

    return 0;
}

int runDashboardLayoutNoCriticalWarningsTest()
{
    capturedCriticalMessages.clear();
    previousMessageHandler = qInstallMessageHandler(captureCriticalMessages);

    DashboardPage page;
    page.setReportHandler([]() {
        return ControllerResult{true, "", "", sampleDashboardPayload().toStdString()};
    });
    page.setLogsHandler([](const QString &) {
        return ControllerResult{true, "", "", ""};
    });
    page.resize(1120, 720);
    page.show();
    page.refreshData();
    QApplication::processEvents();

    QPixmap pixmap(page.size());
    pixmap.fill(Qt::transparent);
    page.render(&pixmap);
    QApplication::processEvents();

    qInstallMessageHandler(previousMessageHandler);
    previousMessageHandler = nullptr;

    if (!capturedCriticalMessages.isEmpty()) {
        return fail("DashboardPage gerou warnings criticos de layout.");
    }

    const QList<QLabel *> labels = page.findChildren<QLabel *>();
    const QStringList expectedTexts = {
        "Estoque monitorado",
        "Mensal",
        "Metas",
        "Monitoramento",
        "Resumo operacional"
    };

    for (const QString &text : expectedTexts) {
        if (!labelHasVisibleText(labels, text)) {
            return fail("DashboardPage nao renderizou textos operacionais esperados.");
        }
    }

    for (QLabel *label : labels) {
        if ((label->objectName() == "InfoCardTitle" ||
             label->objectName() == "InfoCardValue" ||
             label->objectName() == "InfoCardStatus" ||
             label->objectName() == "FeedbackBannerMessage") &&
            !labelHeightFits(label)) {
            return fail("DashboardPage renderizou label com clipping vertical.");
        }
    }

    return 0;
}

int runReportsPageInstantiableTest()
{
    ReportsPage page;
    if (findButtonByText(page, "Atualizar relatório") == nullptr ||
        findButtonByText(page, "Exportar Markdown") == nullptr ||
        findButtonByText(page, "Abrir pasta de relatórios") == nullptr) {
        return fail("ReportsPage nao preservou botoes esperados.");
    }

    return 0;
}

int runReportsPageRefreshDataTest()
{
    ReportsPage page;
    bool refreshCalled = false;
    page.setMonthlyReportHandler([&refreshCalled]() {
        refreshCalled = true;
        return ControllerResult{true, "", "", sampleReportsPayload().toStdString()};
    });
    page.refreshData();

    QTextEdit *view = page.findChild<QTextEdit *>("MonthlyReportView");
    if (!refreshCalled || view == nullptr || view->isHidden() || !view->toPlainText().contains("[RESUMO FINAL]")) {
        return fail("ReportsPage refreshData nao exibiu relatorio consolidado.");
    }

    return 0;
}

int runReportsPageExportViaControllerTest()
{
    ReportsPage page;
    bool exportCalled = false;
    page.setExportReportHandler([&exportCalled]() {
        exportCalled = true;
        return ControllerResult{true, "", "", "Status da exportacao: Relatorio exportado.\nArquivo gerado: reports/test.md\n"};
    });

    QPushButton *exportButton = findButtonByText(page, "Exportar Markdown");
    if (exportButton == nullptr) {
        return fail("Botao Exportar Markdown nao foi encontrado.");
    }

    exportButton->click();
    QApplication::processEvents();

    if (!exportCalled) {
        return fail("ReportsPage nao acionou exportacao via controller.");
    }

    return 0;
}

int runReportsPageNoDataStateTest()
{
    ReportsPage page;
    page.setMonthlyReportHandler([]() {
        return ControllerResult{true, "", "", noDataReportsPayload().toStdString()};
    });
    page.refreshData();

    const QList<QLabel *> labels = page.findChildren<QLabel *>();
    bool messageFound = false;
    for (QLabel *label : labels) {
        messageFound = messageFound || label->text() == "Ainda não há dados suficientes para gerar um relatório completo.";
    }

    if (!messageFound) {
        return fail("ReportsPage nao exibiu estado sem dados.");
    }

    return 0;
}

int runReportsPageErrorHandledTest()
{
    ReportsPage page;
    page.setMonthlyReportHandler([]() {
        return ControllerResult{false, "Erro ao gerar relatorio mensal: falha simulada\n", "SERVICE_ERROR", ""};
    });
    page.refreshData();

    const QList<QLabel *> labels = page.findChildren<QLabel *>();
    bool errorFound = false;
    for (QLabel *label : labels) {
        errorFound = errorFound || label->text().contains("falha simulada");
    }

    if (!errorFound) {
        return fail("ReportsPage nao exibiu erro visualmente.");
    }

    return 0;
}

int runLogsPageInstantiableTest()
{
    LogsPage page;
    if (findButtonByText(page, "Atualizar logs") == nullptr ||
        findButtonByText(page, "Limpar visualização") == nullptr ||
        findButtonByText(page, "Abrir pasta de logs") == nullptr) {
        return fail("LogsPage nao criou botoes esperados.");
    }

    return 0;
}

int runLogsPageRefreshDataTest()
{
    LogsPage page;
    bool refreshCalled = false;
    page.setLogsHandler([&refreshCalled](const QString &level) {
        refreshCalled = level.isEmpty();
        return ControllerResult{true, "", "", sampleLogsPayload().toStdString()};
    });
    page.refreshData();

    QTableWidget *table = page.findChild<QTableWidget *>("LogsTable");
    if (!refreshCalled || table == nullptr || table->rowCount() != 4 ||
        table->item(0, 0)->text() != "2026-05-25 10:00:00" ||
        table->item(0, 1)->text() != "INFO" ||
        table->item(0, 2)->text() != "CsvInventoryWriter") {
        return fail("LogsPage refreshData nao renderizou logs.");
    }

    return 0;
}

int runLogsPageFilterByLevelTest()
{
    LogsPage page;
    QString requestedLevel;
    page.setLogsHandler([&requestedLevel](const QString &level) {
        requestedLevel = level;
        return ControllerResult{true, "", "", QString(
            "KAGE Home Supply - Logs operacionais\n"
            "Arquivo: logs/qkage.log\n"
            "Filtro: ERROR\n\n"
            "[2026-05-25 10:02:00] [ERROR] [CsvInventoryWriter] Falha ao validar item\n").toStdString()};
    });

    QComboBox *filter = page.findChild<QComboBox *>("logsLevelFilter");
    QTableWidget *table = page.findChild<QTableWidget *>("LogsTable");
    if (filter == nullptr || table == nullptr) {
        return fail("Filtro ou tabela da LogsPage nao encontrado.");
    }

    filter->setCurrentText("ERROR");
    QApplication::processEvents();
    if (requestedLevel != "ERROR" || table->rowCount() != 1 || table->item(0, 1)->text() != "ERROR") {
        return fail("Filtro por nivel da LogsPage falhou.");
    }

    return 0;
}

int runLogsPageEmptyStateTest()
{
    LogsPage page;
    page.setLogsHandler([](const QString &) {
        return ControllerResult{true, "", "", emptyLogsPayload().toStdString()};
    });
    page.refreshData();

    const QList<QLabel *> labels = page.findChildren<QLabel *>();
    bool messageFound = false;
    for (QLabel *label : labels) {
        messageFound = messageFound || label->text() == "Nenhum log encontrado.";
    }

    QTableWidget *table = page.findChild<QTableWidget *>("LogsTable");
    if (!messageFound || table == nullptr || table->rowCount() != 0) {
        return fail("Estado sem logs da LogsPage falhou.");
    }

    return 0;
}

int runLogsPageErrorHandledTest()
{
    LogsPage page;
    page.setLogsHandler([](const QString &) {
        return ControllerResult{false, "Erro ao ler logs: falha simulada\n", "SERVICE_ERROR", ""};
    });
    page.refreshData();

    const QList<QLabel *> labels = page.findChildren<QLabel *>();
    bool errorFound = false;
    for (QLabel *label : labels) {
        errorFound = errorFound || label->text().contains("falha simulada");
    }

    if (!errorFound) {
        return fail("LogsPage nao tratou erro visualmente.");
    }

    return 0;
}

int runSettingsPageInstantiableTest()
{
    SettingsPage page;
    if (findButtonByText(page, "Atualizar configurações") == nullptr ||
        findButtonByText(page, "Abrir pasta de dados") == nullptr ||
        findButtonByText(page, "Abrir pasta de backups") == nullptr ||
        findButtonByText(page, "Abrir pasta de relatórios") == nullptr) {
        return fail("SettingsPage nao criou botoes esperados.");
    }

    return 0;
}

int runSettingsPageRefreshDataTest()
{
    SettingsPage page;
    bool refreshCalled = false;
    page.setConfigHandler([&refreshCalled]() {
        refreshCalled = true;
        return ControllerResult{true, "", "", sampleConfigPayload().toStdString()};
    });
    page.refreshData();

    QTableWidget *table = page.findChild<QTableWidget *>("SettingsTable");
    if (!refreshCalled || table == nullptr || table->rowCount() != 9) {
        return fail("SettingsPage refreshData nao carregou tabela.");
    }

    bool currencyFound = false;
    bool reportsFound = false;
    for (int row = 0; row < table->rowCount(); ++row) {
        if (table->item(row, 0)->text() == "currency" && table->item(row, 1)->text() == "BRL") {
            currencyFound = true;
        }
        if (table->item(row, 0)->text() == "reports_enabled" && table->item(row, 1)->text() == "false") {
            reportsFound = true;
        }
    }

    if (!currencyFound || !reportsFound) {
        return fail("SettingsPage nao exibiu configuracoes esperadas.");
    }

    return 0;
}

int runSettingsPageMissingConfigTest()
{
    SettingsPage page;
    page.setConfigHandler([]() {
        return ControllerResult{true, "", "", sampleConfigPayload().toStdString()};
    });
    page.refreshData();

    const QList<QLabel *> labels = page.findChildren<QLabel *>();
    bool friendlyMessageFound = false;
    for (QLabel *label : labels) {
        friendlyMessageFound = friendlyMessageFound || label->text().contains("Configurações carregadas via controller");
    }

    if (!friendlyMessageFound) {
        return fail("SettingsPage nao exibiu mensagem amigavel para config padrao.");
    }

    return 0;
}

int runSettingsPageErrorHandledTest()
{
    SettingsPage page;
    page.setConfigHandler([]() {
        return ControllerResult{false, "Erro [INVALID_CONFIG]: falha simulada\n", "INVALID_CONFIG", ""};
    });
    page.refreshData();

    const QList<QLabel *> labels = page.findChildren<QLabel *>();
    bool errorFound = false;
    for (QLabel *label : labels) {
        errorFound = errorFound || label->text().contains("falha simulada");
    }

    if (!errorFound) {
        return fail("SettingsPage nao tratou erro visualmente.");
    }

    return 0;
}

int runSettingsPageButtonsPresentTest()
{
    SettingsPage page;
    const QStringList expectedButtons = {
        "Atualizar configurações",
        "Abrir pasta de dados",
        "Abrir pasta de backups",
        "Abrir pasta de relatórios"
    };

    for (const QString &buttonText : expectedButtons) {
        if (findButtonByText(page, buttonText) == nullptr) {
            return fail("Botao esperado da SettingsPage nao foi encontrado.");
        }
    }

    return 0;
}

bool dialogContainsLabelText(QWidget &dialog, const QString &expected)
{
    const QList<QLabel *> labels = dialog.findChildren<QLabel *>();
    for (QLabel *label : labels) {
        if (label->text().contains(expected)) {
            return true;
        }
    }

    return false;
}

int runAboutDialogTests()
{
    AboutDialog dialog;
    if (dialog.windowTitle() != "Sobre qKAGE_HOME_SUPPLY") {
        return fail("AboutDialog titulo incorreto.");
    }

    if (!dialogContainsLabelText(dialog, "qKAGE_HOME_SUPPLY")) {
        return fail("AboutDialog nao exibiu nome do projeto.");
    }

    if (!dialogContainsLabelText(dialog, "Version: v1.2.1-beta")) {
        return fail("AboutDialog nao exibiu versao.");
    }

    if (!dialogContainsLabelText(dialog, "Anderson Nogueira")) {
        return fail("AboutDialog nao exibiu autor.");
    }

    if (!dialogContainsLabelText(dialog, "© 2026 Anderson Nogueira")) {
        return fail("AboutDialog nao exibiu copyright.");
    }

    return 0;
}
}

int main(int argc, char *argv[])
{
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", QByteArray("offscreen"));
    }

    QApplication app(argc, argv);

    const int dialogStatus = runAddStockItemDialogTests();
    if (dialogStatus != 0) {
        return dialogStatus;
    }

    const int consumeDialogStatus = runConsumeStockItemDialogTests();
    if (consumeDialogStatus != 0) {
        return consumeDialogStatus;
    }

    const int editDialogStatus = runEditStockItemDialogTests();
    if (editDialogStatus != 0) {
        return editDialogStatus;
    }

    const int editSelectionStatus = runStockPageEditWithoutSelectionTest();
    if (editSelectionStatus != 0) {
        return editSelectionStatus;
    }

    const int removeSelectionStatus = runStockPageRemoveWithoutSelectionTest();
    if (removeSelectionStatus != 0) {
        return removeSelectionStatus;
    }

    const int validRemovalStatus = runStockPageValidRemovalTest();
    if (validRemovalStatus != 0) {
        return validRemovalStatus;
    }

    const int cancelRemovalStatus = runStockPageRemovalCancelTest();
    if (cancelRemovalStatus != 0) {
        return cancelRemovalStatus;
    }

    const int missingRemovalStatus = runStockPageMissingItemRemovalTest();
    if (missingRemovalStatus != 0) {
        return missingRemovalStatus;
    }

    const int searchItemStatus = runStockPageSearchByItemTest();
    if (searchItemStatus != 0) {
        return searchItemStatus;
    }

    const int searchCategoryStatus = runStockPageSearchByCategoryTest();
    if (searchCategoryStatus != 0) {
        return searchCategoryStatus;
    }

    const int filterStatus = runStockPageStatusFilterTest();
    if (filterStatus != 0) {
        return filterStatus;
    }

    const int sortingStatus = runStockPageBasicSortingTest();
    if (sortingStatus != 0) {
        return sortingStatus;
    }

    const int emptyTableStatus = runStockPageEmptyTableTest();
    if (emptyTableStatus != 0) {
        return emptyTableStatus;
    }

    const int buttonsStatus = runStockPageButtonsPreservedTest();
    if (buttonsStatus != 0) {
        return buttonsStatus;
    }

    const int dashboardStatus = runDashboardPageInstantiableTest();
    if (dashboardStatus != 0) {
        return dashboardStatus;
    }

    const int investmentsStatus = runInvestmentsPageOperationalDemoTest();
    if (investmentsStatus != 0) {
        return investmentsStatus;
    }

    const int shoppingOperationalStatus = runShoppingPageOperationalRowsTest();
    if (shoppingOperationalStatus != 0) {
        return shoppingOperationalStatus;
    }

    const int piggyOperationalStatus = runPiggyBankPageOperationalRowsTest();
    if (piggyOperationalStatus != 0) {
        return piggyOperationalStatus;
    }

    const int badgeStatus = runStatusBadgeTest();
    if (badgeStatus != 0) {
        return badgeStatus;
    }

    const int feedbackStatus = runFeedbackBannerTest();
    if (feedbackStatus != 0) {
        return feedbackStatus;
    }

    const int chartStatus = runOperationalChartWidgetTest();
    if (chartStatus != 0) {
        return chartStatus;
    }

    const int infoCardTextLayoutStatus = runInfoCardTextLayoutTest();
    if (infoCardTextLayoutStatus != 0) {
        return infoCardTextLayoutStatus;
    }

    const int pagesBadgeStatus = runPagesUseBadgesTest();
    if (pagesBadgeStatus != 0) {
        return pagesBadgeStatus;
    }

    const int themeStatus = runThemeManagerTests();
    if (themeStatus != 0) {
        return themeStatus;
    }

    const int sidebarIconStatus = runSidebarIconResourceTests();
    if (sidebarIconStatus != 0) {
        return sidebarIconStatus;
    }

    const int dashboardRefreshStatus = runDashboardRefreshDataTest();
    if (dashboardRefreshStatus != 0) {
        return dashboardRefreshStatus;
    }

    const int dashboardOperationalStatusSuccess = runDashboardOperationalStatusSuccessTest();
    if (dashboardOperationalStatusSuccess != 0) {
        return dashboardOperationalStatusSuccess;
    }

    const int dashboardOperationalStatusFailure = runDashboardOperationalStatusFailureTest();
    if (dashboardOperationalStatusFailure != 0) {
        return dashboardOperationalStatusFailure;
    }

    const int dashboardOperationalStatusWarning = runDashboardOperationalStatusWarningTest();
    if (dashboardOperationalStatusWarning != 0) {
        return dashboardOperationalStatusWarning;
    }

    const int dashboardGaugeStatus = runDashboardGaugesRenderTest();
    if (dashboardGaugeStatus != 0) {
        return dashboardGaugeStatus;
    }

    const int dashboardResizeStatus = runDashboardResizeAndInfoCardRenderTest();
    if (dashboardResizeStatus != 0) {
        return dashboardResizeStatus;
    }

    const int dashboardLayoutWarningsStatus = runDashboardLayoutNoCriticalWarningsTest();
    if (dashboardLayoutWarningsStatus != 0) {
        return dashboardLayoutWarningsStatus;
    }

    const int reportsInstantiableStatus = runReportsPageInstantiableTest();
    if (reportsInstantiableStatus != 0) {
        return reportsInstantiableStatus;
    }

    const int reportsRefreshStatus = runReportsPageRefreshDataTest();
    if (reportsRefreshStatus != 0) {
        return reportsRefreshStatus;
    }

    const int reportsExportStatus = runReportsPageExportViaControllerTest();
    if (reportsExportStatus != 0) {
        return reportsExportStatus;
    }

    const int reportsNoDataStatus = runReportsPageNoDataStateTest();
    if (reportsNoDataStatus != 0) {
        return reportsNoDataStatus;
    }

    const int reportsErrorStatus = runReportsPageErrorHandledTest();
    if (reportsErrorStatus != 0) {
        return reportsErrorStatus;
    }

    const int logsInstantiableStatus = runLogsPageInstantiableTest();
    if (logsInstantiableStatus != 0) {
        return logsInstantiableStatus;
    }

    const int logsRefreshStatus = runLogsPageRefreshDataTest();
    if (logsRefreshStatus != 0) {
        return logsRefreshStatus;
    }

    const int logsFilterStatus = runLogsPageFilterByLevelTest();
    if (logsFilterStatus != 0) {
        return logsFilterStatus;
    }

    const int logsEmptyStatus = runLogsPageEmptyStateTest();
    if (logsEmptyStatus != 0) {
        return logsEmptyStatus;
    }

    const int logsErrorStatus = runLogsPageErrorHandledTest();
    if (logsErrorStatus != 0) {
        return logsErrorStatus;
    }

    const int settingsInstantiableStatus = runSettingsPageInstantiableTest();
    if (settingsInstantiableStatus != 0) {
        return settingsInstantiableStatus;
    }

    const int settingsRefreshStatus = runSettingsPageRefreshDataTest();
    if (settingsRefreshStatus != 0) {
        return settingsRefreshStatus;
    }

    const int settingsMissingStatus = runSettingsPageMissingConfigTest();
    if (settingsMissingStatus != 0) {
        return settingsMissingStatus;
    }

    const int settingsErrorStatus = runSettingsPageErrorHandledTest();
    if (settingsErrorStatus != 0) {
        return settingsErrorStatus;
    }

    const int settingsButtonsStatus = runSettingsPageButtonsPresentTest();
    if (settingsButtonsStatus != 0) {
        return settingsButtonsStatus;
    }

    const int aboutStatus = runAboutDialogTests();
    if (aboutStatus != 0) {
        return aboutStatus;
    }

    DashboardPage dashboard;
    StockPage stock;
    ShoppingPage shopping;

    MainWindow window;
    window.show();
    QApplication::processEvents();

    if (!window.isVisible()) {
        return fail("MainWindow nao ficou visivel.");
    }

    if (window.windowTitle() != "qKAGE_HOME_SUPPLY") {
        return fail("Titulo da MainWindow inesperado.");
    }

    if (window.windowIcon().isNull()) {
        return fail("MainWindow nao carregou icone da aplicacao.");
    }

    QLabel *brandLabel = window.findChild<QLabel *>("BrandLabel");
    if (brandLabel == nullptr ||
        !brandLabel->text().contains("qKage") ||
        !brandLabel->text().contains("Home Supply") ||
        brandLabel->wordWrap() ||
        brandLabel->width() + 2 < brandLabel->sizeHint().width()) {
        return fail("Branding principal da sidebar nao esta em linha unica sem clipping.");
    }

    if (window.minimumWidth() < 1120 || window.minimumHeight() < 720) {
        return fail("MainWindow nao definiu tamanho minimo adequado.");
    }

    if (window.statusBar() == nullptr || window.statusBar()->currentMessage() != "Sistema pronto") {
        return fail("Status bar nao exibiu o estado do sistema.");
    }

    bool versionStatusFound = false;
    bool systemStatusFound = false;
    const QList<QLabel *> statusLabels = window.statusBar()->findChildren<QLabel *>();
    for (QLabel *label : statusLabels) {
        versionStatusFound = versionStatusFound || label->text().contains("v1.2.1-beta");
        systemStatusFound = systemStatusFound || label->text().contains("System Status");
    }
    if (!versionStatusFound || !systemStatusFound) {
        return fail("Status bar nao exibiu versao ou System Status.");
    }

    QStackedWidget *pages = window.findChild<QStackedWidget *>();
    if (pages == nullptr || pages->count() < 8) {
        return fail("QStackedWidget principal nao foi encontrado.");
    }

    if (dynamic_cast<DashboardPage *>(pages->currentWidget()) == nullptr) {
        return fail("DashboardPage nao foi carregada inicialmente.");
    }

    QPushButton *stockButton = findButtonByText(window, "Estoque");
    QPushButton *shoppingButton = findButtonByText(window, "Compras");
    QPushButton *investmentsButton = findButtonByText(window, "Investimentos");
    QPushButton *dashboardButton = findButtonByText(window, "Dashboard");
    QPushButton *aboutButton = findButtonByText(window, "Sobre");
    if (stockButton == nullptr || shoppingButton == nullptr || investmentsButton == nullptr || dashboardButton == nullptr || aboutButton == nullptr) {
        return fail("Sidebar nao contem os botoes esperados.");
    }

    stockButton->click();
    QApplication::processEvents();
    if (dynamic_cast<StockPage *>(pages->currentWidget()) == nullptr) {
        return fail("StockPage nao foi carregada pela sidebar.");
    }

    shoppingButton->click();
    QApplication::processEvents();
    if (dynamic_cast<ShoppingPage *>(pages->currentWidget()) == nullptr) {
        return fail("ShoppingPage nao foi carregada pela sidebar.");
    }

    investmentsButton->click();
    QApplication::processEvents();
    if (dynamic_cast<InvestmentsPage *>(pages->currentWidget()) == nullptr) {
        return fail("InvestmentsPage nao foi carregada pela sidebar.");
    }

    dashboardButton->click();
    QApplication::processEvents();
    if (dynamic_cast<DashboardPage *>(pages->currentWidget()) == nullptr) {
        return fail("DashboardPage nao foi recarregada pela sidebar.");
    }

    window.close();
    QApplication::processEvents();
    return 0;
}
