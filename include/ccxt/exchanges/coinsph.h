#ifndef CCXT_EXCHANGE_COINSPH_H
#define CCXT_EXCHANGE_COINSPH_H

#include "ccxt/base/exchange.h"


namespace ccxt {

class coinsph : public Exchange {
public:
    coinsph(const Config& config = Config());
    ~coinsph() = default;

    static Exchange* create(const Config& config = Config()) {
        return new coinsph(config);
    }

protected:
    void init() override;
    Json describeImpl() const override;

    // Market Data
    Json fetchMarketsImpl() const override;
    Json fetchTickerImpl(const std::string& symbol) const override;
    Json fetchTickersImpl(const std::vector<std::string>& symbols = {}) const override;
    Json fetchOrderBookImpl(const std::string& symbol, const std::optional<int>& limit = std::nullopt) const override;
    Json fetchOHLCVImpl(const std::string& symbol, const std::string& timeframe, const std::optional<long long>& since = std::nullopt, const std::optional<int>& limit = std::nullopt) const override;
    Json fetchTradesImpl(const std::string& symbol, const std::optional<long long>& since = std::nullopt, const std::optional<int>& limit = std::nullopt) const override;
    Json fetchTimeImpl() const override;
    Json fetchStatusImpl() const override;
    Json fetchTradingFeeImpl(const std::string& symbol) const override;
    Json fetchTradingFeesImpl() const override;

    // Trading
    Json createOrderImpl(const std::string& symbol, const std::string& type, const std::string& side, double amount, const std::optional<double>& price = std::nullopt) override;
    Json cancelOrderImpl(const std::string& id, const std::string& symbol) override;
    Json fetchOrderImpl(const std::string& id, const std::string& symbol) const override;
    Json fetchOpenOrdersImpl(const std::string& symbol = "", const std::optional<long long>& since = std::nullopt, const std::optional<int>& limit = std::nullopt) const override;
    Json fetchClosedOrdersImpl(const std::string& symbol = "", const std::optional<long long>& since = std::nullopt, const std::optional<int>& limit = std::nullopt) const override;
    Json fetchMyTradesImpl(const std::string& symbol = "", const std::optional<long long>& since = std::nullopt, const std::optional<int>& limit = std::nullopt) const override;
    Json fetchOrderTradesImpl(const std::string& id, const std::string& symbol) const override;
    Json cancelAllOrdersImpl(const std::string& symbol = "") override;

    // Account
    Json fetchBalanceImpl() const override;
    Json fetchDepositAddressImpl(const std::string& code, const std::optional<std::string>& network = std::nullopt) const override;
    Json fetchDepositsImpl(const std::optional<std::string>& code = std::nullopt, const std::optional<long long>& since = std::nullopt, const std::optional<int>& limit = std::nullopt) const override;
    Json fetchWithdrawalsImpl(const std::optional<std::string>& code = std::nullopt, const std::optional<long long>& since = std::nullopt, const std::optional<int>& limit = std::nullopt) const override;
    Json withdrawImpl(const std::string& code, double amount, const std::string& address, const std::optional<std::string>& tag = std::nullopt) override;

private:
    static Exchange* createInstance(const Config& config) {
        return new coinsph(config);
    }

    static const std::string defaultBaseURL;
    static const std::string defaultVersion;
    static const int defaultRateLimit;
    static const bool defaultPro;

    

    // Helper methods for parsing responses
    Json parseTicker(const Json& ticker, const Json& market = Json()) const;
    Json parseTrade(const Json& trade, const Json& market = Json()) const;
    Json parseOrder(const Json& order, const Json& market = Json()) const;
    Json parseTransaction(const Json& transaction, const Json& currency = Json()) const;
    Json parseTradingFee(const Json& fee, const Json& market = Json()) const;
    Json parseDepositAddress(const Json& depositAddress, const Json& currency = Json()) const;

    // Authentication helpers
    std::string sign(const std::string& path, const std::string& api = "public", const std::string& method = "GET",
                    const Json& params = Json::object(), const Json& headers = Json::object(), const Json& body = Json::object()) const;

    // Error handling
    void handleErrors(const std::string& code, const std::string& reason, const std::string& url, const std::string& method,
                     const Json& headers, const Json& body, const Json& response, const std::string& requestHeaders,
                     const std::string& requestBody) const;

    // Helper methods
    std::string parseOrderSide(const std::string& status) const;
    std::string encodeOrderSide(const std::string& status) const;
    std::string parseOrderType(const std::string& status) const;
    std::string encodeOrderType(const std::string& status) const;
    std::string parseOrderStatus(const std::string& status) const;
    std::string parseOrderTimeInForce(const std::string& status) const;
    std::string parseTransactionStatus(const std::string& status) const;
    std::string urlEncodeQuery(const Json& query = Json::object()) const;
    Json parseArrayParam(const Json& array, const std::string& key) const;
};

} // namespace ccxt

#endif // CCXT_EXCHANGE_COINSPH_H
