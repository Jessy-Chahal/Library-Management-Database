#include "book.h"

// constructor
Book::Book(std::string a_name, std::string an_author, long long an_isbn, std::string a_genre): 
the_author(an_author), the_name(a_name), the_isbn(an_isbn), the_genre(a_genre)
{
    is_deleted = false;
}

// getters
std::string Book::name() const{return the_name;}
std::string Book::author() const{return the_author;}
long long Book::isbn() const{return the_isbn;}
std::string Book::genre() const{return the_genre;}
bool Book::deleted() const{return is_deleted;}

// setters
void Book::set_name(const std::string &a_name){the_name = a_name;}
void Book::set_author(const std::string &an_author){the_author = an_author;}
void Book::set_isbn(const long long &an_isbn){the_isbn = an_isbn;}
void Book::set_genre(const std::string &a_genre){the_genre = a_genre;}
void Book::delete_book(){is_deleted = true;}

bool Book::operator ==(const Book &lhs)
{
    if(lhs.deleted() || is_deleted)
    {
        return false;
    }
    if(lhs.name() != the_name)
    {
        return false;
    }
    if(lhs.author() !=  the_author)
    {
        return false;
    }
    if(lhs.isbn() != the_isbn)
    {
        return false;
    }
    if(lhs.genre() != the_genre)
    {
        return false;
    }
    return true;
}
