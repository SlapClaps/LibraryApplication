#pragma once

#include "library_common.h"
#include "library_book.h"


class Member {
public:
	Member(std::string name, std::string memberID)
		: name(name), memberID(memberID) {}

	std::string name;
	std::string memberID;
	std::vector<Book*> loanedBooks;

};