#ifndef CCXT_POLONIEX_WS_H
#define CCXT_POLONIEX_WS_H

#include "ccxt/exchanges/poloniex.h"
#include "ccxt/websocket_client.h"
#include <map>
#include <string>
#include <vector>

namespace ccxt {

class PoloniexWS : public WebSocketClient {
public:
    PoloniexWS(boost::asio::io_context& ioc, boost::asio::ssl::context& ctx, Poloniex& exchange);

    // Public API methods
    void watchTicker(const std::string& symbol);
    void watchTickers(const std::vector<std::string>& symbols);
    void watchOrderBook(const std::string& symbol, const int limit = 100);
    void watchTrades(const std::string& symbol);
    void watchOHLCV(const std::string& symbol, const std::string& timeframe = "1m");

    // Private API methods
    void watchBalance();
    void watchOrders(const std::string& symbol = "");
    void watchMyTrades(const std::string& symbol = "");
    void watchPositions(const std::string& symbol = "");

protected:
    // WebSocket connection management
    std::string getEndpoint(const std::string& type = "public");
    void authenticate();
    void ping();

    // Subscription management
    void subscribe(const std::string& channel, const std::string& symbol = "", bool isPrivate = false);
    void unsubscribe(const std::string& channel, const std::string& symbol = "");

    // Message handlers
    void handleMessage(const std::string& message);
    void handleTickerMessage(const nlohmann::json& data);
    void handleOrderBookMessage(const nlohmann::json& data);
    void handleTradeMessage(const nlohmann::json& data);
    void handleOHLCVMessage(const nlohmann::json& data);
    void handleBalanceMessage(const nlohmann::json& data);
    void handleOrderMessage(const nlohmann::json& data);
    void handleMyTradeMessage(const nlohmann::json& data);
    void handlePositionMessage(const nlohmann::json& data);
    void handleErrorMessage(const nlohmann::json& data);
    void handleAuthMessage(const nlohmann::json& data);
    void handleSubscriptionMessage(const nlohmann::json& data);
    void handleUnsubscriptionMessage(const nlohmann::json& data);

private:
    Poloniex& exchange_;
    bool authenticated_;
    std::map<std::string, std::string> subscriptions_;
    int64_t sequenceNumber_;
    int64_t getNextSequenceNumber();
    std::string generateSignature(const std::string& timestamp, const std::string& method, 
                                const std::string& path, const std::string& body);
};

} // namespace ccxt

#endif // CCXT_POLONIEX_WS_H
