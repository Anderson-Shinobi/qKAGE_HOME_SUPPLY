#pragma once

#include "../../controllers/ControllerResult.h"

#include <QDialog>
#include <QString>
#include <QStringList>

#include <functional>

class QLabel;
class QComboBox;
class QLineEdit;

struct ConsumeStockItemData {
    QString item;
    QString quantity;
    QString observation;
};

class ConsumeStockItemDialog : public QDialog {
public:
    using SubmitHandler = std::function<ControllerResult(const ConsumeStockItemData &)>;

    explicit ConsumeStockItemDialog(const QStringList &items = {}, QWidget *parent = nullptr);

    void setSubmitHandler(SubmitHandler handler);
    bool submit();

private:
    QComboBox *itemCombo_ = nullptr;
    QLineEdit *quantityEdit_ = nullptr;
    QLineEdit *observationEdit_ = nullptr;
    QLabel *errorLabel_ = nullptr;
    SubmitHandler submitHandler_;

    ConsumeStockItemData formData() const;
    QString validationError(const ConsumeStockItemData &data) const;
    ControllerResult submitWithController(const ConsumeStockItemData &data) const;
    void showError(const QString &message);
};
