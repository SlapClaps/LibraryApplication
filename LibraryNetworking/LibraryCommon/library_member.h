#pragma once

#include "library_common.h"
#include "library_book.h"


class Member {
public:
	Member(std::string name, std::string memberID, std::string username, std::string password)
		: name(name), memberID(memberID), username(username), password(password) {}

	std::string name;
	std::string memberID;
	std::vector<Book*> loanedBooks;
	std::string username;
	std::string password;

};