#ifndef CCXT_MIXCOIN_H
#define CCXT_MIXCOIN_H

#include "../exchange.h"
#include <future>

namespace ccxt {

class MixCoin : public Exchange {
public:
    MixCoin(const Config& config = Config());
    ~MixCoin() override = default;

    // Market Data API
    json fetchMarkets(const json& params = json()) override;
    json fetchTicker(const String& symbol, const json& params = json()) override;
    json fetchTickers(const std::vector<String>& symbols = {}, const json& params = json()) override;
    json fetchOrderBook(const String& symbol, int limit = 0, const json& params = json()) override;
    json fetchTrades(const String& symbol, int since = 0, int limit = 0, const json& params = json()) override;
    json fetchOHLCV(const String& symbol, const String& timeframe = "1m",
                    int since = 0, int limit = 0, const json& params = json()) override;

    // Trading API
    json fetchBalance(const json& params = json()) override;
    json createOrder(const String& symbol, const String& type, const String& side,
                    double amount, double price = 0, const json& params = json()) override;
    json cancelOrder(const String& id, const String& symbol = "", const json& params = json()) override;
    json fetchOrder(const String& id, const String& symbol = "", const json& params = json()) override;
    json fetchOrders(const String& symbol = "", int since = 0, int limit = 0, const json& params = json()) override;
    json fetchOpenOrders(const String& symbol = "", int since = 0, int limit = 0, const json& params = json()) override;
    json fetchClosedOrders(const String& symbol = "", int since = 0, int limit = 0, const json& params = json()) override;
    json fetchMyTrades(const String& symbol = "", int since = 0, int limit = 0, const json& params = json()) override;

    // Account API
    json fetchAccounts(const json& params = json());
    json fetchLedger(const String& code, int since = 0, int limit = 0, const json& params = json());
    json fetchTradingFee(const String& symbol, const json& params = json());

    // Async Market Data API
    std::future<json> asyncFetchMarkets(const json& params = json());
    std::future<json> asyncFetchTicker(const String& symbol, const json& params = json());
    std::future<json> asyncFetchTickers(const std::vector<String>& symbols = {}, const json& params = json());
    std::future<json> asyncFetchOrderBook(const String& symbol, int limit = 0, const json& params = json());
    std::future<json> asyncFetchTrades(const String& symbol, int since = 0, int limit = 0, const json& params = json());
    std::future<json> asyncFetchOHLCV(const String& symbol, const String& timeframe = "1m",
                                     int since = 0, int limit = 0, const json& params = json());

    // Async Trading API
    std::future<json> asyncFetchBalance(const json& params = json());
    std::future<json> asyncCreateOrder(const String& symbol, const String& type, const String& side,
                                     double amount, double price = 0, const json& params = json());
    std::future<json> asyncCancelOrder(const String& id, const String& symbol = "", const json& params = json());
    std::future<json> asyncFetchOrder(const String& id, const String& symbol = "", const json& params = json());
    std::future<json> asyncFetchOrders(const String& symbol = "", int since = 0, int limit = 0, const json& params = json());
    std::future<json> asyncFetchOpenOrders(const String& symbol = "", int since = 0, int limit = 0, const json& params = json());
    std::future<json> asyncFetchClosedOrders(const String& symbol = "", int since = 0, int limit = 0, const json& params = json());
    std::future<json> asyncFetchMyTrades(const String& symbol = "", int since = 0, int limit = 0, const json& params = json());

    // Async Account API
    std::future<json> asyncFetchAccounts(const json& params = json());
    std::future<json> asyncFetchLedger(const String& code, int since = 0, int limit = 0, const json& params = json());
    std::future<json> asyncFetchTradingFee(const String& symbol, const json& params = json());

protected:
    String sign(const String& path, const String& api = "public",
               const String& method = "GET", const json& params = json(),
               const std::map<String, String>& headers = {}, const json& body = nullptr) override;

private:
    // Parse Methods
    json parseTicker(const json& ticker, const Market& market);
    json parseTrade(const json& trade, const Market& market);
    json parseOrder(const json& order, const Market& market);
    json parseLedgerEntry(const json& item, const Currency& currency);
    json parseTradingFee(const json& fee, const Market& market);
    String getAccountId(const String& type, const String& currency);
};

} // namespace ccxt

#endif // CCXT_MIXCOIN_H