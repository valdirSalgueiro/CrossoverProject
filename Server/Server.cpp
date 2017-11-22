#include "stdafx.h"

#include "client_http.hpp"
#include "server_http.hpp"
#include "json.hpp"

#include <algorithm>
#include <fstream>
#include <vector>

#include <sqlite_modern_cpp.h>
#include <map>
#include <list>
#include "tinyxml2.h"

#include "CSmtp.h"

using namespace  sqlite;

using namespace std;

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using json = nlohmann::json;

void sendMail();

class Stat
{
public:
	Stat(int _memory, int _cpu, int _processes)
	{
		memory = _memory;
		cpu = _cpu;
		processes = _processes;
	}
private:
	int memory;
	int cpu;
	int processes;
};

class Alert
{
public:
	Alert(std::string _type, int _limit)
	{
		type = _type;
		limit = _limit;
	}
private:
	std::string type;
	int limit;
};

class ClientMachine
{
public:
	ClientMachine(std::string _email)
	{
		email = _email;
	}
	void AddAlert(Alert alert)
	{
		alerts.push_back(alert);
	}
	void AddStat(Stat stat)
	{
		stats.push_back(stat);
	}
private:
	std::string email;
	std::list<Stat> stats;
	std::list<Alert> alerts;
};

std::map<std::string, ClientMachine*> clients;
std::string smtp_host;
std::string smtp_port;
std::string smtp_login;
std::string smtp_password;
std::string smtp_sender;
std::string smtp_sender_mail;

void readConfiguration()
{
	tinyxml2::XMLDocument doc;
	doc.LoadFile("server.xml");
	auto client = doc.FirstChildElement("server");

	smtp_host = client->Attribute("smtp_host");
	smtp_port = client->Attribute("host");
	smtp_login = client->Attribute("smtp_login");
	smtp_password = client->IntAttribute("smtp_password");
	smtp_sender = client->IntAttribute("smtp_sender");
	smtp_sender_mail = client->IntAttribute("smtp_sender_mail");
}

int main() {
	cout << "starting server" << endl;

	cout << "retrieving database" << endl;

	// creates a database file if it does not exists.
	database db("crossover.db");

	// HTTP-server at port 8080 using 1 thread
	// Unless you do more heavy non-threaded processing in the resources,
	// 1 thread is usually faster than several threads
	HttpServer server;
	server.config.port = 8080;

	sendMail();

	server.resource["^/json$"]["POST"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		try {
			auto json = json::parse(request->content);

			auto type = json["type"].get<std::string>();
			if (type == "authenticate")
			{
				auto guid = json["guid"].get<std::string>();
				// check for authentication
				auto email = json["email"].get<std::string>();
				auto memoryAlert = json["alert"]["memory"].get<std::string>();
				auto cpuAlert = json["alert"]["cpu"].get<std::string>();
				auto processesAlert = json["alert"]["processes"].get<std::string>();
				ClientMachine* machine = new ClientMachine(email);
				Alert alert("cpu", 50);
				machine->AddAlert(alert);
				clients[guid] = machine;
			}
			else if (type == "stats")
			{
				auto guid = json["guid"].get<std::string>();
				// check for authentication
				if (clients.find(guid) != clients.end())
				{
					auto memory = json["memory"].get<int>();
					auto cpu = json["cpu"].get<int>();
					auto processes = json["processes"].get<int>();
					Stat stat(memory, cpu, processes);
					clients[guid]->AddStat(stat);
				}
			}

			*response << "HTTP/1.1 200 OK\r\n"
				<< "Content-Length: " << 0 << "\r\n\r\n";
		}
		catch (const exception &e) {
			*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n"
				<< e.what();
		}
	};

	server.on_error = [](shared_ptr<HttpServer::Request> /*request*/, const SimpleWeb::error_code & /*ec*/) {
		// Handle errors here
	};

	thread server_thread([&server]() {
		// Start server
		server.start();
	});

	cout << "server listening on 8080" << endl;
	server_thread.join();
}

void sendMail()
{
	try
	{
		CSmtp mail;

		std::string smtp_host;
		std::string smtp_port;
		std::string smtp_login;
		std::string smtp_password;
		std::string smtp_sender;
		std::string smtp_sender_mail;

		mail.SetSMTPServer(smtp_host.c_str(), 587);
		mail.SetSecurityType(USE_TLS);
		mail.SetLogin(smtp_login.c_str());
		mail.SetPassword(smtp_password.c_str());
		mail.SetSenderName(smtp_sender.c_str());
		mail.SetSenderMail(smtp_sender_mail.c_str());
		mail.SetReplyTo(smtp_sender_mail.c_str());
		mail.SetSubject("Crossover monitoring alert!");
		mail.AddRecipient(smtp_sender_mail.c_str());
		mail.SetXPriority(XPRIORITY_NORMAL);
		mail.SetXMailer("The Bat! (v3.02) Professional");
		mail.AddMsgLine("Hello, you are receinving this message because you configure an alert in Crossover monitoring software.");

		//mail.Send();
	}
	catch (ECSmtp e)
	{
		std::cout << "Error: " << e.GetErrorText().c_str() << ".\n";
	}
}
