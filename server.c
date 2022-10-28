/*
	Live Server on port 8888
*/
//Use linker flag  -lws2_32
#include<stdio.h>
#include<io.h>
#include<windows.h>
#include<winsock2.h>
// Linux will use <sockets.h>
// #include<sys/socket.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

int main(int argc , char *argv[])
{
	WSADATA wsa;
	SOCKET s , new_socket;
	struct sockaddr_in server , client;
	int c;
	char *message;

    // Initializes the library
	if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
	{
		printf("Failed. Error Code : %d",WSAGetLastError());
		return 1;
	}
	
	printf("Library initialized\n");
	
	//Create a socket
    // Type stream tcp
	if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
	{
		printf("Error creating socket : %d" , WSAGetLastError());
	}

	printf("Socket created.\n");
	
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 8080 );
	
	//Bind
	if( bind(s ,(struct sockaddr *)&server , sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind error : %d" , WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	
	puts("Bound");

	listen(s , 3);
	
	puts("Waiting for connections...");
	
	c = sizeof(struct sockaddr_in);
	
	while( (new_socket = accept(s , (struct sockaddr *)&client, &c)) != INVALID_SOCKET )
	{
		puts("Connection accepted");
        //recv(new_socket, message, strlen(message), 0);
        //puts(message);
		//Reply to the client
		message = "Hello socket\n";
		send(new_socket , message , strlen(message) , 0);
		write(new_socket, "HTTP/1.1 200 OK\n", 16);
		write(new_socket, "Content-length: 46\n", 19);
		write(new_socket, "Content-Type: text/html\n\n", 25);
		write(new_socket, "<html><body><H1>Hello world</H1></body></html>",46);
	}
	
	if (new_socket == INVALID_SOCKET)
	{
		printf("accept failed with error code : %d" , WSAGetLastError());
		return 1;
	}

	closesocket(s);
	WSACleanup();
	
	return 0;
}