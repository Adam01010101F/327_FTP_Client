/**
    C++ client example using sockets.
    This programs can be compiled in linux and with minor modification in 
	   mac (mainly on the name of the headers)
    Windows requires extra lines of code and different headers
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment(lib, "Ws2_32.lib")
...
WSADATA wsaData;
iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
...
*/
#include <iostream>    //cout
#include <string>
#include <stdio.h> //printf
#include <stdlib.h>
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <sstream>
#define BUFFER_LENGTH 2048
#define WAITING_TIME 150000

const int connectID = 220;
const int userID = 331;
const int passID = 230;
const int pasvID = 227;
const int listrtID = 150;
const int quitID = 221;

int create_connection(std::string host, int port)
{
    int s;
    struct sockaddr_in socketAddress;
    
    memset(&socketAddress,0, sizeof(socketAddress));
    s = socket(AF_INET,SOCK_STREAM,0);
    socketAddress.sin_family=AF_INET;
    socketAddress.sin_port= htons(port);
    
    int a1,a2,a3,a4;
    if (sscanf(host.c_str(), "%d.%d.%d.%d", &a1, &a2, &a3, &a4 ) == 4)
    {
        std::cout << "by ip";
        socketAddress.sin_addr.s_addr =  inet_addr(host.c_str());
    }
    else {
        std::cout << "by name";
        hostent *record = gethostbyname(host.c_str());
        in_addr *addressptr = (in_addr *)record->h_addr;
        socketAddress.sin_addr = *addressptr;
    }
    if(connect(s,(struct sockaddr *)&socketAddress,sizeof(struct sockaddr))==-1)
    {
        perror("connection fail");
        exit(1);
        return -1;
    }
    return s;
}

int request(int sock, std::string message)
{
    return send(sock, message.c_str(), message.size(), 0);
}

std::string reply(int s)
{
    std::string strReply;
    int count;
    char buffer[BUFFER_LENGTH+1];
    
    usleep(WAITING_TIME);
    do {
        count = recv(s, buffer, BUFFER_LENGTH, 0);
        buffer[count] = '\0';
        strReply += buffer;
    }while (count ==  BUFFER_LENGTH);
    return strReply;
}

std::string request_reply(int s, std::string message)
{
	if (request(s, message) > 0)
    {
    	return reply(s);
	}
	return "";
}

int change_to_passive(char *argv[], int port_one, int port_two) {
    int passiveID, result = 0;
    std::string temp;
    //server DTP listens to data port
    //wait for connection

    //left shift port one by 8 bits
    port_one <<= 8;

    //concatenate port one and port two
    std::ostringstream concat;
    concat << port_one << port_two;
    temp = concat.str();
    std::stringstream convert(temp);
    convert >> result;

    //test connection
    if(create_connection(argv[1], result))
    	passiveID = pasvID;
    return passiveID;
}

int main(int argc , char *argv[])
{
    int sockpi;
    int status = 0;
    std::string strReply;
    std::string::size_type sz;

    //TODO  arg[1] can be a dns or an IP address.
    if (argc > 2)
        sockpi = create_connection(argv[1], atoi(argv[2]));
    if (argc == 2)
        sockpi = create_connection(argv[1], 21);
    else
        sockpi = create_connection("130.179.16.134", 21);
    strReply = reply(sockpi);
    std::cout << strReply  << std::endl;
    
    strReply = request_reply(sockpi, "USER anonymous\r\n");
    status = std::stoi(strReply.substr(0,3), &sz);
    std::cout << strReply << std::endl;

    if(status == 331) {
        strReply = request_reply(sockpi, "PASS asa@asas.com\r\n");
        status = std::stoi(strReply.substr(0,3), &sz);

        std::cout << strReply << std::endl;

        if(status == 230) {
            //TODO implement PASV, LIST, RETR.
            // Hint: implement a function that set the SP in passive mode and accept commands.
            int pid = change_to_passive(&argv[1],21,21);
            if(pid==227){   //Entered Passive Mode
                while(quit==0){
                    std::cout<<"Enter a command(LIST, RETRV, QUIT)"<<std::endl;
                    std::cin>>uReq;
                    switch(toupper(uReq)){
                    case LIST:
                        strReply = request_reply(sockpi, "LIST\r\n");
                        status = std::stoi(strReply.substr((0,3), $sz);
                        std::cout<<strReply<<std::endl;
                        break;
                    case RETRV:
                        strReply = request_reply(sockpi, "RETRV\r\n");
                        status = std::stoi(strReply.substr((0,3), $sz);
                        break;
                    case QUIT:
                        quit=1;
                        break;
                    DEFAULT:
                        std::cout<<"Invalid input. Try again.\n"
                        cin>>uReq;
                        break;
                    }
                }
            }
        }else {
            std::cout << "Invalid Password" << std::endl;
        }
    }
    else {
    	std::cout << "Invalid User" << std::endl;
    }
    return 0;
}
