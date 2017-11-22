#include "stdafx.h"

#include "client_http.hpp"
#include "server_http.hpp"
#include "json.hpp"

#include <algorithm>
#include <fstream>
#include <vector>

#include <sqlite_modern_cpp.h>

#include "CSmtp.h"

using namespace  sqlite;

using namespace std;

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using json = nlohmann::json;

void sendMail();

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

		auto name = json["firstName"].get<std::string>();
			printf("%s %d \n", name.c_str(), name.length());

			*response << "HTTP/1.1 200 OK\r\n"
				<< "Content-Length: " << name.length() << "\r\n\r\n"
				<< name;
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

		mail.SetSMTPServer("smtp.gmail.com", 587);
		mail.SetSecurityType(USE_TLS);
		mail.SetLogin("sombraextra@gmail.com");
		mail.SetPassword("****");
		mail.SetSenderName("User");
		mail.SetSenderMail("sombraextra@gmail.com");
		mail.SetReplyTo("sombraextra@gmail.com");
		mail.SetSubject("The message");
		mail.AddRecipient("sombraextra@gmail.com");
		mail.SetXPriority(XPRIORITY_NORMAL);
		mail.SetXMailer("The Bat! (v3.02) Professional");
		mail.AddMsgLine("Hello,");
		mail.AddMsgLine("");
		mail.AddMsgLine("How are you today?");
		mail.AddMsgLine("");
		mail.AddMsgLine("Regards");
		mail.AddMsgLine("--");
		mail.AddMsgLine("User");

		//mail.Send();
	}
	catch (ECSmtp e)
	{
		std::cout << "Error: " << e.GetErrorText().c_str() << ".\n";
	}
}
