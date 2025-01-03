#include "ccxt/exchanges/coinbaseexchange.h"
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace ccxt {

CoinbaseExchange::CoinbaseExchange(const Params& params) : Exchange(params) {
    this->id = "coinbaseexchange";
    this->name = "Coinbase Exchange";
    this->countries = {"US"};
    this->rateLimit = 100;
    this->version = "v3";
    this->userAgent = "ccxt/1.0";
    this->pro = true;
    
    this->has = {
        {"fetchMarkets", true},
        {"fetchCurrencies", true},
        {"fetchOrderBook", true},
        {"fetchTicker", true},
        {"fetchTickers", true},
        {"fetchTrades", true},
        {"fetchOHLCV", true},
        {"createOrder", true},
        {"cancelOrder", true},
        {"fetchOrder", true},
        {"fetchOrders", true},
        {"fetchOpenOrders", true},
        {"fetchClosedOrders", true},
        {"fetchMyTrades", true},
        {"fetchBalance", true},
        {"fetchLedger", true},
        {"fetchDeposits", true},
        {"fetchWithdrawals", true},
        {"withdraw", true},
        {"deposit", true}
    };
    
    this->urls = {
        {"logo", "https://user-images.githubusercontent.com/1294454/41764625-63b7ffde-760a-11e8-996d-a6328fa9347a.jpg"},
        {"api", {
            {"rest", "https://api.exchange.coinbase.com"}
        }},
        {"www", "https://pro.coinbase.com/"},
        {"doc", {
            "https://docs.cloud.coinbase.com/exchange/reference",
            "https://docs.cloud.coinbase.com/exchange/docs"
        }},
        {"fees", "https://pro.coinbase.com/fees"}
    };
    
    this->requiredCredentials = {
        {"apiKey", true},
        {"secret", true},
        {"password", true}
    };
    
    this->api = {
        {"public", {
            {"get", {
                {"products"},
                {"products/{id}/book"},
                {"products/{id}/ticker"},
                {"products/{id}/trades"},
                {"products/stats"},
                {"time"}
            }}
        }},
        {"private", {
            {"get", {
                {"accounts"},
                {"accounts/{id}"},
                {"accounts/{id}/ledger"},
                {"accounts/{id}/holds"},
                {"orders"},
                {"orders/{id}"},
                {"fills"},
                {"deposits"},
                {"withdrawals"},
                {"payment-methods"}
            }},
            {"post", {
                {"orders"},
                {"deposits/payment-method"},
                {"withdrawals/payment-method"}
            }},
            {"delete", {
                {"orders"},
                {"orders/{id}"}
            }}
        }}
    };
}

OrderBook CoinbaseExchange::fetchOrderBook(const std::string& symbol, int limit, const Params& params) {
    this->loadMarkets();
    auto market = this->market(symbol);
    auto request = {
        {"id", market["id"].get<std::string>()},
        {"level", limit ? std::to_string(limit) : "2"}  // 1 = best bid/ask, 2 = top 50, 3 = full order book
    };
    
    auto response = this->publicGetProductsIdBook(this->extend(request, params));
    
    return this->parseOrderBook(response, symbol, 
                              this->safeTimestamp(response, "timestamp"),
                              "bids", "asks", "price", "size");
}

std::vector<Trade> CoinbaseExchange::fetchTrades(const std::string& symbol, int since, int limit, const Params& params) {
    this->loadMarkets();
    auto market = this->market(symbol);
    auto request = {
        {"id", market["id"].get<std::string>()}
    };
    
    if (limit) {
        request["limit"] = std::to_string(limit);
    }
    
    auto response = this->publicGetProductsIdTrades(this->extend(request, params));
    return this->parseTrades(response, market, since, limit);
}

Ticker CoinbaseExchange::fetchTicker(const std::string& symbol, const Params& params) {
    this->loadMarkets();
    auto market = this->market(symbol);
    auto request = {
        {"id", market["id"].get<std::string>()}
    };
    
    auto response = this->publicGetProductsIdTicker(this->extend(request, params));
    auto timestamp = this->safeTimestamp(response, "time");
    auto last = this->safeFloat(response, "price");
    
    return {
        {"symbol", symbol},
        {"timestamp", timestamp},
        {"datetime", this->iso8601(timestamp)},
        {"high", nullptr},
        {"low", nullptr},
        {"bid", this->safeFloat(response, "bid")},
        {"bidVolume", nullptr},
        {"ask", this->safeFloat(response, "ask")},
        {"askVolume", nullptr},
        {"vwap", nullptr},
        {"open", nullptr},
        {"close", last},
        {"last", last},
        {"previousClose", nullptr},
        {"change", nullptr},
        {"percentage", nullptr},
        {"average", nullptr},
        {"baseVolume", this->safeFloat(response, "volume")},
        {"quoteVolume", nullptr},
        {"info", response}
    };
}

std::map<std::string, Ticker> CoinbaseExchange::fetchTickers(const std::vector<std::string>& symbols, const Params& params) {
    this->loadMarkets();
    auto response = this->publicGetProductsStats(params);
    std::map<std::string, Ticker> result;
    
    for (const auto& entry : response.items()) {
        auto marketId = entry.key();
        if (this->markets_by_id.contains(marketId)) {
            auto market = this->markets_by_id[marketId];
            auto symbol = market["symbol"].get<std::string>();
            if (symbols.empty() || std::find(symbols.begin(), symbols.end(), symbol) != symbols.end()) {
                result[symbol] = this->parseTicker(entry.value(), market);
            }
        }
    }
    
    return result;
}

Trade CoinbaseExchange::parseTrade(const json& trade, const Market* market) {
    auto timestamp = this->safeTimestamp(trade, "time");
    auto price = this->safeFloat(trade, "price");
    auto amount = this->safeFloat(trade, "size");
    auto symbol = market ? market->symbol : "";
    auto side = this->safeString(trade, "side");
    auto tradeId = this->safeString(trade, "trade_id");
    
    return {
        {"id", tradeId},
        {"info", trade},
        {"timestamp", timestamp},
        {"datetime", this->iso8601(timestamp)},
        {"symbol", symbol},
        {"type", nullptr},
        {"side", side},
        {"order", nullptr},
        {"takerOrMaker", nullptr},
        {"price", price},
        {"amount", amount},
        {"cost", price * amount},
        {"fee", nullptr}
    };
}

std::string CoinbaseExchange::createSignature(const std::string& timestamp, const std::string& method,
                                            const std::string& requestPath, const std::string& body) const {
    std::string message = timestamp + method + requestPath + body;
    return this->hmac(message, this->config_.secret, "sha256", "base64");
}

std::string CoinbaseExchange::getTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    return std::to_string(seconds);
}

