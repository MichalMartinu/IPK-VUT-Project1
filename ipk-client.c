/********************************************************           
 * Client -- Client/Server program                      *   
 *                                                      *   
 * Author:   Michal Martinu                             *   
 *                                                      *   
 ********************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_LEN 256

#define PROTOCOL_HEAD "< Martinu Protocol version 1.0 >"
#define PROTOCOL_END "< Martinu protocol -- End >"
#define NEWLINE "\n"

//Errors
#define SOCKET_ERR "Error opening socket"
#define HOST_ERR "Error no such host"
#define CLOSE_ERR "Error when close"
#define CONNECT_ERR "Error connecting"
#define WRITE_ERR "Error on writing to socket"
#define ARG_ERR "Error Wrong arguments"

//Function for error managing
void errorOcured(char *type)
{
    perror(type);
    exit(EXIT_FAILURE);
}

//Function to send message to server
void sendSimple(int newsockfd, char *send)
{
    if (write(newsockfd, send, strlen(send)) < 0)
    {
        errorOcured(WRITE_ERR);
    }
}

//Function to send message to server with newline
void sendLine(int newsockfd, char *send)
{
    sendSimple(newsockfd, send);
    sendSimple(newsockfd, NEWLINE);
}

//Funtion that read one line from server input
int readLine(int fd, char data[])
{
    size_t len = 0;
    while (len < BUFFER_LEN - 1)
    {
        char c;
        int ret = read(fd, &c, 1);
        if (ret < 0)
        {
            data[len] = 0;
            return len; // EOF reached
        }
        if (c == '\n')
        {
            data[len] = 0;
            return len; // EOF reached
        }
        data[len++] = c;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int clientSocket;
    struct sockaddr_in serverAddress;
    struct hostent *server;
    char buffer[BUFFER_LEN];
    char *type, *hostname, *login;
    int portNumber;

    //Check arguments
    int flags, opt;
    
    if (argc == 7)
    {
        flags = 0;
        while ((opt = getopt(argc, argv, "p:h:n:f:l:")) != -1)
        {
            switch (opt)
            {
            case 'p':
                portNumber = atoi(optarg);
                break;
            case 'h':
                hostname = optarg;
                break;
            case 'n':
                login = optarg;
                type = "-n";
                flags += 1;
                break;
            case 'f':
                login = optarg;
                type = "-f";
                flags += 1;
                break;
            case 'l':
                login = optarg;
                type = "-l";
                flags += 1;
                break;        
            default:
                errorOcured(ARG_ERR);
            }
        }
       if (flags != 1)
       {
           errorOcured(ARG_ERR);
       } 
    }
    else if (argc == 6)
    {
        flags = 0;
        while ((opt = getopt(argc, argv, "p:h:l")) != -1)
        {
            switch (opt)
            {
            case 'p':
                portNumber = atoi(optarg);
                break;
            case 'h':
                hostname = optarg;
                break;
            case 'l':
                type = "-L";
                break;        
            default:
                errorOcured(ARG_ERR);
            }
        }
    }
    else
    {
        errorOcured(ARG_ERR);
    }
    //End of checking arguments

    if ((clientSocket = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        errorOcured(SOCKET_ERR);
    }

    if ((server = gethostbyname(hostname)) == NULL)
    {
        errorOcured(HOST_ERR);
    }

    bzero((char *)&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;

    bcopy((char *)server->h_addr, (char *)&serverAddress.sin_addr.s_addr, server->h_length);

    serverAddress.sin_port = htons(portNumber);

    if (connect(clientSocket, &serverAddress, sizeof(serverAddress)) < 0)
    {
        errorOcured(CONNECT_ERR);
    }

    bzero(buffer, BUFFER_LEN);
    sendLine(clientSocket, PROTOCOL_HEAD);

    bzero(buffer, BUFFER_LEN);
    sendLine(clientSocket, type);

    bzero(buffer, BUFFER_LEN);
    sendSimple(clientSocket, login);

    while (strcmp(buffer, PROTOCOL_END) != 0)
    {
        bzero(buffer, BUFFER_LEN);
        readLine(clientSocket, buffer);

        if (strcmp(buffer, PROTOCOL_END) != 0)
        {
            printf("%s\n", buffer);
        }
    }

    if (close(clientSocket) < 0)
    {
        errorOcured(CLOSE_ERR);
    }

    return (EXIT_SUCCESS);
}
