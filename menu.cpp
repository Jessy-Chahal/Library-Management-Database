/*
    References:
    - selection menu
    https://techlister.com/linux/creating-menu-with-ncurses-in-c/1293/
    - whitespace trim
    https://www.toptip.ca/2010/03/trim-leading-or-trailing-white-spaces.html
*/

#include "menu.h"

Menu::Menu()
{
    initscr();
    start_color();
    noecho();
    cbreak();
    resize_event();
    keypad(stdscr, TRUE);
    refresh();
}

Menu::~Menu()
{
    echo();
    curs_set(1);
    endwin();
}

// reset maxY and maxX and warn when terminal size is too small
void Menu::resize_event()
{
    getmaxyx(stdscr, yMax, xMax);
    if (yMax < 20)
    {
        std::string prompt = "CAUTION. DISPLAY MAY NOT WORK CORRECTLY WITH A SMALL TERMINAL.\n";
        prompt += "PLEASE INCREASE HEIGHT FOR A BETTER EXPERIENCE.";
        confirm(prompt, "ok", "");
    }
}

// get row heights to handle multiline menu items
std::vector<int> Menu::get_rows(const std::vector<std::string> &items, int max_width)
{
    std::vector<int> rows;
    for (const std::string &item : items)
    {
        rows.push_back(item.length() / max_width + 1);
    }

    return rows;
}

// draw first items/last items of menu
int Menu::draw_start_menu(std::vector<std::string> choices, float x_padding,
                          WINDOW *choice_win, bool from_bottom)
{
    wclear(choice_win);
    std::vector<int> heights = get_rows(choices, xMax);

    // print all choices and highlight selected item
    int line_indx = 0;
    int max_line = yMax - 7;

    if (from_bottom)
    {
        line_indx = max_line - heights.at(choices.size() - 1) + 1;
        for (int i = choices.size() - 1; i >= 0; i--)
        {
            if (i == choices.size() - 1)
            {
                wattron(choice_win, A_STANDOUT);
            }
            else
            {
                wattroff(choice_win, A_STANDOUT);
            }

            if (line_indx - (heights.at(i) > 1 ? heights.at(i) - 1 : 0) < 0)
            {
                return max_line;
            }

            mvwprintw(choice_win, line_indx - (heights.at(i) > 1 ? heights.at(i) - 1 : 0),
                      xMax * x_padding, &choices.at(i).at(0));

            line_indx -= (heights.at(i) > 1 ? heights.at(i) - 1 : 0);
            line_indx -= 2;

            // scroll so first item is at top
            if (i - 1 < 0)
            {
                for (int i = line_indx; i > 0; i--)
                {
                    wscrl(choice_win, 1);
                }
                line_indx = max_line - heights.at(choices.size() - 1) - line_indx - 1;
            }
        }
    }
    else
    {
        for (int i = 0; i < choices.size(); i++)
        {
            if (i == 0)
            {
                wattron(choice_win, A_STANDOUT);
            }
            else
            {
                wattroff(choice_win, A_STANDOUT);
            }

            mvwprintw(choice_win, 2 + line_indx, xMax * x_padding, &choices.at(i).at(0));

            // if multiline
            if (heights.at(i) > 1)
            {
                line_indx += heights.at(i) - 1;
            }

            line_indx += 2;

            // when lines go out of window max size, stop drawing
            if (line_indx >= max_line)
            {
                line_indx = max_line;
                break;
            }
        }
    }

    wattroff(choice_win, A_STANDOUT);
    wrefresh(choice_win);
    refresh();
    return line_indx;
}

