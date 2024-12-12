#include "okx.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <base64.h>

namespace ccxt {

OKX::OKX() {
    id = "okx";
    name = "OKX";
    version = "v5";
    rateLimit = 100;
    
    // Initialize API endpoints
    baseUrl = "https://www.okx.com";
    
    urls = {
        {"logo", "https://user-images.githubusercontent.com/1294454/152485636-38b19e4a-bece-4dec-979a-5982859ffc04.jpg"},
        {"api", {
            {"public", "https://www.okx.com"},
            {"private", "https://www.okx.com"}
        }},
        {"www", "https://www.okx.com"},
        {"doc", {
            "https://www.okx.com/docs-v5/en/"
        }},
        {"fees", "https://www.okx.com/pages/products/fees.html"}
    };

    initializeApiEndpoints();
}

void OKX::initializeApiEndpoints() {
    api = {
        {"public", {
            {"GET", {
                "/api/v5/public/instruments",
                "/api/v5/market/ticker",
                "/api/v5/market/tickers",
                "/api/v5/market/books",
                "/api/v5/market/trades",
                "/api/v5/market/candles",
                "/api/v5/public/funding-rate",
                "/api/v5/public/funding-rate-history"
            }}
        }},
        {"private", {
            {"GET", {
                "/api/v5/account/balance",
                "/api/v5/trade/order",
                "/api/v5/trade/orders-pending",
                "/api/v5/trade/orders-history",
                "/api/v5/trade/orders-history-archive",
                "/api/v5/trade/fills",
                "/api/v5/account/bills",
                "/api/v5/asset/deposit-history",
                "/api/v5/asset/withdrawal-history",
                "/api/v5/asset/deposit-address"
            }},
            {"POST", {
                "/api/v5/trade/order",
                "/api/v5/trade/batch-orders",
                "/api/v5/account/set-leverage",
                "/api/v5/account/set-position-mode"
            }},
            {"DELETE", {
                "/api/v5/trade/order",
                "/api/v5/trade/batch-orders"
            }}
        }}
    };
}

json OKX::fetchMarkets(const json& params) {
    json response = fetch("/api/v5/public/instruments", "public", "GET", 
                         {{"instType", "SPOT"}});
    json markets = json::array();
    
    for (const auto& market : response["data"]) {
        markets.push_back({
            {"id", market["instId"]},
            {"symbol", market["baseCcy"] + "/" + market["quoteCcy"]},
            {"base", market["baseCcy"]},
            {"quote", market["quoteCcy"]},
            {"baseId", market["baseCcy"]},
            {"quoteId", market["quoteCcy"]},
            {"active", market["state"] == "live"},
            {"precision", {
                {"amount", std::stoi(market["lotSz"].get<String>())},
                {"price", std::stoi(market["tickSz"].get<String>())}
            }},
            {"limits", {
                {"amount", {
                    {"min", std::stod(market["minSz"].get<String>())},
                    {"max", std::stod(market["maxSz"].get<String>())}
                }},
                {"price", {
                    {"min", std::stod(market["tickSz"].get<String>())}
                }}
            }},
            {"info", market}
        });
    }
    
    return markets;
}

json OKX::fetchBalance(const json& params) {
    json response = fetch("/api/v5/account/balance", "private", "GET", params);
    json result = {
        {"info", response},
        {"timestamp", nullptr},
        {"datetime", nullptr}
    };
    
    for (const auto& balance : response["data"][0]["details"]) {
        String currency = balance["ccy"].get<String>();
        double free = std::stod(balance["availBal"].get<String>());
        double used = std::stod(balance["frozenBal"].get<String>());
        double total = free + used;
        
        if (total > 0) {
            result[currency] = {
                {"free", free},
                {"used", used},
                {"total", total}
            };
        }
    }
    
    return result;
}

json OKX::createOrder(const String& symbol, const String& type,
                     const String& side, double amount,
                     double price, const json& params) {
    Market market = this->market(symbol);
    
    json order = {
        {"instId", market.id},
        {"tdMode", "cash"},
        {"side", side.substr(0, 1) + side.substr(1)},  // Capitalize first letter
        {"ordType", type},
        {"sz", std::to_string(amount)}
    };
    
    if (type == "limit") {
        if (price == 0) {
            throw InvalidOrder("For limit orders, price cannot be zero");
        }
        order["px"] = std::to_string(price);
    }
    
    return fetch("/api/v5/trade/order", "private", "POST", order);
}

String OKX::sign(const String& path, const String& api,
                 const String& method, const json& params,
                 const std::map<String, String>& headers,
                 const json& body) {
    String url = baseUrl + path;
    
    if (api == "private") {
        auto authHeaders = getAuthHeaders(method, path, body.dump());
        for (const auto& [key, value] : authHeaders) {
            const_cast<std::map<String, String>&>(headers)[key] = value;
        }
    }
    
    if (!params.empty()) {
        std::stringstream queryString;
        bool first = true;
        for (const auto& [key, value] : params.items()) {
            if (!first) queryString << "&";
            queryString << key << "=" << value.get<String>();
            first = false;
        }
        url += "?" + queryString.str();
    }
    
    return url;
}

String OKX::getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return std::to_string(ms.count());
}

