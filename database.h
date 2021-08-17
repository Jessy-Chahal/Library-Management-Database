/*
	Main database modifying class
*/

#ifndef DATABASE_H
#define DATABASE_H

#include "allIncludes.h"

class Book;
class Menu;

class Database
{
private:
    std::vector<Book> all_books;

    std::vector<int> name_sorted_books;
    std::vector<int> author_sorted_books;
    std::vector<int> isbn_sorted_books;
    std::vector<int> genre_sorted_books;

    std::string save_filename;

    void get_saved_data();
    void output_saved_data();
    std::string format_book(const Book &book, const std::string sep);

    Book get_new_book() const;

    void add_book(const Book &book);

    std::vector<Book> get_books(std::string key, std::string attribute,
                                const std::vector<int> &vec, bool exact) const;

    void remove_book(int indx);

	template<typename T>
    int get_index(const std::vector<T> &vec, T& item);

	template<typename T>
	bool compare(const T &a, const T &b);

public:
    Menu *main_menu;

    Database(std::string filename);

    void add();

    int search();

    void delete_book();

    void list() const;

    void edit();

    ~Database();
};

std::vector<std::string> split_string(std::string content, char sep);

#endif