// creates selection menu and displays on screen. returns choice indx.
// box_on = true draws a box around the menu (not recommended for multiline menus)
int Menu::create_menu(std::vector<std::string> &choices, std::string title, float x_padding,
                      bool box_on)
{
    // make fullscreen window
    WINDOW *win = newwin(0, 0, 0, 0);
    box(win, 0, 0);
    noecho();
    keypad(win, true);
    curs_set(0);

    // print title on screen
    char *content = &title.at(0);
    mvwprintw(win, 1, xMax * x_padding, content);

    // make sub window to contain only menu options
    WINDOW *choice_win = subwin(win, yMax - 3, xMax * 0.95, 2, xMax * 0.03);
    if (box_on)
    {
        box(choice_win, 0, 0);
    }
    keypad(choice_win, true);
    scrollok(choice_win, true);
    draw_start_menu(choices, x_padding, choice_win, false);
    wrefresh(win);

    // get input
    int indx = 0;
    int line_indx = 0;
    int ch;
    const int max_line = yMax - 7;
    std::vector<int> heights = get_rows(choices, xMax);

    while ((ch = wgetch(choice_win)) != '\n')
    {
        // print previous item without highlight
        mvwprintw(choice_win, 2 + line_indx, xMax * x_padding, &choices.at(indx).at(0));

        switch (ch)
        {
            // return -2 so that items can be reformatted to fit terminal size
            case KEY_RESIZE:
                resize_event();
                return -2;

                case KEY_UP:
                // if at first item, redraw entire menu
                if (indx == 0)
                {
                    line_indx = draw_start_menu(choices, x_padding, choice_win, true);
                    line_indx = line_indx + 2 >= max_line ? 
                                max_line - 2 * heights.at(choices.size() - 1) :
                                line_indx - heights.at(choices.size() - 1) + 1;
                    indx = choices.size() - 1;
                    continue;
                }
                indx--;
                line_indx -= heights.at(indx) + 1;

                // scroll up if at top of screen
                if (line_indx < 0)
                {
                    wscrl(choice_win, -(0 - line_indx));
                    line_indx = 0;
                }
                break;

            case KEY_DOWN:
                // if at last item, redraw entire menu
                if (indx == choices.size() - 1)
                {
                    draw_start_menu(choices, x_padding, choice_win, false);
                    indx = 0;
                    line_indx = 0;
                    continue;
                }
                line_indx += heights.at(indx) + 1;
                indx++;

                // scroll down if at bottom of screen
                if (line_indx >= max_line)
                {
                    wscrl(choice_win, line_indx - max_line + heights.at(indx));
                    line_indx = max_line - heights.at(indx);
                }
                break;

            case 27:
                bool esc = confirm(" Exit?", "yes", "cancel");
                if (esc)
                {
                    throw MenuDeletedException();
                }
                break;
        }

        // draw new selected item with highlight
        wattron(choice_win, A_STANDOUT);
        mvwprintw(choice_win, 2 + line_indx, xMax * x_padding, &choices.at(indx).at(0));
        wattroff(choice_win, A_STANDOUT);
    }

    // reset terminal
    clear();
    delwin(win);
    delwin(choice_win);
    curs_set(1);
    refresh();

    return indx;
}

