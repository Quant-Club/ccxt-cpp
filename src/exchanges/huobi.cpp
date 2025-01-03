#include "huobi.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <base64.h>
#include <future>

namespace ccxt {

Huobi::Huobi() {
    id = "huobi";
    name = "Huobi";
    version = "v1";
    rateLimit = 100;
    
    // Initialize API endpoints
    baseUrl = "https://api.huobi.pro";
    
    urls = {
        {"logo", "https://user-images.githubusercontent.com/1294454/76137448-22748a80-604e-11ea-8069-6e389271911d.jpg"},
        {"api", {
            {"market", "https://api.huobi.pro"},
            {"public", "https://api.huobi.pro"},
            {"private", "https://api.huobi.pro"},
            {"v2Public", "https://api.huobi.pro/v2"},
            {"v2Private", "https://api.huobi.pro/v2"}
        }},
        {"www", "https://www.huobi.com"},
        {"doc", {
            "https://huobiapi.github.io/docs/spot/v1/en/"
        }},
        {"fees", "https://www.huobi.com/about/fee/"}
    };

    timeframes = {
        {"1m", "1min"},
        {"5m", "5min"},
        {"15m", "15min"},
        {"30m", "30min"},
        {"1h", "60min"},
        {"4h", "4hour"},
        {"1d", "1day"},
        {"1w", "1week"},
        {"1M", "1mon"}
    };

    initializeApiEndpoints();
}

void Huobi::initializeApiEndpoints() {
    api = {
        {"market", {
            {"GET", {
                "common/symbols",
                "common/currencys",
                "common/timestamp",
                "market/tickers",
                "market/depth",
                "market/history/kline",
                "market/detail/merged",
                "market/history/trade"
            }}
        }},
        {"private", {
            {"GET", {
                "account/accounts",
                "account/accounts/{account-id}/balance",
                "order/orders",
                "order/orders/{order-id}",
                "order/orders/{order-id}/matchresults",
                "order/openOrders"
            }},
            {"POST", {
                "order/orders/place",
                "order/orders/{order-id}/submitcancel"
            }}
        }}
    };
}

json Huobi::fetchMarkets(const json& params) {
    json response = fetch("/v1/common/symbols", "market", "GET", params);
    json markets = json::array();
    
    for (const auto& market : response["data"]) {
        std::string baseId = market["base-currency"];
        std::string quoteId = market["quote-currency"];
        std::transform(baseId.begin(), baseId.end(), baseId.begin(), ::toupper);
        std::transform(quoteId.begin(), quoteId.end(), quoteId.begin(), ::toupper);
        
        markets.push_back({
            {"id", market["symbol"]},
            {"symbol", baseId + "/" + quoteId},
            {"base", baseId},
            {"quote", quoteId},
            {"baseId", market["base-currency"]},
            {"quoteId", market["quote-currency"]},
            {"active", market["state"] == "online"},
            {"precision", {
                {"amount", market["amount-precision"]},
                {"price", market["price-precision"]}
            }},
            {"limits", {
                {"amount", {
                    {"min", std::stod(market["min-order-amt"].get<std::string>())},
                    {"max", std::stod(market["max-order-amt"].get<std::string>())}
                }},
                {"price", {
                    {"min", std::stod(market["min-order-value"].get<std::string>())}
                }}
            }},
            {"info", market}
        });
    }
    
    return markets;
}

json Huobi::fetchTicker(const std::string& symbol, const json& params) {
    Market market = this->market(symbol);
    json response = fetch("/market/detail/merged", "market", "GET", {{"symbol", market.id}});
    json ticker = response["tick"];
    
    return {
        {"symbol", symbol},
        {"timestamp", response["ts"]},
        {"datetime", nullptr},  // TODO: Convert timestamp to ISO8601
        {"high", ticker["high"]},
        {"low", ticker["low"]},
        {"bid", ticker["bid"][0]},
        {"bidVolume", ticker["bid"][1]},
        {"ask", ticker["ask"][0]},
        {"askVolume", ticker["ask"][1]},
        {"vwap", nullptr},
        {"open", ticker["open"]},
        {"close", ticker["close"]},
        {"last", ticker["close"]},
        {"previousClose", nullptr},
        {"change", nullptr},
        {"percentage", nullptr},
        {"average", nullptr},
        {"baseVolume", ticker["amount"]},
        {"quoteVolume", ticker["vol"]},
        {"info", ticker}
    };
}

json Huobi::fetchBalance(const json& params) {
    std::string accountId = getAccountId();
    json response = fetch("/v1/account/accounts/" + accountId + "/balance", "private", "GET");
    json result = {
        {"info", response},
        {"timestamp", nullptr},
        {"datetime", nullptr}
    };
    
    for (const auto& balance : response["data"]["list"]) {
        std::string currency = balance["currency"];
        std::transform(currency.begin(), currency.end(), currency.begin(), ::toupper);
        
        if (balance["type"] == "trade") {
            double free = std::stod(balance["balance"].get<std::string>());
            if (result.contains(currency)) {
                result[currency]["free"] = free;
                result[currency]["total"] = free + result[currency]["used"].get<double>();
            } else {
                result[currency] = {
                    {"free", free},
                    {"used", 0.0},
                    {"total", free}
                };
            }
        } else if (balance["type"] == "frozen") {
            double used = std::stod(balance["balance"].get<std::string>());
            if (result.contains(currency)) {
                result[currency]["used"] = used;
                result[currency]["total"] = used + result[currency]["free"].get<double>();
            } else {
                result[currency] = {
                    {"free", 0.0},
                    {"used", used},
                    {"total", used}
                };
            }
        }
    }
    
    return result;
}

json Huobi::createOrder(const std::string& symbol, const std::string& type,
                       const std::string& side, double amount,
                       double price, const json& params) {
    std::string accountId = getAccountId();
    Market market = this->market(symbol);
    
    json request = {
        {"account-id", accountId},
        {"symbol", market.id},
        {"type", side + "-" + type},
        {"amount", std::to_string(amount)}
    };
    
    if (type == "limit") {
        if (price == 0) {
            throw InvalidOrder("For limit orders, price cannot be zero");
        }
        request["price"] = std::to_string(price);
    }
    
    return fetch("/v1/order/orders/place", "private", "POST", request);
}

std::string Huobi::sign(const std::string& path, const std::string& api,
                   const std::string& method, const json& params,
                   const std::map<std::string, std::string>& headers,
                   const json& body) {
    std::string url = baseUrl + path;
    
    std::map<std::string, std::string> query;
    for (const auto& [key, value] : params.items()) {
        query[key] = value.get<std::string>();
    }
    
    if (api == "private") {
        query["AccessKeyId"] = apiKey;
        query["SignatureMethod"] = "HmacSHA256";
        query["SignatureVersion"] = "2";
        query["Timestamp"] = getTimestamp();
        
        std::string host = "api.huobi.pro";
        std::string signature = createSignature(method, host, path, query);
        query["Signature"] = signature;
    }
    
    if (!query.empty()) {
        std::stringstream querystd::string;
        bool first = true;
        for (const auto& [key, value] : query) {
            if (!first) querystd::string << "&";
            querystd::string << key << "=" << value;
            first = false;
        }
        url += "?" + querystd::string.str();
    }
    
    return url;
}

std::string Huobi::getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto tp = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&tp), "%Y-%m-%dT%H:%M:%S");
    return ss.str();
}

