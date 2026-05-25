#pragma once

#include <string>

struct InvestmentCapitalInput {
    double monthlySavings = 0.0;
    double months = 0.0;
    double initialContribution = 0.0;
};

struct InvestmentCapitalAnalysis {
    bool success = false;
    std::string message;
    double monthlySavings = 0.0;
    double months = 0.0;
    double initialContribution = 0.0;
    double totalReleasedCapital = 0.0;
};

InvestmentCapitalAnalysis analyzeInvestmentCapital(const InvestmentCapitalInput &input);
