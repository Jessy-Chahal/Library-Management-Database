/*
	Class that handles user input
*/

#include "mainMenu.h"

int MainMenu::library_menu()
{
    Database *new_data = new Database("database.txt");
    std::vector<std::string> s = {"1. Add", "2. Search", "3. Delete", "4. Edit", "5. List",
                                  "6. Account Details", "7. Return to Menu", "8. Quit"};

    try
    {
        while (true)
        {
            int choice = new_data->main_menu->create_menu(s, "Please choose an option",
                                                          0.1, true);

            try
            {
                // call function according to user choice
                switch (choice)
                {
                case 0:
                    new_data->add();
                    break;
                case 1:
                    new_data->search();
                    break;
                case 2:
                    new_data->delete_book();
                    break;
                case 3:
                    new_data->edit();
                    break;
                case 4:
                    new_data->list();
                    break;
                case 5:
                    account_details_menu();
                    break;
                case 6:
                    delete new_data;
                    return 1;
                    break;
                case -2:
                    continue;
                default:
                    s.clear();
                    delete new_data;
                    return -1;
                }
            }
            // called when user exits with ESC
            catch (MenuDeletedException)
            {
                continue;
            }
        }
    }
    catch (MenuDeletedException)
    {
        delete new_data;
        return 1;
    }
}

// starting menu
void MainMenu::main_menu()
{
    Menu new_menu;
    std::string title = "Library Management System";
    std::string option_1 = "1. Login";
    std::string option_2 = "2. Register";
    std::string option_3 = "3. Exit";
    std::vector<std::string> s = {option_1, option_2, option_3};

    bool asking = true;
    int exit_code = 1;
    while (asking)
    {
        try
        {
            int choice = new_menu.create_menu(s, title, 0.1, true);
            try
            {
                switch (choice)
                {
                case 0:
                    exit_code = login();
                    if (exit_code == -1)
                    {
                        asking = false;
                    }
                    new_menu.resize_event();
                    break;
                case 1:
                    register_an_account();
                    break;
                case 2:
                    asking = false;
                    break;
                }
            }
            // user pressed ESC
            catch (MenuDeletedException)
            {
                continue;
            }
        }
        // user pressed ESC
        catch (MenuDeletedException)
        {
            asking = false;
        }
    }
}

int MainMenu::login()
{
    Menu login_menu;
    std::string user_name = "", password = "";
    std::string prompt;
    bool incorrect_credentials = true;

    do
    {
        prompt = "Please enter your username: ";
        user_name = login_menu.get_input(&prompt.at(0), false);
    } while (user_name == "");
    do
    {
        prompt = "Please enter your password: ";
        password = login_menu.get_input(&prompt.at(0), true);
    } while (password == "");

    std::ifstream file;
    file.open("accountDetails.txt");

    std::string search_for_username;
    std::string checkPassword;

    if (file.is_open())
    {
        while (getline(file, search_for_username))
        {
            if (search_for_username == user_name)
            {
                tempUsername = user_name;

                getline(file, checkPassword);
                if (checkPassword == password)
                {
                    incorrect_credentials = false;
                    file.close();
                    int exit_code = library_menu();
                    if (exit_code == -1)
                    {
                        file.close();
                        return -1;
                    }
                    break;
                }
            }
            else
            {
                getline(file, search_for_username);
            }
        }
    }
    file.close();

    if (incorrect_credentials == true)
    {
        login_menu.confirm("Invalid username or password. Please try again.", "ok", "");
        if (login() == -1)
        {
            return -1;
        }
        return 1;
    }
    return 1;
}

bool MainMenu::is_valid_input(std::string password)
{
    if (password.length() < 8 || password.find(" ") != std::string::npos)
        return false;

    bool found_digit = false;
    bool found_ch = false;

    for (int i = 48; i < 58; i++) // check for a digit
    {
        char ch = static_cast<char>(i);
        if (password.find(ch) != std::string::npos)
        {
            found_digit = true;
            break;
        }
    }
    if (!found_digit)
    {
        return false;
    }
    // check characters
    for (char ch : "~!@#$%^&*()-+/=:',<>|.[]{};")
    {
        if (password.find(ch) != std::string::npos)
        {
            found_ch = true;
            break;
        }
    }
    if (!found_ch)
    {
        return false;
    }

    bool found_letter = false;

    for (int i = 65; i <= 122; i++)
    {
        if (i > 90 && i < 97)
        {
            continue;
        }
        char lowerCase = static_cast<char>(i);
        if (password.find(lowerCase) != std::string::npos)
            found_letter = true;
    }

    if (!found_letter)
        return false;

    return true;
}

