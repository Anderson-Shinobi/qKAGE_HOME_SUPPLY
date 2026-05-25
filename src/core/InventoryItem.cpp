#include "InventoryItem.h"

double InventoryItem::autonomyInMonths() const
{
    if (monthlyConsumption <= 0.0) {
        return 0.0;
    }

    return quantity / monthlyConsumption;
}

double InventoryItem::monthlyCost() const
{
    return monthlyConsumption * unitPrice;
}

bool InventoryItem::isCritical() const
{
    return quantity <= minimumStock;
}