std::string Huobi::createSignature(const std::string& method, const std::string& host,
                            const std::string& path,
                            const std::map<std::string, std::string>& params) {
    std::stringstream ss;
    ss << method << "\n" << host << "\n" << path << "\n";
    
    std::stringstream querystd::string;
    bool first = true;
    for (const auto& [key, value] : params) {
        if (!first) querystd::string << "&";
        querystd::string << key << "=" << value;
        first = false;
    }
    ss << querystd::string.str();
    
    std::string message = ss.str();
    
    unsigned char* hmac = nullptr;
    unsigned int hmacLen = 0;
    
    HMAC_CTX* ctx = HMAC_CTX_new();
    HMAC_Init_ex(ctx, secret.c_str(), secret.length(), EVP_sha256(), nullptr);
    HMAC_Update(ctx, (unsigned char*)message.c_str(), message.length());
    HMAC_Final(ctx, hmac, &hmacLen);
    HMAC_CTX_free(ctx);
    
    return base64_encode(hmac, hmacLen);
}

std::string Huobi::getAccountId() {
    if (accountId.empty()) {
        json response = fetch("/v1/account/accounts", "private", "GET");
        for (const auto& account : response["data"]) {
            if (account["type"] == "spot") {
                accountId = account["id"];
                break;
            }
        }
    }
    return accountId;
}

