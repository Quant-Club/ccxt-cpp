#include "gemini.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/base64.h>

namespace ccxt {

Gemini::Gemini() {
    id = "gemini";
    name = "Gemini";
    version = "v1";
    rateLimit = 500;
    sandbox = false;

    // Initialize API endpoints
    baseUrl = sandbox ? "https://api.sandbox.gemini.com" : "https://api.gemini.com";
    
    urls = {
        {"logo", "https://user-images.githubusercontent.com/1294454/27816857-ce7be644-6096-11e7-82d6-3c257263229c.jpg"},
        {"api", {
            {"public", baseUrl},
            {"private", baseUrl},
            {"web", "https://www.gemini.com"}
        }},
        {"www", "https://www.gemini.com"},
        {"doc", {
            "https://docs.gemini.com/rest-api/",
            "https://docs.sandbox.gemini.com"
        }},
        {"test", "https://api.sandbox.gemini.com"},
        {"fees", "https://www.gemini.com/fees/"}
    };

    timeframes = {
        {"1m", "1m"},
        {"5m", "5m"},
        {"15m", "15m"},
        {"30m", "30m"},
        {"1h", "1h"},
        {"6h", "6h"},
        {"1d", "1d"}
    };

    options = {
        {"fetchMarketsMethod", "public"},
        {"fetchTickersMaxLength", "20"},
        {"createMarketBuyOrderRequiresPrice", true}
    };

    initializeApiEndpoints();
}

void Gemini::initializeApiEndpoints() {
    api = {
        {"public", {
            {"GET", {
                "v1/symbols",
                "v1/symbols/details/{symbol}",
                "v1/pubticker/{symbol}",
                "v1/trades/{symbol}",
                "v1/book/{symbol}",
                "v2/candles/{symbol}/{timeframe}",
                "v1/pricefeed",
                "v1/auction/{symbol}",
                "v1/auction/{symbol}/history"
            }}
        }},
        {"private", {
            {"POST", {
                "v1/order/new",
                "v1/order/cancel",
                "v1/order/cancel/all",
                "v1/order/status",
                "v1/orders",
                "v1/mytrades",
                "v1/tradevolume",
                "v1/balances",
                "v1/deposit/{currency}/newAddress",
                "v1/withdraw/{currency}",
                "v1/transfers",
                "v1/heartbeat",
                "v1/notionalvolume",
                "v1/clearing/new",
                "v1/clearing/status",
                "v1/clearing/cancel",
                "v1/payment-methods",
                "v1/addresses"
            }}
        }}
    };
}

json Gemini::fetchMarkets(const json& params) {
    json response = fetch("/v1/symbols/details", "public", "GET", params);
    json markets = json::array();
    
    for (const auto& market : response) {
        std::string id = market["symbol"];
        std::string baseId = market["base_currency"];
        std::string quoteId = market["quote_currency"];
        std::string base = this->commonCurrencyCode(baseId);
        std::string quote = this->commonCurrencyCode(quoteId);
        bool active = true;  // Gemini doesn't provide status
        
        markets.push_back({
            {"id", id},
            {"symbol", base + "/" + quote},
            {"base", base},
            {"quote", quote},
            {"baseId", baseId},
            {"quoteId", quoteId},
            {"active", active},
            {"type", "spot"},
            {"spot", true},
            {"future", false},
            {"option", false},
            {"contract", false},
            {"precision", {
                {"amount", market["tick_size"]},
                {"price", market["quote_increment"]}
            }},
            {"limits", {
                {"amount", {
                    {"min", market["min_order_size"]},
                    {"max", market["max_order_size"]}
                }},
                {"price", {
                    {"min", market["quote_increment"]},
                    {"max", nullptr}
                }},
                {"cost", {
                    {"min", market["min_order_value"]},
                    {"max", nullptr}
                }}
            }},
            {"info", market}
        });
    }
    
    return markets;
}

json Gemini::fetchTicker(const std::string& symbol, const json& params) {
    this->loadMarkets();
    Market market = this->market(symbol);
    
    json request = {{"symbol", market.id}};
    json response = fetch("/v1/pubticker/" + market.id, "public", "GET",
                         this->extend(request, params));
    
    return {
        {"symbol", symbol},
        {"timestamp", response["volume"]["timestamp"]},
        {"datetime", this->iso8601(response["volume"]["timestamp"])},
        {"high", nullptr},
        {"low", nullptr},
        {"bid", response["bid"]},
        {"bidVolume", nullptr},
        {"ask", response["ask"]},
        {"askVolume", nullptr},
        {"vwap", nullptr},
        {"open", nullptr},
        {"close", response["last"]},
        {"last", response["last"]},
        {"previousClose", nullptr},
        {"change", nullptr},
        {"percentage", nullptr},
        {"average", nullptr},
        {"baseVolume", response["volume"]["BTC"]},
        {"quoteVolume", response["volume"]["USD"]},
        {"info", response}
    };
}

json Gemini::fetchBalance(const json& params) {
    this->loadMarkets();
    json response = fetch("/v1/balances", "private", "POST", params);
    json result = {"info", response};
    
    for (const auto& balance : response) {
        std::string currencyId = balance["currency"];
        std::string code = this->commonCurrencyCode(currencyId);
        
        result[code] = {
            {"free", balance["available"]},
            {"used", balance["amount"] - balance["available"]},
            {"total", balance["amount"]}
        };
    }
    
    return result;
}

json Gemini::createOrder(const std::string& symbol, const std::string& type,
                        const std::string& side, double amount,
                        double price, const json& params) {
    this->loadMarkets();
    Market market = this->market(symbol);
    
    if (type == "market" && side == "buy" && 
        this->safeValue(this->options, "createMarketBuyOrderRequiresPrice", true)) {
        if (price == 0) {
            throw InvalidOrder("For market buy orders, price is required by the exchange");
        }
    }
    
    json request = {
        {"symbol", market.id},
        {"amount", this->amountToPrecision(symbol, amount)},
        {"side", side},
        {"type", type}
    };
    
    if (type == "limit" || (type == "market" && side == "buy")) {
        request["price"] = this->priceToPrecision(symbol, price);
    }
    
    json response = fetch("/v1/order/new", "private", "POST",
                         this->extend(request, params));
    return this->parseOrder(response, market);
}

std::string Gemini::sign(const std::string& path, const std::string& api,
                    const std::string& method, const json& params,
                    const std::map<std::string, std::string>& headers,
                    const json& body) {
    std::string url = this->urls["api"][api] + "/" + this->implodeParams(path, params);
    std::string query = this->omit(params, this->extractParams(path));
    
    if (api == "public") {
        if (!query.empty()) {
            url += "?" + this->urlencode(query);
        }
    } else {
        this->checkRequiredCredentials();
        
        std::string nonce = this->nonce().str();
        std::string request_path = "/" + this->version + "/" + path;
        
        body = this->extend({
            "request": request_path,
            "nonce": nonce
        }, query);
        
        std::string payload = this->json(body);
        std::string base64_payload = this->base64Encode(payload);
        std::string signature = this->hmac(base64_payload, this->base64ToBinary(this->config_.secret),
                                    "sha384", "hex");
        
        const_cast<std::map<std::string, std::string>&>(headers)["Content-Type"] = "text/plain";
        const_cast<std::map<std::string, std::string>&>(headers)["X-GEMINI-APIKEY"] = this->config_.apiKey;
        const_cast<std::map<std::string, std::string>&>(headers)["X-GEMINI-PAYLOAD"] = base64_payload;
        const_cast<std::map<std::string, std::string>&>(headers)["X-GEMINI-SIGNATURE"] = signature;
        const_cast<std::map<std::string, std::string>&>(headers)["Cache-Control"] = "no-cache";
    }
    
    return url;
}

json Gemini::parseOrder(const json& order, const Market& market) {
    std::string id = this->safeString(order, "order_id");
    std::string timestamp = this->safeInteger(order, "timestamp");
    std::string status = this->parseOrderStatus(this->safeString(order, "is_live"));
    std::string symbol = market.symbol;
    
    return {
        {"id", id},
        {"clientOrderId", this->safeString(order, "client_order_id")},
        {"timestamp", timestamp},
        {"datetime", this->iso8601(timestamp)},
        {"lastTradeTimestamp", nullptr},
        {"status", status},
        {"symbol", symbol},
        {"type", this->safeString(order, "type")},
        {"side", this->safeString(order, "side")},
        {"price", this->safeFloat(order, "price")},
        {"amount", this->safeFloat(order, "original_amount")},
        {"filled", this->safeFloat(order, "executed_amount")},
        {"remaining", this->safeFloat(order, "remaining_amount")},
        {"cost", nullptr},
        {"trades", nullptr},
        {"fee", nullptr},
        {"info", order}
    };
}

json Gemini::parseOrderStatus(const std::string& status) {
    static const std::map<std::string, std::string> statuses = {
        {"true", "open"},
        {"false", "closed"}
    };
    
    return statuses.contains(status) ? statuses.at(status) : status;
}

// Async Market Data API
AsyncPullType Gemini::fetchMarketsAsync(const json& params) {
    return std::async(std::launch::async, [this, params]() {
        return this->fetchMarkets(params);
    });
}

AsyncPullType Gemini::fetchTickerAsync(const std::string& symbol, const json& params) {
    return std::async(std::launch::async, [this, symbol, params]() {
        return this->fetchTicker(symbol, params);
    });
}

AsyncPullType Gemini::fetchOrderBookAsync(const std::string& symbol, int limit, const json& params) {
    return std::async(std::launch::async, [this, symbol, limit, params]() {
        return this->fetchOrderBook(symbol, limit, params);
    });
}

AsyncPullType Gemini::fetchTradesAsync(const std::string& symbol, int since, int limit, const json& params) {
    return std::async(std::launch::async, [this, symbol, since, limit, params]() {
        return this->fetchTrades(symbol, since, limit, params);
    });
}

AsyncPullType Gemini::fetchOHLCVAsync(const std::string& symbol, const std::string& timeframe, int since, int limit, const json& params) {
    return std::async(std::launch::async, [this, symbol, timeframe, since, limit, params]() {
        return this->fetchOHLCV(symbol, timeframe, since, limit, params);
    });
}

// Async Trading API
AsyncPullType Gemini::fetchBalanceAsync(const json& params) {
    return std::async(std::launch::async, [this, params]() {
        return this->fetchBalance(params);
    });
}

AsyncPullType Gemini::createOrderAsync(const std::string& symbol, const std::string& type, const std::string& side,
                                         double amount, double price, const json& params) {
    return std::async(std::launch::async, [this, symbol, type, side, amount, price, params]() {
        return this->createOrder(symbol, type, side, amount, price, params);
    });
}

AsyncPullType Gemini::cancelOrderAsync(const std::string& id, const std::string& symbol, const json& params) {
    return std::async(std::launch::async, [this, id, symbol, params]() {
        return this->cancelOrder(id, symbol, params);
    });
}

AsyncPullType Gemini::fetchOrderAsync(const std::string& id, const std::string& symbol, const json& params) {
    return std::async(std::launch::async, [this, id, symbol, params]() {
        return this->fetchOrder(id, symbol, params);
    });
}

AsyncPullType Gemini::fetchOrdersAsync(const std::string& symbol, int since, int limit, const json& params) {
    return std::async(std::launch::async, [this, symbol, since, limit, params]() {
        return this->fetchOrders(symbol, since, limit, params);
    });
}

AsyncPullType Gemini::fetchOpenOrdersAsync(const std::string& symbol, int since, int limit, const json& params) {
    return std::async(std::launch::async, [this, symbol, since, limit, params]() {
        return this->fetchOpenOrders(symbol, since, limit, params);
    });
}

AsyncPullType Gemini::fetchClosedOrdersAsync(const std::string& symbol, int since, int limit, const json& params) {
    return std::async(std::launch::async, [this, symbol, since, limit, params]() {
        return this->fetchClosedOrders(symbol, since, limit, params);
    });
}

// Async Gemini specific methods
AsyncPullType Gemini::fetchMyTradesAsync(const std::string& symbol, int since, int limit, const json& params) {
    return std::async(std::launch::async, [this, symbol, since, limit, params]() {
        return this->fetchMyTrades(symbol, since, limit, params);
    });
}

AsyncPullType Gemini::fetchDepositsAsync(const std::string& code, int since, int limit, const json& params) {
    return std::async(std::launch::async, [this, code, since, limit, params]() {
        return this->fetchDeposits(code, since, limit, params);
    });
}

AsyncPullType Gemini::fetchWithdrawalsAsync(const std::string& code, int since, int limit, const json& params) {
    return std::async(std::launch::async, [this, code, since, limit, params]() {
        return this->fetchWithdrawals(code, since, limit, params);
    });
}

AsyncPullType Gemini::fetchTransfersAsync(const std::string& code, int since, int limit, const json& params) {
    return std::async(std::launch::async, [this, code, since, limit, params]() {
        return this->fetchTransfers(code, since, limit, params);
    });
}

AsyncPullType Gemini::transferAsync(const std::string& code, double amount, const std::string& fromAccount,
                                      const std::string& toAccount, const json& params) {
    return std::async(std::launch::async, [this, code, amount, fromAccount, toAccount, params]() {
        return this->transfer(code, amount, fromAccount, toAccount, params);
    });
}

AsyncPullType Gemini::fetchDepositAddressAsync(const std::string& code, const json& params) {
    return std::async(std::launch::async, [this, code, params]() {
        return this->fetchDepositAddress(code, params);
    });
}

} // namespace ccxt
