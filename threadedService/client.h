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
#include <sstream>
#include <vector>

using namespace std;

class Client {
public:
    Client(bool);
    ~Client();

    void run();

protected:
    virtual void create();
    virtual void close_socket();
    void get_user_request();
    bool send_request(string);
    bool get_response_by_line();
    bool get_response_by_length(int);
    string analyze_request(vector<string> &);
    string get_request_message();
    void analyze_response();

    int server_;
    int buflen_;
    char* buf_;
    bool debug_;
    string cashe_;
};
