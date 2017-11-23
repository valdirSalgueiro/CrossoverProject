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

#include <stdio.h>

#include "CSmtp.h"

using namespace  sqlite;

using namespace std;

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using json = nlohmann::json;

class Stat
{
public:
	Stat(int _memory, float _cpu, float _processes)
	{
		memory = _memory;
		cpu = _cpu;
		processes = _processes;
	}
private:
	float memory;
	float cpu;
	float processes;
};

class Alert
{
public:
	Alert(std::string _type, float _limit)
	{
		type = _type;
		limit = _limit;
	}
	std::string getType()
	{
		return type;
	}
	float getLimit()
	{
		return limit;
	}

private:
	std::string type;
	float limit;
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
		alerts[alert.getType()] = alert.getLimit();
	}
	float GetAlertLimit(std::string type)
	{
		return alerts[type];
	}
	void AddStat(Stat stat)
	{
		stats.push_back(stat);
	}
	bool HasTriggeredAlert(std::string type, float value)
	{
		return value > alerts[type];
	}
private:
	std::string email;
	std::list<Stat> stats;
	std::map<std::string, float> alerts;
};

void sendMail(ClientMachine*, int, int, int);

std::map<std::string, ClientMachine*> clients;
std::string smtp_host;
int smtp_port;
std::string smtp_login;
std::string smtp_password;
std::string smtp_sender;
std::string smtp_sender_mail;

void readConfiguration()
{
	tinyxml2::XMLDocument doc;
	doc.LoadFile("server.xml");
	auto server = doc.FirstChildElement("server");

	smtp_host = server->Attribute("smtp_host");
	smtp_port = server->IntAttribute("smtp_port");
	smtp_login = server->Attribute("smtp_login");
	smtp_password = server->Attribute("smtp_password");
	smtp_sender = server->Attribute("smtp_sender");
	smtp_sender_mail = server->Attribute("smtp_sender_mail");
}

database* db;

int main() {
	cout << "starting server" << endl;

	cout << "loading configurations" << endl;

	readConfiguration();

	cout << "retrieving database" << endl;

	// creates a database file if it does not exists.
	db = new database("crossover.db");

	cout << "retrieving tables" << endl;
	//populate database with required tables
	*db <<
		"create table if not exists ClientMachine ("
		"   _id integer primary key autoincrement not null,"
		"   guid text,"
		"   email text,"
		"   UNIQUE(guid)"
		");";
	*db <<
		"create table if not exists Alert ("
		"   _id integer primary key autoincrement not null,"
		"   guid text,"
		"   type text,"
		"   alertlimit numeric"
		");";
	*db <<
		"create table if not exists Stat ("
		"   _id integer primary key autoincrement not null,"
		"   guid text,"
		"   memory real,"
		"   cpu real,"
		"   stat_date  text,"
		"   processes real"
		");";
	*db <<
		"create table if not exists TriggeredAlert ("
		"   _id integer primary key autoincrement not null,"
		"   guid text,"
		"   type text,"
		"   alertlimit real,"
		"   alert_date  text,"
		"   value real"
		");";

	HttpServer server;
	server.config.port = 8080;

	server.resource["^/json$"]["POST"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		try {
			auto json = json::parse(request->content);

			auto type = json["type"].get<std::string>();
			if (type == "authenticate")
			{
				auto guid = json["guid"].get<std::string>();
				// check for authentication
				auto email = json["email"].get<std::string>();
				auto memory = std::stof(json["alert"]["memory"].get<std::string>());
				auto cpu = std::stof(json["alert"]["cpu"].get<std::string>());
				auto processes = std::stof(json["alert"]["processes"].get<std::string>());
				ClientMachine* machine = new ClientMachine(email);
				Alert cpuAlert("cpu", cpu);
				Alert memoryAlert("memory", memory);
				Alert processesAlert("processes", memory);
				machine->AddAlert(cpuAlert);
				machine->AddAlert(memoryAlert);
				machine->AddAlert(processesAlert);
				clients[guid] = machine;

				*db << "delete from Alert where guid = ?;" << guid;
				*db << "insert or ignore into ClientMachine (guid,email) values (?,?);"
					<< guid
					<< email;
				*db << "insert into Alert (guid, type, alertlimit) values (?,?,?);"
					<< guid
					<< "cpu"
					<< cpu;
				*db << "insert into Alert (guid, type, alertlimit) values (?,?,?);"
					<< guid
					<< "memory"
					<< memory;
				*db << "insert into Alert (guid, type, alertlimit) values (?,?,?);"
					<< guid
					<< "processes"
					<< processes;
			}
			else if (type == "stats")
			{
				auto guid = json["guid"].get<std::string>();
				// check for authentication
				if (clients.find(guid) != clients.end())
				{
					// client is authenticated, good to go
					auto memory = json["memory"].get<float>();
					auto cpu = json["cpu"].get<float>();
					auto processes = json["processes"].get<float>();
					Stat stat(memory, cpu, processes);
					clients[guid]->AddStat(stat);

					*db << "insert into Stat (guid, memory, processes, cpu, stat_date) values (?,?,?,?, datetime('now', 'localtime'));"
						<< guid
						<< memory
						<< cpu
						<< processes;

					bool triggeredAlert;
					if (clients[guid]->HasTriggeredAlert("cpu", cpu))
					{
						*db << "insert into TriggeredAlert (guid, type, alertlimit, value, alert_date) values (?,?,?,?, datetime('now', 'localtime'));"
							<< guid
							<< "cpu"
							<< clients[guid]->GetAlertLimit("cpu")
							<< cpu;
						triggeredAlert = true;
					}
					if (clients[guid]->HasTriggeredAlert("memory", memory))
					{
						*db << "insert into TriggeredAlert (guid, type, alertlimit, value, alert_date) values (?,?,?,?, datetime('now', 'localtime'));"
							<< guid
							<< "memory"
							<< clients[guid]->GetAlertLimit("memory")
							<< memory;
						triggeredAlert = true;
					}
					if (clients[guid]->HasTriggeredAlert("processes", memory))
					{
						*db << "insert into TriggeredAlert (guid, type, alertlimit, value, alert_date) values (?,?,?,?, datetime('now', 'localtime'));"
							<< guid
							<< "processes"
							<< clients[guid]->GetAlertLimit("processes")
							<< memory;
						triggeredAlert = true;
					}
					if (triggeredAlert)
					{
						sendMail(clients[guid], memory, cpu, processes);
					}
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

void sendMail(ClientMachine* machine, int memory, int cpu, int processes)
{
	try
	{
		std::string message;

		CSmtp mail;

		mail.SetSMTPServer(smtp_host.c_str(), smtp_port);
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
		mail.AddMsgLine("Hello, you are receinving this message because you configure an alert in Crossover monitoring software.\n");
		mail.AddMsgLine("Alert type\tLimit\t\t\tMeasured Value\n");
		message = "cpu\t\t\t" + std::to_string(machine->GetAlertLimit("cpu")) + "\t" + std::to_string(cpu) + "\n";
		mail.AddMsgLine(message.c_str());
		message = "memory\t\t" + std::to_string(machine->GetAlertLimit("memory")) + "\t" + std::to_string(memory) + "\n";
		mail.AddMsgLine(message.c_str());
		message = "processes\t" + std::to_string(machine->GetAlertLimit("processes")) + "\t" + std::to_string(processes) + "\n";
		mail.AddMsgLine(message.c_str());

		mail.Send();
	}
	catch (ECSmtp e)
	{
		std::cout << "Error: " << e.GetErrorText().c_str() << ".\n";
	}
}
