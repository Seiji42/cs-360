#include "server.h"

Server::Server(bool debug) {
    // setup variables
    debug_ = debug;
    buflen_ = 1024;
    buf_ = new char[buflen_+1];
}

Server::~Server() {
    delete buf_;
}

void
Server::run() {
    if(debug_) cout << "[DEBUG] running server" << endl;
    // create and run the server
    create();
    serve();
}

void
Server::create() {
    if(debug_) cout << "[DEBUG] creating server" << endl;
}

void
Server::close_socket() {
    if(debug_) cout << "[DEBUG] closing socket" << endl;
}

void
Server::serve() {
    if(debug_) cout << "[DEBUG] serving" << endl;
    // setup client
    int client;
    struct sockaddr_in client_addr;
    socklen_t clientlen = sizeof(client_addr);

      // accept clients
    while ((client = accept(server_,(struct sockaddr *)&client_addr,&clientlen)) > 0) {

        handle(client);
    }
    close_socket();
}

void
Server::handle(int client) {
    if(debug_) cout << "[DEBUG] handle request" << endl;
    // loop to handle all requests
    while (1) {
        // get a request
        string request = get_request_by_line(client);
        // break if client is done or an error occurred
        if (request.empty())
            break;
        // send response
        string response = analyze_request(client, request);
        bool success = send_response(client,response);
        // break if an error occurred
        if (not success)
            break;
    }
    close(client);
}

string
Server::analyze_request(int client, string request) {
    if(debug_) cout << "[DEBUG] analyze request by length" << endl;
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
            messages.clear();
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
                    string message = get_request_by_length(client, length);
                    if (message.empty())
                    {
                        response = "error could not read entire message";
                    }
                    else {
                        messages[name].push_back(pair<string, string>(subject, message));
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
                if (messages.find(name) != messages.end())
                {
                    cout << "found" << endl;
                    user_messages = messages.at(name);
                }
                ostringstream oss;
                oss << "list " << user_messages.size() << endl;
                for (int i = 0; i < user_messages.size(); ++i)
                {
                    oss << (i + 1) << " " << user_messages.at(i).first << endl;
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
                    if(messages.find(name) != messages.end() && index >= 0 && index < messages.at(name).size()){
                        oss << "message " << messages.at(name).at(index).first 
                            << " " << messages.at(name).at(index).second.size() << endl
                            <<messages.at(name).at(index).second;
                        response = oss.str();
                    }
                    else {
                        response = "error no such message for that user\n";
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
        else {
            response = "error invalid request\n";
        }
        
    }
    return response;
}

string
Server::get_request_by_line(int client) {
    if(debug_) cout << "[DEBUG] getting request by line" << endl;
    // read until we get a newline
    while (cashe_.find("\n") == string::npos) {
        if(debug_) cout << "[DEBUG] entered loop" << endl;
        int nread = recv(client,buf_,1024,0);
        if(debug_) cout << "[DEBUG] nread: " << nread << endl;
        if (nread < 0) {
            if (errno == EINTR) {
                // the socket call was interrupted -- try again
                if(debug_) cout << "[DEBUG] call was interrupted" << endl;
                continue;
            }
            else {
                // an error occurred, so break out
                if(debug_) cout << "[DEBUG] error occured" << endl;
                return "";
            }
        } else if (nread == 0) {
            // the socket is closed
            if(debug_) cout << "[DEBUG] socket closed" << endl;
            return "";
        }
        // be sure to use append in case we have binary data
        cashe_.append(buf_,nread);
        if(debug_) cout << "[DEBUG] contents of cashe:\n" << cashe_ << endl;
    }
    // a better server would cut off anything after the newline and
    // save it in a cache
    int pos = cashe_.find("\n");

    string request = cashe_.substr(0, pos);
    cashe_.erase(cashe_.begin(), cashe_.begin() + (pos + 1));
    if(debug_) cout << "Request:\n" << request << endl;
    return request;
}

string
Server::get_request_by_length (int client, int length) {
    // read until we get a newline
    while (cashe_.size() < length) {
        int nread = recv(client,buf_,1024,0);
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
        cashe_.append(buf_,nread);
    }
    // a better server would cut off anything after the newline and
    // save it in a cache
    string request = cashe_.substr(0, length);
    cashe_.erase(cashe_.begin(), cashe_.begin() + length);
    if(debug_) cout << "Request:\n" << request << endl;
    return request;
}

bool
Server::send_response(int client, string response) {
    if(debug_) cout << "[DEBUG] sending response,\n" << response << endl;
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
