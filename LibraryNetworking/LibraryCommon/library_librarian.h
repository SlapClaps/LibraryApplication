#pragma once

#include "library_common.h"
#include "library_book.h"

class Librarian {
public:
	Librarian(std::string name, std::string librarianID, std::string username, std::string password)
		: name(name), librarianID(librarianID), username(username), password(password) {}

	std::string name;
	std::string librarianID;
	std::string username;
	std::string password;

	std::vector<Book*> booksAdded;
	std::vector<Book*> booksRemoved;

};