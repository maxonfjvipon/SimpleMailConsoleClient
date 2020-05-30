#include "Mailbox.h"

int main() {
    std::string serverIP = "192.168.15.5";

    Mailbox myBox("user1@trunnikov.ru", "12345678", serverIP);

    myBox.delOldMessages();

    short choice = -1;

    std::cout << "Welcome to mail-agent" << std::endl;

    while (choice != 0) {
        do {
            std::cout << "0. Quit\n1. Send message\n2. Check inbox\n>> ";
            std::cin >> choice;
            if (choice < 0 || choice > 2) {
                std::cout << "Error: Invalid input" << std::endl;
            }
        } while (choice < 0 || choice > 2);

        switch(choice) {
            case 0:
                std::cout << "Bye..." << std::endl;
                break;
            case 1:
                myBox.sendMessage();
                break;
            case 2:
                myBox.checkInbox();
                break;
        }
    }

    return 0;
}