void MainMenu::register_an_account()
{

    Menu *registering_menu = new Menu();

    std::string user_name = "", password = "", password_verify = "";
    std::string incorrect_password_format = "";
    std::string prompt;

    incorrect_password_format = "Please note the password has to be a minimum of 8 characters.\n";
    incorrect_password_format += " Please include at least one number, letter";
    incorrect_password_format += " and special character (expl: abcdeF1@).";

    std::ifstream file;

    std::string search_for_username;

    while (user_name == "" || search_for_username == user_name)
    {
        prompt = "Please enter your username: ";
        user_name = registering_menu->get_input(&prompt.at(0), false);

        file.open("accountDetails.txt");
        if (file.is_open())
        {
            while (getline(file, search_for_username))
            {
                if (search_for_username == user_name)
                {
                    registering_menu->confirm("Username already registered. Please try again.",
                                              "ok", "");
                    user_name.clear();
                    break;
                }
            }
        }
        file.close();
    }

    registering_menu->confirm(incorrect_password_format, "ok", "");
    do
    {
        prompt = "Please enter your password: ";
        password = registering_menu->get_input(&prompt.at(0), false);

        if (is_valid_input(password) == false)
        {
            std::string prompt = "Incorrect Password Format. Please enter a valid password. \n";
            prompt += "(Minimum 8 characters, one letter, number and special character)";
            registering_menu->confirm(prompt, "ok", "");
            continue;
        }
    } while (password == "" || is_valid_input(password) == false);
    do
    {
        prompt = "Please reenter your password: ";
        password_verify = registering_menu->get_input(&prompt.at(0), false);

        if (password_verify != password)
        {
            registering_menu->confirm("Passwords do not match. Please enter again.",
                                      "ok", "");
            continue;
        }
    } while (password == "" || password_verify != password);

    std::ofstream out_file("accountDetails.txt", std::ifstream::app);

    out_file << user_name << std::endl
            << password << std::endl;

    out_file.close();

    registering_menu->confirm("Account has been created", "ok", "");
    delete registering_menu;
}

void MainMenu::update_account_details(const std::vector<std::string> v)
{
    std::ofstream out_file("accountDetails.txt");

    for (const std::string &str : v)
    {
        out_file << str << std::endl;
    }
    out_file.close();
}

void MainMenu::account_details_menu()
{

    Menu account_details_menu;

    std::string title = "Account Details";
    std::string option_1 = "1. Change Username";
    std::string option_2 = "2. Change Password";
    std::string option_3 = "3. Return to Library Menu";
    std::vector<std::string> s = {option_1, option_2, option_3};

    bool asking = true;
    while (asking)
    {
        try
        {
            int choice = account_details_menu.create_menu(s, title, 0.1, true);
            switch (choice)
            {
            case 0:
                change_username();
                break;
            case 1:
                change_password();
                break;
            case 2:
                return;
            }
        }
        catch (MenuDeletedException)
        {
            asking = false;
        }
    }
}

void MainMenu::change_password()
{
    std::ifstream file;

    std::string search_for_username, prompt, get_file_data, password = "";

    std::string user_name = tempUsername;

    int username_index = 0;

    Menu *change_password_menu = new Menu();

    std::vector<std::string> temp;
    file.open("accountDetails.txt");

    if (file.is_open())
    {
        while (getline(file, get_file_data))
            temp.push_back(get_file_data);
    }

    file.close();

    for (int i = 0; i < temp.size(); i += 2)
    {
        if (temp.at(i) == user_name)
        {
            username_index = i;
            break;
        }
    }

    do
    {
        prompt = "Please enter your new password: ";
        password = change_password_menu->get_input(&prompt.at(0), false);

        if (is_valid_input(password) == false)
        {
            std::string prompt = "Incorrect Password Format. Please enter a valid password. \n";
            prompt += "(Minimum 8 characters, one letter, number and special character)";
            change_password_menu->confirm(prompt, "ok", "");
            continue;
        }
    } while (password == "" || is_valid_input(password) == false);

    change_password_menu->confirm("Password has been updated", "ok", "");

    temp.at(username_index + 1) = password;

    update_account_details(temp);

    delete change_password_menu;
}

void MainMenu::change_username()
{
    std::ifstream file;

    std::string search_for_username, prompt, get_file_data, new_username = "";

    std::string user_name = tempUsername;

    int username_index = 0;

    Menu *change_username_menu = new Menu();

    std::vector<std::string> temp;
    file.open("accountDetails.txt");

    if (file.is_open())
    {
        while (getline(file, get_file_data))
            temp.push_back(get_file_data);
    }

    file.close();

    for (int i = 0; i < temp.size(); i += 2)
    {
        if (temp.at(i) == user_name)
        {
            username_index = i;
            break;
        }
    }

    while (true)
    {
        prompt = "Please enter your new username: ";
        new_username = change_username_menu->get_input(&prompt.at(0), false);

        if (std::find(temp.begin(), temp.end(), new_username) != temp.end())
        {
            change_username_menu->confirm("Username already exists. Please try again.", "ok", "");
            new_username.clear();
        }
        else
        {
            break;
        }
    }

    change_username_menu->confirm("Username has been updated", "ok", "");

    temp.at(username_index) = new_username;
    tempUsername = new_username;
    update_account_details(temp);

    delete change_username_menu;
}