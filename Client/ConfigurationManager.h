#pragma once
class ConfigurationManager
{
public:
	ConfigurationManager();
	~ConfigurationManager();
	void readConfiguration();

	int getRefreshInterval()
	{
		return refreshInterval;
	}
	int getAlertInterval()
	{
		return alertInterval;
	}

	std::string getKey() {
		return key;
	}
	std::string getHost() {
		return host;
	}
	std::string getEmail() {
		return email;
	}
	std::map<std::string, std::string> getAlerts() {
		return alerts;
	}
private:
	std::string key;
	std::string host;
	std::string email;
	int refreshInterval;
	int alertInterval;
	std::map<std::string, std::string> alerts;
};

