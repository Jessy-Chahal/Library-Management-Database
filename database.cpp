/*
    References:
    - isbn get range
    https://stackoverflow.com/questions/46098389/get-all-vector-elements-greater-than-a-value-c
*/

#include "database.h"

Database::Database(std::string filename) : save_filename(filename)
{
    main_menu = new Menu();
    get_saved_data();
}

Database::~Database()
{
    delete main_menu;
    output_saved_data();
}

// used in lambda statements to insert book at correct position
template<typename T>
bool Database::compare(const T &a, const T &b)
{
	T temp_a = a;
	T temp_b = b;
	std::transform(temp_a.begin(), temp_a.end(), temp_a.begin(),
    [](unsigned char c){ return std::tolower(c); });
	std::transform(temp_b.begin(), temp_b.end(), temp_b.begin(),
    [](unsigned char c){ return std::tolower(c); });

	return temp_a < temp_b;
}

// splits string at char, returns vector
std::vector<std::string> split_string(std::string content, char sep)
{
    std::vector<std::string> split_contents;

    int found = content.find(sep);
    while (found != std::string::npos)
    {
        split_contents.push_back(content.substr(0, found));
        content.erase(0, found + 1);

        found = content.find(sep);
    }

    return split_contents;
}

// read data from txt file, create Book objects and store in all_books vector
void Database::get_saved_data()
{
    std::ifstream file(save_filename);
    while (true)
    {
        std::string content;
        getline(file, content);
        if (file.fail())
        {
            break;
        }

        // get data from line and create new book obj
        std::vector<std::string> split_str = split_string(content, '@');

        Book new_book(split_str.at(0), split_str.at(1), stoll(split_str.at(2)),
                      split_str.at(3));

        add_book(new_book);
    }
    file.close();
}

// add book to vector and index to sorted vectors
void Database::add_book(const Book &book)
{
    all_books.push_back(book);
    int indx = all_books.size() - 1;

    auto pos = lower_bound(name_sorted_books.begin(), name_sorted_books.end(), indx,
                            [&](const int a, const int b) { 
                                return Database::compare(all_books.at(a).name(), all_books.at(b).name()); });
    name_sorted_books.insert(pos, indx);

    pos = lower_bound(author_sorted_books.begin(), author_sorted_books.end(), indx,
                        [&](const int a, const int b) { 
                            return Database::compare(all_books.at(a).author(), all_books.at(b).author()); });
    author_sorted_books.insert(pos, indx);

    pos = lower_bound(isbn_sorted_books.begin(), isbn_sorted_books.end(), indx,
                        [&](const int a, const int b) { 
                            return (all_books.at(a).isbn() < all_books.at(b).isbn()); });
    isbn_sorted_books.insert(pos, indx);

    pos = lower_bound(genre_sorted_books.begin(), genre_sorted_books.end(), indx,
                        [&](const int a, const int b) { 
                            return Database::compare(all_books.at(a).genre(), all_books.at(b).genre()); });
    genre_sorted_books.insert(pos, indx);
}

// prompts user to enter new book data, then creates a book
void Database::add()
{
    std::string name, author, genre, isbn;
    bool all_digits = false;
    std::string prompt;
    
    // get data from user
    do
    {
        prompt = "Please enter the book title";
        name = main_menu->get_input(&prompt.at(0), false);

        // check for duplicate
        std::vector<Book> matches = get_books(name, "name", name_sorted_books, true);
        if (matches.size() > 0)
        {
            std::string prompt = "Title found in database. Is the author ";
            prompt += matches.at(0).author();
            bool check = main_menu->confirm(prompt, "Yes", "No");
			if (check)
			{
				main_menu->confirm("Please enter another book.", "ok", "");
				name = "";
			}
        }
    } while (name == "");

    do
    {
        prompt = "Please enter the book author";
        author = main_menu->get_input(&prompt.at(0), false);

        // check for duplicate
        std::vector<Book> matches = get_books(author, "author", author_sorted_books, true);
        if (matches.size() > 0 && matches.at(0).name() == name)
        {
            main_menu->confirm("That book is already in our database.", "ok", "");
            return;
        }
    } while (author == "");

    do
    {
        prompt = "Please enter the book Isbn";
        isbn = main_menu->get_input(&prompt.at(0), false);
        all_digits = all_of(isbn.begin(), isbn.end(), ::isdigit);

        if (!all_digits || isbn.length() != 13)
        {
            main_menu->confirm("Please enter a number of 13 digits", "ok", "");
            continue;
        }
    } while (isbn == "" || !all_digits || isbn.length() != 13);
    long long int_isbn = stoll(isbn);

    do
    {
        prompt = "Please enter the book genre";
        genre = main_menu->get_input(&prompt.at(0), false);
    } while (genre == "");

    Book new_book(name, author, int_isbn, genre);
    add_book(new_book);

    main_menu->confirm("Book has been successfully added!", "ok", "");
}

