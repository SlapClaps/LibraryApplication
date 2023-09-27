#pragma once

#include "library_common.h"


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