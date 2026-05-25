#include "EditStockItemDialog.h"

#include <QByteArray>
#include <QDate>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QVBoxLayout>

#include <exception>
#include <utility>

#include "../../controllers/StockController.h"

namespace {
QLineEdit *createLineEdit(const QString &objectName, const QString &value, QWidget *parent)
{
    QLineEdit *edit = new QLineEdit(parent);
    edit->setObjectName(objectName);
    edit->setText(value);
    return edit;
}

bool parseNumber(const QString &value, double &number)
{
    bool ok = false;
    number = value.trimmed().toDouble(&ok);
    return ok;
}

bool hasValidExpirationFormat(const QString &value)
{
    const QString trimmed = value.trimmed();
    static const QRegularExpression datePattern("^\\d{4}-\\d{2}-\\d{2}$");
    if (trimmed == "sem_validade") {
        return true;
    }

    if (!datePattern.match(trimmed).hasMatch()) {
        return false;
    }

    return QDate::fromString(trimmed, "yyyy-MM-dd").isValid();
}

QByteArray toUtf8ByteArray(const QString &value)
{
    return value.trimmed().toUtf8();
}
}

EditStockItemDialog::EditStockItemDialog(const EditStockItemData &itemData, QWidget *parent)
    : QDialog(parent),
      originalItem_(itemData.originalItem)
{
    setWindowTitle("Editar item");
    setWindowIcon(QIcon(":/logo/qkage_logo.svg"));
    setObjectName("StockDialog");
    setModal(true);
    setMinimumWidth(500);
    resize(460, 420);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(24, 22, 24, 22);
    root->setSpacing(14);

    QFormLayout *form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight);
    form->setFormAlignment(Qt::AlignTop);
    form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    itemEdit_ = createLineEdit("editItemEdit", itemData.item, this);
    categoryEdit_ = createLineEdit("editCategoryEdit", itemData.category, this);
    quantityEdit_ = createLineEdit("editQuantityEdit", itemData.quantity, this);
    unitEdit_ = createLineEdit("editUnitEdit", itemData.unit, this);
    monthlyConsumptionEdit_ = createLineEdit("editMonthlyConsumptionEdit", itemData.monthlyConsumption, this);
    unitPriceEdit_ = createLineEdit("editUnitPriceEdit", itemData.unitPrice, this);
    expirationEdit_ = createLineEdit("editExpirationEdit", itemData.expirationDate, this);
    minimumStockEdit_ = createLineEdit("editMinimumStockEdit", itemData.minimumStock, this);

    form->addRow("item", itemEdit_);
    form->addRow("categoria", categoryEdit_);
    form->addRow("quantidade", quantityEdit_);
    form->addRow("unidade", unitEdit_);
    form->addRow("consumo mensal", monthlyConsumptionEdit_);
    form->addRow("preço unitário", unitPriceEdit_);
    form->addRow("validade", expirationEdit_);
    form->addRow("estoque mínimo", minimumStockEdit_);
    root->addLayout(form);

    errorLabel_ = new QLabel(this);
    errorLabel_->setObjectName("DialogErrorLabel");
    errorLabel_->setWordWrap(true);
    errorLabel_->setVisible(false);
    root->addWidget(errorLabel_);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok, this);
    buttons->button(QDialogButtonBox::Ok)->setText("Salvar");
    buttons->button(QDialogButtonBox::Cancel)->setText("Cancelar");
    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
        submit();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    root->addWidget(buttons);
}

void EditStockItemDialog::setSubmitHandler(SubmitHandler handler)
{
    submitHandler_ = std::move(handler);
}

bool EditStockItemDialog::submit()
{
    const EditStockItemData data = formData();
    const QString error = validationError(data);
    if (!error.isEmpty()) {
        showError(error);
        return false;
    }

    ControllerResult result;
    try {
        result = submitHandler_ ? submitHandler_(data) : submitWithController(data);
    } catch (const std::exception &exception) {
        showError(QString("Não foi possível editar o item. %1").arg(exception.what()));
        return false;
    } catch (...) {
        showError("Não foi possível editar o item.");
        return false;
    }

    if (!result.success) {
        const QString message = QString::fromStdString(result.message).trimmed();
        const QString code = QString::fromStdString(result.error_code).trimmed();
        showError(message.isEmpty() ? QString("Não foi possível editar o item. %1").arg(code).trimmed() : message);
        return false;
    }

    accept();
    return true;
}

EditStockItemData EditStockItemDialog::formData() const
{
    return {
        originalItem_,
        itemEdit_->text().trimmed(),
        categoryEdit_->text().trimmed(),
        quantityEdit_->text().trimmed(),
        unitEdit_->text().trimmed(),
        monthlyConsumptionEdit_->text().trimmed(),
        unitPriceEdit_->text().trimmed(),
        expirationEdit_->text().trimmed(),
        minimumStockEdit_->text().trimmed()
    };
}

QString EditStockItemDialog::validationError(const EditStockItemData &data) const
{
    if (data.item.isEmpty()) {
        return "Informe o item.";
    }

    if (data.category.isEmpty()) {
        return "Informe a categoria.";
    }

    double value = 0.0;
    if (!parseNumber(data.quantity, value) || value < 0.0) {
        return "Quantidade deve ser maior ou igual a zero.";
    }

    if (!parseNumber(data.monthlyConsumption, value) || value <= 0.0) {
        return "Consumo mensal deve ser maior que zero.";
    }

    if (!parseNumber(data.unitPrice, value) || value < 0.0) {
        return "Preço unitário deve ser maior ou igual a zero.";
    }

    if (!parseNumber(data.minimumStock, value) || value < 0.0) {
        return "Estoque mínimo deve ser maior ou igual a zero.";
    }

    if (!hasValidExpirationFormat(data.expirationDate)) {
        return "Validade deve estar no formato YYYY-MM-DD ou ser sem_validade.";
    }

    return "";
}

ControllerResult EditStockItemDialog::submitWithController(const EditStockItemData &data) const
{
    StockController controller;
    QByteArray programName("qkage-ui");
    QByteArray commandName("edit");
    QByteArray originalItem = toUtf8ByteArray(data.originalItem);
    QByteArray item = toUtf8ByteArray(data.item);
    QByteArray category = toUtf8ByteArray(data.category);
    QByteArray quantity = toUtf8ByteArray(data.quantity);
    QByteArray unit = toUtf8ByteArray(data.unit);
    QByteArray monthlyConsumption = toUtf8ByteArray(data.monthlyConsumption);
    QByteArray unitPrice = toUtf8ByteArray(data.unitPrice);
    QByteArray expirationDate = toUtf8ByteArray(data.expirationDate);
    QByteArray minimumStock = toUtf8ByteArray(data.minimumStock);

    char *argv[] = {
        programName.data(),
        commandName.data(),
        originalItem.data(),
        item.data(),
        category.data(),
        quantity.data(),
        unit.data(),
        monthlyConsumption.data(),
        unitPrice.data(),
        expirationDate.data(),
        minimumStock.data()
    };

    return controller.edit(11, argv);
}

void EditStockItemDialog::showError(const QString &message)
{
    errorLabel_->setText(message);
    errorLabel_->setVisible(true);
}