// prompts user to choose an attribute, enter a key,
// then displays matching books in a menu (used to delete and edit books)
int Database::search()
{
    std::string title = "Please choose the attribute to search with";
    std::vector<std::string> s = {"1. Title ", "2. Author", "3. Isbn  ", "4. Genre "};

    int choice;
    do
    {
        choice = main_menu->create_menu(s, title, 0.02, true);
    } while (choice == -2);

    std::vector<Book> result;
    std::vector<int> indicies;
    bool search_range = false;
    
    // isbn search in range
    if (choice == 2)
    {
        search_range = main_menu->confirm("Search in a range or a specific number?",
                                        "Range", "Number");
        if (search_range)
        {
            // get range
            long long low;
            while (true)
            {
                std::string low_str = main_menu->get_input("Please enter the range's lowest value",
                                                        false);
                if (all_of(low_str.begin(), low_str.end(), ::isdigit) && low_str.length() > 0 &&
                    low_str.length() <= 13)
                {
                    low = stoll(low_str);
                    break;
                }
                main_menu->confirm("Please enter a number of 13 digits or lower", "ok", "");
            }
            long long high;
            while (true)
            {
                std::string high_str = main_menu->get_input(
                                                  "Please enter the range's highest value",
                                                  false);
                if (all_of(high_str.begin(), high_str.end(), ::isdigit) &&
                    high_str.length() > 0 && high_str.length() <= 13)
                {
                    high = stoll(high_str);
                    break;
                }
                main_menu->confirm("Please enter a number of 13 digits or lower", "ok", "");
            }
            // get books
            copy_if(isbn_sorted_books.begin(), isbn_sorted_books.end(),
                    std::back_inserter(indicies),
                    [&](int n) { return all_books.at(n).isbn() >= low &&
                                        all_books.at(n).isbn() <= high; });
            for (int i : indicies)
            {
                result.push_back(all_books.at(i));
            }
        }
    }
    if (!search_range)
    {
        s = {"1. Substring match", "2. Exact match"};
        int exact;
        do
        {
            exact = main_menu->create_menu(s, " ", 0.02, true);
        } while (exact == -2);

        // get key
        std::string key;
        bool conditions_satisfied = true;
        do
        {
            std::string prompt = "Please enter a search term";
            key = main_menu->get_input(&prompt.at(0), false);

            // if isbn check that its a digit of length 13
            if (choice == 2)
            {
                conditions_satisfied = all_of(key.begin(), key.end(), ::isdigit);
                conditions_satisfied = key.length() == 13 ? conditions_satisfied : false;

                if (!conditions_satisfied && exact)
                {
                    main_menu->confirm("Please enter a number of 13 digits", "ok", "");
                    continue;
                }
            }
        } while (key == "");

        //search proper vector
        switch (choice)
        {
            case 0:
                result = get_books(key, "name", name_sorted_books, exact);
                break;
            case 1:
                result = get_books(key, "author", author_sorted_books, exact);
                break;
            case 2:
                result = get_books(key, "isbn", isbn_sorted_books, exact);
                break;
            case 3:
                result = get_books(key, "genre", genre_sorted_books, exact);
                break;
        }
    }

    if (result.size() == 0)
    {
        main_menu->confirm("No matches", "ok", "");
        return -1;
    }

    choice = main_menu->display_books(result);
    return get_index(all_books, result.at(choice));
}

// searches for substring match
std::vector<Book> Database::get_books(std::string key, std::string attribute,
                                      const std::vector<int> &vec, bool exact) const
{
    std::vector<Book> results;
    std::transform(key.begin(), key.end(), key.begin(), [](char c) { return std::tolower(c); });

    // check each book in database with key as a substring
    for (int a_book_indx : vec)
    {
        std::string check = all_books.at(a_book_indx).name();

        if (attribute == "author")
        {
            check = all_books.at(a_book_indx).author();
        }
        else if (attribute == "isbn")
        {
            check = std::to_string(all_books.at(a_book_indx).isbn());
        }
        else if (attribute == "genre")
        {
            check = all_books.at(a_book_indx).genre();
        }

        std::transform(check.begin(), check.end(), check.begin(), 
                       [](char c) { return std::tolower(c); });

        // return only exact matches
        if (exact)
        {
            if (check == key)
            {
                results.push_back(all_books.at(a_book_indx));
            }
            continue;
        }
        // return substring matches
        if (check.find(key) != std::string::npos)
        {
            results.push_back(all_books.at(a_book_indx));
        }
    }
    return results;
}

// converts book data into single saveable string
std::string Database::format_book(const Book &book, const std::string sep)
{
    std::string result;
    for (std::string str : {book.name(), book.author(),
         std::to_string(book.isbn()), book.genre()})
    {
        result += str + sep;
    }
    return result;
}

// saves data to file
void Database::output_saved_data()
{
    std::ofstream file;
    file.open(save_filename);
    for (Book book : all_books)
    {
        if (book.deleted()) // ignore deleted books
        {
            continue;
        }
        std::string data = format_book(book, "@");
        file << data << "\n";
    }
    file.close();
}

