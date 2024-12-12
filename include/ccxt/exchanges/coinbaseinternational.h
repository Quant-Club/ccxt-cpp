#ifndef CCXT_COINBASEINTERNATIONAL_H
#define CCXT_COINBASEINTERNATIONAL_H

#include "ccxt/base/exchange.h"

namespace ccxt {

class coinbaseinternational : public exchange {
public:
    explicit coinbaseinternational(const Config& config = Config());
    ~coinbaseinternational() override = default;

    // Market Data
    json fetch_markets(const json& params = json()) override;
    json fetch_currencies(const json& params = json()) override;
    json fetch_ticker(const std::string& symbol, const json& params = json()) override;
    json fetch_order_book(const std::string& symbol, int limit = 0, const json& params = json()) override;
    json fetch_trades(const std::string& symbol, int since = 0, int limit = 0, const json& params = json()) override;
    json fetch_ohlcv(const std::string& symbol, const std::string& timeframe = "1m", int since = 0, int limit = 0, const json& params = json()) override;
    json fetch_trading_fees(const std::string& symbol = "", const json& params = json()) override;

    // Async Market Data Methods
    boost::future<json> fetch_markets_async(const json& params = json());
    boost::future<json> fetch_currencies_async(const json& params = json());
    boost::future<json> fetch_ticker_async(const std::string& symbol, const json& params = json());
    boost::future<json> fetch_order_book_async(const std::string& symbol, int limit = 0, const json& params = json());
    boost::future<json> fetch_trades_async(const std::string& symbol, int since = 0, int limit = 0, const json& params = json());
    boost::future<json> fetch_ohlcv_async(const std::string& symbol, const std::string& timeframe = "1m", int since = 0, int limit = 0, const json& params = json());
    boost::future<json> fetch_trading_fees_async(const std::string& symbol = "", const json& params = json());

    // Trading
    json create_order(const std::string& symbol, const std::string& type, const std::string& side,
                     double amount, double price = 0, const json& params = json()) override;
    json cancel_order(const std::string& id, const std::string& symbol = "", const json& params = json()) override;
    json cancel_all_orders(const std::string& symbol = "", const json& params = json()) override;
    json edit_order(const std::string& id, const std::string& symbol, const std::string& type, const std::string& side,
                   double amount, double price = 0, const json& params = json()) override;

    // Async Trading Methods
    boost::future<json> create_order_async(const std::string& symbol, const std::string& type, const std::string& side,
                                         double amount, double price = 0, const json& params = json());
    boost::future<json> cancel_order_async(const std::string& id, const std::string& symbol = "", const json& params = json());
    boost::future<json> cancel_all_orders_async(const std::string& symbol = "", const json& params = json());
    boost::future<json> edit_order_async(const std::string& id, const std::string& symbol, const std::string& type, const std::string& side,
                                       double amount, double price = 0, const json& params = json());

    // Account
    json fetch_balance(const json& params = json()) override;
    json fetch_open_orders(const std::string& symbol = "", int since = 0, int limit = 0, const json& params = json()) override;
    json fetch_closed_orders(const std::string& symbol = "", int since = 0, int limit = 0, const json& params = json()) override;
    json fetch_my_trades(const std::string& symbol = "", int since = 0, int limit = 0, const json& params = json()) override;
    json fetch_order(const std::string& id, const std::string& symbol = "", const json& params = json()) override;
    json fetch_deposit_address(const std::string& code, const json& params = json()) override;
    json fetch_deposits(const std::string& code = "", int since = 0, int limit = 0, const json& params = json()) override;
    json fetch_withdrawals(const std::string& code = "", int since = 0, int limit = 0, const json& params = json()) override;
    json withdraw(const std::string& code, double amount, const std::string& address, const std::string& tag = "", const json& params = json()) override;

    // Async Account Methods
    boost::future<json> fetch_balance_async(const json& params = json());
    boost::future<json> fetch_open_orders_async(const std::string& symbol = "", int since = 0, int limit = 0, const json& params = json());
    boost::future<json> fetch_closed_orders_async(const std::string& symbol = "", int since = 0, int limit = 0, const json& params = json());
    boost::future<json> fetch_my_trades_async(const std::string& symbol = "", int since = 0, int limit = 0, const json& params = json());
    boost::future<json> fetch_order_async(const std::string& id, const std::string& symbol = "", const json& params = json());
    boost::future<json> fetch_deposit_address_async(const std::string& code, const json& params = json());
    boost::future<json> fetch_deposits_async(const std::string& code = "", int since = 0, int limit = 0, const json& params = json());
    boost::future<json> fetch_withdrawals_async(const std::string& code = "", int since = 0, int limit = 0, const json& params = json());
    boost::future<json> withdraw_async(const std::string& code, double amount, const std::string& address, const std::string& tag = "", const json& params = json());

protected:
    void sign(Request& request, const std::string& path, const std::string& api = "public",
             const std::string& method = "GET", const json& params = json(),
             const json& headers = json(), const json& body = json()) override;

private:
    std::string get_signature(const std::string& timestamp, const std::string& method,
                            const std::string& path, const std::string& body = "");
};

} // namespace ccxt

#endif // CCXT_COINBASEINTERNATIONAL_H
