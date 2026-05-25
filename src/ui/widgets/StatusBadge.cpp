#include "StatusBadge.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QSizePolicy>

#include "../theme/ThemeManager.h"

StatusBadge::StatusBadge(const QString &status, QWidget *parent)
    : QWidget(parent)
{
    setObjectName("StatusBadge");
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 4, 10, 4);
    layout->setSpacing(0);

    label_ = new QLabel(this);
    label_->setObjectName("StatusBadgeLabel");
    layout->addWidget(label_);

    setStatus(status);
}

void StatusBadge::setStatus(const QString &status)
{
    status_ = normalizedStatus(status);
    label_->setText(status_);
    setToolTip(tooltipForStatus(status_));
    applyStyle();
}

QString StatusBadge::status() const
{
    return status_;
}

QString StatusBadge::category() const
{
    return category_;
}

void StatusBadge::applyStyle()
{
    QString background = "#10251a";
    QString foreground = "#8ee6a3";
    QString border = "#2e7d32";
    category_ = "success";

    if (status_ == "ATENÇÃO" || status_ == "WARNING") {
        background = "#2b2116";
        foreground = "#ffbe72";
        border = ThemeManager::accentColor();
        category_ = "warning";
    } else if (status_ == "CRÍTICO" || status_ == "VENCIDO" || status_ == "ERROR") {
        background = "#33151a";
        foreground = "#ff9aa6";
        border = "#d32f2f";
        category_ = "error";
    } else if (status_ == "SEM_VALIDADE" || status_ == "INFO") {
        background = "#142332";
        foreground = "#9fc7ee";
        border = "#4f7fa8";
        category_ = "neutral";
    }

    setStyleSheet(QString(
        "QWidget#StatusBadge, QWidget#DashboardStatusBadge { background: %1; border: 1px solid %3; border-radius: 11px; }"
        "QLabel#StatusBadgeLabel { color: %2; font-weight: 800; font-size: 12px; padding: 1px 2px; }")
        .arg(background, foreground, border));
}

QString StatusBadge::normalizedStatus(QString status)
{
    status = status.trimmed().toUpper();
    if (status == "CRITICO") {
        return "CRÍTICO";
    }
    if (status == "ATENCAO") {
        return "ATENÇÃO";
    }
    if (status != "OK" &&
        status != "ATENÇÃO" &&
        status != "CRÍTICO" &&
        status != "VENCIDO" &&
        status != "SEM_VALIDADE" &&
        status != "WARNING" &&
        status != "ERROR" &&
        status != "INFO") {
        return "OK";
    }
    return status;
}

QString StatusBadge::tooltipForStatus(const QString &status)
{
    if (status == "OK") return "Operação ou indicador em estado normal.";
    if (status == "ATENÇÃO" || status == "WARNING") return "Requer atenção operacional.";
    if (status == "CRÍTICO") return "Estado crítico; priorize ação corretiva.";
    if (status == "VENCIDO") return "Item ou prazo vencido.";
    if (status == "ERROR") return "Erro operacional detectado.";
    if (status == "SEM_VALIDADE") return "Sem validade cadastrada ou aplicável.";
    if (status == "INFO") return "Informação operacional.";
    return "Status operacional.";
}