// displays all books
void Database::list() const
{
    // get attribute to sort by
    std::string title = "Please choose an attribute to list by";
    std::vector<std::string> s = {"1. Name", "2. Author", "3. Isbn", "4. Genre"};
    int choice;
    do
    {
        choice = main_menu->create_menu(s, title, 0.2, true);
    } while (choice == -2);

    // get ascending or descending
    s = {"1. Ascending", "2. Descending"};
    int choice_2;
    do
    {
        choice_2 = main_menu->create_menu(s, " ", 0.2, true);
    } while (choice_2 == -2);

    // determine vector to display
    std::vector<int> vec;
    std::string attribute;
    switch (choice)
    {
    case 0:
        vec = name_sorted_books;
        attribute = "name";
        break;
    case 1:
        vec = author_sorted_books;
        attribute = "author";
        break;
    case 2:
        vec = isbn_sorted_books;
        attribute = "isbn";
        break;
    default:
        vec = genre_sorted_books;
        attribute = "genre";
        break;
    }

    if (choice_2) // if descending
    {
        std::reverse(vec.begin(), vec.end());
    }

    std::vector<Book> books = get_books("", attribute, vec, false);

    if (books.size() <= 0)
    {
        main_menu->confirm("No books", "ok", "");
        return;
    }

    main_menu->display_books(books);
}

void Database::remove_book(int indx)
{
    // set is_deleted to true
    all_books.at(indx).delete_book();

    // remove from all vectors
    for (std::vector<int> *vec : {&name_sorted_books, &author_sorted_books, &isbn_sorted_books,
                                    &genre_sorted_books})
    {
        vec->erase(vec->begin() + get_index(*vec, indx));
    }
}

// get index of item in vec
template<typename T>
int Database::get_index(const std::vector<T> &vec, T& item)
{
    for (int i = 0; i < vec.size(); i++)
    {
        if (item == vec.at(i))
        {
            return i;
        }
    }
    return vec.size();
}

void Database::delete_book()
{
    // get user to search for book, then confirm
    int indx = search();
    int check = main_menu->confirm("Delete " + all_books.at(indx).name() + "?", "yes", "no");

    if (!check)
    {
        return;
    }

    remove_book(indx);
    main_menu->confirm("Book deleted.", "ok", "");
}

void Database::edit()
{
    int book_indx = search();

	// get attribute to change
    std::vector<std::string> choices = {"1. Name", "2. Author", "3. Isbn", "4. Genre"};
    std::string title = "Please choose an attribute to edit";
    int choice;
    do
    {
        choice = main_menu->create_menu(choices, title, 0.2, true);
    } while (choice == -2);

	// get new value
    bool conditions_satisfied = true;
    std::string new_data;
    do
    {
        new_data = main_menu->get_input("Please enter the new attribute value", false);
        if (choice == 2)
        {
            conditions_satisfied = all_of(new_data.begin(), new_data.end(), ::isdigit);
            conditions_satisfied = new_data.length() == 13 ? conditions_satisfied : false;
        }
        if (!conditions_satisfied)
        {
            main_menu->confirm("Please enter a number of 13 digits.", "ok", "");
        }
    } while (new_data == "" || !conditions_satisfied);

    std::vector<int> *vec;
    switch (choice)
    {
    case 0:
        all_books.at(book_indx).set_name(new_data);
        vec = &name_sorted_books;
        break;
    case 1:
        all_books.at(book_indx).set_author(new_data);
        vec = &author_sorted_books;
        break;
    case 2:
        all_books.at(book_indx).set_isbn(stoll(new_data));
        vec = &isbn_sorted_books;
        break;
    case 3:
        all_books.at(book_indx).set_genre(new_data);
        vec = &genre_sorted_books;
        break;
    }

    // move new attribute to correct place in sorted vectors
    int indx = get_index(*vec, book_indx);
    (*vec).erase((*vec).begin() + indx);

	auto pos = lower_bound((*vec).begin(), (*vec).end(), book_indx,
                            [&](const int a, const int b) { 
                            return Database::compare(all_books.at(a).name(), all_books.at(b).name()); });;
	switch(choice)
	{
		case 0:
			break;
		case 1:
		    pos = lower_bound((*vec).begin(), (*vec).end(), book_indx,
                            [&](const int a, const int b) { 
                            return Database::compare(all_books.at(a).author(), all_books.at(b).author()); });
			break;
		case 2:
		    pos = lower_bound((*vec).begin(), (*vec).end(), book_indx,
                            [&](const int a, const int b) { 
                            return (all_books.at(a).isbn() < all_books.at(b).isbn()); });
			break;
		default:
		    pos = lower_bound((*vec).begin(), (*vec).end(), book_indx,
                            [&](const int a, const int b) { 
                            return Database::compare(all_books.at(a).genre(), all_books.at(b).genre()); });
			break;
	}

    (*vec).insert(pos, book_indx);

    main_menu->confirm("Book attribute updated", "ok", "");
}