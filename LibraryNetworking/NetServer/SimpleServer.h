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
		bool isLogged = false;
		std::string received, token, response, requestData;
		// Received = whatever we received, including token. RequestData = The actual data sent by the user of client. Response = whatever we want to respond with.
		// Whatever we receive from client must have this format: "actualToken;actualData". Unless if you try to log in. Then a session-token will be generated for the client.

		if (msg.header.id == CustomMsgTypes::Login) {
			// Client tries to login. Server must check and respond, to give access or not.
			// First of all check if the client is already logged in with an account.
			for (auto& connection : active_sessions) {  // See if the connection is one of the active sessions.
				if (connection.second == client.get()) {
					response = "You are already logged in with an account.";
					isLogged = true;
				}
			}
			if (isLogged == false) {
				char c;
				while (msg.body.size() > 0) {
					msg >> c;
					received = c + received;  // Prepend the character to the string
				}
				std::stringstream ss(received);  // The whole message
				bool foundInMembers = false; // Flag to determine if a match is found in the members list. This is to avoid double match.
				bool foundAny = false;       // If any match is found in any list.
				response = "Denial;";  // Keep it like this if no match found.
				std::string username, password;
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
					ss << std::hex << std::setfill('0');
					for (int i = 0; i < sizeof(buffer); ++i) {
						ss << std::setw(2) << static_cast<unsigned int>(buffer[i]);
					}
					token = ss.str();

					client->setToken(token);
					active_sessions[token] = client.get();
					response = "Approval;" + token;  // Remember to send the token to the client and make sure the client saves it as its token.
					std::cout << response << std::endl;  // For debugging.
				}
			}
			// Respond with Approval or Denial of login.
			std::vector vec(response.begin(), response.end());  // Vector of characters from the string response.
			std::cout << response << std::endl;
			olc::net::message<CustomMsgTypes> newMsg;
			newMsg.header.id = CustomMsgTypes::Login;
			for (int i = 0; i < vec.size(); ++i)  // This loop will add all the characters from the vector into the newMsg to send back to the client.
			{
				newMsg << vec[i];
			}
			client->Send(newMsg);  // Send the new msg.
		}
		else {
			char c;
			while (msg.body.size() > 0) {
				msg >> c;
				received = c + received;  // Prepend the character to the string
			}

			std::stringstream ss(received);  // The whole message
			//std::cout << received << std::endl;

			std::getline(ss, token, ';');    // Token
			std::getline(ss, requestData);   // The actual data sent by the client-user.

			//std::cout << token << std::endl;  // SOMETHING IS WRONG HERE, CANNOT COUT TOKEN.
			//std::cout << requestData << std::endl;  // SOMETHING IS WRONG HERE, CANNOT COUT requestData.

			// Validate token
			if (active_sessions.find(token) == active_sessions.end()) {
				// Token is invalid.
				// Only handle request if it is a login request.


					// Token didn't match and user didn't request to log in.
				response = "Token is invalid, please log in.";
				std::vector vec(response.begin(), response.end());  // Vector of characters from the string response.
				olc::net::message<CustomMsgTypes> newMsg;
				newMsg.header.id = CustomMsgTypes::ServerMessage;
				for (int i = 0; i < vec.size(); ++i)  // This loop will add all the characters from the vector into the newMsg to send back to the client.
				{
					newMsg << vec[i];
				}
				client->Send(newMsg);  // Send the new msg.
				return;  // Token didn't match so don't handle any request other than login. Exit this function.

			}
			else {  // Token was valid so handle any requests received from client.
				// The previous Stringstream: "ss", is now empty because we seperated token and requestData.
				std::stringstream requestDataStream(requestData); // Make a new stringstream to extract from.
				switch (msg.header.id)
				{
				case CustomMsgTypes::AddMember:
				{
					std::string name, memberID, username, password;
					if (client->getClientType() == olc::net::connection<CustomMsgTypes>::client::member) {
						response = "You are not a librarian and therefore cannot add member.\n";
					}
					else {
						if (getline(requestDataStream, name, ';') && getline(requestDataStream, memberID, ';') &&
							getline(requestDataStream, username, ';') && getline(requestDataStream, password, ';')) {  // If reading was successful.
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
					std::cout << "[Client " << client->GetID() << " message]: " << requestData << '\n';
				}
				break;

				case CustomMsgTypes::AddBook:
				{
					if (client->getClientType() == olc::net::connection<CustomMsgTypes>::client::member) {
						response = "You are not a librarian and therefore cannot add book.\n";
					}
					else {
						std::string title, author, genre, isbn, publicationYearString;
						int publicationYear;

						if (getline(requestDataStream, title, ';') && getline(requestDataStream, author, ';') && 
							getline(requestDataStream, genre, ';') && getline(requestDataStream, isbn, ';') && 
							getline(requestDataStream, publicationYearString) && 
							(std::istringstream(publicationYearString) >> publicationYear))  // If reading was successful.
						{
							myLibrary.addBook(Book(title, author, genre, isbn, publicationYear));  // Add the book to the library, into the vector of books.
							std::cout << "[" << client->GetID() << "]: Add book: " << requestData << '\n';
							response = "Book was added.";
						}
						else  // If reading was not successful.
						{
							response = "Book couldn't be added.\n";
							std::cout << "Wrong format for ADD BOOK command." << "[" << client->GetID() << "]: " << requestData << ".\n";
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
					response = myLibrary.listAllBooks();
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
		}
	}
private:
	std::map<std::string, olc::net::connection<CustomMsgTypes>*> active_sessions; // Map to store active sessions. Key: Token, Value: Connection Pointer

};