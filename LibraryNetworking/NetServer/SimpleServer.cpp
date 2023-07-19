#include <iostream>
#include <olc_net.h>

enum class CustomMsgTypes : uint32_t
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
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

class Book {
public:
	Book(std::string title, std::string author, std::string genre, std::string isbn, int publicationYear)
		: title(title), author(author), genre(genre), isbn(isbn), publicationYear(publicationYear), isAvailable(true) {}

	std::string title;
	std::string author;
	std::string genre;
	std::string isbn;
	int publicationYear;
	bool isAvailable;
};

class Member {
public:
	Member(std::string name, std::string memberID)
		: name(name), memberID(memberID) {}

	std::string name;
	std::string memberID;
	std::vector<Book*> loanedBooks;
};

class Librarian {
public:
	Librarian(std::string name, std::string librarianID)
		: name(name), librarianID(librarianID) {}

	std::string name;
	std::string librarianID;
	std::vector<Book*> booksAdded;
	std::vector<Book*> booksRemoved;
};

class Library {
public:
	void addBook(const Book& book) {
		books.push_back(book);
	}

	void removeBook(const std::string& isbn) {
		for (auto it = books.begin(); it != books.end(); it++) {
			if (it->isbn == isbn) {
				books.erase(it);
				return;
			}
		}
	}

	std::string getBookInfo(const std::string& isbn) {
		for (const auto& book : books) {
			if (book.isbn == isbn) {
				std::stringstream ss;
				ss << book.title << " by " << book.author << " (ISBN: " << book.isbn << ")";
				ss << (book.isAvailable ? " - Available\n" : " - Not Available\n");
				return ss.str();
			}
		}
		return "Book not found.\n";
	}

	void loanBookToMember(const std::string& isbn, const std::string& memberID) {
		for (auto& book : books) {
			if (book.isbn == isbn && book.isAvailable) {
				for (auto& member : members) {
					if (member.memberID == memberID) {
						member.loanedBooks.push_back(&book);
						book.isAvailable = false;
						return;
					}
				}
			}
		}
	}

	void returnBookFromMember(const std::string& isbn, const std::string& memberID) {
		for (auto& member : members) {
			if (member.memberID == memberID) {
				for (auto it = member.loanedBooks.begin(); it != member.loanedBooks.end(); it++) {
					if ((*it)->isbn == isbn) {
						(*it)->isAvailable = true;
						member.loanedBooks.erase(it);
						return;
					}
				}
			}
		}
	}

	bool isBookAvailable(const std::string& isbn) {
		for (const auto& book : books) {
			if (book.isbn == isbn) {
				return book.isAvailable;
			}
		}
		return false;
	}

	void addMember(const std::string& name, const std::string& memberID) {
		members.emplace_back(name, memberID);
	}

	void removeMember(const std::string& memberID) {
		for (auto it = members.begin(); it != members.end(); it++) {
			if (it->memberID == memberID) {
				members.erase(it);
				return;
			}
		}
	}

	void addLibrarian(const std::string& name, const std::string& librarianID) {
		librarians.emplace_back(name, librarianID);
	}

	void removeLibrarian(const std::string& librarianID) {
		for (auto it = librarians.begin(); it != librarians.end(); it++) {
			if (it->librarianID == librarianID) {
				librarians.erase(it);
				return;
			}
		}
	}

	std::string listAllBooks() {
		if (books.empty()) {
			return "No books available in the library. Please add books.\n";
		}

		std::stringstream ss;  // Create StringStream object. Used to manipulate strings.
		for (const auto& book : books) {
			ss << book.title << " by " << book.author << " (ISBN: " << book.isbn << ")";  // Concatenate strings into ss.
			ss << (book.isAvailable ? " - Available\n" : " - Not Available\n");  // Concatenate more, either available or not.
		}
		return ss.str();
	}

	std::string listAllMembers() {
		if (members.empty()) {
			return "No members registered in the library. Please add members.\n";
		}

		std::stringstream ss;
		for (const auto& member : members) {
			ss << member.name << " (ID: " << member.memberID << ")\n";  // Concatenate a string with all members including newlines.
		}
		return ss.str();
	}

	std::string listAllLibrarians() {
		if (librarians.empty()) {
			return "No librarians registered in the library. Please add librarians.\n";
		}

		std::stringstream ss;
		for (const auto& librarian : librarians) {
			ss << librarian.name << " (ID: " << librarian.librarianID << ")\n";
		}
		return ss.str();
	}



private:
	std::vector<Book> books;
	std::vector<Member> members;
	std::vector<Librarian> librarians;
};

class CustomServer : public olc::net::server_interface<CustomMsgTypes>
{
public:
	CustomServer(uint16_t nPort) : olc::net::server_interface<CustomMsgTypes>(nPort)
	{

	}

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
		case CustomMsgTypes::ServerPing:
		{
			std::cout << "[" << client->GetID() << "]: Server Ping\n";

			// Simply bounce message back to client
			client->Send(msg);
		}
		break;

		case CustomMsgTypes::MessageAll:
		{
			std::cout << "[" << client->GetID() << "]: Message All\n";

			// Construct a new message and send it to all clients
			olc::net::message<CustomMsgTypes> msg;
			msg.header.id = CustomMsgTypes::ServerMessage;
			msg << client->GetID();
			MessageAllClients(msg, client);

		}
		break;

		case CustomMsgTypes::AddBook:
		{
			std::cout << "[" << client->GetID() << "]: Added book\n";


		}
		}
	}
};


int main()
{
	CustomServer server(60000);
	server.Start();

	while (1)
	{
		server.Update(-1, true);
	}



	return 0;
}