#ifndef CCXT_EXCHANGE_EXMO_WS_H
#define CCXT_EXCHANGE_EXMO_WS_H

#include "../../websocket_client.h"
#include "../../exmo.h"
#include <string>
#include <map>
#include <vector>

namespace ccxt {

class ExmoWS : public WebSocketClient {
public:
    ExmoWS(boost::asio::io_context& ioc, boost::asio::ssl::context& ctx, Exmo& exchange);
    ~ExmoWS() = default;

    // Public API
    void watchTicker(const std::string& symbol, const std::map<std::string, std::string>& params = {});
    void watchTickers(const std::vector<std::string>& symbols, const std::map<std::string, std::string>& params = {});
    void watchOrderBook(const std::string& symbol, int limit = 50, const std::map<std::string, std::string>& params = {});
    void watchTrades(const std::string& symbol, const std::map<std::string, std::string>& params = {});

    // Private API
    void watchBalance(const std::map<std::string, std::string>& params = {});
    void watchOrders(const std::string& symbol = "", const std::map<std::string, std::string>& params = {});
    void watchMyTrades(const std::string& symbol = "", const std::map<std::string, std::string>& params = {});

private:
    Exmo& exchange_;
    std::map<std::string, std::string> subscriptions_;
    std::map<std::string, OrderBook> orderbooks_;
    std::map<std::string, ArrayCache<Trade>> trades_;
    std::map<std::string, Ticker> tickers_;
    bool authenticated_;
    std::string sessionId_;
    
    // Helper methods
    void authenticate();
    void subscribe(const std::string& channel, const std::string& symbol = "", bool isPrivate = false);
    void subscribeMultiple(const std::string& channel, const std::vector<std::string>& symbols, bool isPrivate = false);
    void unsubscribe(const std::string& channel, const std::string& symbol = "");
    void unsubscribeMultiple(const std::string& channel, const std::vector<std::string>& symbols);
    std::string getEndpoint(const std::string& type = "public");
    std::string getMarketId(const std::string& symbol);
    std::string getSymbol(const std::string& marketId);
    std::string getChannel(const std::string& channel, const std::string& symbol);
    int getNextRequestId();

    // Message handlers
    void handleMessage(const std::string& message);
    void handleTickerMessage(const nlohmann::json& data);
    void handleOrderBookMessage(const nlohmann::json& data);
    void handleTradeMessage(const nlohmann::json& data);
    void handleBalanceMessage(const nlohmann::json& data);
    void handleOrderMessage(const nlohmann::json& data);
    void handleMyTradeMessage(const nlohmann::json& data);
    void handleErrorMessage(const nlohmann::json& data);
    void handleSubscriptionMessage(const nlohmann::json& data);
    void handleUnsubscriptionMessage(const nlohmann::json& data);
    void handleAuthenticationMessage(const nlohmann::json& data);
    void handleInfoMessage(const nlohmann::json& data);

    // Market parsing
    Ticker parseWsTicker(const nlohmann::json& ticker, const Market* market = nullptr);
    Trade parseWsTrade(const nlohmann::json& trade, const Market* market = nullptr);
    Order parseWsOrder(const nlohmann::json& order, const Market* market = nullptr);
    std::map<std::string, std::string> parseMarket(const std::string& marketId);
};

} // namespace ccxt

#endif // CCXT_EXCHANGE_EXMO_WS_H
