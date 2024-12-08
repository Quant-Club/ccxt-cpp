#include "ccxt/exchanges/async/independentreserve_async.h"
#include "ccxt/async_base/async_request.h"

namespace ccxt {

IndependentReserveAsync::IndependentReserveAsync(const boost::asio::io_context& context, const Config& config)
    : AsyncExchange(context)
    , independentreserve(config)
    , context_(const_cast<boost::asio::io_context&>(context)) {}

// Market Data API
boost::future<std::vector<Market>> IndependentReserveAsync::fetch_markets_async() {
    return async_request<std::vector<Market>>(context_, [this]() {
        return fetchMarketsImpl();
    });
}

boost::future<std::vector<Currency>> IndependentReserveAsync::fetch_currencies_async() {
    return async_request<std::vector<Currency>>(context_, [this]() {
        return fetchCurrenciesImpl();
    });
}

boost::future<Ticker> IndependentReserveAsync::fetch_ticker_async(const std::string& symbol) {
    return async_request<Ticker>(context_, [this, symbol]() {
        return fetchTickerImpl(symbol);
    });
}

boost::future<std::vector<Ticker>> IndependentReserveAsync::fetch_tickers_async(const std::vector<std::string>& symbols) {
    return async_request<std::vector<Ticker>>(context_, [this, symbols]() {
        return fetchTickersImpl(symbols);
    });
}

boost::future<OrderBook> IndependentReserveAsync::fetch_order_book_async(const std::string& symbol,
                                                                       const std::optional<int>& limit) {
    return async_request<OrderBook>(context_, [this, symbol, limit]() {
        return fetchOrderBookImpl(symbol, limit);
    });
}

boost::future<std::vector<Trade>> IndependentReserveAsync::fetch_trades_async(const std::string& symbol,
                                                                            const std::optional<long long>& since,
                                                                            const std::optional<int>& limit) {
    return async_request<std::vector<Trade>>(context_, [this, symbol, since, limit]() {
        return fetchTradesImpl(symbol, since, limit);
    });
}

boost::future<std::vector<OHLCV>> IndependentReserveAsync::fetch_ohlcv_async(const std::string& symbol,
                                                                            const std::string& timeframe,
                                                                            const std::optional<long long>& since,
                                                                            const std::optional<int>& limit) {
    return async_request<std::vector<OHLCV>>(context_, [this, symbol, timeframe, since, limit]() {
        return fetchOHLCVImpl(symbol, timeframe, since, limit);
    });
}

// Trading API
boost::future<Order> IndependentReserveAsync::create_order_async(const std::string& symbol,
                                                               const std::string& type,
                                                               const std::string& side,
                                                               double amount,
                                                               const std::optional<double>& price) {
    return async_request<Order>(context_, [this, symbol, type, side, amount, price]() {
        return createOrderImpl(symbol, type, side, amount, price);
    });
}

boost::future<Order> IndependentReserveAsync::cancel_order_async(const std::string& id,
                                                               const std::string& symbol) {
    return async_request<Order>(context_, [this, id, symbol]() {
        return cancelOrderImpl(id, symbol);
    });
}

boost::future<Order> IndependentReserveAsync::fetch_order_async(const std::string& id,
                                                              const std::string& symbol) {
    return async_request<Order>(context_, [this, id, symbol]() {
        return fetchOrderImpl(id, symbol);
    });
}

boost::future<std::vector<Order>> IndependentReserveAsync::fetch_open_orders_async(const std::string& symbol,
                                                                                 const std::optional<long long>& since,
                                                                                 const std::optional<int>& limit) {
    return async_request<std::vector<Order>>(context_, [this, symbol, since, limit]() {
        return fetchOpenOrdersImpl(symbol, since, limit);
    });
}

boost::future<std::vector<Order>> IndependentReserveAsync::fetch_closed_orders_async(const std::string& symbol,
                                                                                   const std::optional<long long>& since,
                                                                                   const std::optional<int>& limit) {
    return async_request<std::vector<Order>>(context_, [this, symbol, since, limit]() {
        return fetchClosedOrdersImpl(symbol, since, limit);
    });
}

boost::future<std::vector<Trade>> IndependentReserveAsync::fetch_my_trades_async(const std::string& symbol,
                                                                               const std::optional<long long>& since,
                                                                               const std::optional<int>& limit) {
    return async_request<std::vector<Trade>>(context_, [this, symbol, since, limit]() {
        return fetchMyTradesImpl(symbol, since, limit);
    });
}

// Account API
boost::future<Balance> IndependentReserveAsync::fetch_balance_async() {
    return async_request<Balance>(context_, [this]() {
        return fetchBalanceImpl();
    });
}

boost::future<DepositAddress> IndependentReserveAsync::fetch_deposit_address_async(const std::string& code,
                                                                                 const std::optional<std::string>& network) {
    return async_request<DepositAddress>(context_, [this, code, network]() {
        return fetchDepositAddressImpl(code, network);
    });
}

boost::future<std::vector<Transaction>> IndependentReserveAsync::fetch_deposits_async(const std::optional<std::string>& code,
                                                                                    const std::optional<long long>& since,
                                                                                    const std::optional<int>& limit) {
    return async_request<std::vector<Transaction>>(context_, [this, code, since, limit]() {
        return fetchDepositsImpl(code, since, limit);
    });
}

boost::future<std::vector<Transaction>> IndependentReserveAsync::fetch_withdrawals_async(const std::optional<std::string>& code,
                                                                                       const std::optional<long long>& since,
                                                                                       const std::optional<int>& limit) {
    return async_request<std::vector<Transaction>>(context_, [this, code, since, limit]() {
        return fetchWithdrawalsImpl(code, since, limit);
    });
}

} // namespace ccxt