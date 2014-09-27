#include "server.h"

void * consume(void *);

Server::Server(bool debug) {
    // setup variables
    debug_ = debug;
    buflen_ = 1024;
}

Server::~Server() {
}

void
Server::run() {
    if(debug_) cout << "[DEBUG] running server\n";
    // create and run the server
    create();
    sem_init(&message_lock, 0, 1);
    sem_init(&lock, 0, 0);
    sem_init(&requests, 0, 10);

    int const ARRAY_SIZE = 10;
    pthread_t threads[ARRAY_SIZE];
    for(int i = 0; i < ARRAY_SIZE; ++i) {
        pthread_create(&threads[i], NULL, &consume, this);
    }

    serve();
}

void
Server::create() {
    if(debug_) cout << "[DEBUG] creating server\n";
}

void
Server::close_socket() {
    if(debug_) cout << "[DEBUG] closing socket\n";
}

void
Server::serve() {
    if(debug_) cout << "[DEBUG] serving\n";
    // setup client
    int client;
    struct sockaddr_in client_addr;
    socklen_t clientlen = sizeof(client_addr);

    while (true) {

    if(debug_) cout << "[DEBUG] waiting for request\n";
        sem_wait(&requests);
        if((client = accept(server_,(struct sockaddr *)&client_addr,&clientlen)) > 0) {
            cout << client;
            clients.push(client);
        }
        sem_post(&requests);
        sem_post(&lock);
    }
    close_socket();
}

void
Server::handle(int client) {
    if(debug_) cout << "[DEBUG] handle request\n";
    // loop to handle all requests

    char* buf = new char[buflen_+1];
    string cache;
    while (1) {
        // get a request
        string request = get_request_by_line(client, buf, cache);
        // break if client is done or an error occurred
        if (request.empty())
            break;
        // send response
        string response = analyze_request(client, request, buf, cache);
        bool success = send_response(client,response);
        // break if an error occurred
        if (not success)
            break;
    }
    delete buf;
    close(client);
}

string
Server::analyze_request(int client, string request, char * & buf, string & cache) {
    if(debug_) cout << "[DEBUG] analyze request by length\n";
    vector<string> tokens;
    string token;
    istringstream iss(request);
    while (iss >> token) {
        tokens.push_back(token);
    }

    string response;
    
    if (tokens.empty())
    {
        response = "error invalid message\n";
    }
    else {
        if (tokens.at(0) == "reset")
        {
            sem_wait(get_message_lock());
            messages.clear();
            sem_post(get_message_lock());
            response = "OK\n";
        }
        else if (tokens.at(0) == "put")
        {
            if (tokens.size() >= 4)
            {
                string name = tokens.at(1);
                string subject = tokens.at(2);
                int length;
                iss.clear();
                iss.str(tokens.at(3));
                if(iss >> length) {
                    string message = get_request_by_length(client, length, buf, cache);
                    if (message.empty())
                    {
                        response = "error could not read entire message";
                    }
                    else {
                        sem_wait(get_message_lock());
                        messages[name].push_back(pair<string, string>(subject, message));
                        sem_post(get_message_lock());
                        response = "OK\n";
                    }
                }
                else {
                    response = "error invalid length\n";
                }
            }
            else {
                response = "error invalid number of arguments\n";
            }
        }
        else if (tokens.at(0) == "list")
        { // check if name exists in map
            if (tokens.size() >= 2)
            {
                string name = tokens.at(1);
                vector< pair <string, string> > user_messages;
                sem_wait(get_message_lock());
                if (messages.find(name) != messages.end())
                {
                    user_messages = messages.at(name);
                }
                sem_post(get_message_lock());
                ostringstream oss;
                oss << "list " << user_messages.size() << "\n";
                for (int i = 0; i < user_messages.size(); ++i)
                {
                    oss << (i + 1) << " " << user_messages.at(i).first << "\n";
                }
                response = oss.str();
            }
            else {
                response = "error invalid number of arguments\n";
            }
        }
        else if (tokens.at(0) == "get")
        {
            if (tokens.size() >= 3)
            { // out of range error
                string name = tokens.at(1);
                int index;
                iss.clear();
                iss.str(tokens.at(2));
                if(iss >> index) {
                    index = index - 1;
                    ostringstream oss;
                    sem_wait(get_message_lock());
                    if(messages.find(name) != messages.end() && index >= 0 && index < messages.at(name).size()){
                        oss << "message " << messages.at(name).at(index).first 
                            << " " << messages.at(name).at(index).second.size() << "\n"
                            << messages.at(name).at(index).second;
                        response = oss.str();
                    }
                    else {
                        response = "error no such message for that user\n";
                    }
                    sem_post(get_message_lock());

                }
                else {
                    response = "error invalid length\n";
                }
            }
            else {
                response = "error invalid number of arguments\n";
            }
        }
        else {
            response = "error invalid request\n";
        }
        
    }
    return response;
}