// Market Data Methods - Asynchronous
boost::future<OrderBook> CoinbaseExchange::fetchOrderBookAsync(const std::string& symbol, int limit, const Params& params) const {
    return boost::async(boost::launch::async, [this, symbol, limit, params]() {
        return this->fetchOrderBook(symbol, limit, params);
    });
}

boost::future<std::vector<Trade>> CoinbaseExchange::fetchTradesAsync(const std::string& symbol, int since, int limit, const Params& params) const {
    return boost::async(boost::launch::async, [this, symbol, since, limit, params]() {
        return this->fetchTrades(symbol, since, limit, params);
    });
}

boost::future<Ticker> CoinbaseExchange::fetchTickerAsync(const std::string& symbol, const Params& params) const {
    return boost::async(boost::launch::async, [this, symbol, params]() {
        return this->fetchTicker(symbol, params);
    });
}

boost::future<std::map<std::string, Ticker>> CoinbaseExchange::fetchTickersAsync(const std::vector<std::string>& symbols, const Params& params) const {
    return boost::async(boost::launch::async, [this, symbols, params]() {
        return this->fetchTickers(symbols, params);
    });
}

// Trading Methods - Asynchronous
boost::future<Order> CoinbaseExchange::createOrderAsync(const std::string& symbol, const std::string& type, const std::string& side,
                                                    double amount, double price, const Params& params) {
    return boost::async(boost::launch::async, [this, symbol, type, side, amount, price, params]() {
        return this->createOrder(symbol, type, side, amount, price, params);
    });
}

