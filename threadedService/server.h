#pragma once

#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <pthread.h>
#include <semaphore.h>

#include <string>
#include <vector>
#include <map>
#include <queue>

#include <iostream>
#include <sstream>

using namespace std;

class Server {
public:
    Server(bool);
    ~Server();

    void run();
    void handle(int);
    sem_t * get_message_lock();
    sem_t * get_lock();
    sem_t * get_requests();
    queue <int>* get_queue();

    
protected:
    virtual void create();
    virtual void close_socket();
    void serve();
    string analyze_request(int, string,  char * &, string &);
    string get_request_by_line(int, char * &, string &);
    string get_request_by_length (int, int, char * &, string &);
    bool send_response(int, string);

    sem_t message_lock, lock, requests;
    queue<int> clients;
    map<string, vector <pair<string, string> > > messages;

    int server_;
    int buflen_;
    bool debug_;
};
