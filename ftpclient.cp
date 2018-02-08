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
#include <cstdlib>
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

// bool isDone(std::string sockPI){
    
// }

int change_to_passive(std::string message) {
    int passiveID=0, changes=0, result = 0;
    std::string temp;
    size_t pos;

    int ipStartPosition = message.find("(")+1;
    int ipEndPosition = message.find(")");

    std::string strReply = message.substr(ipStartPosition, ipEndPosition-ipStartPosition);

    std::string currentChar;

    int a1,a2,a3,a4,p1,p2;
    sscanf(strReply.c_str(), "%d,%d,%d,%d,%d,%d", &a1, &a2, &a3, &a4, &p1, &p2);
    //printf("This is the ip address: %d.%d.%d.%d Here is port 1: %d and here is port 2: %d\n", a1,a2,a3,a4,p1,p2);

    std::string ip_server_dtp = std::to_string(a1) + "." + std::to_string(a2) + "." + std::to_string(a3) + "." + std::to_string(a4);

    // std::string port1 = std::to_string(p1);
    // std::string port2 = std::to_string(p2);

    printf("IP Address: %s\n", ip_server_dtp.c_str());

    printf("Port 1: %d\n", p1);
    printf("Port 2: %d\n", p2);

    int test_num = p1*256+p2;
    printf("Hello: %d\n", test_num);
    
    //left shift port one by 8 bits
    p1 <<= 8;

    //concatenate port one and port two
    std::ostringstream concat;
    concat << p1 << p2;
    temp = concat.str();
    std::stringstream convert(temp);
    convert >> result;
    printf("Concat: %d\n", result);

    int sock_dtp = create_connection(ip_server_dtp,  test_num);
    return sock_dtp;

    /* Pass the response retrieved from Server PI after entering PASV
     Response would be parsed and the following would be used
     1. New IP Address
     2. (2) ports that need to concatenate

     Then use the new ip address to create a new connection
     this will give us a new sockPI
     
     At this point we will likely request from the orignial socket // request(orig_socket, ...)
     and recieve a reply from the new socket // reply(new_socket, ...)

     */
}
void downloadFile(int sock_dtp, std::string fileName)
{
    FILE * file = fopen(fileName.c_str(), "w");
    do{
        fputs(reply(sock_dtp).c_str(), file);
    }while(!feof(file));
    fclose(file);
}
int main(int argc , char *argv[])
{
    int sockpi;
    // This should be changed, make them one for each var
    int quit, uReq, status = 0;
    std::string strReply, fileName; 
    std::string::size_type sz;

    std::string default_ip = "130.179.16.134";
    int default_port = 21;

    //TODO  arg[1] can be a dns or an IP address.
    // If the argument count is greater than two: pass IP and Port to connect...
    if (argc > 2)
        sockpi = create_connection(argv[1], atoi(argv[2]));
    // If the argument count is exactly two pass IP and default Port to connect...
    if (argc == 2)
        sockpi = create_connection(argv[1], 21);
    // If the user enters no arguments, pass a defualt IP and Port to connect...
    else{
        
        sockpi = create_connection(default_ip, default_port);
    }
    // Seek a response code from server and then print the response (220 success code desired)
    strReply = reply(sockpi);
    std::cout << strReply  << std::endl;
    // Send user info to the server and print out the response
    strReply = request_reply(sockpi, "USER anonymous\r\n");
    // Isolates the status code from the rest of the message 
    status = std::stoi(strReply.substr(0,3), &sz);
    std::cout << strReply << std::endl;
    // If user is valid/ If status code is 331
    if(status == 331) {
        // Send password to server and ask for code
        strReply = request_reply(sockpi, "PASS asa@asas.com\r\n");
        // Isolate status code from rest of message
        status = std::stoi(strReply.substr(0,3), &sz);
        //std::cout << strReply << std::endl;
        // If the password was good
        if(status == 230) {
            strReply = request_reply(sockpi, "PASV\r\n");
            status = std::stoi(strReply.substr(0,3), &sz);
            int sock_dtp = change_to_passive(strReply);
            std::cout << "Before" << std::endl;
            std::cout << sock_dtp << std::endl;
            std::cout << "After" << std::endl;
            if(status==227){   //Entered Passive Mode
                while(quit==0){
                    std::cout<<"\t\t\tMAIN MENU\n"
                             <<"1. List Files\n"
                             <<"2. Retrieve a File\n"
                             <<"3. Quit\n"
                             <<"Make selection: ";

                    // uReq starts with RETR
                    std::cin>>uReq;

                    switch(uReq){
                    case 1:{
                        
                        strReply = request_reply(sockpi, "LIST\r\n");
                        status = std::stoi(strReply.substr(0,3), &sz);

                        if(status == 150){
                            strReply = reply(sock_dtp);
                            std::cout<<strReply<<std::endl;
                        }else{
                            std::cout<<"You are not be connected. Restart Application.\n";
                        }
                        break;
                        }
                    case 2: {
                        std::cout<<"Enter whole filename: ";
                        std::cin>>fileName;
                        strReply = request_reply(sockpi, "RETR"+fileName+"\r\n");
                        status = std::stoi(strReply.substr(0,3), &sz);
                        if(status == 150)
                        {  
                            downloadFile(sock_dtp, fileName);
                        }
                        break;
                        }
                    case 3: {
                        strReply = request_reply(sockpi, "QUIT\r\n");
                        status = std::stoi(strReply.substr(0,3), &sz);
                        if(status == 221)
                        quit=1;
                        break;
                        }
                    default: {
                        std::cout<<"Invalid input. Try again.\n";
                        std::cin>>uReq;
                        break;
                        }
                    }
                }
            }else{
                std::cout<<"Connnection Failed."<<std::endl;
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
