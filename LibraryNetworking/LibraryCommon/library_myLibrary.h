#pragma once

#include "library_common.h"
#include "library_book.h"
#include "library_member.h"
#include "library_librarian.h"

class Library {
public:
    std::vector<Book> books;
    std::vector<Member> members;
    std::vector<Librarian> librarians;

    bool isBookAvailable(const std::string& isbn) {
        for (const auto& book : books) {
            if (book.isbn == isbn) {
                return book.isAvailable;
            }
        }
        return false;
    }

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

    std::string listAllBooks() {
        if (books.empty()) {
            return "No books available in the library. Please add books.\n";
        }

        std::stringstream ss;  // Create StringStream object. Used to manipulate strings.
        for (const auto& book : books) {
            ss << book.title << " by " << book.author << ". Genre: " << book.genre << ". (ISBN: " << book.isbn << ")";  // Concatenate strings into ss.
            ss << (book.isAvailable ? " - Available.\n" : " - Not Available.\n");  // Concatenate more, either available or not.
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
};
