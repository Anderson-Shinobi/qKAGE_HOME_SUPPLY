#pragma once

#include "../../controllers/ControllerResult.h"

#include <QDialog>
#include <QString>

#include <functional>

class QLabel;
class QLineEdit;

struct EditStockItemData {
    QString originalItem;
    QString item;
    QString category;
    QString quantity;
    QString unit;
    QString monthlyConsumption;
    QString unitPrice;
    QString expirationDate;
    QString minimumStock;
};

class EditStockItemDialog : public QDialog {
public:
    using SubmitHandler = std::function<ControllerResult(const EditStockItemData &)>;

    explicit EditStockItemDialog(const EditStockItemData &itemData, QWidget *parent = nullptr);

    void setSubmitHandler(SubmitHandler handler);
    bool submit();

private:
    QString originalItem_;
    QLineEdit *itemEdit_ = nullptr;
    QLineEdit *categoryEdit_ = nullptr;
    QLineEdit *quantityEdit_ = nullptr;
    QLineEdit *unitEdit_ = nullptr;
    QLineEdit *monthlyConsumptionEdit_ = nullptr;
    QLineEdit *unitPriceEdit_ = nullptr;
    QLineEdit *expirationEdit_ = nullptr;
    QLineEdit *minimumStockEdit_ = nullptr;
    QLabel *errorLabel_ = nullptr;
    SubmitHandler submitHandler_;

    EditStockItemData formData() const;
    QString validationError(const EditStockItemData &data) const;
    ControllerResult submitWithController(const EditStockItemData &data) const;
    void showError(const QString &message);
};
