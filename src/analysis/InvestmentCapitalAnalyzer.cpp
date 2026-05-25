#include "InvestmentCapitalAnalyzer.h"

InvestmentCapitalAnalysis analyzeInvestmentCapital(const InvestmentCapitalInput &input)
{
    if (input.monthlySavings < 0.0) {
        return {false, "economia mensal deve ser maior ou igual a zero."};
    }

    if (input.months <= 0.0) {
        return {false, "meses deve ser maior que zero."};
    }

    if (input.initialContribution < 0.0) {
        return {false, "aporte inicial deve ser maior ou igual a zero."};
    }

    InvestmentCapitalAnalysis analysis;
    analysis.success = true;
    analysis.message = "Analise de capital liberado concluida.";
    analysis.monthlySavings = input.monthlySavings;
    analysis.months = input.months;
    analysis.initialContribution = input.initialContribution;
    analysis.totalReleasedCapital = input.initialContribution + (input.monthlySavings * input.months);
    return analysis;
}