// Async implementations
AsyncPullType Huobi::fetchMarketsAsync(const json& params) {
    return std::async(std::launch::async, [this, params]() {
        return this->fetchMarkets(params);
    });
}

AsyncPullType Huobi::fetchTickerAsync(const std::string& symbol, const json& params) {
    return std::async(std::launch::async, [this, symbol, params]() {
        return this->fetchTicker(symbol, params);
    });
}

AsyncPullType Huobi::fetchTickersAsync(const std::vector<std::string>& symbols, const json& params) {
    return std::async(std::launch::async, [this, symbols, params]() {
        return this->fetchTickers(symbols, params);
    });
}

AsyncPullType Huobi::fetchOrderBookAsync(const std::string& symbol, int limit, const json& params) {
    return std::async(std::launch::async, [this, symbol, limit, params]() {
        return this->fetchOrderBook(symbol, limit, params);
    });
}

AsyncPullType Huobi::fetchTradesAsync(const std::string& symbol, int since, int limit, const json& params) {
    return std::async(std::launch::async, [this, symbol, since, limit, params]() {
        return this->fetchTrades(symbol, since, limit, params);
    });
}

AsyncPullType Huobi::fetchOHLCVAsync(const std::string& symbol, const std::string& timeframe,
                                        int since, int limit, const json& params) {
    return std::async(std::launch::async, [this, symbol, timeframe, since, limit, params]() {
        return this->fetchOHLCV(symbol, timeframe, since, limit, params);
    });
}

AsyncPullType Huobi::fetchBalanceAsync(const json& params) {
    return std::async(std::launch::async, [this, params]() {
        return this->fetchBalance(params);
    });
}

AsyncPullType Huobi::createOrderAsync(const std::string& symbol, const std::string& type,
                                        const std::string& side, double amount,
                                        double price, const json& params) {
    return std::async(std::launch::async, [this, symbol, type, side, amount, price, params]() {
        return this->createOrder(symbol, type, side, amount, price, params);
    });
}

AsyncPullType Huobi::cancelOrderAsync(const std::string& id, const std::string& symbol, const json& params) {
    return std::async(std::launch::async, [this, id, symbol, params]() {
        return this->cancelOrder(id, symbol, params);
    });
}

AsyncPullType Huobi::fetchOrderAsync(const std::string& id, const std::string& symbol, const json& params) {
    return std::async(std::launch::async, [this, id, symbol, params]() {
        return this->fetchOrder(id, symbol, params);
    });
}

AsyncPullType Huobi::fetchOrdersAsync(const std::string& symbol, int since, int limit, const json& params) {
    return std::async(std::launch::async, [this, symbol, since, limit, params]() {
        return this->fetchOrders(symbol, since, limit, params);
    });
}

AsyncPullType Huobi::fetchOpenOrdersAsync(const std::string& symbol, int since, int limit, const json& params) {
    return std::async(std::launch::async, [this, symbol, since, limit, params]() {
        return this->fetchOpenOrders(symbol, since, limit, params);
    });
}

AsyncPullType Huobi::fetchClosedOrdersAsync(const std::string& symbol, int since, int limit, const json& params) {
    return std::async(std::launch::async, [this, symbol, since, limit, params]() {
        return this->fetchClosedOrders(symbol, since, limit, params);
    });
}

} // namespace ccxt
