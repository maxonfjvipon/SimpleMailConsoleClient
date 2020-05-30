#include "Mailbox.h"

void send_request(int sock, const char *buf_request) {
    const char *buf = (char *) buf_request;    // can't do pointer arithmetic on void*
    int send_size; // size in bytes sent or -1 on error
    size_t size_left; // size left to send
    const int flags = 0;

    size_left = strlen(buf_request);
    while (size_left > 0) {
        if ((send_size = send(sock, buf, size_left, flags)) == -1) {
            std::cout << "send error: " << std::endl;
        }
        size_left -= send_size;
        buf += send_size;
    }
}

std::string get_response(int sock) {
    int recv_size; // size in bytes received or -1 on error
    const int flags = 0;
    const int size_buf = 2048;
    char *buf = new char[size_buf];

    if ((recv_size = recv(sock, buf, size_buf, flags)) == -1) {
        std::cout << "recv error: " << strerror(errno) << std::endl;
    }
    std::string str(buf, recv_size);
    delete buf;
    return str;
}

void quit(int &sock) {
    send_request(sock, std::string("QUIT\r\n").c_str());
    get_response(sock);
}

void connect(int &sock, int port, std::string &serverIP) {
    sockaddr_in address;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = inet_addr(serverIP.c_str());
    if (connect(sock, (sockaddr *) &address, sizeof(address)) < 0) {
        perror("socket");
    }
}

Mailbox::Mailbox(const std::string &email, const std::string &password,
                 const std::string &serverIP) {
    this->email = email;
    this->password = password;
    this->serverIP = serverIP;
}

void Mailbox::sendMessage() {
    int sock;
    std::string receiver, response, message;
    connect(sock, smtpPort, serverIP);
    send_request(sock, "HELO\r\n");
    get_response(sock);
    send_request(sock, ("MAIL FROM <" + email + ">\r\n").c_str());
    get_response(sock);

    bool flag;
    do {
        flag = false;
        std::cout << "Enter receiver mailbox >> ";
        std::cin >> receiver;
        send_request(sock, ("RCPT TO <" + receiver + ">\r\n").c_str());
        response = get_response(sock);
        if (response.substr(0, response.find(' ')) != "250") {
            std::cout << response;
            std::cout << "Try again";
            flag = true;
        }
    } while (flag);

    std::cout << "Enter message >> ";
    std::cin >> message;

    send_request(sock, "DATA\r\n");
    send_request(sock, (message + "\r\n").c_str());
    send_request(sock, ".\r\n");
    response = get_response(sock);
    std::cout << "response: " << response;

    if (response.substr(0, response.find(' ')) != "250") {
        std::cout << "Unknown error" << std::endl;
    }

    quit(sock);

    close(sock);
}

void Mailbox::checkInbox() {
    int sock;
    connect(sock, pop3Port, serverIP);
    get_response(sock);
    send_request(sock, ("USER " + email + "\r\n").c_str());
    get_response(sock);
    send_request(sock, ("PASS " + password + "\r\n").c_str());
    get_response(sock);

    std::string response;
    char responseFlag;
    uint count = 1;
    do {
        send_request(sock, ("RETR " + std::to_string(count) + "\r\n").c_str());
        sleep(1);
        response = get_response(sock);
        std::cout << "Response: " << response;
        responseFlag = response[0];
        if (responseFlag != '-') {
//            std::cout << get_response(sock);
            std::cout << response;
            std::cout << std::endl << std::endl;
            count++;
        }

    } while (responseFlag != '-');

    quit(sock);

    close(sock);
}

struct Date {
    int d, m, y;
};

int getMonthNumber(const std::string &month) {
    if (month == "Jan") {
        return 1;
    }
    if (month == "Feb") {
        return 2;
    }
    if (month == "Mar") {
        return 3;
    }
    if (month == "Apr") {
        return 4;
    }
    if (month == "May") {
        return 5;
    }
    if (month == "Jun") {
        return 6;
    }
    if (month == "Jul") {
        return 7;
    }
    if (month == "Aug") {
        return 8;
    }
    if (month == "Sep") {
        return 9;
    }
    if (month == "Oct") {
        return 10;
    }
    if (month == "Nov") {
        return 11;
    }
    if (month == "Dec") {
        return 12;
    }
}

void Mailbox::delOldMessages() {
    std::string someDateString("25.05.2019");
    Date someDate, messageDate;
    uint firstDate, secondDate, deletedMessagesCount = 0;
    sscanf(someDateString.c_str(), "%d.%d.%d", &someDate.d, &someDate.m, &someDate.y);

    firstDate = someDate.y * 10000 + someDate.m * 100 + someDate.d;

    int sock;
    connect(sock, pop3Port, serverIP);
    get_response(sock);
    send_request(sock, ("USER " + email + "\r\n").c_str());
    get_response(sock);
    send_request(sock, ("PASS " + password + "\r\n").c_str());
    get_response(sock);

    std::string response, date, month;
    char responseFlag;
    uint count = 1;
    size_t pos;
    do {
        send_request(sock, ("RETR " + std::to_string(count) + "\r\n").c_str());
        sleep(1);
        response = get_response(sock);
        responseFlag = response[0];
        if (responseFlag != '-') {
            pos = response.find("ESMTP; ") + 12;
            // dd mmm yyyy
            date = response.substr(pos, 11); // get date

            messageDate.d = std::stoi(date.substr(0, 2));
            messageDate.m = getMonthNumber(date.substr(3, 3));
            messageDate.y = std::stoi(date.substr(7, 4));

//            std::cout << messageDate.d << "." << messageDate.m << "." << messageDate.y << std::endl;

            secondDate = messageDate.y * 10000 + messageDate.m * 100 + messageDate.d;

            if (secondDate <= firstDate) {
                deletedMessagesCount++;
                send_request(sock, ("DELE " + std::to_string(count) + "\r\n").c_str());
                sleep(1);
                get_response(sock);
            }
            count++;
        }

    } while (responseFlag != '-');

    std::cout << "Quantity of messages which older than " << someDateString << " is "
              << deletedMessagesCount << std::endl;

    quit(sock);
    close(sock);
}
