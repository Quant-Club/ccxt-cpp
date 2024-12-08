#ifndef CCXT_BITMART_ASYNC_H
#define CCXT_BITMART_ASYNC_H

#include "ccxt/exchanges/async/exchange_async.h"
#include "ccxt/exchanges/bitmart.h"

namespace ccxt {

class BitmartAsync : public ExchangeAsync, public Bitmart {
public:
    explicit BitmartAsync(const boost::asio::io_context& context);
    ~BitmartAsync() = default;

    // Base methods
    boost::future<json> fetchAsync(const String& path, const String& api = "public",
                                const String& method = "GET", const json& params = json({}),
                                const std::map<String, String>& headers = {});

    // Market Data
    boost::future<json> fetchMarketsAsync(const json& params = json({}));
    boost::future<json> fetchCurrenciesAsync(const json& params = json({}));
    boost::future<json> fetchTickerAsync(const String& symbol, const json& params = json({}));
    boost::future<json> fetchTickersAsync(const std::vector<String>& symbols = {}, const json& params = json({}));
    boost::future<json> fetchOrderBookAsync(const String& symbol, int limit = 0, const json& params = json({}));
    boost::future<json> fetchTradesAsync(const String& symbol, int since = 0, int limit = 0, const json& params = json({}));
    boost::future<json> fetchOHLCVAsync(const String& symbol, const String& timeframe = "1m",
                                     int since = 0, int limit = 0, const json& params = json({}));

    // Trading
    boost::future<json> fetchBalanceAsync(const json& params = json({}));
    boost::future<json> createOrderAsync(const String& symbol, const String& type, const String& side,
                                     double amount, double price = 0, const json& params = json({}));
    boost::future<json> cancelOrderAsync(const String& id, const String& symbol = "", const json& params = json({}));
    boost::future<json> cancelAllOrdersAsync(const String& symbol = "", const json& params = json({}));
    boost::future<json> fetchOrderAsync(const String& id, const String& symbol = "", const json& params = json({}));
    boost::future<json> fetchOrdersAsync(const String& symbol = "", int since = 0, int limit = 0,
                                     const json& params = json({}));
    boost::future<json> fetchOpenOrdersAsync(const String& symbol = "", int since = 0, int limit = 0,
                                         const json& params = json({}));
    boost::future<json> fetchClosedOrdersAsync(const String& symbol = "", int since = 0, int limit = 0,
                                           const json& params = json({}));
    boost::future<json> fetchMyTradesAsync(const String& symbol = "", int since = 0, int limit = 0,
                                       const json& params = json({}));

    // Margin Trading
    boost::future<json> fetchPositionsAsync(const String& symbols = "", const json& params = json({}));
    boost::future<json> fetchPositionRiskAsync(const String& symbols = "", const json& params = json({}));
    boost::future<json> setLeverageAsync(int leverage, const String& symbol, const json& params = json({}));
    boost::future<json> setMarginModeAsync(const String& marginMode, const String& symbol, const json& params = json({}));

    // Funding
    boost::future<json> fetchDepositAddressAsync(const String& code, const json& params = json({}));
    boost::future<json> fetchDepositsAsync(const String& code = "", int since = 0, int limit = 0,
                                       const json& params = json({}));
    boost::future<json> fetchWithdrawalsAsync(const String& code = "", int since = 0, int limit = 0,
                                          const json& params = json({}));
    boost::future<json> withdrawAsync(const String& code, double amount, const String& address,
                                  const String& tag = "", const json& params = json({}));
    boost::future<json> fetchTransactionsAsync(const String& code = "", int since = 0, int limit = 0,
                                           const json& params = json({}));
    boost::future<json> fetchTransferAsync(const String& id, const json& params = json({}));
    boost::future<json> fetchTransfersAsync(const json& params = json({}));
    boost::future<json> transferAsync(const String& code, double amount, const String& fromAccount,
                                  const String& toAccount, const json& params = json({}));

    // Fees
    boost::future<json> fetchTradingFeesAsync(const json& params = json({}));
    boost::future<json> fetchFundingFeesAsync(const json& params = json({}));
};

} // namespace ccxt

#endif // CCXT_BITMART_ASYNC_H
