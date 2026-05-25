#include "SettingsPage.h"

#include <QByteArray>
#include <QDesktopServices>
#include <QGridLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
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

SettingsPage::SettingsPage(QWidget *parent)
    : QWidget(parent)
{
    configHandler_ = []() {
        SystemController controller;
        QByteArray programName("qkage-ui");
        QByteArray commandName("config-show");
        char *argv[] = {programName.data(), commandName.data()};
        return controller.configShow(2, argv);
    };

    openFolderHandler_ = [](const QString &path) {
        const QUrl url = QUrl::fromLocalFile(QString::fromStdString(std::filesystem::absolute(path.toStdString()).string()));
        return QDesktopServices::openUrl(url);
    };

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(28, 28, 28, 28);
    root->setSpacing(20);

    QLabel *title = new QLabel("Configurações", this);
    title->setObjectName("PageTitle");

    QPushButton *refreshButton = new QPushButton("Atualizar configurações", this);
    refreshButton->setObjectName("PrimaryActionButton");
    QPushButton *openDataButton = new QPushButton("Abrir pasta de dados", this);
    QPushButton *openBackupButton = new QPushButton("Abrir pasta de backups", this);
    QPushButton *openReportsButton = new QPushButton("Abrir pasta de relatórios", this);

    QHBoxLayout *header = new QHBoxLayout();
    header->addWidget(title);
    header->addStretch(1);
    header->addWidget(refreshButton);
    header->addWidget(openDataButton);
    header->addWidget(openBackupButton);
    header->addWidget(openReportsButton);
    root->addLayout(header);

    statusBadge_ = new StatusBadge("INFO", this);
    statusBadge_->setObjectName("SettingsStatusBadge");
    feedbackBanner_ = new FeedbackBanner(this);
    root->addWidget(statusBadge_);
    root->addWidget(feedbackBanner_);

    QGridLayout *cards = new QGridLayout();
    cards->setSpacing(16);
    statusCard_ = new InfoCard("Configuração", "Sem dados", "Aguardando leitura", this);
    systemCard_ = new InfoCard("Sistema", "Não carregado", "Leitura via controller", this);
    pathsCard_ = new InfoCard("Caminhos", "Não carregado", "Diretórios configurados", this);
    cards->addWidget(statusCard_, 0, 0);
    cards->addWidget(systemCard_, 0, 1);
    cards->addWidget(pathsCard_, 0, 2);
    root->addLayout(cards);

    messageLabel_ = new QLabel("Configurações ainda não carregadas.", this);
    messageLabel_->setObjectName("PanelPlaceholder");
    messageLabel_->setAlignment(Qt::AlignCenter);
    root->addWidget(messageLabel_);

    settingsTable_ = new QTableWidget(this);
    settingsTable_->setObjectName("SettingsTable");
    settingsTable_->setColumnCount(2);
    settingsTable_->setHorizontalHeaderLabels({"configuração", "valor"});
    settingsTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    settingsTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    settingsTable_->setAlternatingRowColors(true);
    settingsTable_->verticalHeader()->setVisible(false);
    settingsTable_->verticalHeader()->setDefaultSectionSize(36);
    settingsTable_->horizontalHeader()->setStretchLastSection(true);
    root->addWidget(settingsTable_, 1);

    connect(refreshButton, &QPushButton::clicked, this, [this]() {
        refreshData();
    });
    connect(openDataButton, &QPushButton::clicked, this, [this]() {
        openFolder(configValue(QString(), "data_dir"));
    });
    connect(openBackupButton, &QPushButton::clicked, this, [this]() {
        openFolder(configValue(QString(), "backup_dir"));
    });
    connect(openReportsButton, &QPushButton::clicked, this, [this]() {
        openFolder(configValue(QString(), "reports_dir"));
    });

    refreshData();
}

void SettingsPage::refreshData()
{
    const ControllerResult result = configHandler_();
    if (!result.success) {
        const QString message = !result.message.empty()
            ? QString::fromStdString(result.message).trimmed()
            : "Não foi possível carregar as configurações.";
        showError(message);
        return;
    }

    renderConfig(QString::fromStdString(result.payload));
}

void SettingsPage::setConfigHandler(std::function<ControllerResult()> handler)
{
    configHandler_ = std::move(handler);
}

void SettingsPage::setOpenFolderHandler(std::function<bool(const QString &)> handler)
{
    openFolderHandler_ = std::move(handler);
}

void SettingsPage::renderConfig(const QString &payload)
{
    const QStringList keys = {
        "version",
        "currency",
        "backup_enabled",
        "reports_enabled",
        "logs_enabled",
        "debug_mode",
        "data_dir",
        "backup_dir",
        "reports_dir"
    };

    settingsTable_->setRowCount(keys.size());
    for (int row = 0; row < keys.size(); ++row) {
        const QString key = keys.at(row);
        settingsTable_->setItem(row, 0, new QTableWidgetItem(key));
        settingsTable_->setItem(row, 1, new QTableWidgetItem(configValue(payload, key)));
    }
    settingsTable_->resizeColumnsToContents();

    statusCard_->setValue("Carregada");
    statusCard_->setStatus(payload.contains("Arquivo: data/config.ini") ? "Fluxo padrão via controller" : "Configuração lida");
    systemCard_->setValue(configValue(payload, "version"));
    systemCard_->setStatus("Moeda: " + configValue(payload, "currency"));
    pathsCard_->setValue(configValue(payload, "data_dir"));
    pathsCard_->setStatus("Backups: " + configValue(payload, "backup_dir"));

    messageLabel_->setText("Configurações carregadas via controller. Edição completa ainda não habilitada.");
    messageLabel_->setVisible(true);
    settingsTable_->setVisible(true);
    statusBadge_->setStatus("OK");
    feedbackBanner_->showSuccess("Configuração carregada via controller.");
}

void SettingsPage::showError(const QString &message)
{
    settingsTable_->setRowCount(0);
    settingsTable_->setVisible(false);
    messageLabel_->setText(message);
    messageLabel_->setVisible(true);
    statusCard_->setValue("Erro");
    statusCard_->setStatus("Falha ao carregar");
    statusBadge_->setStatus("ERROR");
    feedbackBanner_->showError(message);
}

void SettingsPage::openFolder(const QString &path)
{
    const QString target = path.isEmpty() ? "data" : path;
    if (!openFolderHandler_(target)) {
        QMessageBox::information(this, "Configurações", "Não foi possível abrir a pasta solicitada.");
    }
}

QString SettingsPage::configValue(const QString &payload, const QString &key) const
{
    const QString source = payload.isEmpty() && settingsTable_ != nullptr ? QString() : payload;
    if (!source.isEmpty()) {
        const QString prefix = key + "=";
        for (const QString &line : source.split('\n')) {
            if (line.startsWith(prefix)) {
                return line.mid(prefix.size()).trimmed();
            }
        }
    }

    if (settingsTable_ != nullptr) {
        for (int row = 0; row < settingsTable_->rowCount(); ++row) {
            QTableWidgetItem *keyItem = settingsTable_->item(row, 0);
            QTableWidgetItem *valueItem = settingsTable_->item(row, 1);
            if (keyItem != nullptr && valueItem != nullptr && keyItem->text() == key) {
                return valueItem->text();
            }
        }
    }

    if (key == "data_dir") return "data";
    if (key == "backup_dir") return "backups";
    if (key == "reports_dir") return "reports";
    return QString();
}
