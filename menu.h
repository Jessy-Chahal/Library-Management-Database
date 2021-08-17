/*
    Contains functions to display stuff to the screen using ncurses.
    Also contains functions to format strings/book data
*/

#ifndef MENU_H
#define MENU_H

#include "allIncludes.h"

class Book;
class MenuDeletedException{};

class Menu
{
private:
    float yMax, xMax;

    std::vector<int> get_rows(const std::vector<std::string> &items, int max_width);

    int draw_start_menu(std::vector<std::string> choices, float x_padding,
                        WINDOW *choice_win, bool from_bottom);


    std::string get_formatted_string(const Book &a_book);
    std::vector<std::string> text_wrap(std::string s, int max_len, bool column);

public:
    Menu();

    void resize_event();
    
    int create_menu(std::vector<std::string> &choices, std::string title, float x_padding,
                    bool box_on);

    std::string get_input(std::string title, bool hide);

    int display_books(const std::vector<Book> &books);

    bool confirm(std::string prompt, std::string yes_text, std::string no_text);

    ~Menu();
};

#endif