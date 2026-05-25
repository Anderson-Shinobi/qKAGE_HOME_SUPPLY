#include "StockAutonomyAnalyzer.h"

namespace {
std::string classifyAutonomy(double autonomyMonths)
{
    if (autonomyMonths <= 1.0) {
        return "CRÍTICO";
    }

    if (autonomyMonths < 2.0) {
        return "ATENÇÃO";
    }

    return "OK";
}
}

StockAutonomyAnalysis analyzeStockAutonomy(const std::vector<InventoryItem> &items)
{
    StockAutonomyAnalysis analysis;
    analysis.success = true;

    for (const InventoryItem &item : items) {
        if (item.monthlyConsumption <= 0.0) {
            return {
                false,
                "consumo_mensal deve ser maior que zero para o item: " + item.name,
                {}
            };
        }

        if (item.quantity < 0.0) {
            return {
                false,
                "quantidade deve ser maior ou igual a zero para o item: " + item.name,
                {}
            };
        }

        StockAutonomyEntry entry;
        entry.item = item.name;
        entry.quantity = item.quantity;
        entry.monthlyConsumption = item.monthlyConsumption;
        entry.autonomyMonths = item.quantity / item.monthlyConsumption;
        entry.status = classifyAutonomy(entry.autonomyMonths);
        analysis.entries.push_back(entry);
    }

    analysis.message = "Analise de autonomia concluida.";
    return analysis;
}
