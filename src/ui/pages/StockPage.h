#pragma once

#include <functional>

#include <QStringList>
#include <QVector>
#include <QWidget>

#include "../../controllers/ControllerResult.h"

class InfoCard;
class FeedbackBanner;
class QComboBox;
class QLabel;
class QLineEdit;
class StatusBadge;
class QTableWidget;

struct StockPageItemData {
    QString item;
    QString category;
    QString quantity;
    QString unit;
    QString monthlyConsumption;
    QString unitPrice;
    QString minimumStock;
    QString expiration;
    QString autonomy;
    QString status;
};

class StockPage : public QWidget {
public:
    explicit StockPage(QWidget *parent = nullptr);
    void refreshData();
    bool removeSelectedItem();

    void setListHandler(std::function<ControllerResult()> handler);
    void setRemoveHandler(std::function<ControllerResult(const QString &)> handler);
    void setRemoveConfirmationHandler(std::function<bool(const QString &)> handler);

private:
    InfoCard *itemsCard_ = nullptr;
    InfoCard *autonomyCard_ = nullptr;
    StatusBadge *statusBadge_ = nullptr;
    FeedbackBanner *feedbackBanner_ = nullptr;
    QLabel *emptyStateLabel_ = nullptr;
    QLabel *counterLabel_ = nullptr;
    QLineEdit *searchEdit_ = nullptr;
    QComboBox *statusFilter_ = nullptr;
    QTableWidget *stockTable_ = nullptr;
    QStringList loadedItemNames_;
    QVector<StockPageItemData> loadedItems_;
    QVector<StockPageItemData> displayedItems_;
    int sortColumn_ = 0;
    Qt::SortOrder sortOrder_ = Qt::AscendingOrder;
    std::function<ControllerResult()> listHandler_;
    std::function<ControllerResult(const QString &)> removeHandler_;
    std::function<bool(const QString &)> removeConfirmationHandler_;

    void applyFiltersAndRender();
    void renderTable(const QVector<StockPageItemData> &rows);
    void sortDisplayedItems();
    bool matchesCurrentFilters(const StockPageItemData &item) const;
    bool editSelectedItem();
    void showPlaceholderDialog(const QString &title);
};
