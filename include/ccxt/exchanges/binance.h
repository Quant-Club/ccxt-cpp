#pragma once

#include <ccxt/base/exchange.h>
#include <boost/beast/ssl.hpp>
#include <boost/asio/ssl.hpp>
#include <openssl/evp.h>
#include <openssl/params.h>

namespace ccxt {
class Binance : public Exchange {
public:
    static const std::string defaultHostname;
    static const int defaultRateLimit;
    static const bool defaultPro;
    

    explicit Binance(boost::asio::io_context& context, const Config& config = Config());
    virtual ~Binance() = default;

    void init() override;
    void describe() const override;

protected:
    std::string getMarketType(const std::string& symbol) const;
    std::string getEndpoint(const std::string& path, const std::string& type) const;
    Market findMarket(const std::string& symbol) const;
    std::string urlencode(const json& params) const;

    // Market Data API
    json fetchMarketsImpl() const override;
    json fetchTickerImpl(const std::string& symbol) const override;
    json fetchTickersImpl(const std::vector<std::string>& symbols = {}) const override;
    json fetchOrderBookImpl(const std::string& symbol, const std::optional<int>& limit = std::nullopt) const override;
    json fetchTradesImpl(const std::string& symbol, const std::optional<long long>& since = std::nullopt,
                      const std::optional<int>& limit = std::nullopt) const override;
    json fetchOHLCVImpl(const std::string& symbol, const std::string& timeframe = "1m",
                     const std::optional<long long>& since = std::nullopt,
                     const std::optional<int>& limit = std::nullopt) const override;
    json fetchTimeImpl() const override;
    json fetchCurrenciesImpl() const override;
    json fetchTradingFeesImpl() const override;
    json fetchBalanceImpl() const override;
    json fetchDepositAddressImpl(const std::string& code, const json& params = json::object()) const override;
    json fetchDepositsImpl(const std::string& code = "", const std::optional<long long>& since = std::nullopt,
                        const std::optional<int>& limit = std::nullopt) const override;
    json fetchWithdrawalsImpl(const std::string& code = "", const std::optional<long long>& since = std::nullopt,
                          const std::optional<int>& limit = std::nullopt) const override;
    json fetchDepositsWithdrawalsImpl(const std::string& code = "", const std::optional<long long>& since = std::nullopt,
                                   const std::optional<int>& limit = std::nullopt) const override;
    json fetchDepositWithdrawFeesImpl() const override;
    json fetchFundingRatesImpl(const std::vector<std::string>& symbols = {}) const override;
    json fetchFundingRateHistoryImpl(const std::string& symbol, const std::optional<long long>& since = std::nullopt,
                                  const std::optional<int>& limit = std::nullopt) const override;
    json fetchLeverageImpl(const std::string& symbol) const override;
    json fetchMarginModesImpl(const std::vector<std::string>& symbols = {}) const override;
    json fetchPositionsImpl(const std::vector<std::string>& symbols = {}) const override;
    json fetchBorrowRatesImpl() const override;
    json fetchBorrowRateHistoryImpl(const std::string& code, const std::optional<long long>& since = std::nullopt,
                                 const std::optional<int>& limit = std::nullopt) const override;
    json fetchBorrowInterestImpl(const std::string& code = "", const std::optional<long long>& since = std::nullopt,
                              const std::optional<int>& limit = std::nullopt) const override;
    json fetchMyTradesImpl(const std::string& symbol = "", const std::optional<long long>& since = std::nullopt,
                        const std::optional<int>& limit = std::nullopt) const override;
    json fetchOrdersImpl(const std::string& symbol = "", const std::optional<long long>& since = std::nullopt,
                      const std::optional<int>& limit = std::nullopt) const override;
    json fetchOpenOrdersImpl(const std::string& symbol = "", const std::optional<long long>& since = std::nullopt,
                         const std::optional<int>& limit = std::nullopt) const override;
    json fetchClosedOrdersImpl(const std::string& symbol = "", const std::optional<long long>& since = std::nullopt,
                           const std::optional<int>& limit = std::nullopt) const override;
    json fetchOrderImpl(const std::string& id, const std::string& symbol = "") const override;

