/** \file */

/** \brief This program implements an FTP client protocol.
 *
 * The FTP client works in passive mode. This allows it to list
 * the remote directory and lets the user copy a file
 * to a user's hard drive. This client supports PASV, LIST, QUIT
 * and RETR [filename] commands.
 */

#include <iostream>
#include <cstdlib>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
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

/**
 * Allows the client to connect to a server
 * \param host the IP address used for connection
 * \param port the port the server should listen to
 * \return the value for the socket
 */

int create_connection(std::string host, int port)
{
    int s;
    struct sockaddr_in socketAddress;
    
     /**< initializes socket and its information */
    memset(&socketAddress,0, sizeof(socketAddress));
    s = socket(AF_INET,SOCK_STREAM,0);
    socketAddress.sin_family=AF_INET;
    socketAddress.sin_port= htons(port);
    
    int a1,a2,a3,a4;
    /**< adds IP or name info to socket */
    if (sscanf(host.c_str(), "%d.%d.%d.%d", &a1, &a2, &a3, &a4 ) == 4)
    {
        socketAddress.sin_addr.s_addr =  inet_addr(host.c_str());
    }
    else {
        std::cout << "by name";
        hostent *record = gethostbyname(host.c_str());
        in_addr *addressptr = (in_addr *)record->h_addr;
        socketAddress.sin_addr = *addressptr;
    }

    /**< attempts to connect using socket */
    if(connect(s,(struct sockaddr *)&socketAddress,sizeof(struct sockaddr))==-1)
    {
        perror("connection fail");
        exit(1);
        return -1;
    }
    return s;
}

/**
 * Requests a server to perform an action
 * \param sock the socket used to connect to a server
 * \param message the intended command for action
 * \return the value used to send to server
 */
int request(int sock, std::string message)
{
    return send(sock, message.c_str(), message.size(), 0);
}

/**
 * Sends a reply stating the result of the command
 * \param s the socket used to connect to a server
 * \return a message that shows the result of the command
 */

std::string reply(int s)
{
    std::string strReply;
    int count;
    char buffer[BUFFER_LENGTH+1];
    
    usleep(WAITING_TIME);

    /**< appends buffer information to a string in order to get a reply */
    do {
        count = recv(s, buffer, BUFFER_LENGTH, 0);
        buffer[count] = '\0';
        strReply += buffer;
    }while (count ==  BUFFER_LENGTH);
    return strReply;
}

/**
 * Sends a request to server to do an action
 * and notifies a user if the request is made
 * \param s the socket used to connect to a server
 * \param message the intended command for action
 * \return a message that shows the result of the command
 */

std::string request_reply(int s, std::string message)
{
	if (request(s, message) > 0)
    {
    	return reply(s);
	}
	return "";
}

/**
 * Switches from server PI to server DTP
 * \param message the information containing the IP address and intended port
 * \return the socket value used to connect to server DTP
 */

int change_to_passive(std::string message) {
    int passiveID=0, changes=0, result = 0;
    std::string temp;
    size_t pos;

    int ipStartPosition = message.find("(")+1;
    int ipEndPosition = message.find(")");

    /**< parses IP and port from message */
    std::string strReply = message.substr(ipStartPosition, ipEndPosition-ipStartPosition);

    std::string currentChar;

    int a1,a2,a3,a4,p1,p2;
    sscanf(strReply.c_str(), "%d,%d,%d,%d,%d,%d", &a1, &a2, &a3, &a4, &p1, &p2);

    std::string ip_server_dtp = std::to_string(a1) + "." + std::to_string(a2) + "." + std::to_string(a3) + "." + std::to_string(a4);

    /**< concatenates an 8-bit shifted port one with a port two */
    int test_num = p1*256+p2;

    /**< connects to server with resulting IP and port value */
    int sock_dtp = create_connection(ip_server_dtp,  test_num);
    return sock_dtp;
}

/**
 * Downloads a given file to a user's hard drive
 * \param sock_dtp the socket value used to connect to server DTP
 * \param fileName the file a user wants to download
 */

void downloadFile(int sock_dtp, std::string fileName)
{
    std::ofstream file("filename.text");
    std::string my_string = "Hello text in file\n";
    file << my_string;
}

/**
 * Lists all files in a given directory
 * \param sockpi socket for server PI
 * \param sockdtp socket for server DTP
 */