// draw textbox and return userinput
std::string Menu::get_input(std::string title, bool hide)
{
    char *content = &title.at(0);
    printw(content);

    WINDOW *win = newwin(3, xMax * 0.6, 1, 0);
    wborder(win, 0, 0, 0, 0, 0, 0, 0, 0);

    // move cursor into textbox and show
    wmove(win, 1, 1);
    curs_set(1);
    keypad(win, true);
    refresh();
    wrefresh(win);

    // get input
    int ch;
    int cursor_pos = 0;
    std::string input = "";
    while ((ch = getch()) != '\n' || input.length() == 0)
    {
        box(win, ' ', ' ');

		// on resize, resize textbox
        if (ch == KEY_RESIZE)
        {
            resize_event();
            wresize(win, 3, xMax * 0.6);
            box(win, 0, 0);
            wrefresh(win);
            continue;
        }

		if(ch == '\n' && input.length() == 0)
		{
			confirm("Please input something", "ok", "");
			continue;
		}

        if (ch == KEY_BACKSPACE)
        {
            // if there is char to delete
            if (input.length() > 0 && cursor_pos > 0)
            {
                // remove box to avoid display issues
                box(win, ' ', ' ');

                wmove(win, 1, cursor_pos);
                wdelch(win);

                if (cursor_pos > 0)
                {
                    cursor_pos--;
                }
                input.erase(cursor_pos, 1);

                box(win, 0, 0);
                wrefresh(win);
            }
            continue;
        }

        else if (ch == KEY_LEFT)
        {
            if (cursor_pos < 1)
            {
                continue;
            }
            wmove(win, 1, cursor_pos);
            cursor_pos--;
            wrefresh(win);
            continue;
        }

        else if (ch == KEY_RIGHT)
        {
            wmove(win, 1, cursor_pos + 1);

            if (cursor_pos < input.length())
            {
                cursor_pos++;
            }
            wrefresh(win);
            continue;
        }

        else if (ch == 27)
        {
            bool esc = confirm(" Exit?", "yes", "cancel");
            if (esc)
            {
                throw MenuDeletedException();
            }
            continue;
        }

		// limit characters
        else if (input.length() >= xMax * 0.5)
        {
            confirm("Character limit reached.", "ok", "");
            continue;
        }

        wmove(win, 1, cursor_pos + 1);
        if (hide)
        {
            waddch(win, '*');
        }
        else
        {
            waddch(win, ch);
        }

        // redraw characters after new character if cursor not at end
        if (cursor_pos != input.length() - 1)
        {
            for (int i = cursor_pos; i < input.length(); i++)
            {
                if (hide)
                {
                    waddch(win, '*');
                }
                else
                {
                    waddch(win, input.at(i));
                }
            }
        }

        wmove(win, 1, cursor_pos + 2);
        if (input.length() > 0)
        {
            input.insert(input.begin() + cursor_pos, ch);
        }
        else
        {
            input += ch;
        }
        cursor_pos++;
        
        box(win, 0, 0);
        wrefresh(win);
    }

    // reset terminal
    clear();
    delwin(win);
    curs_set(1);
    refresh();

    // trim leading/trailing whitespace
    int pos = input.find_first_not_of(" \t");
    input.erase(0, pos);
    pos = input.find_last_not_of(" \t");
    if (pos != std::string::npos)
    {
        input.erase(pos + 1, input.length());
    }

    return input;
}

// divide content into rows of max_len length.
std::vector<std::string> Menu::text_wrap(std::string s, int max_len, bool column)
{
    std::string row = "";
    std::vector<std::string> rows;
    for (char letter : s)
    {
        if ((row + letter).length() < max_len && letter != '\n')
        {
            row += letter;
            continue;
        }
        else
        {
            if (column)
            {
                row = "|" + row;
            }
            rows.push_back(row);
            if(letter != '\n')
            {
                row = letter;
            }
            else
            {
                row = "";
            }
        }
    }

    row.insert(row.end(), max_len - row.length() - 1, ' ');
    if (column)
    {
        row = "|" + row;
    }
    rows.push_back(row);

    return rows;
}

// format Book data so it fits on screen, in columns
std::string Menu::get_formatted_string(const Book &a_book)
{
    // max width for each column
    std::vector<int> col_widths = {int(xMax * 0.3), int(xMax * 0.2),
                                    int(xMax * 0.2), int(xMax * 0.2)};

    // split attributes into rows
    std::vector<std::string> name_vec = text_wrap(a_book.name(), col_widths.at(0), true);
    std::vector<std::string> author_vec = text_wrap(a_book.author(), col_widths.at(1), true);
    std::vector<std::string> isbn_vec = text_wrap(std::to_string(a_book.isbn()),
                                                    col_widths.at(2), true);
    std::vector<std::string> genre_vec = text_wrap(a_book.genre(), col_widths.at(3), true);

    std::vector<std::vector<std::string>> split_contents = {name_vec, author_vec,
                                                            isbn_vec, genre_vec};

    // limit max row to 2 (for display issues)
    for (int i = 0; i < split_contents.size(); i++)
    {
        if (split_contents.at(i).size() > 2)
        {
            split_contents.at(i).erase(split_contents.at(i).begin() + 2,
                                       split_contents.at(i).end());
        }
    }

    // get max # of rows
    int max_rows = std::max({name_vec.size(), author_vec.size(),
                            isbn_vec.size(), genre_vec.size()});
    max_rows = max_rows > 2 ? 2 : max_rows;

    std::vector<std::string> rows(max_rows, "");

    // concatenate attribute rows
    for (int i = 0; i < max_rows; i++)
    {
        for (int j = 0; j < split_contents.size(); j++)
        {
            std::string content = split_contents.at(j).size() > i ? split_contents.at(j).at(i) :
                                "|" + std::string(col_widths.at(j) - 1, ' ');
            rows.at(i) += content;
        }
    }

    // join rows with newlines
    std::string content = "";
    for (std::string str : rows)
    {
        content += "\n" + str;
    }

    // remove last newline
    content.pop_back();

    return content;
}