    // Trading API
    json createOrderImpl(const std::string& symbol, const std::string& type, const std::string& side,
                      double amount, const std::optional<double>& price = std::nullopt) override;
    json cancelOrderImpl(const std::string& id, const std::string& symbol = "") override;
    json cancelAllOrdersImpl(const std::string& symbol = "") override;
    json editOrderImpl(const std::string& id, const std::string& symbol, const std::string& type,
                    const std::string& side, const std::optional<double>& amount = std::nullopt,
                    const std::optional<double>& price = std::nullopt) override;
    json setLeverageImpl(int leverage, const std::string& symbol = "") override;
    json setMarginModeImpl(const std::string& marginMode, const std::string& symbol = "") override;
    json addMarginImpl(const std::string& symbol, double amount) override;
    json reduceMarginImpl(const std::string& symbol, double amount) override;
    json borrowCrossMarginImpl(const std::string& code, double amount, const std::string& symbol = "") override;
    json borrowIsolatedMarginImpl(const std::string& symbol, const std::string& code, double amount) override;
    json repayCrossMarginImpl(const std::string& code, double amount, const std::string& symbol = "") override;
    json repayIsolatedMarginImpl(const std::string& symbol, const std::string& code, double amount) override;
    json transferImpl(const std::string& code, double amount, const std::string& fromAccount,
                   const std::string& toAccount) override;
    json parseOrderBook(const json& orderbook, const std::string& symbol, const Market& market) const override;
    json parseOHLCV(const json& ohlcv, const Market& market, const std::string& timeframe) const override;
    json parseTicker(const json& ticker, const Market& market) const override;

    json loadMarkets() const;

    json privateGetAccount(const json& params = json::object()) const;

    json parseMarket(const json& market) const;
  
private:
    // Helper methods
    std::string sign(const std::string& path, const std::string& api = "public",
                  const std::string& method = "GET", const json& params = json::object(),
                  const std::map<std::string, std::string>& headers = {},
                  const json& body = nullptr) const override;
    std::string getTimestamp() const;
    std::string createSignature(const std::string& queryString) const;
    std::string parseSymbol(const std::string& symbol) const;
    std::string parseTimeInForce(const std::string& timeInForce) const;
    std::string parseOrderType(const std::string& type) const;
    std::string parseOrderSide(const std::string& side) const;
    std::string parseOrderStatus(const std::string& status) const;
    std::string parseTransactionStatus(const std::string& status) const;
    json parseBidsAsks(const json& bidasks, const json& market) const;
    json parseOrder(const json& order, const Market& market = Market()) const override;
    json parseTrade(const json& trade, const Market& market = Market()) const override;
    json parseBalance(const json& response) const override;
    json parseFee(const json& fee, const Market& market = Market()) const override;
    json parsePosition(const json& position, const Market& market = Market()) const override;
    json parseFundingRate(const json& fundingRate, const Market& market = Market()) const override;
    json parseTransaction(const json& transaction, const std::string& currency = "") const override;
    json parseDepositAddress(const json& depositAddress, const std::string& currency = "") const override;
    json parseWithdrawal(const json& withdrawal, const std::string& currency = "") const override;
    json parseDeposit(const json& deposit, const std::string& currency = "") const override;

    mutable std::mutex markets_mutex;
    mutable bool markets_loaded{false};

    // Member variables
    std::map<std::string, std::string> timeframes;
    std::map<std::string, std::string> marketTypes;
    std::map<std::string, std::string> options;
    std::map<int, std::string> errorCodes;
    std::map<std::string, json> fees;
    bool hasPublicAPI;
    bool hasPrivateAPI;
    bool hasFiatAPI;
    bool hasMarginAPI;
    bool hasFuturesAPI;
    bool hasOptionsAPI;
    bool hasLeveragedAPI;
};

} // namespace ccxt
