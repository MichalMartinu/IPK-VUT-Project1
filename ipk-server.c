/********************************************************           
 * Server -- Client/Server program                      *   
 *                                                      *   
 * Author:   Michal Martinu                             *   
 *                                                      *   
 ********************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pwd.h>
#include <string.h>
#include <stdbool.h>

#define BUFFER_LEN 256

#define PROTOCOL_HEAD "< Martinu Protocol version 1.0 >"
#define PROTOCOL_END "< Martinu protocol -- End >"
#define NEWLINE "\n"

//Errors
#define SOCKET_ERR "Error opening serverSocket"
#define BINDING_ERR "Error on binding"
#define ACCEPT_ERR "Error on accept"
#define FORK_ERR "Error when fork"
#define CLOSE_ERR "Error when close"
#define WRITE_ERR "Error on writing to serverSocket"
#define ARG_ERR "Error Wrong arguments"

//Server warning messages
#define SERVER_WARNING "Server: Wrong protocol header or client arguments!"

//Server messages
#define LOGIN_UNKNOWN "Server: Login unknown"

//Function for error managing
void errorOcured(char *type)
{
  perror(type);
  exit(EXIT_FAILURE);
}

//Function for checking prefix
bool startsWith(const char *pre, const char *str)
{
  size_t lenpre = strlen(pre),
         lenstr = strlen(str);
  return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

//Function to send message to client
void sendSimple(int newSocket, char *send)
{
  if (write(newSocket, send, strlen(send)) < 0)
  {
    errorOcured(WRITE_ERR);
  }
}

//Function to send message to client with newline
void sendLine(int newSocket, char *send)
{
  sendSimple(newSocket, send);
  sendSimple(newSocket, NEWLINE);
}

//Function to send message to client for ending output
void sendEndOfProtocol(int newSocket)
{
  sendLine(newSocket, PROTOCOL_END);
}

//Function that send user info to client
void sendUserIdInfo(char *login, int newSocket)
{
  struct passwd *pswd;

  if ((pswd = getpwnam(login)) != 0)
  {
    sendLine(newSocket, pswd->pw_gecos);
  }
  else
  {
    sendLine(newSocket, LOGIN_UNKNOWN);
  }
}

//Function that send user dir info to client
void sendUserDirInfo(char *login, int newSocket)
{
  struct passwd *pswd;

  if ((pswd = getpwnam(login)) != 0)
  {

    sendLine(newSocket, pswd->pw_dir);
  }
  else
  {
    sendLine(newSocket, LOGIN_UNKNOWN);
  }
}

//Function that send all users in /etc/passw
void sendAllUsers(int newSocket)
{
  struct passwd *login;

  while ((login = getpwent()) != NULL)
  {
    sendLine(newSocket, login->pw_name);
  }

  endpwent();
}

//Function that send users by prefix in /etc/passw
void sendUsersByPrefix(char *prefix, int newSocket)
{
  struct passwd *login;
  while ((login = getpwent()) != NULL)
  {
    if (startsWith(prefix, login->pw_name))
    {
      sendLine(newSocket, login->pw_name);
    }
  }

  endpwent();
}

//Function for checking header from client
void checkProtocolHeader(char *string, int newSocket)
{
  if (strcmp(string, PROTOCOL_HEAD))
  {
    sendSimple(newSocket, SERVER_WARNING);
  }
}

//Funtion that read one line from client input
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
      return len; // EOL reached
    }
    if (c == '\n')
    {
      data[len] = 0;
      return len; // EOL reached
    }
    data[len++] = c;
  }

  return 0;
}

int main(int argc, char *argv[])
{

  int serverSocket, newSocket;
  unsigned int portNumber, clilen;
  struct sockaddr_in serverAddress, clientAddress;
  char buffer[BUFFER_LEN];
  pid_t pid;

  //Check arguments
  if (argc == 3)
  {
    int opt;
    while ((opt = getopt(argc, argv, "p:")) != -1)
    {
      switch (opt)
      {
      case 'p':
        portNumber = atoi(optarg);
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
  //End of arguments

  if ((serverSocket = socket(PF_INET, SOCK_STREAM, 0)) < 0)
  {
    errorOcured(SOCKET_ERR);
  }

  bzero((char *)&serverAddress, sizeof(serverAddress));

  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = INADDR_ANY;
  serverAddress.sin_port = htons(portNumber);

  if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
  {
    errorOcured(BINDING_ERR);
  }

  listen(serverSocket, 5);

  printf("Server is running     Portnumber: %d\n", portNumber); //Server message

  clilen = sizeof(clientAddress);

  while (1) //Infinite loop in server
  {
    if ((newSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clilen)) < 0)
    {
      errorOcured(ACCEPT_ERR);
    }

    if ((pid = fork()) < 0)
    {
      errorOcured(FORK_ERR);
    }
    if (pid == 0)
    { // Child process to allow multiple clients

      bzero(buffer, BUFFER_LEN);
      readLine(newSocket, buffer);
      checkProtocolHeader(buffer, newSocket);

      bzero(buffer, BUFFER_LEN);
      readLine(newSocket, buffer);

      if (strcmp(buffer, "-n") == 0)
      {
        bzero(buffer, BUFFER_LEN);
        read(newSocket, buffer, BUFFER_LEN - 1);
        sendUserIdInfo(buffer, newSocket);
      }
      else if (strcmp(buffer, "-f") == 0)
      {
        bzero(buffer, BUFFER_LEN);
        read(newSocket, buffer, BUFFER_LEN - 1);
        sendUserDirInfo(buffer, newSocket);
      }
      else if (strcmp(buffer, "-l") == 0)
      {
        bzero(buffer, BUFFER_LEN);
        read(newSocket, buffer, BUFFER_LEN - 1);
        sendUsersByPrefix(buffer, newSocket);
      }
      else if (strcmp(buffer, "-L") == 0)
      {
        bzero(buffer, BUFFER_LEN);
        sendAllUsers(newSocket);
      }
      else
      { //When wrong arguments from client
        sendSimple(newSocket, SERVER_WARNING);
      }

      bzero(buffer, BUFFER_LEN);

      sendEndOfProtocol(newSocket);

      if (close(serverSocket) < 0)
      {
        errorOcured(CLOSE_ERR);
      }

      return (EXIT_SUCCESS);
    }
  }
}