boost::future<Order> CoinbaseExchange::cancelOrderAsync(const std::string& id, const std::string& symbol, const Params& params) {
    return boost::async(boost::launch::async, [this, id, symbol, params]() {
        return this->cancelOrder(id, symbol, params);
    });
}

boost::future<std::vector<Order>> CoinbaseExchange::fetchOrdersAsync(const std::string& symbol, int since, int limit, const Params& params) const {
    return boost::async(boost::launch::async, [this, symbol, since, limit, params]() {
        return this->fetchOrders(symbol, since, limit, params);
    });
}

boost::future<std::vector<Order>> CoinbaseExchange::fetchOpenOrdersAsync(const std::string& symbol, int since, int limit, const Params& params) const {
    return boost::async(boost::launch::async, [this, symbol, since, limit, params]() {
        return this->fetchOpenOrders(symbol, since, limit, params);
    });
}

boost::future<std::vector<Order>> CoinbaseExchange::fetchClosedOrdersAsync(const std::string& symbol, int since, int limit, const Params& params) const {
    return boost::async(boost::launch::async, [this, symbol, since, limit, params]() {
        return this->fetchClosedOrders(symbol, since, limit, params);
    });
}

boost::future<Order> CoinbaseExchange::fetchOrderAsync(const std::string& id, const std::string& symbol, const Params& params) const {
    return boost::async(boost::launch::async, [this, id, symbol, params]() {
        return this->fetchOrder(id, symbol, params);
    });
}

// Account Methods - Asynchronous
boost::future<Balance> CoinbaseExchange::fetchBalanceAsync(const Params& params) const {
    return boost::async(boost::launch::async, [this, params]() {
        return this->fetchBalance(params);
    });
}

boost::future<std::vector<Account>> CoinbaseExchange::fetchAccountsAsync(const Params& params) const {
    return boost::async(boost::launch::async, [this, params]() {
        return this->fetchAccounts(params);
    });
}

boost::future<TradingFees> CoinbaseExchange::fetchTradingFeesAsync(const Params& params) const {
    return boost::async(boost::launch::async, [this, params]() {
        return this->fetchTradingFees(params);
    });
}

// Funding Methods - Asynchronous
boost::future<std::vector<Transaction>> CoinbaseExchange::fetchDepositsAsync(const std::string& code, int since, int limit, const Params& params) const {
    return boost::async(boost::launch::async, [this, code, since, limit, params]() {
        return this->fetchDeposits(code, since, limit, params);
    });
}

boost::future<std::vector<Transaction>> CoinbaseExchange::fetchWithdrawalsAsync(const std::string& code, int since, int limit, const Params& params) const {
    return boost::async(boost::launch::async, [this, code, since, limit, params]() {
        return this->fetchWithdrawals(code, since, limit, params);
    });
}

boost::future<DepositAddress> CoinbaseExchange::fetchDepositAddressAsync(const std::string& code, const Params& params) const {
    return boost::async(boost::launch::async, [this, code, params]() {
        return this->fetchDepositAddress(code, params);
    });
}

boost::future<std::vector<LedgerEntry>> CoinbaseExchange::fetchLedgerAsync(const std::string& code, int since, int limit, const Params& params) const {
    return boost::async(boost::launch::async, [this, code, since, limit, params]() {
        return this->fetchLedger(code, since, limit, params);
    });
}

} // namespace ccxt
