#include "Portfolio.h"

Portfolio::Portfolio(double initialCash)
    : cash_(initialCash), assetSymbol_("SYMBOL") {}

void Portfolio::update(const TradeList& trades) {
    for (const auto& trade : trades) {
        const TradeChild& buyOrder = trade.getBuyOrder();
        const TradeChild& sellOrder = trade.getSellOrder();
        Quantity qty = trade.getTradedQuantity();
        Price price = trade.getPrice();

        if (buyOrder.isPersonalOrder) {
            positions_[assetSymbol_] += qty;
            cash_ -= qty * price;
        }

        if (sellOrder.isPersonalOrder) {
            positions_[assetSymbol_] -= qty;
            cash_ += qty * price;
        }
    }
}


double Portfolio::getPortfolioValue(const Price currentPrice) const {
    double positionsValue = 0.0;
    auto it = positions_.find(assetSymbol_);
    if (it != positions_.end()) {
        positionsValue = it->second * currentPrice;
    }
    return positionsValue + cash_;
}

double Portfolio::getCash() const {
    return cash_;
}

const std::unordered_map<std::string, Quantity>& Portfolio::getPositions() const {
    return positions_;
}
