#pragma once
#include "ConfigurationManager.h"
#include "Metrics.h"

// http
using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;
// json
using json = nlohmann::json;

class HttpHelper
{
public:
	HttpHelper(ConfigurationManager* configuration, std::shared_ptr<spdlog::logger> logger);
	~HttpHelper();
	void sendAuthentication();
	void sendStats(Metrics* metrics);
private:
	HttpClient* client;
	std::shared_ptr<spdlog::logger> logger;
	ConfigurationManager* configuration;

	std::string host;
	std::map<std::string, std::string> alerts;
};

