#pragma once

#include <string>

struct CompoundInterestInput {
    double monthlyContribution = 0.0;
    double monthlyRate = 0.0;
    double months = 0.0;
    double initialContribution = 0.0;
};

struct CompoundInterestProjection {
    bool success = false;
    std::string message;
    double monthlyContribution = 0.0;
    double monthlyRate = 0.0;
    double months = 0.0;
    double initialContribution = 0.0;
    double totalContributed = 0.0;
    double finalEstimatedValue = 0.0;
    double accumulatedInterest = 0.0;
};

CompoundInterestProjection projectCompoundInterest(const CompoundInterestInput &input);
