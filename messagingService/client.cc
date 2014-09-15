#include "client.h"

Client::Client(bool debug) {
    // setup variables
    buflen_ = 1024;
    buf_ = new char[buflen_+1];
    debug_ = debug;
}

Client::~Client() {
}

void Client::run() {
    if(debug_) cout << "[DEBUG] running client" << endl;    // connect to the server and run message program
    create();
    get_user_request();
}

void
Client::create() {
    if(debug_) cout << "[DEBUG] creating socket" << endl;
}

void
Client::close_socket() {
    if(debug_) cout << "[DEBUG] closing socket" << endl;
}

void
Client::get_user_request() {
    if(debug_) cout << "[DEBUG] getting user input" << endl;

    string line;

    cout << "% ";
    while(getline(cin, line)) {
        istringstream iss (line);
        ostringstream oss;
        vector<string> tokens;
        string token;
        while (iss >> token) {
            tokens.push_back(token);
        }
        string request;
        if(!tokens.empty()) {
            request = analyze_request(tokens);
            if(request == "invalid") {
                cout << "invalid command" << endl;
            }
            else if (request == "quit") {
                break;
            }
            else {
                if(send_request(request)) {
                    get_response_by_line();
                    analyze_response();
                }
            }
        }
        cout << "% ";
    }
    close_socket();
}

string
Client::analyze_request(vector<string> & tokens) {
    string response = "invalid";
    if (tokens.at(0) == "quit")
    {
        response = tokens.at(0);
    }
    else if (tokens.at(0) == "send")
    {
        if (tokens.size() >= 3)
        {
            string message = get_request_message();
            ostringstream oss;
            oss << "put " << tokens.at(1) << " " << tokens.at(2) << 
                " " << message.size() << endl << message;
            response = oss.str();
        }
    }
    else if (tokens.at(0) == "list")
    {
        if (tokens.size() >= 2)
        {
            ostringstream oss;
            oss << "list " << tokens.at(1) << endl;
            response = oss.str();
        }
    }
    else if (tokens.at(0) == "read")
    {
        if (tokens.size() >= 3 )
        {
            istringstream iss(tokens.at(2));
            int index;
            if((iss >> index)) {
                ostringstream oss;
                oss << "get " << tokens.at(1) << " " << index << endl;
                response = oss.str();
            }
        }
    }
    return response;
}

string
Client::get_request_message() {
    cout << "- Type your message. End with a blank line -" << endl;
    string line, message;
    while (getline(cin, line)) {
        if(line.size() == 0) {
            break;
        }
        message += line + "\n";
    }
    return message;
}

bool
Client::send_request(string request) {
    if(debug_) cout << "[DEBUG] sending request:\n" << request << endl;
    // prepare to send request
    const char* ptr = request.c_str();
    int nleft = request.length();
    int nwritten;
    // loop to be sure it is all sent
    while (nleft) {
        if ((nwritten = send(server_, ptr, nleft, 0)) < 0) {
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

bool
Client::get_response_by_line() {
    if(debug_) cout << "[DEBUG] getting response by line" << endl;
    // read until we get a newline
    while (cashe_.find("\n") == string::npos) {
        int nread = recv(server_,buf_,1024,0);
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
    return true;
}

void
Client::analyze_response() {
    int pos = cashe_.find("\n");

    string response = cashe_.substr(0, pos);
    cashe_.erase(cashe_.begin(), cashe_.begin() + (pos + 1));
    
    istringstream iss(response);

    vector<string> tokens;
    string token;
    while (iss >> token) {
        tokens.push_back(token);
    }

    if(tokens.at(0) == "error") {
        if(debug_) cout << "[DEBUG] analyzing error response" << endl;
        cout << response << endl;
    }
    else if(tokens.at(0) == "list") {
        if(debug_) cout << "[DEBUG] analyzing list response" << endl;
        int number;
        iss.clear();
        iss.str(tokens.at(1));
        if(iss >> number) {
            for (int i = 0; i < number; ++i)
            {
                get_response_by_line();
                analyze_response();
            }
        }
    }
    else if(tokens.at(0) == "message") {
        if(debug_) cout << "[DEBUG] analyzing message response" << endl;
        cout << tokens.at(1) << endl;
        int number;
        iss.clear();
        iss.str(tokens.at(2));
        if(iss >> number) {
            get_response_by_length(number);
            cout << cashe_.substr(0, number);
            cashe_.erase(cashe_.begin(), cashe_.begin() + number);
        }
    }
    else if(isdigit(tokens.at(0).at(0)) != 0) {
        cout << atoi(tokens.at(0).c_str()) << " " << tokens.at(1) << endl;
    }
}

bool
Client::get_response_by_length(int length) {
    if(debug_) cout << "[DEBUG] getting response by length " << length << endl;

    while(cashe_.size() < length) {
        int nread = recv(server_,buf_,1024,0);
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
    return true;
}