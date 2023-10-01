#include "SimpleClient.h"

int main()
{
	CustomClient c;
	c.Connect("127.0.0.1", 60000);

	bool cQuit = false;

	std::thread inputThread(&CustomClient::readInput, &c);
	inputThread.detach();

	while (!cQuit)
	{
		if (c.IsConnected())
		{

			// If you are not logged in, you don't get access to other functionalities than to log in.
			if (c.isLoggedIn == false) 
			{
				std::string inputUsername, inputPassword;
				std::cout << "Write username and password\n";
				while (c.isLoggedIn == false)
				{
					while(c.inputQueue.empty()){}
					if (!c.inputQueue.empty())
					{
						std::string input = c.inputQueue.pop_front();
						std::stringstream ss(input);
						getline(ss, inputUsername, ';');
						getline(ss, inputPassword, ';');

						c.handleLogin(inputUsername, inputPassword);
						c.waitForResponse(c);

					}
					// Check if the user is logged in
					if (c.isLoggedIn) {
						std::cout << "Login successful! Welcome, " << inputUsername << ".\n";
					}
					else
					{
						std::cout << "Login failed. Please check your username and password.\n";
					}
				}
			}

			// Else if you are logged in, you get access to more functionalities.
			else
			{
				// Handle any inputs from user in the input queue (from client to server).
				if (!c.inputQueue.empty())
				{
					std::string input = c.inputQueue.pop_front();
					std::stringstream ss(input);
					std::string command;
					std::getline(ss, command, ';');

					// Different commands means the client requests for different functionalities.
					if (command == "ADD BOOK")
					{
						std::string msgToSend;
						if (getline(ss, msgToSend))  // Read from the input into the msgToSend string.
						{
							c.addBookCommand(msgToSend);  // Function will turn the string into a vector of chars and construct the msg and send it.
							std::cout << "Book information sent.\n";
						}
						else  // If ss(input) was empty.
						{
							std::cout << "Stringstream is empty. Couldn't send book information.\n";
						}
					}

					else if (command == "ADD MEMBER")
					{
						std::string msgToSend;
						if (getline(ss, msgToSend))
						{
							c.addMemberCommand(msgToSend);
							std::cout << "Member information sent.\n";
						}
						else
						{
							std::cout << "Stringstream is empty. Couldn't send member information.\n";
						}
					}

					else if (command == "MESSAGE")
					{
						std::string msgToSend;
						if (getline(ss, msgToSend))
						{
							c.MessageServer(msgToSend);  // Function will turn the string into a vector of chars and construct the msg and send it.
							std::cout << "Message Sent.\n";
						}
						else  // If ss(input was empty.
						{
							std::cout << "There was no message to send, or there was an error reading it.\n";
						}
					}
					else if (command == "LIST BOOKS")
					{
						c.listBooks();  // Function will construct a msg of type ListBooks and send it to the server.
						std::cout << "Sent request to list books.\n";
					}
					else if (command == "LIST MEMBERS")
					{
						c.listMembers(); // Function will construct a msg of type ListMembers and send it to the server.
						std::cout << "Sent request to list members.\n";
					}
				}

				// Handle any messages in the incoming.queue (message from server).
				if (!c.Incoming().empty())
				{
					auto msg = c.Incoming().pop_front().msg;  // "Moves" the msg from the queue into a new msg variable to use here.

					switch (msg.header.id)  // Check what type of msg was received.
					{
					case CustomMsgTypes::ServerAccept:
					{
						// Server has responded to a ping request				
						std::cout << "Server Accepted Connection\n";
					}
					break;


					case CustomMsgTypes::ServerPing:
					{
						// Server has responded to a ping request
						std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
						std::chrono::system_clock::time_point timeThen;
						msg >> timeThen;
						std::cout << "Ping: " << std::chrono::duration<double>(timeNow - timeThen).count() << "\n";
					}
					break;

					case CustomMsgTypes::ServerMessage:
					{
						std::string output;
						char c;
						while (msg.body.size() > 0) {
							msg >> c;
							output = c + output;  // Prepend the character to the string
						}
						std::cout << "[Server message]: " << output << "\n";
					}
					break;
					}
				}
			}
		}
		else
		{
			std::cout << "Server Down.\n";
			cQuit = true;
		}
	}
	return 0;
}
