#pragma once

#include "ccxt/base/exchange.h"

namespace ccxt {

class Lbank : public Exchange {
public:
    Lbank();
    ~Lbank() override = default;

    // Market Data API
    json fetchMarkets(const json& params = json::object()) override;
    json fetchTicker(const String& symbol, const json& params = json::object()) override;
    json fetchTickers(const std::vector<String>& symbols = {}, const json& params = json::object()) override;
    json fetchOrderBook(const String& symbol, int limit = 0, const json& params = json::object()) override;
    json fetchTrades(const String& symbol, int since = 0, int limit = 0, const json& params = json::object()) override;
    json fetchOHLCV(const String& symbol, const String& timeframe = "1m",
                    int since = 0, int limit = 0, const json& params = json::object()) override;

    // Async Market Data API
    std::future<json> asyncFetchMarkets(const json& params = json::object());
    std::future<json> asyncFetchTicker(const String& symbol, const json& params = json::object());
    std::future<json> asyncFetchTickers(const std::vector<String>& symbols = {}, const json& params = json::object());
    std::future<json> asyncFetchOrderBook(const String& symbol, int limit = 0, const json& params = json::object());
    std::future<json> asyncFetchTrades(const String& symbol, int since = 0, int limit = 0, const json& params = json::object());
    std::future<json> asyncFetchOHLCV(const String& symbol, const String& timeframe = "1m",
                                     int since = 0, int limit = 0, const json& params = json::object());

    // Trading API
    json fetchBalance(const json& params = json::object()) override;
    json createOrder(const String& symbol, const String& type, const String& side,
                    double amount, double price = 0, const json& params = json::object()) override;
    json cancelOrder(const String& id, const String& symbol = "", const json& params = json::object()) override;
    json fetchOrder(const String& id, const String& symbol = "", const json& params = json::object()) override;
    json fetchOrders(const String& symbol = "", int since = 0, int limit = 0, const json& params = json::object()) override;
    json fetchOpenOrders(const String& symbol = "", int since = 0, int limit = 0, const json& params = json::object()) override;
    json fetchClosedOrders(const String& symbol = "", int since = 0, int limit = 0, const json& params = json::object()) override;

    // Async Trading API
    std::future<json> asyncFetchBalance(const json& params = json::object());
    std::future<json> asyncCreateOrder(const String& symbol, const String& type, const String& side,
                                     double amount, double price = 0, const json& params = json::object());
    std::future<json> asyncCancelOrder(const String& id, const String& symbol = "", const json& params = json::object());
    std::future<json> asyncFetchOrder(const String& id, const String& symbol = "", const json& params = json::object());
    std::future<json> asyncFetchOrders(const String& symbol = "", int since = 0, int limit = 0, const json& params = json::object());
    std::future<json> asyncFetchOpenOrders(const String& symbol = "", int since = 0, int limit = 0, const json& params = json::object());
    std::future<json> asyncFetchClosedOrders(const String& symbol = "", int since = 0, int limit = 0, const json& params = json::object());

    // Account API
    json fetchMyTrades(const String& symbol = "", int since = 0, int limit = 0, const json& params = json::object());
    json fetchDeposits(const String& code = "", int since = 0, int limit = 0, const json& params = json::object());
    json fetchWithdrawals(const String& code = "", int since = 0, int limit = 0, const json& params = json::object());
    json fetchDepositAddress(const String& code, const json& params = json::object());
    json withdraw(const String& code, double amount, const String& address, const String& tag = "", const json& params = json::object());

    // Async Account API
    std::future<json> asyncFetchMyTrades(const String& symbol = "", int since = 0, int limit = 0, const json& params = json::object());
    std::future<json> asyncFetchDeposits(const String& code = "", int since = 0, int limit = 0, const json& params = json::object());
    std::future<json> asyncFetchWithdrawals(const String& code = "", int since = 0, int limit = 0, const json& params = json::object());
    std::future<json> asyncFetchDepositAddress(const String& code, const json& params = json::object());
    std::future<json> asyncWithdraw(const String& code, double amount, const String& address, const String& tag = "", const json& params = json::object());

    // Additional Features
    json fetchCurrencies(const json& params = json::object());
    json fetchTradingFees(const json& params = json::object());
    json fetchFundingFees(const json& params = json::object());
    json fetchTransactionFees(const json& params = json::object());
    json fetchSystemStatus(const json& params = json::object());
    json fetchTime(const json& params = json::object());

    // Async Additional Features
    std::future<json> asyncFetchCurrencies(const json& params = json::object());
    std::future<json> asyncFetchTradingFees(const json& params = json::object());
    std::future<json> asyncFetchFundingFees(const json& params = json::object());
    std::future<json> asyncFetchTransactionFees(const json& params = json::object());
    std::future<json> asyncFetchSystemStatus(const json& params = json::object());
    std::future<json> asyncFetchTime(const json& params = json::object());

protected:
    String sign(const String& path, const String& api = "public",
               const String& method = "GET", const json& params = json::object(),
               const std::map<String, String>& headers = {}, const json& body = nullptr) override;

private:
    void initializeApiEndpoints();
    String getLbankSymbol(const String& symbol);
    String getCommonSymbol(const String& lbankSymbol);
    json parseOrder(const json& order, const Market& market = Market());
    json parseTrade(const json& trade, const Market& market = Market());
    json parseOrderStatus(const String& status);
    json parseTicker(const json& ticker, const Market& market = Market());
    json parseOHLCV(const json& ohlcv, const Market& market = Market());
    json parseBalance(const json& response);
    json parseFee(const json& fee, const Market& market = Market());
    json parseTransaction(const json& transaction, const String& currency = "");
    json parseDepositAddress(const json& depositAddress, const String& currency = "");
    String createSignature(const String& timestamp, const String& method,
                         const String& path, const String& body = "");
    String getNonce();

    std::map<String, String> timeframes;
    std::map<String, String> options;
    std::map<int, String> errorCodes;
    std::map<String, String> currencyIds;
    bool hasPublicAPI;
    bool hasPrivateAPI;
};

} // namespace ccxt