String OKX::createSignature(const String& timestamp, const String& method,
                          const String& requestPath, const String& body) {
    String message = timestamp + method + requestPath + body;
    
    unsigned char* digest = nullptr;
    unsigned int digestLen = 0;
    
    HMAC_CTX* ctx = HMAC_CTX_new();
    HMAC_Init_ex(ctx, secret.c_str(), secret.length(), EVP_sha256(), nullptr);
    HMAC_Update(ctx, (unsigned char*)message.c_str(), message.length());
    HMAC_Final(ctx, digest, &digestLen);
    HMAC_CTX_free(ctx);
    
    return base64_encode(digest, digestLen);
}

std::map<String, String> OKX::getAuthHeaders(const String& method,
                                           const String& requestPath,
                                           const String& body) {
    String timestamp = getTimestamp();
    String signature = createSignature(timestamp, method, requestPath, body);
    
    return {
        {"OK-ACCESS-KEY", apiKey},
        {"OK-ACCESS-SIGN", signature},
        {"OK-ACCESS-TIMESTAMP", timestamp},
        {"OK-ACCESS-PASSPHRASE", password}
    };
}

json OKX::fetchMyTrades(const String& symbol, int since, int limit, const json& params) {
    if (symbol.empty()) {
        throw ArgumentsRequired("fetchMyTrades requires a symbol argument");
    }
    this->loadMarkets();
    Market market = this->market(symbol);
    json request = {
        {"instId", market.id}
    };
    if (limit > 0) {
        request["limit"] = limit;
    }
    if (since > 0) {
        request["after"] = since;
    }
    json response = fetch("/api/v5/trade/fills", "private", "GET", request);
    return this->parseTrades(response["data"], market, since, limit);
}

json OKX::fetchLedger(const String& code, int since, int limit, const json& params) {
    this->loadMarkets();
    json request = {};
    String currency;
    if (!code.empty()) {
        currency = this->currency(code).id;
        request["ccy"] = currency;
    }
    if (since > 0) {
        request["after"] = since;
    }
    if (limit > 0) {
        request["limit"] = limit;
    }
    json response = fetch("/api/v5/account/bills", "private", "GET", request);
    return this->parseLedger(response["data"], currency, since, limit);
}

json OKX::fetchDeposits(const String& code, int since, int limit, const json& params) {
    this->loadMarkets();
    json request = {};
    String currency;
    if (!code.empty()) {
        currency = this->currency(code).id;
        request["ccy"] = currency;
    }
    if (since > 0) {
        request["after"] = since;
    }
    if (limit > 0) {
        request["limit"] = limit;
    }
    json response = fetch("/api/v5/asset/deposit-history", "private", "GET", request);
    return this->parseTransactions(response["data"], currency, since, limit, "deposit");
}

json OKX::fetchWithdrawals(const String& code, int since, int limit, const json& params) {
    this->loadMarkets();
    json request = {};
    String currency;
    if (!code.empty()) {
        currency = this->currency(code).id;
        request["ccy"] = currency;
    }
    if (since > 0) {
        request["after"] = since;
    }
    if (limit > 0) {
        request["limit"] = limit;
    }
    json response = fetch("/api/v5/asset/withdrawal-history", "private", "GET", request);
    return this->parseTransactions(response["data"], currency, since, limit, "withdrawal");
}

json OKX::fetchDepositAddress(const String& code, const json& params) {
    this->loadMarkets();
    String currency = this->currency(code).id;
    json request = {
        {"ccy", currency}
    };
    json response = fetch("/api/v5/asset/deposit-address", "private", "GET", request);
    return this->parseDepositAddress(response["data"][0]);
}

json OKX::fetchFundingRate(const String& symbol, const json& params) {
    this->loadMarkets();
    Market market = this->market(symbol);
    json request = {
        {"instId", market.id}
    };
    json response = fetch("/api/v5/public/funding-rate", "public", "GET", request);
    return this->parseFundingRate(response["data"][0], market);
}

json OKX::fetchFundingRateHistory(const String& symbol, int since, int limit, const json& params) {
    this->loadMarkets();
    Market market = this->market(symbol);
    json request = {
        {"instId", market.id}
    };
    if (limit > 0) {
        request["limit"] = limit;
    }
    if (since > 0) {
        request["after"] = since;
    }
    json response = fetch("/api/v5/public/funding-rate-history", "public", "GET", request);
    return this->parseFundingRateHistory(response["data"], market, since, limit);
}

json OKX::setLeverage(int leverage, const String& symbol, const String& type, const json& params) {
    this->loadMarkets();
    Market market = this->market(symbol);
    json request = {
        {"instId", market.id},
        {"lever", leverage}
    };
    if (!type.empty()) {
        request["mgnMode"] = type;
    }
    return fetch("/api/v5/account/set-leverage", "private", "POST", request);
}

json OKX::setMarginMode(const String& marginMode, const String& symbol, const json& params) {
    this->loadMarkets();
    Market market = this->market(symbol);
    json request = {
        {"instId", market.id},
        {"mgnMode", marginMode}
    };
    return fetch("/api/v5/account/set-position-mode", "private", "POST", request);
}

