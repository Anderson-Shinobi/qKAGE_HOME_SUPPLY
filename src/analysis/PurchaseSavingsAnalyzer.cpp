#include "PurchaseSavingsAnalyzer.h"

namespace {
bool isBlank(const std::string &value)
{
    for (const unsigned char ch : value) {
        if (ch != ' ' && ch != '\t' && ch != '\n' && ch != '\r') {
            return false;
        }
    }

    return true;
}

std::string classifySavings(double localUnitPrice, double wholesaleUnitPrice)
{
    if (wholesaleUnitPrice < localUnitPrice) {
        return "ECONOMIA";
    }

    if (wholesaleUnitPrice == localUnitPrice) {
        return "SEM GANHO";
    }

    return "ALERTA";
}
}

PurchaseSavingsAnalysis analyzePurchaseSavings(const PurchaseSavingsInput &input)
{
    if (isBlank(input.item)) {
        return {false, "item e obrigatorio."};
    }

    if (input.quantity <= 0.0) {
        return {false, "quantidade deve ser maior que zero."};
    }

    if (input.localUnitPrice <= 0.0) {
        return {false, "preco local deve ser maior que zero."};
    }

    if (input.wholesaleUnitPrice <= 0.0) {
        return {false, "preco atacado deve ser maior que zero."};
    }

    PurchaseSavingsAnalysis analysis;
    analysis.success = true;
    analysis.message = "Analise de economia concluida.";
    analysis.item = input.item;
    analysis.quantity = input.quantity;
    analysis.localUnitPrice = input.localUnitPrice;
    analysis.wholesaleUnitPrice = input.wholesaleUnitPrice;
    analysis.unitSavings = input.localUnitPrice - input.wholesaleUnitPrice;
    analysis.totalSavings = analysis.unitSavings * input.quantity;
    analysis.status = classifySavings(input.localUnitPrice, input.wholesaleUnitPrice);
    return analysis;
}
