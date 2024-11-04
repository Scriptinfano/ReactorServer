#include <iostream>
#include <string>
#include <unistd.h>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <chrono>
#include "public.hpp"
using namespace std;
static const int BUFFERSIZE = 1024;
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cout << "Usage: ./client <IP> <port>" << endl;
        cout << "Example: ./client 192.168.150.128 5085" << endl;
        return -1;
    }

    int sockfd;
    struct sockaddr_in servaddr;
    string buf;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        cerr << "socket() failed: " << strerror(errno) << endl;
        return -1;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));

    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
    {
        cerr << "Invalid IP address: " << argv[1] << endl;
        close(sockfd);
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
    {
        cerr << "connect(" << argv[1] << ":" << argv[2] << ") failed: " << strerror(errno) << endl;
        close(sockfd);
        return -1;
    }

    cout << "Connect successful." << endl;

    while (true)
    {
        char buffer[BUFFERSIZE] = {0};
        cout << "Please input: ";
        cin.getline(buffer, sizeof(buffer));
        cout<<"是否有换行符"<<hasNewlineAtEnd(buffer);
        if (send(sockfd, buffer, sizeof(buffer), 0) <= 0)
        {
            cerr << "send() failed: " << strerror(errno) << endl;
            close(sockfd);
            return -1;
        }

        char recv_buf[BUFFERSIZE] = {0};
        if (recv(sockfd, recv_buf, sizeof(recv_buf), 0) <= 0)
        {
            cerr << "recv() failed: " << strerror(errno) << endl;
            close(sockfd);
            return -1;
        }

        cout << "Received: " << recv_buf << endl;
    }


    close(sockfd);
    return 0;
}
