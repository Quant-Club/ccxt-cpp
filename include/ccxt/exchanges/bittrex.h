#pragma once

#include "../base/exchange.h"

namespace ccxt {

class Bittrex : public Exchange {
public:
    Bittrex();
    ~Bittrex() override = default;

    // Market Data API
    json fetchMarkets(const json& params = json::object()) override;
    json fetchTicker(const String& symbol, const json& params = json::object()) override;
    json fetchTickers(const std::vector<String>& symbols = {}, const json& params = json::object()) override;
    json fetchOrderBook(const String& symbol, int limit = 0, const json& params = json::object()) override;
    json fetchTrades(const String& symbol, int since = 0, int limit = 0, const json& params = json::object()) override;
    json fetchOHLCV(const String& symbol, const String& timeframe = "1m",
                    int since = 0, int limit = 0, const json& params = json::object()) override;

    // Trading API
    json fetchBalance(const json& params = json::object()) override;
    json createOrder(const String& symbol, const String& type, const String& side,
                    double amount, double price = 0, const json& params = json::object()) override;
    json cancelOrder(const String& id, const String& symbol = "", const json& params = json::object()) override;
    json fetchOrder(const String& id, const String& symbol = "", const json& params = json::object()) override;
    json fetchOrders(const String& symbol = "", int since = 0, int limit = 0, const json& params = json::object()) override;
    json fetchOpenOrders(const String& symbol = "", int since = 0, int limit = 0, const json& params = json::object()) override;
    json fetchClosedOrders(const String& symbol = "", int since = 0, int limit = 0, const json& params = json::object()) override;

    // Bittrex specific methods
    json fetchMyTrades(const String& symbol = "", int since = 0, int limit = 0, const json& params = json::object());
    json fetchDeposits(const String& code = "", int since = 0, int limit = 0, const json& params = json::object());
    json fetchWithdrawals(const String& code = "", int since = 0, int limit = 0, const json& params = json::object());
    json fetchDepositAddress(const String& code, const json& params = json::object());
    json createDepositAddress(const String& code, const json& params = json::object());
    json withdraw(const String& code, double amount, const String& address,
                 const String& tag = "", const json& params = json::object());
    json fetchCurrencies(const json& params = json::object());
    json fetchTransactionFees(const json& params = json::object());
    json fetchTradingFees(const json& params = json::object());
    json fetchDepositFees(const json& params = json::object());
    json fetchWithdrawalFees(const json& params = json::object());

protected:
    String sign(const String& path, const String& api = "public",
               const String& method = "GET", const json& params = json::object(),
               const std::map<String, String>& headers = {}, const json& body = nullptr) override;

private:
    void initializeApiEndpoints();
    String getBittrexSymbol(const String& symbol);
    String getCommonSymbol(const String& bittrexSymbol);
    json parseOrder(const json& order, const Market& market = Market());
    json parseTrade(const json& trade, const Market& market = Market());
    json parseTransaction(const json& transaction);
    json parseOrderStatus(const String& status);
    json parseTradingFee(const json& fee, const Market& market = Market());
    json parseTransactionType(const String& type);
    json parseOrderSide(const String& orderType);
    String createSignature(const String& timestamp, const String& uri,
                         const String& method, const String& body = "");
    String getTimestamp();

    std::map<String, String> timeframes;
    std::map<String, String> options;
    bool hasPrivateAPI;
};

} // namespace ccxt