#pragma once

#include "library_common.h"
#include "library_book.h"

class Librarian {
public:
	Librarian(std::string name, std::string librarianID)
		: name(name), librarianID(librarianID) {}

	std::string name;
	std::string librarianID;

	std::vector<Book*> booksAdded;
	std::vector<Book*> booksRemoved;



};