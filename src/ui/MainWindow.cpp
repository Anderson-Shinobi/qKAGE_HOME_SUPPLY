#include "MainWindow.h"

#include <QButtonGroup>
#include <QDateTime>
#include <QDialog>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QScreen>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>

#include "dialogs/AboutDialog.h"
#include "pages/DashboardPage.h"
#include "pages/InvestmentsPage.h"
#include "pages/LogsPage.h"
#include "pages/PiggyBankPage.h"
#include "pages/ReportsPage.h"
#include "pages/SettingsPage.h"
#include "pages/ShoppingPage.h"
#include "pages/StockPage.h"
#include "theme/ThemeManager.h"

namespace {
QString iconNameForLabel(const QString &label)
{
    if (label == "Dashboard") return "dashboard";
    if (label == "Estoque") return "stock";
    if (label == "Compras") return "shopping";
    if (label == "Cofrinhos") return "piggy";
    if (label == "Investimentos") return "investments";
    if (label == "Relatórios") return "reports";
    if (label == "Logs") return "logs";
    if (label == "Configurações") return "settings";
    if (label == "Sobre") return "about";
    return "dashboard";
}

QIcon sidebarIcon(const QString &name, bool active)
{
    return QIcon(QString(":/icons/%1%2.svg").arg(name, active ? "_active" : ""));
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("qKAGE_HOME_SUPPLY");
    setWindowIcon(QIcon(":/logo/qkage_logo.svg"));
    setMinimumSize(1120, 720);
    resize(1280, 800);

    QWidget *root = new QWidget(this);
    root->setObjectName("RootWidget");
    QHBoxLayout *rootLayout = new QHBoxLayout(root);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    pages_ = new QStackedWidget(root);
    pages_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    const QStringList modules = {
        "Dashboard",
        "Estoque",
        "Compras",
        "Cofrinhos",
        "Investimentos",
        "Relatórios",
        "Logs",
        "Configurações"
    };

    pages_->addWidget(new DashboardPage(pages_));
    pages_->addWidget(new StockPage(pages_));
    pages_->addWidget(new ShoppingPage(pages_));
    pages_->addWidget(new PiggyBankPage(pages_));
    pages_->addWidget(new InvestmentsPage(pages_));
    pages_->addWidget(new ReportsPage(pages_));
    pages_->addWidget(new LogsPage(pages_));
    pages_->addWidget(new SettingsPage(pages_));

    QWidget *sidebar = createSidebar();
    for (int i = 0; i < modules.size(); ++i) {
        addNavigationButton(sidebar, modules.at(i), i);
    }
    addAboutButton(sidebar);

    rootLayout->addWidget(sidebar);
    rootLayout->addWidget(pages_);
    setCentralWidget(root);

    setStyleSheet(ThemeManager::applicationStyleSheet());
    configureStatusBar();
    centerOnScreen();

    if (navigationGroup_->button(0) != nullptr) {
        navigationGroup_->button(0)->setChecked(true);
    }
}

void MainWindow::configureStatusBar()
{
    statusBar()->showMessage("Sistema pronto");

    versionStatusLabel_ = new QLabel("Version: v1.2.1-beta", statusBar());
    versionStatusLabel_->setObjectName("StatusBarVersion");
    systemStatusLabel_ = new QLabel("System Status  Release: beta  CLI: OK  Controllers: OK  Services: OK  Logs: OK  Backup: OK", statusBar());
    systemStatusLabel_->setObjectName("StatusBarSystemStatus");
    clockStatusLabel_ = new QLabel(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm"), statusBar());
    clockStatusLabel_->setObjectName("StatusBarClock");

    statusBar()->addPermanentWidget(versionStatusLabel_);
    statusBar()->addPermanentWidget(systemStatusLabel_, 1);
    statusBar()->addPermanentWidget(clockStatusLabel_);
}

void MainWindow::centerOnScreen()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen == nullptr) {
        return;
    }

    const QRect available = screen->availableGeometry();
    move(available.center() - rect().center());
}

QWidget *MainWindow::createSidebar()
{
    QWidget *sidebar = new QWidget(this);
    sidebar->setObjectName("Sidebar");
    sidebar->setFixedWidth(264);

    QVBoxLayout *layout = new QVBoxLayout(sidebar);
    layout->setContentsMargins(16, 16, 16, 12);
    layout->setSpacing(9);

    QLabel *brand = new QLabel(
        "<span style=\"color:#f28c28; font-weight:800;\">qKage</span>"
        "<span style=\"color:#f2f5f7; font-weight:650;\"> Home Supply</span>",
        sidebar);
    brand->setObjectName("BrandLabel");
    brand->setTextFormat(Qt::RichText);
    brand->setWordWrap(false);
    brand->setMinimumHeight(46);
    brand->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addWidget(brand);

    navigationGroup_ = new QButtonGroup(sidebar);
    navigationGroup_->setExclusive(true);

    layout->addStretch(1);
    return sidebar;
}

void MainWindow::addNavigationButton(QWidget *sidebar, const QString &label, int pageIndex)
{
    QPushButton *button = new QPushButton(label, sidebar);
    button->setCheckable(true);
    button->setMinimumHeight(46);
    button->setIconSize(QSize(20, 20));
    const QString iconName = iconNameForLabel(label);
    button->setIcon(sidebarIcon(iconName, false));

    navigationGroup_->addButton(button, pageIndex);

    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(sidebar->layout());
    const int insertPosition = layout->count() - 1;
    layout->insertWidget(insertPosition, button);

    connect(button, &QPushButton::clicked, this, [this, pageIndex]() {
        pages_->setCurrentIndex(pageIndex);
    });
    connect(button, &QPushButton::toggled, this, [button, iconName](bool checked) {
        button->setIcon(sidebarIcon(iconName, checked));
    });
}

void MainWindow::addAboutButton(QWidget *sidebar)
{
    QPushButton *button = new QPushButton("Sobre", sidebar);
    button->setMinimumHeight(46);
    button->setIconSize(QSize(20, 20));
    button->setIcon(sidebarIcon("about", false));

    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(sidebar->layout());
    layout->addWidget(button);

    connect(button, &QPushButton::clicked, this, [this]() {
        QPushButton *senderButton = qobject_cast<QPushButton *>(sender());
        if (senderButton != nullptr) {
            senderButton->setIcon(sidebarIcon("about", true));
        }
        AboutDialog dialog(this);
        dialog.exec();
        if (senderButton != nullptr) {
            senderButton->setIcon(sidebarIcon("about", false));
        }
    });
}
