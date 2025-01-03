#include "ccxt/exchanges/coinex.h"
#include <ctime>
#include <sstream>
#include <iomanip>
#include <openssl/hmac.h>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/strand.hpp>

namespace ccxt {

CoinEx::CoinEx(boost::asio::io_context& context)
    : Exchange()
    , context_(context) {
    this->id = "coinex";
    this->name = "CoinEx";
    this->countries = {"CN"};
    this->version = "v1";
    this->rateLimit = 50;
    this->has = {
        {"cancelAllOrders", true},
        {"cancelOrder", true},
        {"createOrder", true},
        {"fetchBalance", true},
        {"fetchClosedOrders", true},
        {"fetchDeposits", true},
        {"fetchMarkets", true},
        {"fetchMyTrades", true},
        {"fetchOHLCV", true},
        {"fetchOpenOrders", true},
        {"fetchOrder", true},
        {"fetchOrderBook", true},
        {"fetchTicker", true},
        {"fetchTickers", true},
        {"fetchTrades", true},
        {"fetchWithdrawals", true},
        {"withdraw", true}
    };

    this->timeframes = {
        {"1m", "1min"},
        {"3m", "3min"},
        {"5m", "5min"},
        {"15m", "15min"},
        {"30m", "30min"},
        {"1h", "1hour"},
        {"2h", "2hour"},
        {"4h", "4hour"},
        {"6h", "6hour"},
        {"12h", "12hour"},
        {"1d", "1day"},
        {"3d", "3day"},
        {"1w", "1week"}
    };

    this->urls = {
        {"logo", "https://user-images.githubusercontent.com/51840849/87182089-1e05fa00-c2ec-11ea-8da9-cc73b45abbbc.jpg"},
        {"api", {
            {"public", "https://api.coinex.com/v1"},
            {"private", "https://api.coinex.com/v1"}
        }},
        {"www", "https://www.coinex.com"},
        {"doc", {
            "https://github.com/coinexcom/coinex_exchange_api/wiki"
        }},
        {"fees", "https://www.coinex.com/fees"}
    };

    this->api = {
        {"public", {
            {"GET", {
                "market/list",
                "market/ticker",
                "market/ticker/all",
                "market/depth",
                "market/deals",
                "market/kline"
            }}
        }},
        {"private", {
            {"GET", {
                "balance/info",
                "order",
                "order/pending",
                "order/finished",
                "order/finished/{id}",
                "order/user/deals",
                "margin/account",
                "margin/config",
                "order/status",
                "order/deals",
                "order/user/deals"
            }},
            {"POST", {
                "order/limit",
                "order/market",
                "order/ioc",
                "order/pending",
                "order/cancel",
                "margin/loan",
                "margin/repay"
            }}
        }}
    };

    this->fees = {
        {"trading", {
            {"maker", 0.001},
            {"taker", 0.001}
        }},
        {"funding", {
            {"withdraw", {}},
            {"deposit", {}}
        }}
    };
}

nlohmann::json CoinEx::fetch_markets() {
    auto response = this->fetch("market/list", "public");
    auto result = nlohmann::json::array();

    for (const auto& market : response["data"]) {
        auto id = market["market"].get<std::string>();
        auto parts = this->split(id, "/");
        auto baseId = parts[0];
        auto quoteId = parts[1];
        auto base = this->safe_currency_code(baseId);
        auto quote = this->safe_currency_code(quoteId);
        auto symbol = base + "/" + quote;

        result.push_back({
            {"id", id},
            {"symbol", symbol},
            {"base", base},
            {"quote", quote},
            {"baseId", baseId},
            {"quoteId", quoteId},
            {"active", true},
            {"precision", {
                {"amount", market["stock_prec"].get<int>()},
                {"price", market["money_prec"].get<int>()}
            }},
            {"limits", {
                {"amount", {
                    {"min", this->safe_number(market, "min_amount")},
                    {"max", nullptr}
                }},
                {"price", {
                    {"min", nullptr},
                    {"max", nullptr}
                }},
                {"cost", {
                    {"min", nullptr},
                    {"max", nullptr}
                }}
            }},
            {"info", market}
        });
    }
    return result;
}

std::future<nlohmann::json> CoinEx::fetch_markets_async() {
    return std::async(std::launch::async, [this]() { return fetch_markets(); });
}

nlohmann::json CoinEx::create_order(const std::string& symbol, const std::string& type,
                                  const std::string& side, double amount, double price) {
    this->check_required_credentials();
    auto market = this->market(symbol);
    
    auto request = {
        {"market", market["id"].get<std::string>()},
        {"type", type},
        {"amount", this->amount_to_precision(symbol, amount)}
    };

    std::string method = "order/" + type;
    if (type == "limit") {
        request["price"] = this->price_to_precision(symbol, price);
    }

    auto response = this->fetch(method, "private", "POST", request);
    return this->parse_order(response["data"]);
}

std::future<nlohmann::json> CoinEx::create_order_async(const std::string& symbol,
                                             const std::string& type,
                                             const std::string& side,
                                             double amount,
                                             double price) {
    return std::async(std::launch::async, [this, symbol, type, side, amount, price]() {
        return create_order(symbol, type, side, amount, price);
    });
}

nlohmann::json CoinEx::cancel_order(const std::string& id, const std::string& symbol) {
    this->check_required_credentials();
    auto request = {
        {"id", id}
    };
    if (!symbol.empty()) {
        auto market = this->market(symbol);
        request["market"] = market["id"].get<std::string>();
    }
    return this->fetch("order/pending", "private", "DELETE", request);
}

std::future<nlohmann::json> CoinEx::cancel_order_async(const std::string& id, const std::string& symbol) {
    return std::async(std::launch::async, [this, id, symbol]() { return cancel_order(id, symbol); });
}

nlohmann::json CoinEx::fetch_balance() {
    this->check_required_credentials();
    auto response = this->fetch("balance/info", "private");
    return this->parse_balance(response);
}

std::future<nlohmann::json> CoinEx::fetch_balance_async() {
    return std::async(std::launch::async, [this]() { return fetch_balance(); });
}

std::string CoinEx::sign(const std::string& path, const std::string& api,
                        const std::string& method, const nlohmann::json& params,
                        const std::map<std::string, std::string>& headers) {
    auto url = this->urls["api"][api].get<std::string>() + "/" + this->implode_params(path, params);
    auto query = this->omit(params, this->extract_params(path));

    if (api == "private") {
        this->check_required_credentials();
        auto tonce = this->get_tonce();
        auto auth = "";
        
        if (method == "GET") {
            if (!query.empty()) {
                url += "?" + this->urlencode(query);
            }
            auth = url + "&secret_key=" + this->config_.secret;
        } else {
            if (!query.empty()) {
                auth = this->urlencode(query) + "&secret_key=" + this->config_.secret;
            } else {
                auth = "secret_key=" + this->config_.secret;
            }
        }

        auto signature = this->hash(auth, "md5", "hex").upper();
        auto new_headers = headers;
        new_headers["authorization"] = signature;
        new_headers["AccessId"] = this->config_.apiKey;
        new_headers["tonce"] = tonce;
    } else {
        if (!query.empty()) {
            url += "?" + this->urlencode(query);
        }
    }

    return url;
}

nlohmann::json CoinEx::parse_ticker(const nlohmann::json& ticker, const nlohmann::json& market) {
    auto timestamp = this->safe_timestamp(ticker, "date");
    auto symbol = market ? market["symbol"].get<std::string>() : "";
    
    return {
        {"symbol", symbol},
        {"timestamp", timestamp},
        {"datetime", this->iso8601(timestamp)},
        {"high", this->safe_number(ticker, "high")},
        {"low", this->safe_number(ticker, "low")},
        {"bid", this->safe_number(ticker, "buy")},
        {"ask", this->safe_number(ticker, "sell")},
        {"last", this->safe_number(ticker, "last")},
        {"close", this->safe_number(ticker, "last")},
        {"baseVolume", this->safe_number(ticker, "vol")},
        {"quoteVolume", nullptr},
        {"info", ticker}
    };
}

nlohmann::json CoinEx::parse_balance(const nlohmann::json& response) {
    auto balances = response["data"];
    auto result = {
        {"info", response},
        {"timestamp", nullptr},
        {"datetime", nullptr}
    };

    for (const auto& balance : balances.items()) {
        auto currencyId = balance.key();
        auto code = this->safe_currency_code(currencyId);
        auto account = this->account();
        account["free"] = this->safe_string(balance.value(), "available");
        account["used"] = this->safe_string(balance.value(), "frozen");
        result[code] = account;
    }

    return result;
}

std::string CoinEx::get_market_id(const std::string& symbol) {
    auto market = this->market(symbol);
    return market["id"].get<std::string>();
}

std::string CoinEx::get_currency_id(const std::string& code) {
    if (this->currencies.find(code) != this->currencies.end()) {
        return this->currencies[code]["id"].get<std::string>();
    }
    return code;
}

std::string CoinEx::get_tonce() {
    return std::to_string(this->milliseconds());
}

std::future<nlohmann::json> CoinEx::fetch_ticker_async(const std::string& symbol) {
    return std::async(std::launch::async, [this, symbol]() { return fetch_ticker(symbol); });
}

std::future<nlohmann::json> CoinEx::fetch_order_book_async(const std::string& symbol, int limit) {
    return std::async(std::launch::async, [this, symbol, limit]() { return fetch_order_book(symbol, limit); });
}

std::future<nlohmann::json> CoinEx::fetch_trades_async(const std::string& symbol, long since, int limit) {
    return std::async(std::launch::async, [this, symbol, since, limit]() {
        return fetch_trades(symbol, since, limit);
    });
}

std::future<nlohmann::json> CoinEx::fetch_ohlcv_async(const std::string& symbol,
                                                 const std::string& timeframe,
                                                 long since,
                                                 int limit) {
    return std::async(std::launch::async, [this, symbol, timeframe, since, limit]() {
        return fetch_ohlcv(symbol, timeframe, since, limit);
    });
}

std::future<nlohmann::json> CoinEx::cancel_all_orders_async(const std::string& symbol) {
    return std::async(std::launch::async, [this, symbol]() { return cancel_all_orders(symbol); });
}

std::future<nlohmann::json> CoinEx::fetch_order_async(const std::string& id, const std::string& symbol) {
    return std::async(std::launch::async, [this, id, symbol]() { return fetch_order(id, symbol); });
}

std::future<nlohmann::json> CoinEx::fetch_orders_async(const std::string& symbol, long since, int limit) {
    return std::async(std::launch::async, [this, symbol, since, limit]() {
        return fetch_orders(symbol, since, limit);
    });
}

std::future<nlohmann::json> CoinEx::fetch_open_orders_async(const std::string& symbol, long since, int limit) {
    return std::async(std::launch::async, [this, symbol, since, limit]() {
        return fetch_open_orders(symbol, since, limit);
    });
}

std::future<nlohmann::json> CoinEx::fetch_closed_orders_async(const std::string& symbol, long since, int limit) {
    return std::async(std::launch::async, [this, symbol, since, limit]() {
        return fetch_closed_orders(symbol, since, limit);
    });
}

std::future<nlohmann::json> CoinEx::fetch_my_trades_async(const std::string& symbol, long since, int limit) {
    return std::async(std::launch::async, [this, symbol, since, limit]() {
        return fetch_my_trades(symbol, since, limit);
    });
}

std::future<nlohmann::json> CoinEx::fetch_deposit_address_async(const std::string& code) {
    return std::async(std::launch::async, [this, code]() { return fetch_deposit_address(code); });
}

std::future<nlohmann::json> CoinEx::fetch_deposits_async(const std::string& code, long since, int limit) {
    return std::async(std::launch::async, [this, code, since, limit]() {
        return fetch_deposits(code, since, limit);
    });
}

std::future<nlohmann::json> CoinEx::fetch_withdrawals_async(const std::string& code, long since, int limit) {
    return std::async(std::launch::async, [this, code, since, limit]() {
        return fetch_withdrawals(code, since, limit);
    });
}

std::future<nlohmann::json> CoinEx::withdraw_async(const std::string& code,
                                              double amount,
                                              const std::string& address,
                                              const std::string& tag,
                                              const nlohmann::json& params) {
    return std::async(std::launch::async, [this, code, amount, address, tag, params]() {
        return withdraw(code, amount, address, tag, params);
    });
}

std::future<nlohmann::json> CoinEx::fetch_margin_balance_async() {
    return std::async(std::launch::async, [this]() { return fetch_margin_balance(); });
}

std::future<nlohmann::json> CoinEx::create_margin_order_async(const std::string& symbol,
                                                 const std::string& type,
                                                 const std::string& side,
                                                 double amount,
                                                 double price) {
    return std::async(std::launch::async, [this, symbol, type, side, amount, price]() {
        return create_margin_order(symbol, type, side, amount, price);
    });
}

std::future<nlohmann::json> CoinEx::borrow_margin_async(const std::string& code,
                                             double amount,
                                             const std::string& symbol) {
    return std::async(std::launch::async, [this, code, amount, symbol]() {
        return borrow_margin(code, amount, symbol);
    });
}

std::future<nlohmann::json> CoinEx::repay_margin_async(const std::string& code,
                                             double amount,
                                             const std::string& symbol) {
    return std::async(std::launch::async, [this, code, amount, symbol]() {
        return repay_margin(code, amount, symbol);
    });
}

std::future<nlohmann::json> CoinEx::fetch_async(const std::string& url,
                                             const std::string& method,
                                             const std::map<std::string, std::string>& headers,
                                             const std::string& body) {
    return std::async(std::launch::async, [this, url, method, headers, body]() {
        return fetch(url, method, headers, body);
    });
}

} // namespace ccxt
