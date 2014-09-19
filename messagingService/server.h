#pragma once

#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <string>
#include <vector>
#include <map>

#include <iostream>
#include <sstream>

using namespace std;

class Server {
public:
    Server(bool);
    ~Server();

    void run();
    
protected:
    virtual void create();
    virtual void close_socket();
    void serve();
    void handle(int);
    string analyze_request(int, string);
    string get_request_by_line(int);
    string get_request_by_length (int, int);
    bool send_response(int, string);

    int server_;
    int buflen_;
    char* buf_;
    bool debug_;
    string cashe_;
    map<string, vector <pair<string, string> > > messages;
};