json OKX::fetchMarketsAsync(const json& params) {
    return std::async(std::launch::async, [this, params]() {
        return this->fetchMarkets(params);
    });
}

json OKX::fetchTickerAsync(const String& symbol, const json& params) {
    return std::async(std::launch::async, [this, symbol, params]() {
        return this->fetchTicker(symbol, params);
    });
}

json OKX::fetchTickersAsync(const std::vector<String>& symbols, const json& params) {
    return std::async(std::launch::async, [this, symbols, params]() {
        return this->fetchTickers(symbols, params);
    });
}

json OKX::fetchOrderBookAsync(const String& symbol, int limit, const json& params) {
    return std::async(std::launch::async, [this, symbol, limit, params]() {
        return this->fetchOrderBook(symbol, limit, params);
    });
}

json OKX::fetchTradesAsync(const String& symbol, int since, int limit, const json& params) {
    return std::async(std::launch::async, [this, symbol, since, limit, params]() {
        return this->fetchTrades(symbol, since, limit, params);
    });
}

json OKX::fetchOHLCVAsync(const String& symbol, const String& timeframe,
                         int since, int limit, const json& params) {
    return std::async(std::launch::async, [this, symbol, timeframe, since, limit, params]() {
        return this->fetchOHLCV(symbol, timeframe, since, limit, params);
    });
}

json OKX::fetchBalanceAsync(const json& params) {
    return std::async(std::launch::async, [this, params]() {
        return this->fetchBalance(params);
    });
}

json OKX::createOrderAsync(const String& symbol, const String& type,
                         const String& side, double amount,
                         double price, const json& params) {
    return std::async(std::launch::async, [this, symbol, type, side, amount, price, params]() {
        return this->createOrder(symbol, type, side, amount, price, params);
    });
}

json OKX::cancelOrderAsync(const String& id, const String& symbol, const json& params) {
    return std::async(std::launch::async, [this, id, symbol, params]() {
        return this->cancelOrder(id, symbol, params);
    });
}

json OKX::fetchOrderAsync(const String& id, const String& symbol, const json& params) {
    return std::async(std::launch::async, [this, id, symbol, params]() {
        return this->fetchOrder(id, symbol, params);
    });
}

json OKX::fetchOrdersAsync(const String& symbol, int since, int limit, const json& params) {
    return std::async(std::launch::async, [this, symbol, since, limit, params]() {
        return this->fetchOrders(symbol, since, limit, params);
    });
}

json OKX::fetchOpenOrdersAsync(const String& symbol, int since, int limit, const json& params) {
    return std::async(std::launch::async, [this, symbol, since, limit, params]() {
        return this->fetchOpenOrders(symbol, since, limit, params);
    });
}

json OKX::fetchClosedOrdersAsync(const String& symbol, int since, int limit, const json& params) {
    return std::async(std::launch::async, [this, symbol, since, limit, params]() {
        return this->fetchClosedOrders(symbol, since, limit, params);
    });
}

json OKX::fetchMyTradesAsync(const String& symbol, int since, int limit, const json& params) {
    return std::async(std::launch::async, [this, symbol, since, limit, params]() {
        return this->fetchMyTrades(symbol, since, limit, params);
    });
}

json OKX::fetchLedgerAsync(const String& code, int since, int limit, const json& params) {
    return std::async(std::launch::async, [this, code, since, limit, params]() {
        return this->fetchLedger(code, since, limit, params);
    });
}

json OKX::fetchDepositsAsync(const String& code, int since, int limit, const json& params) {
    return std::async(std::launch::async, [this, code, since, limit, params]() {
        return this->fetchDeposits(code, since, limit, params);
    });
}

json OKX::fetchWithdrawalsAsync(const String& code, int since, int limit, const json& params) {
    return std::async(std::launch::async, [this, code, since, limit, params]() {
        return this->fetchWithdrawals(code, since, limit, params);
    });
}

json OKX::fetchDepositAddressAsync(const String& code, const json& params) {
    return std::async(std::launch::async, [this, code, params]() {
        return this->fetchDepositAddress(code, params);
    });
}

json OKX::fetchFundingRateAsync(const String& symbol, const json& params) {
    return std::async(std::launch::async, [this, symbol, params]() {
        return this->fetchFundingRate(symbol, params);
    });
}

json OKX::fetchFundingRateHistoryAsync(const String& symbol, int since, int limit, const json& params) {
    return std::async(std::launch::async, [this, symbol, since, limit, params]() {
        return this->fetchFundingRateHistory(symbol, since, limit, params);
    });
}

json OKX::setLeverageAsync(int leverage, const String& symbol, const String& type, const json& params) {
    return std::async(std::launch::async, [this, leverage, symbol, type, params]() {
        return this->setLeverage(leverage, symbol, type, params);
    });
}

json OKX::setMarginModeAsync(const String& marginMode, const String& symbol, const json& params) {
    return std::async(std::launch::async, [this, marginMode, symbol, params]() {
        return this->setMarginMode(marginMode, symbol, params);
    });
}

} // namespace ccxt
