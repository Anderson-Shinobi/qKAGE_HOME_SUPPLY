#include "ConsumeStockItemDialog.h"

#include <QByteArray>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include <exception>
#include <utility>

#include "../../controllers/StockController.h"

namespace {
bool parseNumber(const QString &value, double &number)
{
    bool ok = false;
    number = value.trimmed().toDouble(&ok);
    return ok;
}

QByteArray toUtf8ByteArray(const QString &value)
{
    return value.trimmed().toUtf8();
}
}

ConsumeStockItemDialog::ConsumeStockItemDialog(const QStringList &items, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Consumir item");
    setWindowIcon(QIcon(":/logo/qkage_logo.svg"));
    setObjectName("StockDialog");
    setModal(true);
    setMinimumWidth(460);
    resize(420, 240);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(24, 22, 24, 22);
    root->setSpacing(14);

    QFormLayout *form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight);
    form->setFormAlignment(Qt::AlignTop);
    form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    itemCombo_ = new QComboBox(this);
    itemCombo_->setObjectName("consumeItemCombo");
    itemCombo_->setEditable(true);
    itemCombo_->addItems(items);
    if (itemCombo_->lineEdit() != nullptr) {
        itemCombo_->lineEdit()->setPlaceholderText("Digite ou selecione um item");
    }

    quantityEdit_ = new QLineEdit(this);
    quantityEdit_->setObjectName("consumeQuantityEdit");
    quantityEdit_->setPlaceholderText("1");

    observationEdit_ = new QLineEdit(this);
    observationEdit_->setObjectName("consumeObservationEdit");
    observationEdit_->setPlaceholderText("opcional");

    form->addRow("item", itemCombo_);
    form->addRow("quantidade consumida", quantityEdit_);
    form->addRow("observação", observationEdit_);
    root->addLayout(form);

    errorLabel_ = new QLabel(this);
    errorLabel_->setObjectName("DialogErrorLabel");
    errorLabel_->setWordWrap(true);
    errorLabel_->setVisible(false);
    root->addWidget(errorLabel_);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok, this);
    buttons->button(QDialogButtonBox::Ok)->setText("Consumir");
    buttons->button(QDialogButtonBox::Cancel)->setText("Cancelar");
    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
        submit();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    root->addWidget(buttons);
}

void ConsumeStockItemDialog::setSubmitHandler(SubmitHandler handler)
{
    submitHandler_ = std::move(handler);
}

bool ConsumeStockItemDialog::submit()
{
    const ConsumeStockItemData data = formData();
    const QString error = validationError(data);
    if (!error.isEmpty()) {
        showError(error);
        return false;
    }

    ControllerResult result;
    try {
        result = submitHandler_ ? submitHandler_(data) : submitWithController(data);
    } catch (const std::exception &exception) {
        showError(QString("Não foi possível consumir o item. %1").arg(exception.what()));
        return false;
    } catch (...) {
        showError("Não foi possível consumir o item.");
        return false;
    }

    if (!result.success) {
        const QString message = QString::fromStdString(result.message).trimmed();
        const QString code = QString::fromStdString(result.error_code).trimmed();
        showError(message.isEmpty() ? QString("Não foi possível consumir o item. %1").arg(code).trimmed() : message);
        return false;
    }

    accept();
    return true;
}

ConsumeStockItemData ConsumeStockItemDialog::formData() const
{
    return {
        itemCombo_->currentText().trimmed(),
        quantityEdit_->text().trimmed(),
        observationEdit_->text().trimmed()
    };
}

QString ConsumeStockItemDialog::validationError(const ConsumeStockItemData &data) const
{
    if (data.item.isEmpty()) {
        return "Informe o item.";
    }

    double quantity = 0.0;
    if (!parseNumber(data.quantity, quantity) || quantity <= 0.0) {
        return "Quantidade consumida deve ser maior que zero.";
    }

    return "";
}

ControllerResult ConsumeStockItemDialog::submitWithController(const ConsumeStockItemData &data) const
{
    StockController controller;
    QByteArray programName("qkage-ui");
    QByteArray commandName("consume");
    QByteArray item = toUtf8ByteArray(data.item);
    QByteArray quantity = toUtf8ByteArray(data.quantity);
    QByteArray observation = toUtf8ByteArray(data.observation);

    char *argv[] = {
        programName.data(),
        commandName.data(),
        item.data(),
        quantity.data(),
        observation.data()
    };

    return controller.consume(5, argv);
}

void ConsumeStockItemDialog::showError(const QString &message)
{
    errorLabel_->setText(message);
    errorLabel_->setVisible(true);
}
