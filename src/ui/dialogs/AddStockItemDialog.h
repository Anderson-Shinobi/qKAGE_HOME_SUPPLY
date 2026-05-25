#pragma once

#include "../../controllers/ControllerResult.h"

#include <QDialog>
#include <QString>

#include <functional>

class QLabel;
class QLineEdit;

struct AddStockItemData {
    QString item;
    QString category;
    QString quantity;
    QString unit;
    QString monthlyConsumption;
    QString unitPrice;
    QString expirationDate;
    QString minimumStock;
};

class AddStockItemDialog : public QDialog {
public:
    using SubmitHandler = std::function<ControllerResult(const AddStockItemData &)>;

    explicit AddStockItemDialog(QWidget *parent = nullptr);

    void setSubmitHandler(SubmitHandler handler);
    bool submit();

private:
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

    AddStockItemData formData() const;
    QString validationError(const AddStockItemData &data) const;
    ControllerResult submitWithController(const AddStockItemData &data) const;
    void showError(const QString &message);
};
