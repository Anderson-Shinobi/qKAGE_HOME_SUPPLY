#include "ShoppingListGenerator.h"

#include "../csv/CsvInventoryReader.h"

ShoppingListResult ShoppingListGenerator::generate(const std::filesystem::path &inventoryCsvPath) const
{
    CsvInventoryReader reader;
    const CsvReadResult readResult = reader.read(inventoryCsvPath);
    if (!readResult.success) {
        return {false, readResult.message};
    }

    ShoppingListResult result;
    result.success = true;
    result.message = "Lista de compras gerada com sucesso.";

    for (const InventoryItem &item : readResult.items) {
        if (item.quantity < 0.0) {
            return {false, "quantidade negativa para o item: " + item.name};
        }

        if (item.monthlyConsumption <= 0.0) {
            return {false, "consumo_mensal invalido para o item: " + item.name};
        }

        if (item.minimumStock < 0.0) {
            return {false, "estoque_minimo invalido para o item: " + item.name};
        }

        const double estimatedAutonomy = item.quantity / item.monthlyConsumption;
        if (item.quantity > item.minimumStock && estimatedAutonomy >= 2.0) {
            continue;
        }

        ShoppingListItem shoppingItem;
        shoppingItem.item = item.name;
        shoppingItem.category = item.category;
        shoppingItem.currentQuantity = item.quantity;
        shoppingItem.minimumStock = item.minimumStock;
        shoppingItem.monthlyConsumption = item.monthlyConsumption;
        shoppingItem.estimatedAutonomy = estimatedAutonomy;
        shoppingItem.suggestedPurchase = (item.monthlyConsumption * 3.0) - item.quantity;
        if (shoppingItem.suggestedPurchase < 0.0) {
            shoppingItem.suggestedPurchase = 0.0;
        }
        shoppingItem.unitPrice = item.unitPrice;
        shoppingItem.estimatedCost = shoppingItem.suggestedPurchase * item.unitPrice;
        shoppingItem.unit = item.unit;
        result.items.push_back(shoppingItem);
    }

    return result;
}
