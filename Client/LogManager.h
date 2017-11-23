#pragma once
class LogManager
{
public:
	LogManager();
	~LogManager();

private:
	std::shared_ptr<spdlog::logger> logger;
};

