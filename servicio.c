#include <stdio.h>
#include <malloc.h>
#include <sys/types.h>
#include <time.h>
#include <sys/timeb.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <windows.h>
#include <winsvc.h>
#include<winsock2.h>
#include<io.h>


long estado=0;	//Indicador para rastrear la actividad del servicio
				// (no muy importante en este ejemplo)
int fh_dia,fh_mes,fh_anio,fh_hora,fh_minuto,fh_segundo;
WSADATA wsa;
SOCKET s , new_socket;
struct sockaddr_in server , client;
int c;
char *message;
FILE *arch;

SERVICE_STATUS_HANDLE interfaz_al_SCM;
SERVICE_STATUS serviceStatus;

SERVICE_TABLE_ENTRY tabla;
char nombreServicio[]="ProgAva_Srv";

HANDLE elThread;
DWORD elThreadID;

VOID WINAPI ServiceMain(DWORD dwArgc,LPTSTR *lpszArgv);
VOID WINAPI mainHandle(DWORD fdwControl);

void main(void)
{
	tabla.lpServiceName=nombreServicio;	
	tabla.lpServiceProc=&ServiceMain;
	StartServiceCtrlDispatcher(&tabla);
}


void setServiceStatusState(unsigned int state) {
	serviceStatus.dwServiceType=SERVICE_WIN32_OWN_PROCESS;
	serviceStatus.dwCurrentState=state;
	serviceStatus.dwControlsAccepted=SERVICE_CONTROL_INTERROGATE
										|SERVICE_ACCEPT_STOP
										|SERVICE_ACCEPT_PAUSE_CONTINUE
										|SERVICE_ACCEPT_SHUTDOWN;
	serviceStatus.dwWin32ExitCode=NO_ERROR;
	serviceStatus.dwServiceSpecificExitCode=0;
	serviceStatus.dwCheckPoint=estado++;
	serviceStatus.dwWaitHint=100;	
}

DWORD WINAPI corre(LPVOID lpParameter) {
	setServiceStatusState(SERVICE_RUNNING);	//Avisar que YA ESTAMOS CORRIENDO!!!
	SetServiceStatus(interfaz_al_SCM,&serviceStatus);	//Enviar mensaje al manejador de servicios
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

VOID WINAPI mainHandle(DWORD fdwControl) {
	switch(fdwControl) {
		case SERVICE_CONTROL_STOP:
					setServiceStatusState(SERVICE_STOP_PENDING);
					SetServiceStatus(interfaz_al_SCM,&serviceStatus);	//Enviar mensaje al manejador de servicios

					if (TerminateThread(elThread,0)) {
						setServiceStatusState(SERVICE_STOPPED);
						SetServiceStatus(interfaz_al_SCM,&serviceStatus);	//Enviar mensaje al manejador de servicios
					}
					break;
		case SERVICE_CONTROL_PAUSE:
					setServiceStatusState(SERVICE_PAUSE_PENDING);	//Avisar que YA ESTAMOS CORRIENDO!!!
					SetServiceStatus(interfaz_al_SCM,&serviceStatus);	//Enviar mensaje al manejador de servicios

					SuspendThread(elThread);
					
					setServiceStatusState(SERVICE_PAUSED);
					SetServiceStatus(interfaz_al_SCM,&serviceStatus);	//Enviar mensaje al manejador de servicios
					break;
		case SERVICE_CONTROL_CONTINUE:
					setServiceStatusState(SERVICE_CONTINUE_PENDING);
					SetServiceStatus(interfaz_al_SCM,&serviceStatus);	//Enviar mensaje al manejador de servicios

					ResumeThread(elThread);

					setServiceStatusState(SERVICE_RUNNING);	//Avisar que YA ESTAMOS CORRIENDO!!!
					SetServiceStatus(interfaz_al_SCM,&serviceStatus);	//Enviar mensaje al manejador de servicios
					break;
		default:
					setServiceStatusState(SERVICE_RUNNING);
					SetServiceStatus(interfaz_al_SCM,&serviceStatus);	//Enviar mensaje al manejador de servicios
					break;
	}
}


VOID WINAPI ServiceMain(DWORD dwArgc,LPTSTR *lpszArgv) {
	interfaz_al_SCM=RegisterServiceCtrlHandler(nombreServicio,mainHandle);

	setServiceStatusState(SERVICE_START_PENDING);
	SetServiceStatus(interfaz_al_SCM,&serviceStatus);	//Avisar que estamos "alive and kicking!"

	elThread=CreateThread(NULL,0,corre,NULL,0,&elThreadID);
}
