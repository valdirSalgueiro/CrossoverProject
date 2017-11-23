#include "stdafx.h"
#include "HttpHelper.h"

using namespace std;

HttpHelper::HttpHelper(ConfigurationManager* configuration, std::shared_ptr<spdlog::logger> logger)
{
	this->logger = logger;
	this->configuration = configuration;
	alerts = configuration->getAlerts();
	host = configuration->getHost();

	client = new HttpClient(host);
}

HttpHelper::~HttpHelper()
{
}

void HttpHelper::sendAuthentication()
{

	json j;
	j["guid"] = configuration->getKey();
	j["type"] = "authenticate";
	j["email"] = configuration->getEmail();
	j["alert"]["memory"] = alerts["memory"];
	j["alert"]["cpu"] = alerts["cpu"];
	j["alert"]["processes"] = alerts["processes"];


	try {
		auto r = client->request("POST", "/json", j.dump());
		cout << r->content.rdbuf() << endl; // Alternatively, use the convenience function r1->content.string()
	}
	catch (const exception &e) {
		logger->critical("Could not authenticate to server");
	}
}

void HttpHelper::sendStats( Metrics* metrics)
{
	json j;
	j["guid"] = configuration->getKey();
	j["type"] = "stats";
	j["memory"] = (int)metrics->getMemoryPercentStatistic();
	j["cpu"] = (int)metrics->getCpuValueStatistic();
	j["processes"] = (int)metrics->getNumberProcessesStatistic();

	try {
		auto r = client->request("POST", "/json", j.dump());
		cout << r->content.rdbuf() << endl;
	}
	catch (const exception &e) {
		logger->critical("Could not send stats");
	}
}


