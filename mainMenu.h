#ifndef MAINMENU_H
#define MAINMENU_H

#include "allIncludes.h"

class MainMenu
{
private:
    std::string tempUsername;
    int login();
    void main_screen();
    int library_menu();
    void change_password();
    void change_username();
    void output_file();
    void register_an_account();
    void account_details_menu();
    void update_account_details(const std::vector<std::string> v);
    bool is_valid_input(std::string password);
public:
    void main_menu();
};


#endif