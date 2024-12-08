#ifndef CCXT_ONETRADING_WS_H
#define CCXT_ONETRADING_WS_H

#include "../../websocket_client.h"
#include "../../onetrading.h"
#include <string>
#include <map>
#include <vector>

namespace ccxt {

class OneTradingWS : public WebSocketClient {
public:
    OneTradingWS(boost::asio::io_context& ioc, boost::asio::ssl::context& ctx, OneTrading& exchange);
    ~OneTradingWS() = default;

    // Public API
    void watchTicker(const std::string& symbol);
    void watchTickers(const std::vector<std::string>& symbols);
    void watchOrderBook(const std::string& symbol, const int limit = 100);
    void watchTrades(const std::string& symbol);
    void watchOHLCV(const std::string& symbol, const std::string& timeframe);

    // Private API
    void watchBalance();
    void watchOrders(const std::string& symbol = "");
    void watchMyTrades(const std::string& symbol = "");

private:
    OneTrading& exchange_;
    std::map<std::string, std::string> subscriptions_;
    bool authenticated_;
    int sequenceNumber_;

    void authenticate();
    void handleMessage(const std::string& message) override;
    std::string getEndpoint(const std::string& type = "public") override;
    void ping();

    void subscribe(const std::string& channel, const std::string& symbol = "", bool isPrivate = false);
    void unsubscribe(const std::string& channel, const std::string& symbol = "");
    int getNextSequenceNumber();

    // Message Handlers
    void handleTickerMessage(const nlohmann::json& data);
    void handleOrderBookMessage(const nlohmann::json& data);
    void handleTradeMessage(const nlohmann::json& data);
    void handleOHLCVMessage(const nlohmann::json& data);
    void handleBalanceMessage(const nlohmann::json& data);
    void handleOrderMessage(const nlohmann::json& data);
    void handleMyTradeMessage(const nlohmann::json& data);
    void handleErrorMessage(const nlohmann::json& data);
    void handleAuthMessage(const nlohmann::json& data);
    void handleSubscriptionMessage(const nlohmann::json& data);
    void handleUnsubscriptionMessage(const nlohmann::json& data);
};

} // namespace ccxt

#endif // CCXT_ONETRADING_WS_H
