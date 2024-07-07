#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <atomic>
#include <sstream>
#include <cstring>
#include <regex> 

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "aprsstr.h"

#define MAX_CONNECTIONS 5

std::atomic<bool> stopServer(false);

uint32_t port;
uint32_t udp_port;
char* udp_server;

void handleClient(int clientSocket) {
    char buffer[1024];
    while (!stopServer) {
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            break;
        }
        
        std::string s;
        s += buffer;

        std::smatch m;

        std::regex strRegex("^user [a-zA-Z0-9]{1,3}[0123456789][a-zA-Z0-9]{0,3}[a-zA-Z](-[a-zA-Z0-9]{1,2})?");                    
        if (regex_search(s, m, strRegex)) { 
            std::string sOut = "# logresp " + m.str(0) + " verified, server is2axudp\n";
         
            send(clientSocket, sOut.c_str() , sOut.length(), 0);
        } else {
            int cliSockDes;
            struct sockaddr_in serAddr;

            if((cliSockDes = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                perror("socket creation error...\n");
                exit(-1);
            }

            std::cout << buffer << "\n";

            char raw[255];

            serAddr.sin_family = AF_INET;
            serAddr.sin_port = htons(udp_port);
            serAddr.sin_addr.s_addr = inet_addr(udp_server);

            aprs_gencrctab();

            int rawlen = aprsstr_mon2raw(buffer, raw, APRS_MAXLEN);

            sendto(cliSockDes, raw, rawlen, 0, (struct sockaddr*)&serAddr, sizeof(serAddr));
        }
    }
    close(clientSocket);
}

void startServer(int port) {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(serverSocket, MAX_CONNECTIONS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server started on port " << port << std::endl;

    while (!stopServer) {
        if ((clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }
        
        std::thread clientThread(handleClient, clientSocket);
        clientThread.detach();  
    }

    close(serverSocket);
}

void usage(void) {
    fprintf(stderr, "Usage: is2axudp [options]\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "\t-a <Server Port> \n");
    fprintf(stderr, "\t-p <UDP Port>\n");
    fprintf(stderr, "\t-u <UDP IP>\n");
    exit(1);
}

int main(int argc, char* argv[]) {
    int opt;

    while ((opt = getopt(argc, argv, "a:p:u:")) != -1) {
        switch (opt) {
            case 'a':
                port = (uint32_t)atol(optarg);
                break;
            case 'p':
                udp_port = (uint32_t)atol(optarg);
                break;
            case 'u':
                udp_server = optarg;
                break;
            default:
               usage();
        }
    }

    std::thread serverThread(startServer, port);

    std::string input;
    std::getline(std::cin, input);
    if (input == "stop") {
        stopServer = true;
    }

    serverThread.join();

    std::cout << "Server stopped." << std::endl;

    return 0;
}
