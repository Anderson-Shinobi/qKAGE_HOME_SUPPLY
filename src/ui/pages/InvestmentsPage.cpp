#include "InvestmentsPage.h"

#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QVector>

#include <cmath>

#include "../widgets/InfoCard.h"
#include "../widgets/OperationalChartWidget.h"
#include "../widgets/StatusBadge.h"

namespace {
QString money(double value)
{
    return QString("R$ %1").arg(value, 0, 'f', 2);
}

QVector<double> investmentSeries(double monthlyContribution, double monthlyRate, double initialContribution)
{
    QVector<double> values;
    values.reserve(12);
    double balance = initialContribution;
    for (int month = 0; month < 12; ++month) {
        balance = (balance + monthlyContribution) * (1.0 + monthlyRate);
        values.append(balance);
    }
    return values;
}
}

InvestmentsPage::InvestmentsPage(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(28, 28, 28, 28);
    root->setSpacing(20);

    QLabel *title = new QLabel("Investimentos", this);
    title->setObjectName("PageTitle");
    root->addWidget(title);

    const double capitalLivre = 48.50;
    const double aporteMensal = 420.00;
    const double taxaMensal = 0.008;
    const double aporteInicial = 1200.00;
    const QVector<double> series = investmentSeries(aporteMensal, taxaMensal, aporteInicial);
    const double valorFinal = series.isEmpty() ? 0.0 : series.last();
    const double totalAportado = aporteInicial + aporteMensal * 12.0;
    const double juros = valorFinal - totalAportado;

    QGridLayout *cards = new QGridLayout();
    cards->setHorizontalSpacing(16);
    cards->setVerticalSpacing(18);
    cards->setColumnStretch(0, 1);
    cards->setColumnStretch(1, 1);

    cards->addWidget(new InfoCard("Capital livre estimado", money(capitalLivre), "Economia operacional", "$", this), 0, 0);
    cards->addWidget(new InfoCard("Projeção mensal", money(aporteMensal), "Aporte simulado", "~", this), 0, 1);
    cards->addWidget(new InfoCard("Projeção anual", money(valorFinal), "Últimos 12 meses", "%", this), 1, 0);
    cards->addWidget(new InfoCard("Juros compostos simulados", money(juros), "Taxa simulada: 0.80% a.m.", "+", this), 1, 1);
    root->addLayout(cards);

    StatusBadge *badge = new StatusBadge("OK", this);
    badge->setObjectName("InvestmentsStatusBadge");
    root->addWidget(badge);

    QLabel *chartTitle = new QLabel("Projeção temporal de capital", this);
    chartTitle->setObjectName("PageSectionTitle");
    root->addWidget(chartTitle);

    OperationalChartWidget *chart = new OperationalChartWidget(this);
    chart->setObjectName("InvestmentsProjectionChart");
    chart->setValues(series, "Crescimento gradual com aportes mensais e rendimento composto");
    root->addWidget(chart, 1);

    QGridLayout *details = new QGridLayout();
    details->setHorizontalSpacing(16);
    details->setVerticalSpacing(18);
    details->addWidget(new InfoCard("Rendimento estimado", money(juros), "Delta sobre aportes", "^", this), 0, 0);
    details->addWidget(new InfoCard("Taxa simulada", "0.80% a.m.", "Cenário conservador", "%", this), 0, 1);
    details->addWidget(new InfoCard("Projeção futura", money(valorFinal * std::pow(1.0 + taxaMensal, 12.0)), "Horizonte expandido", ">", this), 0, 2);
    root->addLayout(details);
}
