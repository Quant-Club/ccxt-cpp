#ifndef CCXT_HITBTC3_H
#define CCXT_HITBTC3_H

#include "ccxt/base/exchange.h"

namespace ccxt {

class hitbtc3 : public exchange {
public:
    explicit hitbtc3(const Config& config = Config());
    ~hitbtc3() override = default;

    // Market Data
    json fetch_markets(const json& params = json()) override;
    json fetch_currencies(const json& params = json()) override;
    json fetch_ticker(const std::string& symbol, const json& params = json()) override;
    json fetch_order_book(const std::string& symbol, int limit = 0, const json& params = json()) override;
    json fetch_trades(const std::string& symbol, int since = 0, int limit = 0, const json& params = json()) override;
    json fetch_ohlcv(const std::string& symbol, const std::string& timeframe = "1m", int since = 0, int limit = 0, const json& params = json()) override;
    json fetch_trading_fees(const std::string& symbol = "", const json& params = json()) override;
    json fetch_funding_rate(const std::string& symbol, const json& params = json());
    json fetch_funding_rate_history(const std::string& symbol = "", int since = 0, int limit = 0, const json& params = json());
    json fetch_index_ohlcv(const std::string& symbol, const std::string& timeframe = "1m", int since = 0, int limit = 0, const json& params = json());
    json fetch_mark_ohlcv(const std::string& symbol, const std::string& timeframe = "1m", int since = 0, int limit = 0, const json& params = json());

    // Trading
    json create_order(const std::string& symbol, const std::string& type, const std::string& side,
                     double amount, double price = 0, const json& params = json()) override;
    json create_market_buy_order(const std::string& symbol, double amount, const json& params = json());
    json create_market_sell_order(const std::string& symbol, double amount, const json& params = json());
    json cancel_order(const std::string& id, const std::string& symbol = "", const json& params = json()) override;
    json cancel_all_orders(const std::string& symbol = "", const json& params = json()) override;
    json edit_order(const std::string& id, const std::string& symbol, const std::string& type, const std::string& side,
                   double amount, double price = 0, const json& params = json()) override;

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

    // Futures/Margin Trading
    json fetch_positions(const std::string& symbol = "", const json& params = json());
    json fetch_position(const std::string& symbol, const json& params = json());
    json set_leverage(const std::string& symbol, int leverage, const json& params = json());
    json set_margin_mode(const std::string& symbol, const std::string& marginMode, const json& params = json());
    json fetch_leverage_tiers(const std::string& symbols = "", const json& params = json());
    json fetch_funding_history(const std::string& symbol = "", int since = 0, int limit = 0, const json& params = json());

protected:
    void sign(Request& request, const std::string& path, const std::string& api = "public",
             const std::string& method = "GET", const json& params = json(),
             const json& headers = json(), const json& body = json()) override;

private:
    std::string get_signature(const std::string& timestamp, const std::string& method,
                            const std::string& path, const std::string& body = "");
    
    // Helper methods
    json parse_ticker(const json& ticker, const json& market = json());
    json parse_order_book(const json& orderbook, const std::string& symbol, int timestamp = 0, const std::string& bids_key = "bids", const std::string& asks_key = "asks");
    json parse_trade(const json& trade, const json& market = json());
    json parse_ohlcv(const json& ohlcv, const json& market = json());
    json parse_balance(const json& balance);
    json parse_order(const json& order, const json& market = json());
    json parse_position(const json& position, const json& market = json());
};

} // namespace ccxt

#endif // CCXT_HITBTC3_H
