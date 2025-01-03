#pragma once

#include <string>
#include <map>
#include <vector>
#include <optional>
#include <ccxt/base/types.h>
#include <ccxt/base/config.h>
#include <boost/asio.hpp>
#include <curl/curl.h>

namespace ccxt {
class ExchangeBase {
public:
    ExchangeBase( boost::asio::io_context& context, const Config& config = Config()) : config_(config), context_(context) {}
    virtual ~ExchangeBase() = default;

    // Basic exchange properties
    std::string id;
    std::string name;
    std::vector<std::string> countries;
    std::string version;
    int rateLimit;
    bool pro;
    bool certified;
    std::map<std::string, std::map<std::string, std::string>> urls;
    std::map<std::string, std::optional<bool>> has;
    std::map<std::string, std::string> timeframes;
    long long lastRestRequestTimestamp;
    std::map<std::string, Market> markets;
    std::map<std::string, Market> markets_by_id;

    // Time functions
    virtual long long milliseconds() const {
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    }

protected:
    Config config_;
    boost::asio::io_context& context_;
    CURL* curl_ = nullptr;
    
};

} // namespace ccxt
