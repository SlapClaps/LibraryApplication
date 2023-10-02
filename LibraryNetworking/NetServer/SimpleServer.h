#include <iostream>
#include <olc_net.h>
#include <library_framework.h>
#include <openssl/rand.h>
#include <map>

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
		// Different behaviour on different message types.
		switch (msg.header.id)
		{
			// Client tries to login. Server must check and respond, to give access or not.
		case CustomMsgTypes::Login:
		{
			bool foundInMembers = false; // Flag to determine if a match is found in the members list. This is to avoid double match.
			bool foundAny = false;       // If any match is found in any list.
			std::string output;
			std::string response = "Denial;";  // Keep it like this if no match found.
			std::string token;
			char c;
			while (msg.body.size() > 0) {
				msg >> c;
				output = c + output;  // Prepend the character to the string
			}
			std::string username, password;
			std::stringstream ss(output);
			getline(ss, username, ';');
			getline(ss, password, ';');

			// Iterate through member list to find match.
			for (int i = 0; i < myLibrary.members.size(); ++i) {
				if (myLibrary.members[i].username == username && myLibrary.members[i].password == password) {
					foundInMembers = true;
					foundAny = true;
				}
			}
			// Iterate through librarian list to find match, if it didn't find it in the members list.
			if (foundInMembers == false) {
				for (int i = 0; i < myLibrary.librarians.size(); ++i) {
					if (myLibrary.librarians[i].username == username && myLibrary.librarians[i].password == password) {
						client->setClientType(olc::net::connection<CustomMsgTypes>::client::librarian);  // Set clientType to librarian if username/password matches one of the librarians.
						foundAny = true;
					}
				}
			}
			if (foundAny == true) {
				// Generate random token with OpenSSL. Not encryption, but a cryptographic-quality random number.
				unsigned char buffer[16]; // 128 bits
				RAND_bytes(buffer, sizeof(buffer));
				//token.assign(reinterpret_cast<char*>(buffer), sizeof(buffer));
				std::stringstream ss;
				ss << std::hex << std::setfill('0');
				for (int i = 0; i < sizeof(buffer); ++i) {
					ss << std::setw(2) << static_cast<unsigned int>(buffer[i]);
				}
				token = ss.str();

				client->setToken(token);
				active_sessions[token] = client.get();
				response = "Approval;" + token + ';';
				std::cout << response << std::endl;  // For debugging.
			}

			// Respond with Approval or Denial of login.
			std::vector vec(response.begin(), response.end());  // Vector of characters from the string response.
			olc::net::message<CustomMsgTypes> newMsg;
			newMsg.header.id = CustomMsgTypes::Login;
			for (int i = 0; i < vec.size(); ++i)  // This loop will add all the characters from the vector into the newMsg to send back to the client.
			{
				newMsg << vec[i];
			}
			client->Send(newMsg);  // Send the new msg.
		}
		break;

		case CustomMsgTypes::AddMember:
		{
			std::string name, memberID, username, password, output, response;
			if (client->getClientType() == olc::net::connection<CustomMsgTypes>::client::member) {
				response = "You are not a librarian and therefore cannot add member.\n";
			}
			else {
				char c;
				while (msg.body.size() > 0) {
					msg >> c;
					output = c + output;  // Prepend the character to the string
				}

				std::stringstream ss(output);
				if (getline(ss, name, ';') && getline(ss, memberID, ';') && getline(ss, username, ';') && getline(ss, password, ';')) {  // If reading was successful.
					myLibrary.addMember(name, memberID, username, password);
					response = "Member was added.\n";
				}
				else {
					response = "Couldn't add member. Check format.\n";
				}
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

		case CustomMsgTypes::ListMembers:
		{
			std::string response;
			if (client->getClientType() == olc::net::connection<CustomMsgTypes>::client::member) // If you are a member, you cannot do this.
			{
				response = "You are not a librarian, and therefore cannot list members.\n";
			}
			else                                                                                // Else if you are a librarian, you can do this.
			{
				std::cout << "[" << client->GetID() << "]: List members.\n";
				response = myLibrary.listAllMembers();
				if (response.empty())  // If the list of members are empty or there was an issue, then the response will be to inform the client of that.
				{
					response = "Either there was an issue listing the members or the list is empty.\n";
				}
				else
				{
					std::cout << "Sending list of members to client.\n";
				}
			}
			std::vector<char> vec(response.begin(), response.end());  // Vector of characters of the string with the list of members.
			olc::net::message<CustomMsgTypes> newMsg; // Construct new message
			newMsg.header.id = CustomMsgTypes::ServerMessage;
			for (int i = 0; i < vec.size(); ++i)  // Put the characters into the message body.
			{
				newMsg << vec[i];
			}
			client->Send(newMsg);
		}

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
			std::string response;  // This is the string we will send back to the client.

			if (client->getClientType() == olc::net::connection<CustomMsgTypes>::client::member) {
				response = "You are not a librarian and therefore cannot add book.\n";
			}
			else {
				std::string output;
				char c;
				while (msg.body.size() > 0) {  // While body is not empty this loop will construct a string from all the characters in it.
					msg >> c;
					output = c + output;  // Prepend the character to the string
				}
				std::stringstream ss(output);  // Move string into stringstream.
				std::string title, author, genre, isbn, publicationYearString;
				int publicationYear;

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
private:
	std::map<std::string, olc::net::connection<CustomMsgTypes>*> active_sessions; // Map to store active sessions. Key: Token, Value: Connection Pointer

};