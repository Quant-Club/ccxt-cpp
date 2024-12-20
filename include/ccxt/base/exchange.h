#pragma once

#include <string>
#include <map>
#include <vector>
#include <optional>
#include <future>
#include <nlohmann/json.hpp>
#include <boost/coroutine2/coroutine.hpp>
#include "ccxt/base/exchange_base.h"

namespace ccxt {

using json = nlohmann::json;
using String = std::string;

class Exchange : public ExchangeBase {
public:
    Exchange(boost::asio::io_context& context, const Config& config = Config());
    virtual ~Exchange() = default;

    // Common methods
    virtual void init();
    virtual json describe() const;
    virtual AsyncPullType performHttpRequest(const std::string& host, const std::string& target, const std::string& method);
    // Usually methods
    virtual std::string implodeParams(const std::string& path, const json& params);
    virtual json omit(const json& params, const std::vector<std::string>& keys);
    virtual std::vector<std::string> extractParams(const std::string& path);
    virtual std::string urlencode(const json& params);
    virtual std::string encode(const std::string& string);
    virtual std::string hmac(const std::string& message, const std::string& secret,
                     const std::string& algorithm, const std::string& digest);
    virtual std::string costToPrecision(const std::string& symbol, double cost);
    virtual std::string priceToPrecision(const std::string& symbol, double price);
    virtual std::string amountToPrecision(const std::string& symbol, double amount);
    virtual std::string currencyToPrecision(const std::string& currency, double fee);
    virtual std::string feeToPrecision(const std::string& symbol, double fee);
    virtual long long parse8601(const std::string& datetime);
    virtual long long milliseconds();
    virtual std::string uuid();
    virtual std::string iso8601(long long timestamp);
    virtual Market market(const std::string& symbol);
    virtual std::string marketId(const std::string& symbol);

    // Synchronous REST API methods
    virtual json fetchMarkets(const json& params = json::object());
    virtual json fetchTicker(const String& symbol, const json& params = json::object());
    virtual json fetchTickers(const std::vector<String>& symbols = {}, const json& params = json::object());
    virtual json fetchOrderBook(const String& symbol, int limit = 0, const json& params = json::object());
    virtual json fetchTrades(const String& symbol, int since = 0, int limit = 0, const json& params = json::object());
    virtual json fetchOHLCV(const String& symbol, const String& timeframe = "1m",
                          int since = 0, int limit = 0, const json& params = json::object());
    virtual json fetchBalance(const json& params = json::object());
    virtual json createOrder(const String& symbol, const String& type, const String& side,
                           double amount, double price = 0, const json& params = json::object());
    virtual json cancelOrder(const String& id, const String& symbol = "", const json& params = json::object());
    virtual json fetchOrder(const String& id, const String& symbol = "", const json& params = json::object());
    virtual json fetchOrders(const String& symbol = "", int since = 0, int limit = 0, const json& params = json::object());
    virtual json fetchOpenOrders(const String& symbol = "", int since = 0, int limit = 0, const json& params = json::object());
    virtual json fetchClosedOrders(const String& symbol = "", int since = 0, int limit = 0, const json& params = json::object());
    virtual void loadMarkets(bool reload = false);
    std::string symbol(const std::string& marketId);

    // Asynchronous REST API methods
    virtual AsyncPullType fetchMarketsAsync(const json& params = json::object());
    virtual AsyncPullType fetchTickerAsync(const String& symbol, const json& params = json::object());
    virtual AsyncPullType fetchTickersAsync(const std::vector<String>& symbols = {}, const json& params = json::object());
    virtual AsyncPullType fetchOrderBookAsync(const String& symbol, int limit = 0, const json& params = json::object());
    virtual AsyncPullType fetchTradesAsync(const String& symbol, int since = 0, int limit = 0, const json& params = json::object());
    virtual AsyncPullType fetchOHLCVAsync(const String& symbol, const String& timeframe = "1m",
                                           int since = 0, int limit = 0, const json& params = json::object());
    virtual AsyncPullType fetchBalanceAsync(const json& params = json::object());
    virtual AsyncPullType createOrderAsync(const String& symbol, const String& type, const String& side,
                                           double amount, double price = 0, const json& params = json::object());
    virtual AsyncPullType cancelOrderAsync(const String& id, const String& symbol = "", const json& params = json::object());
    virtual AsyncPullType fetchOrderAsync(const String& id, const String& symbol = "", const json& params = json::object());
    virtual AsyncPullType fetchOrdersAsync(const String& symbol = "", int since = 0, int limit = 0, const json& params = json::object());
    virtual AsyncPullType fetchOpenOrdersAsync(const String& symbol = "", int since = 0, int limit = 0, const json& params = json::object());
    virtual AsyncPullType fetchClosedOrdersAsync(const String& symbol = "", int since = 0, int limit = 0, const json& params = json::object());

protected:
    // Synchronous HTTP methods
    virtual json fetch(const String& url,
                      const String& method = "GET",
                      const std::map<String, String>& headers = {},
                      const String& body = "");

    // Asynchronous HTTP methods
    virtual AsyncPullType fetchAsync(const String& url,
                                     const String& method = "GET",
                                     const std::map<String, String>& headers = {},
                                     const String& body = "");

    // Utility methods
    virtual String sign(const String& path, const String& api = "public",
                     const String& method = "GET",
                     const std::map<String, String>& params = {},
                     const std::map<String, String>& headers = {});

    // Parsing methods
    virtual json parseMarket(const json& market) const;
    virtual json parseTicker(const json& ticker, const json& market = json::object()) const;
    virtual json parseOrderBook(const json& orderbook, const String& symbol = "", const json& market = json::object()) const;
    virtual json parseOHLCV(const json& ohlcv, const json& market = json::object()) const;
    virtual json parseOrder(const json& order, const json& market = json::object()) const;
    virtual json parseTrade(const json& trade, const json& market = json::object()) const;
    virtual json parseBalance(const json& balance) const;
};

} // namespace ccxt