string
Server::get_request_by_line(int client, char * & buf, string & cache) {
    if(debug_) cout << "[DEBUG] getting request by line\n";
    // read until we get a newline
    while (cache.find("\n") == string::npos) {
        if(debug_) cout << "[DEBUG] entered loop\n";
        int nread = recv(client,buf,1024,0);
        if(debug_) cout << "[DEBUG] nread: " << nread << "\n";
        if (nread < 0) {
            if (errno == EINTR) {
                // the socket call was interrupted -- try again
                if(debug_) cout << "[DEBUG] call was interrupted" << "\n";
                continue;
            }
            else {
                // an error occurred, so break out
                if(debug_) cout << "[DEBUG] error occured" << "\n";
                return "";
            }
        } else if (nread == 0) {
            // the socket is closed
            if(debug_) cout << "[DEBUG] socket closed" << "\n";
            return "";
        }
        // be sure to use append in case we have binary data
        cache.append(buf,nread);
        if(debug_) cout << "[DEBUG] contents of cache:\n" << cache << "\n";
    }
    // a better server would cut off anything after the newline and
    // save it in a cache
    int pos = cache.find("\n");

    string request = cache.substr(0, pos);
    cache.erase(cache.begin(), cache.begin() + (pos + 1));
    if(debug_) cout << "Request:\n" << request << "\n";
    return request;
}

string
Server::get_request_by_length (int client, int length, char * & buf, string & cache) {
    // read until we get a newline
    while (cache.size() < length) {
        int nread = recv(client,buf,1024,0);
        if (nread < 0) {
            if (errno == EINTR)
                // the socket call was interrupted -- try again
                continue;
            else
                // an error occurred, so break out
                return "";
        } else if (nread == 0) {
            // the socket is closed
            return "";
        }
        // be sure to use append in case we have binary data
        cache.append(buf,nread);
    }
    // a better server would cut off anything after the newline and
    // save it in a cache
    string request = cache.substr(0, length);
    cache.erase(cache.begin(), cache.begin() + length);
    if(debug_) cout << "Request:\n" << request << "\n";
    return request;
}

bool
Server::send_response(int client, string response) {
    if(debug_) cout << "[DEBUG] sending response,\n" << response << "\n";
    // prepare to send response
    const char* ptr = response.c_str();
    int nleft = response.length();
    int nwritten;
    // loop to be sure it is all sent
    while (nleft) {
        if ((nwritten = send(client, ptr, nleft, 0)) < 0) {
            if (errno == EINTR) {
                // the socket call was interrupted -- try again
                continue;
            } else {
                // an error occurred, so break out
                perror("write");
                return false;
            }
        } else if (nwritten == 0) {
            // the socket is closed
            return false;
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return true;
}

sem_t *
Server::get_message_lock() {
    return &message_lock;
}

sem_t *
Server::get_lock() {
    return &lock;
}

sem_t *
Server::get_requests() {
    return &requests;
}

queue <int>*
Server::get_queue() {
    return &clients;
}

void * consume(void * pointer) {
    Server* server = static_cast<Server*>(pointer);
    while(true){
        int item;
        sem_wait(server->get_lock());
        sem_wait(server->get_requests());
        if (server->get_queue()->size() > 0)
        {
            item = server->get_queue()->front();
            server->get_queue()->pop();
        }
        sem_post(server->get_requests());
        if(item > 0)
            server->handle(item);
    }
}