// display vector of Books in a menu, return selected book index
int Menu::display_books(const std::vector<Book> &books)
{
    int choice;
    do
    {
        std::vector<std::string> data;

        // get contents of each book in string form
        for (Book a_book : books)
        {
            std::string book_str = get_formatted_string(a_book);

            data.push_back(book_str);
        }

        choice = create_menu(data, "Results: ", 0.1, false);
    } while (choice == -2);

    return choice;
}

// pop up box that returns true or false
bool Menu::confirm(std::string prompt, std::string yes_text, std::string no_text)
{
    std::vector<std::string> formatted_str = {prompt};

    // text wrap prompt
    if (prompt.length() > xMax * 0.5)
    {
        formatted_str = text_wrap(prompt, xMax * 0.6, false);
    }
	else
	{
		formatted_str.at(0).insert(0, " ");
	}

    // create new window in center of terminal
    int line_len = formatted_str.at(0).length() > 10 ? formatted_str.at(0).length() : 10;
    WINDOW *win = newwin(yMax * 0.5, xMax * 0.2 + line_len, yMax * 0.25,
                        (xMax - xMax * 0.2 - line_len) / 2);

    keypad(win, true);
    curs_set(0);
    // print prompt
    for (int i = 0; i < formatted_str.size(); i++)
    {
        mvwprintw(win, 2 + i, line_len * 0.05, &formatted_str.at(i).at(0));
    }
    box(win, 0, 0);

    // check if no option is enabled
    bool choice = true;
    wattron(win, A_STANDOUT);
    if (no_text == "")
    {
        // draw yes at half screen
        mvwprintw(win, yMax * 0.4, xMax * 0.3, &yes_text.at(0));
        wattroff(win, A_STANDOUT);
    }
    else
    {
        // draw two options
        // draw yes
        mvwprintw(win, yMax * 0.4, line_len * 0.25, &yes_text.at(0));
        wattroff(win, A_STANDOUT);

        // draw no
        mvwprintw(win, yMax * 0.4, line_len, &no_text.at(0));
    }

    int ch;
    while ((ch = wgetch(win)) != '\n')
    {
        // handle terminal resize
        if (ch == KEY_RESIZE)
        {
            box(win, ' ', ' ');
            wrefresh(win);
            resize_event();
            wresize(win, yMax * 0.5, xMax * 0.2 + line_len);
            box(win, 0, 0);
            wrefresh(win);

			// if terminal size warning already displayed, delete pop up
            if (prompt.find("CAUTION") != std::string::npos)
            {
                wclear(win);
                wrefresh(win);
                delwin(win);
                refresh();
                return true;
            }
        }

        // skip if no not enabled
        if (no_text == "")
        {
            continue;
        }
        if (ch == KEY_LEFT || ch == KEY_RIGHT)
        {
            // if no was selected
            if (choice)
            {
                // yes without highlight
                wattroff(win, A_STANDOUT);
                mvwprintw(win, yMax * 0.4, line_len * 0.25, &yes_text.at(0));
                // no with highlight
                wattron(win, A_STANDOUT);
                mvwprintw(win, yMax * 0.4, line_len, &no_text.at(0));
            }
            else
            {
                // no without highlight
                wattroff(win, A_STANDOUT);
                mvwprintw(win, yMax * 0.4, line_len, &no_text.at(0));
                // yes with highlight
                wattron(win, A_STANDOUT);
                mvwprintw(win, yMax * 0.4, line_len * 0.25, &yes_text.at(0));
            }

            choice = !choice;
            wrefresh(win);
        }
    }

    // reset terminal
    wclear(win);
    wrefresh(win);
    delwin(win);
    refresh();

    return choice;
}