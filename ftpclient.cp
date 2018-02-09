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
#include <fstream>
#include <string>
#include <iostream>

#define BUFFER_LENGTH 2048
#define WAITING_TIME 150000

const std::string connectID = "220";
const std::string userID = "331";
const std::string passID = "230";
const std::string pasvID = "227";
const std::string listrtID = "150";
const std::string quitID = "221";
const std::string closeID = "226";
const std::string cwdSuccess = "250";


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
        //std::cout << "by ip";
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

// bool isDone(std::string sockpi){
    
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

    std::string ip_server_dtp = std::to_string(a1) + "." + std::to_string(a2) + "." + std::to_string(a3) + "." + std::to_string(a4);

    int test_num = p1*256+p2;

    int sock_dtp = create_connection(ip_server_dtp,  test_num);
    return sock_dtp;
}
void downloadFile(int sock_dtp, std::string fileName)
{
    //FILE * file = fopen(fileName.c_str(), "wb");

    std::ofstream file("filename.text");
    std::string my_string = "Hello text in file\n";
    file << my_string;
}

void list_dir(int sockpi, int sock_dtp){
    // Send message to Server PI , LIST. Then parse response
    std::string strReply = request_reply(sockpi, "LIST\r\n");
    std::string status = strReply.substr(0,3);

    // If response was success code then seek reply from Server DTP
    if(status == "150"){
        strReply = reply(sock_dtp);
        std::cout<<strReply<<std::endl;
    }else{
    // If response code was not success then inform user
        std::cout<<"You are not be connected. Restart Application.\n";
    }
    // Closes the socket in order to avoid program hang, then looks for completion code
    close(sock_dtp);
    std::string test = reply(sockpi);
    if (test == closeID)
    {
        std::cout << "Success! Connection closed." << std::endl;
    }
}

int main(int argc , char *argv[])
{
    int sockpi;
    // This should be changed, make them one for each var
    int quit, uReq = 0;
    std::string status;
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
    status = strReply.substr(0,3);
    std::cout << strReply << std::endl;
    // If user is valid/ If status code is 331
    if(status == "331") {
        // Send password to server and ask for code
        strReply = request_reply(sockpi, "PASS asa@asas.com\r\n");
        // Isolate status code from rest of message
        status = strReply.substr(0,3);
        //std::cout << strReply << std::endl;
        // If the password was good
        if(status == "230") {
            
  
            if(true){   //Entered Passive Mode
                while(quit==0){
                    strReply = request_reply(sockpi, "PASV\r\n");
                    status = strReply.substr(0,3);
                    int sock_dtp = change_to_passive(strReply);
                    
                    std::cout<<"///////////MAIN MENU///////////\n"
                             <<"1. List Files\n"
                             <<"3. Change Directory\n"
                             <<"2. Retrieve a File\n"
                             <<"4. Quit\n"
                             <<"///////////////////////////////\n";
                             

                    // uReq starts with RETR
                    std::cout << "Make selection: ";
                    std::cin >>uReq;
                    std::cout << std::endl;
                    
                    switch(uReq){
                    case 1:{
                        list_dir(sockpi, sock_dtp);
                        break;

                        }
                    case 2: {
                        std::string dir_name;
                        std::cout << "Enter name of directory: ";
                        std::cin >> dir_name;
                        std::cout << std::endl;
                        strReply = request_reply(sockpi, "CWD "+dir_name+"\r\n");
                        // std::cout << strReply << std::endl;
                        
                        status = strReply.substr(0,3);
                        if (status == cwdSuccess)
                        {
                            list_dir(sockpi, sock_dtp);
                        }
                        
                        break;
                    }
                    case 3: {
                        std::cout<<"Enter whole filename: ";
                        std::cin>>fileName;
                        strReply = request_reply(sockpi, "RETR "+fileName+"\r\n");
                        status = strReply.substr(0,3);
                        std::cout << "RETR status: ";
                        std::cout << status << std::endl;
                        if(status == "150")
                        {  
                            strReply = reply(sock_dtp);
                            //std::cout << strReply << std::endl;
                        }
                        std::ofstream out(fileName);
                        out << strReply;
                        out.close();
                        close(sock_dtp);
                        std::cout << reply(sockpi) << std::endl;
                        break;
                        }
                    case 4: {
                        
                        strReply = request_reply(sockpi, "QUIT\r\n");
                        status = strReply.substr(0,3);
                        if(status == "221")
                        close(sockpi);
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
