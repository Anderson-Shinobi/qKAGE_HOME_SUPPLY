#include "CompoundInterestProjection.h"

#include <cmath>

CompoundInterestProjection projectCompoundInterest(const CompoundInterestInput &input)
{
    if (input.monthlyContribution < 0.0) {
        return {false, "aporte mensal deve ser maior ou igual a zero."};
    }

    if (input.monthlyRate < 0.0) {
        return {false, "taxa mensal deve ser maior ou igual a zero."};
    }

    if (input.months <= 0.0) {
        return {false, "meses deve ser maior que zero."};
    }

    if (input.initialContribution < 0.0) {
        return {false, "aporte inicial deve ser maior ou igual a zero."};
    }

    const int wholeMonths = static_cast<int>(std::floor(input.months));
    if (static_cast<double>(wholeMonths) != input.months) {
        return {false, "meses deve ser um numero inteiro."};
    }

    double balance = input.initialContribution;
    for (int month = 0; month < wholeMonths; ++month) {
        balance *= (1.0 + input.monthlyRate);
        balance += input.monthlyContribution;
    }

    CompoundInterestProjection projection;
    projection.success = true;
    projection.message = "Projecao de juros compostos concluida.";
    projection.monthlyContribution = input.monthlyContribution;
    projection.monthlyRate = input.monthlyRate;
    projection.months = input.months;
    projection.initialContribution = input.initialContribution;
    projection.totalContributed = input.initialContribution + (input.monthlyContribution * input.months);
    projection.finalEstimatedValue = balance;
    projection.accumulatedInterest = projection.finalEstimatedValue - projection.totalContributed;
    return projection;
}
