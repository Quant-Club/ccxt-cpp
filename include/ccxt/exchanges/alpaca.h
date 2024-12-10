#pragma once

#include "ccxt/base/exchange_impl.h"
#include "ccxt/base/exchange_registry.h"

namespace ccxt {

class alpaca : public ExchangeImpl<alpaca> {
public:
    static const std::string defaultHostname;
    static const int defaultRateLimit;
    static const bool defaultPro;
    static ExchangeRegistry::Factory factory;

    explicit alpaca(const Config& config = Config());
    virtual ~alpaca() = default;

    static Exchange* create(const Config& config = Config()) {
        return new alpaca(config);
    }

    void init() override;

protected:
    json describeImpl() const override;
    json fetchMarketsImpl() const override;
    json fetchTickerImpl(const std::string& symbol) const override;
    json fetchTickersImpl(const std::vector<std::string>& symbols = {}) const override;
    json fetchOrderBookImpl(const std::string& symbol, const std::optional<int>& limit = std::nullopt) const override;
    json fetchOHLCVImpl(const std::string& symbol, const std::string& timeframe,
                      const std::optional<long long>& since = std::nullopt,
                      const std::optional<int>& limit = std::nullopt) const override;
    json createOrderImpl(const std::string& symbol, const std::string& type,
                      const std::string& side, double amount,
                      const std::optional<double>& price = std::nullopt) override;
    json cancelOrderImpl(const std::string& id, const std::string& symbol) override;
    json fetchOrderImpl(const std::string& id, const std::string& symbol) const override;
    json fetchOpenOrdersImpl(const std::string& symbol = "",
                          const std::optional<long long>& since = std::nullopt,
                          const std::optional<int>& limit = std::nullopt) const override;
    json fetchMyTradesImpl(const std::string& symbol = "",
                        const std::optional<long long>& since = std::nullopt,
                        const std::optional<int>& limit = std::nullopt) const override;
    json fetchOrderTradesImpl(const std::string& id, const std::string& symbol) const override;
    json fetchBalanceImpl() const override;

    virtual json parseOrder(const json& order, const std::optional<json>& market = std::nullopt) const;
    virtual json parseOrders(const json& orders, const std::string& symbol,
                         const std::optional<long long>& since = std::nullopt,
                         const std::optional<int>& limit = std::nullopt) const;
    virtual json parseTrade(const json& trade, const std::optional<json>& market = std::nullopt) const;
    virtual json parseTrades(const json& trades, const std::string& symbol,
                         const std::optional<long long>& since = std::nullopt,
                         const std::optional<int>& limit = std::nullopt) const;

private:
    static Exchange* createInstance(const Config& config) {
        return new alpaca(config);
    }

};

} // namespace ccxt
