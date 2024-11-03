#include <iostream>
#include <string>
#include <unistd.h>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <chrono>

using namespace std;

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

    auto start = chrono::system_clock::now();

    for (int ii = 0; ii < 200000; ii++)
    {
        cout << "Please input: ";
        getline(cin, buf);

        if (send(sockfd, buf.c_str(), buf.length(), 0) <= 0)
        {
            cerr << "send() failed: " << strerror(errno) << endl;
            close(sockfd);
            return -1;
        }

        char recv_buf[1024] = {0};
        if (recv(sockfd, recv_buf, sizeof(recv_buf) - 1, 0) <= 0)
        {
            cerr << "recv() failed: " << strerror(errno) << endl;
            close(sockfd);
            return -1;
        }

        cout << "Received: " << recv_buf << endl;
    }

    auto end = chrono::system_clock::now();
    chrono::duration<double> elapsed_seconds = end - start;
    cout << "Elapsed time: " << elapsed_seconds.count() << "s\n";

    close(sockfd);
    return 0;
}
