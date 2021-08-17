/*
  Class representing a single book.
*/

#ifndef BOOK_H
#define BOOK_H

#include "allIncludes.h"

class Book
{   
private:
    std::string the_author;
    std::string the_name;
    long long the_isbn;
    std::string the_genre;
    bool is_deleted;

public:

    Book(std::string a_name, std::string an_author, long long an_isbn, std::string a_genre);

    std::string name() const;
    std::string author() const;
    long long isbn() const;
    bool deleted() const;
    std::string genre() const;

    void set_name(const std::string &a_name);
    void set_author(const std::string &an_author);
    void set_isbn(const long long &an_isbn);
    void set_genre(const std::string &a_genre);
    void delete_book();

    bool operator ==(const Book &lhs);
};

#endif