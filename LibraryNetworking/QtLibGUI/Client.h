#pragma once

#include <iostream>
#include <olc_net.h>
#include <QObject>


enum class CustomMsgTypes : uint32_t
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	MessageServer,
	ServerMessage,

	Login,

	LoanBook,
	ReturnBook,
	AddBook,
	RemoveBook,
	InformationBook,
	IsAvailableBook,
	ListBooks,
	ListMembers,
	ListLibrarians,
	AddMember,
	RemoveMember,
	AddLibrarian,
	RemoveLibrarian,
};

class CustomClient : public QObject, public olc::net::client_interface<CustomMsgTypes>
{
	Q_OBJECT

public:
	olc::net::tsqueue<std::string> inputQueue;
	bool isLoggedIn = false;
	bool loginApproval = false;  // brukes ikke enda


	void PingServer()
	{
		olc::net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::ServerPing;

		// Caution with this...
		std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();

		msg << timeNow;
		Send(msg);
	}

	void MessageAll()
	{
		olc::net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::MessageAll;
		Send(msg);
	}

	void MessageServer(const std::string& stringMsg)
	{
		olc::net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::MessageServer;
		std::string msgToSend = sessionToken + ';' + stringMsg;
		std::vector<char> vec(msgToSend.begin(), msgToSend.end());
		for (int i = 0; i < vec.size(); ++i)
		{
			msg << vec[i];
		}
		Send(msg);
	}

	void addBookCommand(std::string& stringMsg)
	{
		olc::net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::AddBook;
		std::string msgToSend = sessionToken + ';' + stringMsg;
		std::vector<char> vec(msgToSend.begin(), msgToSend.end());
		for (int i = 0; i < vec.size(); ++i)
		{
			msg << vec[i];
		}
		Send(msg);
	}

	void addMemberCommand(std::string& stringMsg)
	{
		olc::net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::AddMember;
		std::string msgToSend = sessionToken + ';' + stringMsg;
		std::vector<char> vec(msgToSend.begin(), msgToSend.end());
		for (int i = 0; i < vec.size(); ++i)
		{
			msg << vec[i];
		}
		Send(msg);
	}

	void listBooks()
	{
		olc::net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::ListBooks;
		std::string msgToSend = sessionToken + ';';
		std::vector<char> vec(msgToSend.begin(), msgToSend.end());
		for (int i = 0; i < vec.size(); ++i)
		{
			msg << vec[i];
		}
		Send(msg);
	}

	void listMembers()
	{
		olc::net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::ListMembers;
		std::string msgToSend = sessionToken + ';';
		std::vector<char> vec(msgToSend.begin(), msgToSend.end());
		for (int i = 0; i < vec.size(); ++i)
		{
			msg << vec[i];
		}
		Send(msg);
	}

	void readInput()
	{
		std::string input;
		while (true)
		{
			std::getline(std::cin, input);
			inputQueue.push_back(input);
		}
	}

	// Sends username and password in a format like this: username;password;.
	void handleLogin(std::string& inputUsername, std::string& inputPassword)
	{
		olc::net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::Login;
		std::vector<char> uvec(inputUsername.begin(), inputUsername.end());
		for (int i = 0; i < uvec.size(); ++i)
		{
			msg << uvec[i];
		}
		msg << ';';
		std::vector<char> pvec(inputPassword.begin(), inputPassword.end());
		for (int i = 0; i < pvec.size(); ++i)
		{
			msg << pvec[i];
		}
		msg << ';';
		Send(msg);
	}

	// For the login response
	std::string waitForResponse(CustomClient& c)
	{
		std::string output, validationMsg, token;  // Output is the whole message. Seperate into validationMsg and token. ';' seperates them.

		int i = 0;
		while (i < 5)
		{
			while (!c.Incoming().empty())
			{
				auto msg = c.Incoming().pop_front().msg;  // "Moves" the msg from the queue into a new msg variable to use here.

				if (msg.header.id == CustomMsgTypes::Login)  // Check what type of msg was received.
				{
					// Server has responded to a login request	
					char ch;
					while (msg.body.size() > 0)
					{
						msg >> ch;
						output = ch + output;  // Prepend the character to the string.
					}
					std::cout << "[Server message]: " << output << "\n";

					std::stringstream ss(output);
					std::getline(ss, validationMsg, ';');
					std::getline(ss, token);

					if (validationMsg == "Approval")  // If server sends an approval to client.
					{
						c.isLoggedIn = true;
						c.sessionToken = token;
						i = 5;
					}
				}
				if (c.Incoming().empty()) {
					return output;
				}
			}
			++i;
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		return output;
	}

	// For any message, but doesn't check what kind of message.
	std::string getMessage(CustomClient& c) {
		std::string response;
		if (!c.Incoming().empty()) {
			auto msg = c.Incoming().pop_front().msg;
			char ch;
			while (msg.body.size() > 0)
			{
				msg >> ch;
				response = ch + response;  // Prepend the character to the string
			}
		}
		return response;
	}

private:
	std::string sessionToken;

};