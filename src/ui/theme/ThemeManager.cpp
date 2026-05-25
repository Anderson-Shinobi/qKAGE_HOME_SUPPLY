#include "ThemeManager.h"

QString ThemeManager::applicationStyleSheet()
{
    return R"(
        QMainWindow {
            background: #0d141c;
            color: #f2f5f7;
        }

        QWidget#RootWidget {
            background: #0d141c;
            color: #f2f5f7;
            font-family: "Inter", "Segoe UI", sans-serif;
            font-size: 14px;
        }

        QWidget#Sidebar {
            background: #090f15;
            border-right: 1px solid #27313d;
        }

        QLabel#BrandLabel {
            color: #f2f5f7;
            font-size: 17px;
            font-weight: 700;
            padding: 12px 8px 10px 8px;
        }

        QPushButton {
            background: transparent;
            color: #d7dee7;
            border: 1px solid transparent;
            border-radius: 8px;
            padding: 11px 14px;
            text-align: left;
        }

        QPushButton:hover {
            background: #171f28;
            border-color: #f28c28;
            color: #ffffff;
        }

        QPushButton:checked {
            background: #1a222c;
            border-left: 4px solid #f28c28;
            border-top: 1px solid #3a4654;
            border-right: 1px solid #3a4654;
            border-bottom: 1px solid #3a4654;
            color: #ffffff;
            font-weight: 700;
        }

        QPushButton#PrimaryActionButton {
            background: #182331;
            border: 1px solid #f28c28;
            color: #ffffff;
            text-align: center;
            font-weight: 700;
            padding: 9px 16px;
        }

        QPushButton#PrimaryActionButton:hover {
            background: #231d18;
            border-color: #ffab5c;
        }

        QStackedWidget {
            background: #0f1720;
            border: none;
        }

        QScrollArea#DashboardScrollArea {
            background: #0f1720;
            border: none;
        }

        QScrollArea#DashboardScrollArea > QWidget > QWidget,
        QWidget#DashboardContent {
            background: #0f1720;
        }

        QLabel#PlaceholderLabel {
            color: #c1cad4;
            font-size: 24px;
            font-weight: 600;
        }

        QLabel#PageTitle {
            color: #f2f5f7;
            font-size: 28px;
            font-weight: 700;
            padding: 0 0 4px 0;
        }

        QLabel#PageSectionTitle {
            color: #f28c28;
            font-size: 16px;
            font-weight: 800;
            padding: 4px 0 0 0;
        }

        QWidget#InfoCard {
            background: #151c24;
            border: 1px solid #2d3743;
            border-radius: 12px;
        }

        QLabel#InfoCardTitle {
            color: #f28c28;
            font-size: 13px;
            font-weight: 600;
        }

        QLabel#InfoCardValue {
            color: #f2f5f7;
            font-size: 22px;
            font-weight: 700;
        }

        QLabel#InfoCardStatus {
            color: #c1cad4;
            font-size: 12px;
            font-weight: 600;
        }

        QLabel#InfoCardIcon {
            color: #f28c28;
            font-size: 18px;
            font-weight: 800;
        }

        QLabel#PanelPlaceholder {
            background: #111922;
            color: #c1cad4;
            border: 1px dashed #3b4652;
            border-radius: 10px;
            padding: 26px;
            min-height: 180px;
        }

        QLabel#DialogErrorLabel {
            background: #3a1820;
            color: #ffd6dc;
            border: 1px solid #8b3442;
            border-radius: 6px;
            padding: 10px;
            font-weight: 600;
        }

        QLineEdit, QComboBox, QTextEdit {
            background: #121a23;
            color: #f2f5f7;
            border: 1px solid #2d3743;
            border-radius: 8px;
            padding: 8px;
            selection-background-color: #f28c28;
            selection-color: #111111;
        }

        QLineEdit:focus, QComboBox:focus, QTextEdit:focus {
            border-color: #f28c28;
        }

        QTableWidget#DataTable,
        QTableWidget#LogsTable,
        QTableWidget#SettingsTable {
            background: #0f1720;
            alternate-background-color: #141d27;
            color: #f2f5f7;
            border: 1px solid #2d3743;
            border-radius: 8px;
            gridline-color: #283442;
            selection-background-color: #f28c28;
            selection-color: #111111;
        }

        QTableWidget#DataTable::item,
        QTableWidget#LogsTable::item,
        QTableWidget#SettingsTable::item {
            padding: 8px;
        }

        QTableWidget#DataTable::item:selected,
        QTableWidget#LogsTable::item:selected,
        QTableWidget#SettingsTable::item:selected {
            background: #f28c28;
            color: #111111;
        }

        QTextEdit#MonthlyReportView {
            background: #101821;
            color: #f2f5f7;
            border: 1px solid #2d3743;
            border-radius: 8px;
        }

        QDialog {
            background: #0d141c;
            color: #f2f5f7;
            font-family: "Inter", "Segoe UI", sans-serif;
        }

        QDialog QLabel {
            color: #d7dee7;
            font-weight: 600;
        }

        QDialog#StockDialog {
            border: 1px solid #2d3743;
            border-radius: 12px;
        }

        QDialog QPushButton {
            background: #182331;
            border: 1px solid #f28c28;
            border-radius: 8px;
            color: #ffffff;
            text-align: center;
            font-weight: 700;
            padding: 8px 14px;
        }

        QDialog QPushButton:hover {
            background: #231d18;
            border-color: #ffab5c;
        }

        QHeaderView::section {
            background: #151c24;
            color: #f2f5f7;
            border: none;
            border-right: 1px solid #2d3743;
            border-bottom: 2px solid #f28c28;
            padding: 9px;
            font-weight: 700;
        }

        QStatusBar {
            background: #090f15;
            color: #c1cad4;
            border-top: 1px solid #27313d;
        }

        QLabel#StatusBarVersion,
        QLabel#StatusBarSystemStatus,
        QLabel#StatusBarClock {
            color: #c1cad4;
            padding: 0 8px;
            font-size: 12px;
            font-weight: 600;
        }

        QLabel#StatusBarSystemStatus {
            color: #f28c28;
        }
    )";
}

QString ThemeManager::primaryFontFamily()
{
    return "Inter";
}

QString ThemeManager::accentColor()
{
    return "#f28c28";
}

QString ThemeManager::backgroundColor()
{
    return "#0d141c";
}

QString ThemeManager::panelColor()
{
    return "#151c24";
}

QString ThemeManager::primaryTextColor()
{
    return "#f2f5f7";
}

QString ThemeManager::secondaryTextColor()
{
    return "#c1cad4";
}
