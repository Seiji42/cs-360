#pragma once

#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <string>

using namespace std;

class Client {
public:
    Client(bool);
    ~Client();

    void run();

protected:
    virtual void create();
    virtual void close_socket();
    void get_user_command();
    bool send_request(string);
    bool get_response();

    int server_;
    int buflen_;
    char* buf_;
    bool debug_;
};
