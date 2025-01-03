#include "btcbox.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <openssl/hmac.h>

namespace ccxt {

BTCBOX::BTCBOX() {
    id = "btcbox";
    name = "BtcBox";
    version = "1";
    rateLimit = 1000;
    certified = false;
    pro = false;

    // Initialize API endpoints
    baseUrl = "https://www.btcbox.co.jp/api/v1";
    
    urls = {
        {"logo", "https://user-images.githubusercontent.com/51840849/87327317-98c55400-c53c-11ea-9a11-81f7d951cc74.jpg"},
        {"api", {
            {"public", "https://www.btcbox.co.jp/api/v1"},
            {"private", "https://www.btcbox.co.jp/api/v1"}
        }},
        {"www", "https://www.btcbox.co.jp/"},
        {"doc", {
            "https://blog.btcbox.jp/en/archives/8762",
            "https://blog.btcbox.jp/en/archives/8766"
        }},
        {"fees", "https://www.btcbox.co.jp/help/fees"}
    };

    timeframes = {
        {"1m", "1min"},
        {"5m", "5min"},
        {"15m", "15min"},
        {"30m", "30min"},
        {"1h", "1hour"},
        {"4h", "4hour"},
        {"1d", "1day"},
        {"1w", "1week"}
    };

    options = {
        {"adjustForTimeDifference", true},
        {"recvWindow", 5000},
        {"timeDifference", 0}
    };

    errorCodes = {
        {10000, "URL does not exist"},
        {10001, "A system error occurred. Please contact support"},
        {10002, "API authentication failed"},
        {10003, "API key does not exist"},
        {10004, "API key has been disabled"},
        {10005, "Invalid nonce parameter"},
        {10006, "Invalid signature"},
        {10007, "Invalid IP address"},
        {10008, "Required parameters are missing"},
        {10009, "Invalid parameters"},
        {10010, "Order does not exist"},
        {10011, "Insufficient balance"},
        {10012, "Order quantity is too small"},
        {10013, "Order price is invalid"},
        {10014, "Rate limit exceeded"}
    };

    currencyIds = {
        {"BTC", "btc"},
        {"ETH", "eth"},
        {"LTC", "ltc"},
        {"BCH", "bch"},
        {"JPY", "jpy"}
    };

    initializeApiEndpoints();
}

void BTCBOX::initializeApiEndpoints() {
    api = {
        {"public", {
            {"GET", {
                "ticker",
                "depth",
                "trades",
                "candlestick",
                "currencies",
                "fees",
                "time",
                "status"
            }}
        }},
        {"private", {
            {"POST", {
                "balance",
                "trade_add",
                "trade_cancel",
                "trade_view",
                "trade_list",
                "trade_history",
                "deposit_history",
                "withdraw_history",
                "withdraw_coin",
                "transaction_history"
            }}
        }}
    };
}

json BTCBOX::fetchMarkets(const json& params) {
    json response = fetch("/spot/markets", "public", "GET", params);
    json result = json::array();
    
    for (const auto& market : response) {
        std::string id = market["id"];
        std::string baseId = market["base_currency"];
        std::string quoteId = market["quote_currency"];
        std::string base = this->safeCurrencyCode(baseId);
        std::string quote = this->safeCurrencyCode(quoteId);
        std::string symbol = base + "/" + quote;
        
        result.push_back({
            {"id", id},
            {"symbol", symbol},
            {"base", base},
            {"quote", quote},
            {"baseId", baseId},
            {"quoteId", quoteId},
            {"active", true},
            {"type", "spot"},
            {"spot", true},
            {"future", false},
            {"option", false},
            {"margin", false},
            {"contract", false},
            {"precision", {
                {"amount", market["amount_precision"]},
                {"price", market["price_precision"]}
            }},
            {"limits", {
                {"amount", {
                    {"min", market["min_order_amount"]},
                    {"max", market["max_order_amount"]}
                }},
                {"price", {
                    {"min", market["min_order_price"]},
                    {"max", market["max_order_price"]}
                }},
                {"cost", {
                    {"min", market["min_order_value"]},
                    {"max", nullptr}
                }}
            }},
            {"info", market}
        });
    }
    
    return result;
}

json BTCBOX::fetchBalance(const json& params) {
    this->loadMarkets();
    json response = this->privatePostBalance(params);
    return this->parseBalance(response);
}

json BTCBOX::parseBalance(const json& response) {
    json result = {{"info", response}};
    
    for (const auto& [currency, balance] : response.items()) {
        if (currency != "success" && currency != "result") {
            std::string code = this->safeCurrencyCode(currency);
            std::string account = {
                {"free", this->safeFloat(balance, "available")},
                {"used", this->safeFloat(balance, "in_use")},
                {"total", this->safeFloat(balance, "total")}
            };
            result[code] = account;
        }
    }
    
    return result;
}

json BTCBOX::createOrder(const std::string& symbol, const std::string& type,
                        const std::string& side, double amount,
                        double price, const json& params) {
    this->loadMarkets();
    Market market = this->market(symbol);
    json request = {
        {"coin", market["baseId"]},
        {"amount", this->amountToPrecision(symbol, amount)},
        {"price", this->priceToPrecision(symbol, price)},
        {"type", side}
    };
    json response = this->privatePostTradeAdd(this->extend(request, params));
    return this->parseOrder(response, market);
}

json BTCBOX::cancelOrder(const std::string& id, const std::string& symbol, const json& params) {
    if (symbol.empty()) {
        throw ExchangeError("symbol is required for cancelOrder");
    }
    this->loadMarkets();
    Market market = this->market(symbol);
    json request = {
        {"coin", market["baseId"]},
        {"id", id}
    };
    return this->privatePostTradeCancel(this->extend(request, params));
}

json BTCBOX::fetchOrder(const std::string& id, const std::string& symbol, const json& params) {
    if (symbol.empty()) {
        throw ExchangeError("symbol is required for fetchOrder");
    }
    this->loadMarkets();
    Market market = this->market(symbol);
    json request = {
        {"coin", market["baseId"]},
        {"id", id}
    };
    json response = this->privatePostTradeView(this->extend(request, params));
    return this->parseOrder(response, market);
}

json BTCBOX::fetchOrders(const std::string& symbol, int since, int limit, const json& params) {
    if (symbol.empty()) {
        throw ExchangeError("symbol is required for fetchOrders");
    }
    this->loadMarkets();
    Market market = this->market(symbol);
    json request = {
        {"coin", market["baseId"]}
    };
    if (limit != 0) {
        request["limit"] = limit;
    }
    json response = this->privatePostTradeList(this->extend(request, params));
    return this->parseOrders(response, market, since, limit);
}

json BTCBOX::fetchOpenOrders(const std::string& symbol, int since, int limit, const json& params) {
    json request = this->extend({
        {"type", "open"}
    }, params);
    return this->fetchOrders(symbol, since, limit, request);
}

json BTCBOX::fetchClosedOrders(const std::string& symbol, int since, int limit, const json& params) {
    json request = this->extend({
        {"type", "closed"}
    }, params);
    return this->fetchOrders(symbol, since, limit, request);
}

std::string BTCBOX::sign(const std::string& path, const std::string& api,
                   const std::string& method, const json& params,
                   const std::map<std::string, std::string>& headers,
                   const json& body) {
    std::string url = this->urls["api"][api] + "/" + this->implodeParams(path, params);
    json query = this->omit(params, this->extractParams(path));
    
    if (api == "public") {
        if (!query.empty()) {
            url += "?" + this->urlencode(query);
        }
    } else {
        this->checkRequiredCredentials();
        std::string nonce = this->nonce().str();
        json auth = this->extend({
            "key", this->config_.apiKey,
            "nonce", nonce
        }, query);
        
        std::string querystd::string = this->urlencode(this->keysort(auth));
        std::string signature = this->hmac(querystd::string, this->encode(this->config_.secret),
                                    "sha256", "hex");
        
        auth["signature"] = signature;
        body = this->json(auth);
        const_cast<std::map<std::string, std::string>&>(headers)["Content-Type"] = "application/json";
    }
    
    return url;
}

std::string BTCBOX::getNonce() {
    return std::to_string(this->milliseconds());
}

json BTCBOX::parseOrder(const json& order, const Market& market) {
    std::string id = this->safeString(order, "id");
    std::string timestamp = this->safeString(order, "datetime");
    std::string status = this->parseOrderStatus(this->safeString(order, "status"));
    std::string symbol = nullptr;
    
    if (!market.empty()) {
        symbol = market["symbol"];
    }
    
    std::string type = this->safeString(order, "type");
    std::string side = this->safeString(order, "side");
    
    return {
        {"id", id},
        {"clientOrderId", nullptr},
        {"datetime", this->iso8601(timestamp)},
        {"timestamp", this->parse8601(timestamp)},
        {"lastTradeTimestamp", nullptr},
        {"type", type},
        {"timeInForce", nullptr},
        {"postOnly", nullptr},
        {"status", status},
        {"symbol", symbol},
        {"side", side},
        {"price", this->safeFloat(order, "price")},
        {"stopPrice", nullptr},
        {"cost", this->safeFloat(order, "total")},
        {"amount", this->safeFloat(order, "amount")},
        {"filled", this->safeFloat(order, "filled_amount")},
        {"remaining", this->safeFloat(order, "remaining_amount")},
        {"trades", nullptr},
        {"fee", {
            {"currency", market["quote"]},
            {"cost", this->safeFloat(order, "fee")},
            {"rate", this->safeFloat(order, "fee_rate")}
        }},
        {"info", order}
    };
}

std::string BTCBOX::parseOrderStatus(const std::string& status) {
    static const std::map<std::string, std::string> statuses = {
        {"open", "open"},
        {"closed", "closed"},
        {"canceled", "canceled"},
        {"expired", "expired"}
    };
    
    return this->safeString(statuses, status, status);
}

json BTCBOX::fetchTicker(const std::string& symbol, const json& params) {
    this->loadMarkets();
    Market market = this->market(symbol);
    json request = {
        {"coin", market["baseId"]}
    };
    json response = this->publicGetTicker(this->extend(request, params));
    return this->parseTicker(response, market);
}

json BTCBOX::fetchTickers(const std::vector<std::string>& symbols, const json& params) {
    this->loadMarkets();
    json response = this->publicGetTicker(params);
    json result = json::object();
    for (const auto& market : this->markets) {
        std::string symbol = market["symbol"];
        if (symbols.empty() || std::find(symbols.begin(), symbols.end(), symbol) != symbols.end()) {
            json request = {
                {"coin", market["baseId"]}
            };
            json ticker = this->publicGetTicker(this->extend(request, params));
            result[symbol] = this->parseTicker(ticker, market);
        }
    }
    return result;
}

json BTCBOX::fetchOrderBook(const std::string& symbol, int limit, const json& params) {
    this->loadMarkets();
    Market market = this->market(symbol);
    json request = {
        {"coin", market["baseId"]}
    };
    if (limit != 0) {
        request["limit"] = limit;
    }
    json response = this->publicGetDepth(this->extend(request, params));
    return this->parseOrderBook(response, symbol);
}

json BTCBOX::fetchTrades(const std::string& symbol, int since, int limit, const json& params) {
    this->loadMarkets();
    Market market = this->market(symbol);
    json request = {
        {"coin", market["baseId"]}
    };
    if (limit != 0) {
        request["limit"] = limit;
    }
    json response = this->publicGetTrades(this->extend(request, params));
    return this->parseTrades(response, market, since, limit);
}

json BTCBOX::parseTicker(const json& ticker, const Market& market) {
    double timestamp = this->safeTimestamp(ticker, "timestamp");
    std::string symbol = this->safeString(market, "symbol");
    return {
        {"symbol", symbol},
        {"timestamp", timestamp},
        {"datetime", this->iso8601(timestamp)},
        {"high", this->safeFloat(ticker, "high")},
        {"low", this->safeFloat(ticker, "low")},
        {"bid", this->safeFloat(ticker, "buy")},
        {"ask", this->safeFloat(ticker, "sell")},
        {"last", this->safeFloat(ticker, "last")},
        {"close", this->safeFloat(ticker, "last")},
        {"baseVolume", this->safeFloat(ticker, "vol")},
        {"quoteVolume", this->safeFloat(ticker, "volume")},
        {"info", ticker}
    };
}

json BTCBOX::parseTrade(const json& trade, const Market& market) {
    std::string id = this->safeString(trade, "tid");
    double timestamp = this->safeTimestamp(trade, "date");
    double price = this->safeFloat(trade, "price");
    double amount = this->safeFloat(trade, "amount");
    std::string side = this->safeString(trade, "type");
    return {
        {"id", id},
        {"info", trade},
        {"timestamp", timestamp},
        {"datetime", this->iso8601(timestamp)},
        {"symbol", market["symbol"]},
        {"type", "limit"},
        {"side", side},
        {"price", price},
        {"amount", amount},
        {"cost", price * amount}
    };
}

json BTCBOX::fetchMyTrades(const std::string& symbol, int since, int limit, const json& params) {
    if (symbol.empty()) {
        throw ExchangeError("symbol is required for fetchMyTrades");
    }
    this->loadMarkets();
    Market market = this->market(symbol);
    json request = {
        {"coin", market["baseId"]}
    };
    if (limit != 0) {
        request["limit"] = limit;
    }
    json response = this->privatePostTradeHistory(this->extend(request, params));
    return this->parseTrades(response, market, since, limit);
}

json BTCBOX::fetchDeposits(const std::string& code, int since, int limit, const json& params) {
    if (code.empty()) {
        throw ExchangeError("code is required for fetchDeposits");
    }
    this->loadMarkets();
    std::string currency = this->currency(code);
    json request = {
        {"coin", currency["id"]}
    };
    if (limit != 0) {
        request["limit"] = limit;
    }
    json response = this->privatePostDepositHistory(this->extend(request, params));
    return this->parseTransactions(response, code, since, limit, "deposit");
}

json BTCBOX::fetchWithdrawals(const std::string& code, int since, int limit, const json& params) {
    if (code.empty()) {
        throw ExchangeError("code is required for fetchWithdrawals");
    }
    this->loadMarkets();
    std::string currency = this->currency(code);
    json request = {
        {"coin", currency["id"]}
    };
    if (limit != 0) {
        request["limit"] = limit;
    }
    json response = this->privatePostWithdrawHistory(this->extend(request, params));
    return this->parseTransactions(response, code, since, limit, "withdrawal");
}

json BTCBOX::fetchDepositAddress(const std::string& code, const json& params) {
    if (code.empty()) {
        throw ExchangeError("code is required for fetchDepositAddress");
    }
    this->loadMarkets();
    std::string currency = this->currency(code);
    json request = {
        {"coin", currency["id"]}
    };
    json response = this->privatePostDepositAddress(this->extend(request, params));
    return this->parseDepositAddress(response, currency);
}

json BTCBOX::withdraw(const std::string& code, double amount, const std::string& address,
                     const std::string& tag, const json& params) {
    this->checkAddress(address);
    if (code.empty()) {
        throw ExchangeError("code is required for withdraw");
    }
    this->loadMarkets();
    std::string currency = this->currency(code);
    json request = {
        {"coin", currency["id"]},
        {"amount", this->currencyToPrecision(code, amount)},
        {"address", address}
    };
    if (!tag.empty()) {
        request["tag"] = tag;
    }
    json response = this->privatePostWithdrawCoin(this->extend(request, params));
    return this->parseTransaction(response, currency);
}

json BTCBOX::parseTransaction(const json& transaction, const std::string& currency) {
    std::string id = this->safeString(transaction, "id");
    double timestamp = this->safeTimestamp(transaction, "timestamp");
    std::string address = this->safeString(transaction, "address");
    std::string tag = this->safeString(transaction, "tag");
    double amount = this->safeFloat(transaction, "amount");
    double fee = this->safeFloat(transaction, "fee");
    std::string type = this->safeString(transaction, "type");
    std::string status = this->safeString(transaction, "status");
    return {
        {"id", id},
        {"info", transaction},
        {"timestamp", timestamp},
        {"datetime", this->iso8601(timestamp)},
        {"currency", currency},
        {"address", address},
        {"tag", tag},
        {"type", type},
        {"amount", amount},
        {"status", status},
        {"fee", {
            {"cost", fee},
            {"currency", currency}
        }}
    };
}

json BTCBOX::parseDepositAddress(const json& depositAddress, const std::string& currency) {
    std::string address = this->safeString(depositAddress, "address");
    std::string tag = this->safeString(depositAddress, "tag");
    return {
        {"currency", currency},
        {"address", address},
        {"tag", tag},
        {"info", depositAddress}
    };
}

json BTCBOX::fetchCurrencies(const json& params) {
    json response = this->publicGetCurrencies(params);
    json result = json::object();
    for (const auto& entry : response.items()) {
        std::string id = entry.key();
        json currency = entry.value();
        std::string code = this->safeCurrencyCode(id);
        std::string name = this->safeString(currency, "name");
        bool active = this->safeInteger(currency, "status") == 1;
        result[code] = {
            {"id", id},
            {"code", code},
            {"name", name},
            {"active", active},
            {"fee", this->safeFloat(currency, "fee")},
            {"precision", this->safeInteger(currency, "precision")},
            {"limits", {
                {"amount", {
                    {"min", this->safeFloat(currency, "min_amount")},
                    {"max", this->safeFloat(currency, "max_amount")}
                }},
                {"withdraw", {
                    {"min", this->safeFloat(currency, "min_withdraw")},
                    {"max", this->safeFloat(currency, "max_withdraw")}
                }}
            }},
            {"info", currency}
        };
    }
    return result;
}

json BTCBOX::fetchTradingFees(const json& params) {
    this->loadMarkets();
    json response = this->publicGetFees(params);
    return {
        {"info", response},
        {"maker", this->safeFloat(response, "maker_fee")},
        {"taker", this->safeFloat(response, "taker_fee")}
    };
}

json BTCBOX::fetchTime(const json& params) {
    json response = this->publicGetTime(params);
    return this->safeTimestamp(response, "timestamp");
}

json BTCBOX::fetchStatus(const json& params) {
    json response = this->publicGetStatus(params);
    std::string status = this->safeString(response, "status");
    return {
        {"status", status == "ok" ? "ok" : "maintenance"},
        {"updated", this->safeTimestamp(response, "timestamp")},
        {"eta", this->safeString(response, "eta")},
        {"url", this->safeString(response, "url")},
        {"info", response}
    };
}

json BTCBOX::fetchTransactions(const std::string& code, int since, int limit, const json& params) {
    if (code.empty()) {
        throw ExchangeError("code is required for fetchTransactions");
    }
    this->loadMarkets();
    std::string currency = this->currency(code);
    json request = {
        {"coin", currency["id"]}
    };
    if (since != 0) {
        request["since"] = since;
    }
    if (limit != 0) {
        request["limit"] = limit;
    }
    json response = this->privatePostTransactionHistory(this->extend(request, params));
    return this->parseTransactions(response, code, since, limit);
}

} // namespace ccxt
