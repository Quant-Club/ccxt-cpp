#pragma once

#include "ccxt/base/exchange.h"

namespace ccxt {

class WavesExchange : public Exchange {
public:
    WavesExchange();
    ~WavesExchange() override = default;

    // Market Data
    nlohmann::json fetch_markets() override;
    nlohmann::json fetch_ticker(const std::string& symbol);
    nlohmann::json fetch_order_book(const std::string& symbol, int limit = 0);
    nlohmann::json fetch_trades(const std::string& symbol, int since = 0, int limit = 0);
    nlohmann::json fetch_ohlcv(const std::string& symbol, const std::string& timeframe = "1m",
                              int since = 0, int limit = 0);

    // Trading
    nlohmann::json create_order(const std::string& symbol, const std::string& type,
                               const std::string& side, double amount,
                               double price = 0);
    nlohmann::json cancel_order(const std::string& id, const std::string& symbol = "");
    nlohmann::json fetch_order(const std::string& id, const std::string& symbol = "");
    nlohmann::json fetch_orders(const std::string& symbol = "", int since = 0, int limit = 0);
    nlohmann::json fetch_open_orders(const std::string& symbol = "", int since = 0, int limit = 0);
    nlohmann::json fetch_closed_orders(const std::string& symbol = "", int since = 0, int limit = 0);
    nlohmann::json fetch_my_trades(const std::string& symbol = "", int since = 0, int limit = 0);

    // Account
    nlohmann::json fetch_balance();
    nlohmann::json fetch_deposits(const std::string& code = "", int since = 0, int limit = 0);
    nlohmann::json fetch_withdrawals(const std::string& code = "", int since = 0, int limit = 0);
    nlohmann::json fetch_deposit_address(const std::string& code);

    // Additional Waves-specific methods
    nlohmann::json sign_order(const nlohmann::json& order);
    nlohmann::json get_waves_balance(const std::string& address);
    nlohmann::json get_matcher_public_key();

protected:
    std::string sign(const std::string& path, const std::string& api = "public",
                    const std::string& method = "GET", const nlohmann::json& params = nlohmann::json({}),
                    const std::map<std::string, std::string>& headers = {}) override;

private:
    nlohmann::json parse_ticker(const nlohmann::json& ticker, const nlohmann::json& market = nlohmann::json({}));
    nlohmann::json parse_trade(const nlohmann::json& trade, const nlohmann::json& market = nlohmann::json({}));
    nlohmann::json parse_order(const nlohmann::json& order, const nlohmann::json& market = nlohmann::json({}));
    nlohmann::json parse_balance(const nlohmann::json& response);
    nlohmann::json parse_transaction(const nlohmann::json& transaction, const std::string& currency = "");
    
    std::string get_asset_bytes(const std::string& assetId);
    std::string get_order_bytes(const nlohmann::json& order);
    std::string sign_message(const std::string& message);
    std::string get_network_byte();
    
    std::string matcherPublicKey;
};

} // namespace ccxt
