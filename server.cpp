// =============================
// server.cpp
// Simple demo server: register/login/logout/list online users, get peer port for P2P chat
// Assumptions: localhost only, no encryption, minimal protocol.
// Build: g++ -std=c++17 -pthread server.cpp -o server
// Run:   ./server 8888
// =============================

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

using namespace std;

struct User {
    string password;
    int port = -1;
    bool online = false;
};

unordered_map<string, User> users;
pthread_mutex_t users_mtx = PTHREAD_MUTEX_INITIALIZER;

static void send_line(int fd, const string &s) {
    string line = s + "\n";
    // TODO: Use send() to send 'line'
    send(fd, line.data(),line.size(), 0);
}

static bool recv_line(int fd, string &out) {
    out.clear();
    // TODO: implement recv() loop until '\n'
    int n;
    char buf[1];
    while(1){
        n = recv(fd, buf, 1, 0);
        if(n <= 0){
            return false;
        }
        if(buf[0] == '\n'){
            break;
        }
        out += buf[0];
    }
    
    return true;
}

void* handle_client(void* arg) {
    int fd = *(int*)arg;
    delete (int*)arg; 

    string line;
    while (recv_line(fd, line)) {
        istringstream iss(line);
        string cmd; iss >> cmd;

        if (cmd == "REGISTER") {
            string id, pw; iss >> id >> pw;
            bool regisre = false;

            pthread_mutex_lock(&users_mtx);
            // TODO: implement REGISTER logic
            
            for(auto it = users.begin(); it != users.end(); it++){
                if(it->first == id){
                    send_line(fd, "Register Repeatedly!");
                    regisre = true;
                    break;
                }
            }
            if(!regisre){
                users[id].password = pw;
                send_line(fd, "User Register Successfully!"); 
            }
            pthread_mutex_unlock(&users_mtx);
            
        } 
        else if (cmd == "LOGIN") {
            string id, pw; int port; iss >> id >> pw >> port;
            bool portre = false;

            pthread_mutex_lock(&users_mtx);
            // TODO: implement LOGIN logic

            auto it = users.find(id);
            if(it == users.end()){
                send_line(fd, "Login User Not Found!");
                pthread_mutex_unlock(&users_mtx);
                continue;
            }else if(users[id].password != pw){ 
                send_line(fd, "Password Error!");
                pthread_mutex_unlock(&users_mtx);
                continue;
            }
            for(auto it = users.begin(); it != users.end(); it++){
                if(port == it->second.port){
                    send_line(fd, "Port Repeatedly!");
                    portre = true;
                    break;
                }
            }
            if(!portre){ 
                if(!users[id].online){
                    users[id].port = port;
                    users[id].online = true;
                    send_line(fd, "User Login Successfully!");
                }else{
                    send_line(fd, "User Login Repeatedly!");
                }        
            }
            pthread_mutex_unlock(&users_mtx);
            
        } 
        else if (cmd == "LOGOUT") {
            string id; iss >> id;

            pthread_mutex_lock(&users_mtx);
            // TODO: implement LOGOUT logic

            if(id.empty()){
                send_line(fd, "User Not Login!");
            }else{
                users[id].online = false;
                users[id].port = -1;
                send_line(fd, "User Logout Successfully!");     
            }
            pthread_mutex_unlock(&users_mtx);
            
        } 
        else if (cmd == "LIST") {
            bool online = false;

            pthread_mutex_lock(&users_mtx);
            // TODO: implement LIST logic

            cout << "==============================" << endl;
            for(auto it = users.begin(); it != users.end(); it++){
                if(it->second.online){
                    online = true;
                    cout << "Name: " << it->first << " is at port : " << it->second.port << endl;
                }
            }
            if(!online){
                cout << "No one is online right now!" << endl;
            }   
            cout << "==============================" << endl;
            send_line(fd, "List Successfully!");
            pthread_mutex_unlock(&users_mtx);
            
        } 
        else if (cmd == "QUIT"){
            string id; iss >> id;
            
            pthread_mutex_lock(&users_mtx);
            // TODO: implement LOGOUT logic

            if(!id.empty()){
                users[id].online = false;
                users[id].port = -1;
                send_line(fd, "User Quit, and Logout Automatically!");        
            }else{
                send_line(fd, "User Quit, GoodBye!");
            }
            pthread_mutex_unlock(&users_mtx);

        }
        else {
            send_line(fd, "ERROR UnknownCmd");
        }
    }

    close(fd);
    pthread_exit(nullptr);
}

int main(int argc, char **argv) {
    int port = (argc >= 2) ? stoi(argv[1]) : 8888;

    int listenfd;
    sockaddr_in addr{}, client_addr;
    /*
    TODO:
    1. Create a TCP socket 
    2. Bind the socket to the given port
    3. Start listening for connections
    */
    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        cerr << "socket" << endl;
        return -1;
    }

    int opt = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        cerr << "setsockopt" << endl;
        close(listenfd);
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        cerr << "bind" << endl;
        close(listenfd);
        return -1;
    }

    if (listen(listenfd, 10) == -1) {
        cerr << "listen" << endl;
        close(listenfd);
        return -1;
    }

    cout << "Server listening on 127.0.0.1: " << port << endl;

    fd_set mfd, rfd;
    FD_ZERO(&rfd);
    FD_ZERO(&mfd);
    FD_SET(listenfd, &mfd);

    while (true) {
        int cfd;
        // TODO: accept connection using listenfd
        // cfd = accept(listenfd, ...) 
        socklen_t addrlen = sizeof(client_addr);
        rfd = mfd;

        if(FD_ISSET(listenfd, &rfd)){
            cfd = accept(listenfd, (struct sockaddr*)&client_addr, &addrlen);
            if (cfd == -1) {
                cerr << "accept" << endl;
                continue;
            }
            pthread_t tid;
            int *arg = new int(cfd);
            pthread_create(&tid, nullptr, handle_client, arg);
            pthread_detach(tid);

        }

    }

    close(listenfd);
    
    return 0;
}