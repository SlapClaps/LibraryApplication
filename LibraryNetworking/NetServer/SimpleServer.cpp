#include <iostream>
#include <olc_net.h>
#include <library_framework.h>

enum class CustomMsgTypes : uint32_t
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	MessageServer,
	ServerMessage,

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



class CustomServer : public olc::net::server_interface<CustomMsgTypes>
{
public:
	CustomServer(uint16_t nPort) : olc::net::server_interface<CustomMsgTypes>(nPort)
	{

	}

	Library myLibrary;

protected:
	bool OnClientConnect(std::shared_ptr<olc::net::connection<CustomMsgTypes>> client) override
	{
		olc::net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::ServerAccept;
		client->Send(msg);
		return true;
	}

	// Called when a client appears to have disconnected
	void OnClientDisconnect(std::shared_ptr<olc::net::connection<CustomMsgTypes>> client) override
	{
		std::cout << "Removing client [" << client->GetID() << "]\n";
	}

	// Called when a message arrives
	void OnMessage(std::shared_ptr<olc::net::connection<CustomMsgTypes>> client, olc::net::message<CustomMsgTypes>& msg) override
	{
		switch (msg.header.id)
		{
		case CustomMsgTypes::MessageServer:
		{
			std::string output;
			char c;
			while (msg.body.size() > 0) {
				msg >> c;
				output = c + output;  // Prepend the character to the string
			}
			std::cout << "[Client " << client->GetID() << " message]: " << output << '\n';
		}
		break;

		case CustomMsgTypes::AddBook:
		{
			std::string output;
			char c;
			while (msg.body.size() > 0) {  // While body is not empty this loop will construct a string from all the characters in it.
				msg >> c;
				output = c + output;  // Prepend the character to the string
			}
			std::stringstream ss(output);  // Move string into stringstream.
		    std::string title, author, genre, isbn, publicationYearString;
			int publicationYear;
			std::string response;  // This is the string we will send back to the client.

			if (getline(ss, title, ';') && getline(ss, author, ';') && getline(ss, genre, ';') &&
				getline(ss, isbn, ';') && getline(ss, publicationYearString) && (std::istringstream(publicationYearString) >> publicationYear))  // If reading was successful.
			{
				myLibrary.addBook(Book(title, author, genre, isbn, publicationYear));  // Add the book to the library, into the vector of books.
				std::cout << "[" << client->GetID() << "]: Add book: " << output << '\n';
				response = "Book was added.";
			}
			else  // If reading was not successful.
			{
				response = "Book couldn't be added.\n";
				std::cout << "Wrong format for ADD BOOK command." << "[" << client->GetID() << "]: " << output << ".\n";
			}
			std::vector vec(response.begin(), response.end());  // Vector of characters from the string response.
			olc::net::message<CustomMsgTypes> newMsg;
			newMsg.header.id = CustomMsgTypes::ServerMessage;
			for (int i = 0; i < vec.size(); ++i)  // This loop will add all the characters from the vector into the newMsg to send back to the client.
			{
				newMsg << vec[i];
			}
			client->Send(newMsg);  // Send the new msg.
		}
		break;

		case CustomMsgTypes::ListBooks:
		{
			std::cout << "[" << client->GetID() << "]: List books.\n";
			std::string response = myLibrary.listAllBooks();
			if (response.empty())  // If the list of books are empty or there was an issue, then the response will be to inform the client of that.
			{
				response = "Either there was an issue listing the books or the list is empty.\n";
				std::cout << response;
			}
			else
			{
				std::cout << "Sending list of books to client.\n";
			}
			std::vector<char> vec(response.begin(), response.end());  // Vector of characters of the string with the list of books.
			olc::net::message<CustomMsgTypes> newMsg; // Construct new message
			newMsg.header.id = CustomMsgTypes::ServerMessage;
			for (int i = 0; i < vec.size(); ++i)  // Put the characters into the message body.
			{
				newMsg << vec[i];
			}
			client->Send(newMsg);
		}
		break;

		case CustomMsgTypes::ServerPing:
		{
			std::cout << "[" << client->GetID() << "]: Server Ping.\n";

			// Simply bounce message back to client
			client->Send(msg);
		}
		break;

		case CustomMsgTypes::MessageAll:
		{
			std::cout << "[" << client->GetID() << "]: Message All.\n";

			// Construct a new message and send it to all clients
			olc::net::message<CustomMsgTypes> newMsg;
			newMsg.header.id = CustomMsgTypes::ServerMessage;
			newMsg << client->GetID();
			MessageAllClients(newMsg, client);

		}
		break;
		}
	}
};

int main()
{
	CustomServer server(60000);
	server.Start();
	Library myLibrary;
	while (1)
	{
		server.Update(-1, true);
	}
	return 0;
}