void list_dir(int sockpi, int sock_dtp){
    /**< Send message to Server PI , LIST. Then parse response */
    std::string strReply = request_reply(sockpi, "LIST\r\n");
    std::string status = strReply.substr(0,3);

    /**< if response was success code then seek reply from Server DTP */
    if(status == "150"){
        strReply = reply(sock_dtp);
        std::cout<<strReply<<std::endl;
    }else{
    /**<  If response code was not success then inform user */
        std::cout<<"You are not be connected. Restart Application.\n";
    }
    /**< Closes the socket in order to avoid program hang, then looks for completion code */
    close(sock_dtp);
    std::string test = reply(sockpi);
    if (test == closeID)
    {
        std::cout << "Success! Connection closed." << std::endl;
    }
}

/**
 * Allows a user to connect to a server and send commands to
 * it in order to get desired results
 * \param argc the argument count
 * \param argv the argument values
 */

int main(int argc , char *argv[])
{
    int sockpi;
    int quit, uReq = 0;
    std::string status;
    std::string strReply, fileName; 
    std::string::size_type sz;

    std::string default_ip = "130.179.16.134";
    int default_port = 21;

    /**< creates connection based on number of args passed in main */
    if (argc > 2)
        sockpi = create_connection(argv[1], atoi(argv[2]));
    if (argc == 2)
        sockpi = create_connection(argv[1], 21);
    else
        sockpi = create_connection(default_ip, default_port);

    /**< checks if connection is valid */
    strReply = reply(sockpi);
    std::cout << strReply  << std::endl;

    /**< sends user info to server and get response */
    strReply = request_reply(sockpi, "USER anonymous\r\n");
    status = strReply.substr(0,3);
    std::cout << strReply << std::endl;

    /**< checks if user is valid */
    if(status == "331") {
        /**< sends password to server and get response */
        strReply = request_reply(sockpi, "PASS asa@asas.com\r\n");
        status = strReply.substr(0,3);

        /**< checks if password is valid */
        if(status == "230") {
            while(quit==0){
                /**< switches to passive mode in order to do more commands */
                strReply = request_reply(sockpi, "PASV\r\n");
                status = strReply.substr(0,3);
                int sock_dtp = change_to_passive(strReply);
                    

                    std::cout<<"///////////MAIN MENU///////////\n"
                             <<"1. List Files\n"
                             <<"2. Change Directory\n"
                             <<"3. Retrieve a File\n"
                             <<"4. Quit\n"
                             <<"///////////////////////////////\n"
                             <<"Make selection: ";
                    std::cin>>uReq;
                    while(std::cin.fail()||uReq>4||uReq<=0){
                        std::cout<<"Enter a valid number (1-4): ";
                        std::cin.clear();
                        std::cin.ignore(10000, '\n');
                        std::cin>>uReq;
                    }
                    switch(uReq){
                    case 1:{
                        /**< attempts to request a LIST command to server */
                        list_dir(sockpi, sock_dtp);
                        break;
                        }
                    case 2: {
                        /**< attempts to request a CWD command to server */
                        std::cout<<"Enter a directory ('..' to go back): \n";
                        std::string dir_name;
                        std::cout << "Enter name of directory: ";
                        std::cin >> dir_name;
                        std::cout << std::endl;
                        strReply = request_reply(sockpi, "CWD "+dir_name+"\r\n");
                        
                        status = strReply.substr(0,3);
                        if (status == cwdSuccess)
                        {
                            list_dir(sockpi, sock_dtp);
                        }else{
                            std::cout << "Can't find that directory." << std::endl;
                        }
                        
                        break;
                    }
                    case 3: {
                        /**< attempts to request a RETR [filename] command to server */
                        std::cout<<"Enter whole filename: ";
                        std::cin>>fileName;
                        strReply = request_reply(sockpi, "RETR "+fileName+"\r\n");
                        status = strReply.substr(0,3);
                        std::cout << "RETR status: ";
                        std::cout << status << std::endl;
                        if(status == "150")
                        {  
                            strReply = reply(sock_dtp);
                        } else if(status == "550"){
                            std::cout<<"Invalid filename.\n";
                            break;
                        }
                        std::ofstream out(fileName);
                        out << strReply;
                        out.close();
                        close(sock_dtp);
                        std::cout << reply(sockpi) << std::endl;
                        break;
                        }
                    case 4: {
                        /**< attempts to request a QUIT command to server */
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
        }else {
            std::cout << "Invalid Password" << std::endl;
        }
    }
    else {
    	std::cout << "Invalid User" << std::endl;
    }
    return 0;
}
