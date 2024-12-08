#ifndef CCXT_EXCHANGES_ASYNC_COINEX_ASYNC_H
#define CCXT_EXCHANGES_ASYNC_COINEX_ASYNC_H

#include "ccxt/async_base/exchange.h"
#include "ccxt/exchanges/coinex.h"
#include <boost/asio.hpp>
#include <string>
#include <map>
#include <vector>

namespace ccxt {

class CoinExAsync : public AsyncExchange, public CoinEx {
public:
    explicit CoinExAsync(const boost::asio::io_context& context);
    ~CoinExAsync() override = default;

    // Market Data
    boost::future<std::vector<Market>> fetch_markets_async();
    boost::future<Ticker> fetch_ticker_async(const std::string& symbol);
    boost::future<OrderBook> fetch_order_book_async(const std::string& symbol, int limit = 0);
    boost::future<std::vector<Trade>> fetch_trades_async(const std::string& symbol, long since = 0, int limit = 0);
    boost::future<std::vector<OHLCV>> fetch_ohlcv_async(const std::string& symbol,
                                                       const std::string& timeframe = "1m",
                                                       long since = 0,
                                                       int limit = 0);

    // Trading
    boost::future<Order> create_order_async(const std::string& symbol,
                                          const std::string& type,
                                          const std::string& side,
                                          double amount,
                                          double price = 0);
    boost::future<Order> cancel_order_async(const std::string& id,
                                          const std::string& symbol = "");
    boost::future<std::vector<Order>> cancel_all_orders_async(const std::string& symbol = "");
    boost::future<Order> fetch_order_async(const std::string& id,
                                         const std::string& symbol = "");
    boost::future<std::vector<Order>> fetch_orders_async(const std::string& symbol = "",
                                                       long since = 0,
                                                       int limit = 0);
    boost::future<std::vector<Order>> fetch_open_orders_async(const std::string& symbol = "",
                                                            long since = 0,
                                                            int limit = 0);
    boost::future<std::vector<Order>> fetch_closed_orders_async(const std::string& symbol = "",
                                                              long since = 0,
                                                              int limit = 0);
    boost::future<std::vector<Trade>> fetch_my_trades_async(const std::string& symbol = "",
                                                          long since = 0,
                                                          int limit = 0);

    // Account
    boost::future<Balance> fetch_balance_async();
    boost::future<DepositAddress> fetch_deposit_address_async(const std::string& code);
    boost::future<std::vector<Transaction>> fetch_deposits_async(const std::string& code = "",
                                                               long since = 0,
                                                               int limit = 0);
    boost::future<std::vector<Transaction>> fetch_withdrawals_async(const std::string& code = "",
                                                                  long since = 0,
                                                                  int limit = 0);
    boost::future<Transaction> withdraw_async(const std::string& code,
                                            double amount,
                                            const std::string& address,
                                            const std::string& tag = "",
                                            const std::map<std::string, std::string>& params = {});

    // Margin Trading
    boost::future<Balance> fetch_margin_balance_async();
    boost::future<Order> create_margin_order_async(const std::string& symbol,
                                                 const std::string& type,
                                                 const std::string& side,
                                                 double amount,
                                                 double price = 0);
    boost::future<MarginLoan> borrow_margin_async(const std::string& code,
                                                 double amount,
                                                 const std::string& symbol = "");
    boost::future<MarginLoan> repay_margin_async(const std::string& code,
                                                double amount,
                                                const std::string& symbol = "");

protected:
    boost::asio::io_context& context_;
};

} // namespace ccxt

#endif // CCXT_EXCHANGES_ASYNC_COINEX_ASYNC_H
