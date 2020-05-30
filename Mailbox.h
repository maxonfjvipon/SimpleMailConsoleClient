#ifndef MAIL_MAILBOX_H
#define MAIL_MAILBOX_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <algorithm>
#include <set>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <time.h>

class Mailbox {

    std::string email, password, serverIP;
    ushort smtpPort = 25, pop3Port = 110;

public:

    Mailbox(const std::string &email, const std::string &password, const std::string &serverIP);

    void sendMessage();

    void checkInbox();

    void delOldMessages();

};


#endif //MAIL_MAILBOX_H
