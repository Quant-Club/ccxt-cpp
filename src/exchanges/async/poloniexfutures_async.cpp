#include "ccxt/exchanges/async/poloniexfutures_async.h"

namespace ccxt {

PoloniexFuturesAsync::PoloniexFuturesAsync(const boost::asio::io_context& context, const Config& config)
    : AsyncExchange(context, config)
    , poloniexfutures(config)
    , context_(const_cast<boost::asio::io_context&>(context)) {}

// Market Data API
boost::future<std::vector<Market>> PoloniexFuturesAsync::fetch_markets_async() {
    return async_request<std::vector<Market>>([this]() { return this->fetch_markets(); });
}

boost::future<std::vector<Currency>> PoloniexFuturesAsync::fetch_currencies_async() {
    return async_request<std::vector<Currency>>([this]() { return this->fetch_currencies(); });
}

boost::future<Ticker> PoloniexFuturesAsync::fetch_ticker_async(const std::string& symbol) {
    return async_request<Ticker>([this, symbol]() { return this->fetch_ticker(symbol); });
}

boost::future<std::vector<Ticker>> PoloniexFuturesAsync::fetch_tickers_async(const std::vector<std::string>& symbols) {
    return async_request<std::vector<Ticker>>([this, symbols]() { return this->fetch_tickers(symbols); });
}

boost::future<OrderBook> PoloniexFuturesAsync::fetch_order_book_async(const std::string& symbol,
                                                                    const std::optional<int>& limit) {
    return async_request<OrderBook>([this, symbol, limit]() { return this->fetch_order_book(symbol, limit); });
}

boost::future<std::vector<Trade>> PoloniexFuturesAsync::fetch_trades_async(const std::string& symbol,
                                                                         const std::optional<long long>& since,
                                                                         const std::optional<int>& limit) {
    return async_request<std::vector<Trade>>([this, symbol, since, limit]() {
        return this->fetch_trades(symbol, since, limit);
    });
}

boost::future<std::vector<OHLCV>> PoloniexFuturesAsync::fetch_ohlcv_async(const std::string& symbol,
                                                                         const std::string& timeframe,
                                                                         const std::optional<long long>& since,
                                                                         const std::optional<int>& limit) {
    return async_request<std::vector<OHLCV>>([this, symbol, timeframe, since, limit]() {
        return this->fetch_ohlcv(symbol, timeframe, since, limit);
    });
}

// Trading API
boost::future<Order> PoloniexFuturesAsync::create_order_async(const std::string& symbol,
                                                            const std::string& type,
                                                            const std::string& side,
                                                            double amount,
                                                            const std::optional<double>& price) {
    return async_request<Order>([this, symbol, type, side, amount, price]() {
        return this->create_order(symbol, type, side, amount, price);
    });
}

boost::future<Order> PoloniexFuturesAsync::cancel_order_async(const std::string& id,
                                                            const std::string& symbol) {
    return async_request<Order>([this, id, symbol]() { return this->cancel_order(id, symbol); });
}

boost::future<Order> PoloniexFuturesAsync::fetch_order_async(const std::string& id,
                                                           const std::string& symbol) {
    return async_request<Order>([this, id, symbol]() { return this->fetch_order(id, symbol); });
}

boost::future<std::vector<Order>> PoloniexFuturesAsync::fetch_open_orders_async(const std::string& symbol,
                                                                              const std::optional<long long>& since,
                                                                              const std::optional<int>& limit) {
    return async_request<std::vector<Order>>([this, symbol, since, limit]() {
        return this->fetch_open_orders(symbol, since, limit);
    });
}

boost::future<std::vector<Order>> PoloniexFuturesAsync::fetch_closed_orders_async(const std::string& symbol,
                                                                                const std::optional<long long>& since,
                                                                                const std::optional<int>& limit) {
    return async_request<std::vector<Order>>([this, symbol, since, limit]() {
        return this->fetch_closed_orders(symbol, since, limit);
    });
}

boost::future<std::vector<Trade>> PoloniexFuturesAsync::fetch_my_trades_async(const std::string& symbol,
                                                                            const std::optional<long long>& since,
                                                                            const std::optional<int>& limit) {
    return async_request<std::vector<Trade>>([this, symbol, since, limit]() {
        return this->fetch_my_trades(symbol, since, limit);
    });
}

// Account API
boost::future<Balance> PoloniexFuturesAsync::fetch_balance_async() {
    return async_request<Balance>([this]() { return this->fetch_balance(); });
}

boost::future<std::vector<Position>> PoloniexFuturesAsync::fetch_positions_async(const std::vector<std::string>& symbols) {
    return async_request<std::vector<Position>>([this, symbols]() { return this->fetch_positions(symbols); });
}

boost::future<std::vector<Transaction>> PoloniexFuturesAsync::fetch_deposits_async(const std::optional<std::string>& code,
                                                                                 const std::optional<long long>& since,
                                                                                 const std::optional<int>& limit) {
    return async_request<std::vector<Transaction>>([this, code, since, limit]() {
        return this->fetch_deposits(code, since, limit);
    });
}

boost::future<std::vector<Transaction>> PoloniexFuturesAsync::fetch_withdrawals_async(const std::optional<std::string>& code,
                                                                                    const std::optional<long long>& since,
                                                                                    const std::optional<int>& limit) {
    return async_request<std::vector<Transaction>>([this, code, since, limit]() {
        return this->fetch_withdrawals(code, since, limit);
    });
}

} // namespace ccxt
