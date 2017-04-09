/*----------------------------------------
   MESSENGER.CPP (TCP) -- (C) ISCOOP 2003
 ----------------------------------------*/

#include <windows.h>
#include <winsock.h>
#include <process.h>
#include <string>
#include <vector>
#include "resource.h"
using namespace std;


LRESULT APIENTRY WndProc  (HWND, UINT, WPARAM, LPARAM) ;
LRESULT CALLBACK WndProc2 (HWND, UINT, WPARAM, LPARAM) ;
LRESULT CALLBACK ListenPortProc (HWND, UINT, WPARAM, LPARAM) ;
LRESULT CALLBACK TalkPortProc (HWND, UINT, WPARAM, LPARAM) ;
LRESULT CALLBACK MessageDisplayProc (HWND, UINT, WPARAM, LPARAM) ;

int validIP(TCHAR s[], HWND hwnd);
int stringToInt(TCHAR n[]);
int charToInt(TCHAR c);
void EditPrintf (HWND hwndEdit, TCHAR * szFormat, ...);
void wait(int s);
void signal(int s);
WNDPROC OldListen[2];
WNDPROC OldMessageDisplay[2];
WNDPROC OldTalk[4];

vector<int> sockets;
int max;
int mutex_fds = 1;
int mutex_messengerWindowIndex = 1;
int messengerWindowIndex = 1;

BOOL bKillListen, bKillTalk, bMessageButtonClicked;
BOOL bFromListen = FALSE, bFromTalk = FALSE;
BOOL connectionWindow = FALSE;
BOOL messengerWindow = FALSE;

#define ID_CONNECT_BUTTON     10001 
#define ID_TALK_IP            10002
#define ID_TALK_IP_EDIT       10003  
#define ID_TALK_PORT          10004
#define ID_TALK_PORT_EDIT     10005

#define WM_CREATE_MESSAGE_WINDOW (WM_USER + 1)
#define WM_SET_FOCUS (WM_USER + 2)

#define PORT 1024  //port we're listening on
#define MAX_MESSENGER_WINDOW     100
  
#define DEBUG_MODE  0
//typedef struct sMessenger
struct sMessenger
{
	HWND hwndMessageWindow;
    HWND hwndMessageDisplay;
	HWND hwndMessageButton;
	HWND hwndMessageEditor;
	int socket;
	int idFocus; 
};

struct sMessenger messenger[MAX_MESSENGER_WINDOW];
struct sMessenger * pMessenger = messenger; 

fd_set master_read;
fd_set master_write;


typedef struct
{
	HINSTANCE hInstance;
	HWND hwnd;
} PARAMS, *PPARAMS;


TCHAR szWinName[] = TEXT ("MessengerWindow") ;

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{
          
	 static int ClientAreaWidth = 250;
     static int ClientAreaHeight = 400;
	 TCHAR szAppName[] = TEXT ("messenger") ; 
	 int n;
	 for (n = 0; n < MAX_MESSENGER_WINDOW; n++)
     {
		messenger[n].hwndMessageWindow = NULL;
        messenger[n].hwndMessageDisplay = NULL;
	    messenger[n].hwndMessageButton = NULL;
	    messenger[n].hwndMessageEditor = NULL;
	
	    messenger[n].socket = 0;
	    messenger[n].idFocus = 0; 
	 }

     HWND         hwnd ;
     MSG          msg ;
     WNDCLASS wndclass;
	 

     wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
     wndclass.lpfnWndProc   = WndProc ;
     wndclass.cbClsExtra    = 0 ;
     wndclass.cbWndExtra    = 0 ;
     wndclass.hInstance     = hInstance ;
     wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION) ;
     wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
     wndclass.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH) ;
     wndclass.lpszMenuName  = szAppName ;
     wndclass.lpszClassName = szAppName ;
     
     if (!RegisterClass (&wndclass))
     {
          MessageBox (NULL, TEXT ("This program requires Windows NT!"),
                      szAppName, MB_ICONERROR) ;
          return 0 ;
     }
	 
     wndclass.lpszMenuName = NULL;
     wndclass.lpszClassName = szWinName;
	 
	 RegisterClass(&wndclass);

     hwnd = CreateWindow (szAppName, TEXT ("Messenger"),
                         WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU |
                          WS_BORDER | WS_MINIMIZEBOX,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          NULL, NULL, hInstance, NULL) ;
 
	 
	 SetWindowPos (hwnd, NULL, 0, 0, ClientAreaWidth,
                   ClientAreaHeight, SWP_NOMOVE) ;
 

     ShowWindow (hwnd, iCmdShow) ;
     UpdateWindow (hwnd) ;
     

     while (GetMessage (&msg, NULL, 0, 0))
     {
          TranslateMessage (&msg) ;
          DispatchMessage  (&msg) ;
     }
     return msg.wParam ;
}
		

void Thread (PVOID pvoid)
{
	PPARAMS pparams;
	WSADATA WSAData;
	fd_set read_fds;
	fd_set write_fds;
	struct timeval select_interval;

	
	struct sockaddr_in myaddr;
	struct sockaddr_in remoteaddr;
	int listener, max;
	int newfd;
	char buf[500]; 
	int nbytes;
	char yes = '1';
	int addrlen;
	int i, result;

	static int ClientAreaWidth = 350;
    static int ClientAreaHeight = 450;
	
	pparams = (PPARAMS) pvoid;


    wait(mutex_fds);

	FD_ZERO(&master_read);
	FD_ZERO(&master_write);
    FD_ZERO(&read_fds);
	FD_ZERO(&write_fds);

	signal(mutex_fds); 
	
    select_interval.tv_sec = 0;
    select_interval.tv_usec = 0;  

	if (WSAStartup (MAKEWORD(2, 0), &WSAData))
	{
		MessageBox(pparams->hwnd, TEXT("Startup error.\r\n"),
		  TEXT("Error Message"), 0);
		
		_endthread();
	} 
	
	if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		MessageBox(pparams->hwnd, TEXT("Socket creation error.\r\n"),
		  TEXT("Message"), 0);
		
		_endthread();
	}
	sockets.push_back(listener);
	max = listener;

    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR,
			&yes, sizeof(int)) == -1)
    {
		MessageBox(pparams->hwnd, TEXT("setsockopt() error.\r\n"),
					  TEXT("Message"), 0);
		
		_endthread();
    }
	
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.S_un.S_addr = /*inet_addr("127.0.0.1");*/INADDR_ANY;
	myaddr.sin_port = htons(PORT);
    memset(&(myaddr.sin_zero), '\0', 8);

	if (bind(listener, (struct sockaddr *)&myaddr, 
			sizeof(myaddr)) == -1)
    {
		MessageBox(pparams->hwnd, TEXT("bind() error.\r\n"),
			  TEXT("Message"), 0);
		
		_endthread();
    }
	

	if (listen(listener, 10) == -1)
    {
		MessageBox(pparams->hwnd, TEXT("listen() error.\r\n"),
		  TEXT("Message"), 0);
		
		_endthread();
    }  
	
    
	//add the listener to the master set
	wait(mutex_fds);

	FD_SET(listener, &master_read);
	
    signal(mutex_fds);
    
	
	while (!(bKillListen))
    {		
		wait(mutex_fds);

		read_fds = master_read; 
		write_fds = master_write;

		signal(mutex_fds);
		

		if ((result = select(read_fds.fd_count, &read_fds, 
			&write_fds, NULL, &select_interval)) == -1)
        {
			MessageBox(pparams->hwnd, TEXT("select() error.\r\n"),
	          TEXT("Message"), 0);
		     
			_endthread();
		}

		if (result == 0) {
			// no sockets have data
			continue;
		}

		// check for errors
		if (result == SOCKET_ERROR) {
			
			continue;
		}
		
		for (i = 0; i < sockets.size(); i++)
        {
			if (FD_ISSET(sockets[i], &read_fds))
            {
				if (sockets[i] == listener)
				{
					addrlen = sizeof(struct sockaddr_in);
					
					if ((newfd = accept(listener, 
	 				    (struct sockaddr *)&remoteaddr,
						  &addrlen)) == -1)
                    {
						MessageBox(pparams->hwnd, TEXT("accept() error.\r\n"),
					           TEXT("Message"), 0);
					    
						_endthread();
                    }
					else 
                    {
                        wait(mutex_fds);

						FD_SET(newfd, &master_read);
						sockets.push_back(newfd);
                        FD_SET(newfd, &master_write);

						signal(mutex_fds);

						if (newfd > max)
							max = newfd;

						SendMessage(pparams->hwnd, WM_CREATE_MESSAGE_WINDOW, 
							 (WPARAM) sockets.size() - 1, 0);
					}
				}
				else   
				{   
					nbytes = recv(sockets[i], buf, sizeof(buf), 0);

                    if (nbytes == SOCKET_ERROR) {

				    // Uh Oh!  We have our first real use of error handling!  Something can happen here that is
				    // pretty significant.  Let's grab the error number from winsock.
				int error = WSAGetLastError();

				// The error we got should be WSAECONNRESET.  This error says that the connection was either
				// closed or somehow reset from the other end.  That means that our client has shutdown his
				// application.  We have 2 real ways of dealing with closing connections.  One is to send a
				// disconnect message from the client to the server.  The other is to handle the dropped
				// connection error.  The disconnect message is something that we would have to write ourselves.
				// We would somehow make an identifier message that tells our server that we are disconnecting,
				// that way we can handle it properly.  That will work assuming that all our connections are closed
				// cleanly (like they are supposed to).  But what if someone just shuts down their application, or
				// looses power to their computer.  They can't send a message if their computer isn't on!
				// So we still need to check for the error condition.

				// handle the dropped connection
				if (error == WSAECONNRESET) {

					// When we receive this error.  Just get a lock on the master set, and remove this socket from
					// set using the FD_CLR() macro.

					// lock our mutex
					wait(mutex_fds);

					// remove the socket from our master set

					FD_CLR(sockets[i], &master_read);
                    FD_CLR(sockets[i], &master_write);
					// unlock our mutex
					signal(mutex_fds);
					

					// close the socket on our side, so our computer cleans up properly
					closesocket(sockets[i]);

					// a quick message
					//printf("client on %d disconnected\n", clientSocket);

					// move on to the next client
					continue;

				} //else {

					// we failed, but it wasn't an error we were expecting ... so kill the server
					//printf("Recv Failed!\n");
					//gQuitFlag = true;
					//break;

				//}
			}
                    
            
                    
					if (nbytes <= 0)
					{   
						if (nbytes == 0)
						{
							EditPrintf(messenger[i].hwndMessageDisplay, 
				                        TEXT("socket hung up."));
						}
						else 
						{							
							EditPrintf(messenger[i].hwndMessageDisplay, 
				                        TEXT("recv() failed."));
						}
						closesocket(sockets[i]);

						wait(mutex_fds);

						FD_CLR(sockets[i], &master_read);
						FD_CLR(sockets[i], &master_write);

						signal(mutex_fds);

						continue;
					}
					else
					{  
									buf[nbytes] = '\0';
									
						            EditPrintf(messenger[i].hwndMessageDisplay, 
				                        buf);
									EditPrintf(messenger[i].hwndMessageDisplay, "\r\n");

									SendMessage(pparams->hwnd, WM_SET_FOCUS, i, 0);
					}
				}
			}
		}
	}   //end while loop
	MessageBox(pparams->hwnd, TEXT("Inside Thread"), 
				TEXT("Caption Bar"), 0);
	
	_endthread();
}




LRESULT APIENTRY WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
     int cxClient, cyClient;
	 char yes = '1';
	 TCHAR ipaddress[15], port[5], messageA[5000];
	 int talker, portNumber, returnValue, iLength;
	 WSADATA WSAData;
     PAINTSTRUCT ps;
     
	 struct sockaddr_in myaddr;
     
     static HBRUSH hBrushStatic;
	 static int cxChar, cyChar, cxCaps;
	 HINSTANCE hInstance;
     static PARAMS paramsListener;
	 static int ClientAreaWidth = 400;
     static int ClientAreaHeight = 500;
     
	 TCHAR tempBuffer[50];
	 HWND hwndMWindow = NULL;	 
	 static HWND hwndConnectionButton, 
		         hwndTalkIp,
	             hwndTalkPort,
				 hwndTalkIpEdit,
				 hwndTalkPortEdit,
				 hwndConnectionWindow;
	 
     static BOOL connectWindowOnScreen = FALSE;
	 int i;
	 
	 HDC         hdc ;
     
	 TEXTMETRIC tm;
	 int s;
	      
     switch (message)
     {
     case WM_CREATE :        
		if(!messengerWindow && !connectionWindow)
        { 
		  hdc = GetDC(hwnd);

		  GetTextMetrics(hdc, &tm);
		  cxChar = tm.tmAveCharWidth;
		  cxCaps = (tm.tmPitchAndFamily & 1 ? 3:2) * cxChar / 2;
		  cyChar = tm.tmHeight + tm.tmExternalLeading;

		  ReleaseDC(hwnd, hdc);

          hInstance = (HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE);

		  paramsListener.hInstance = hInstance;
		  paramsListener.hwnd = hwnd;
		  bKillListen = FALSE;		  
		  _beginthread(Thread, 0, &paramsListener);
		}
        else if (messengerWindow)
		{
		  hInstance = (HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE);
		  pMessenger->idFocus = (messengerWindowIndex-1) * 3 + 2;
		  pMessenger->socket = sockets[messengerWindowIndex];

		  
#if DEBUG_MODE
		  wsprintf(tempBuffer, TEXT("messengerWindowIndex (WM_CREATE)= %d"), messengerWindowIndex);
		  MessageBox(hwnd, tempBuffer, TEXT("Message"), 0);
#endif

          pMessenger->hwndMessageDisplay = CreateWindow(TEXT("edit"), NULL,
			   WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER | ES_LEFT |
			   ES_MULTILINE | ES_AUTOVSCROLL, 0, 0, 0, 0, 
			   hwnd, 
			   (HMENU) ((messengerWindowIndex-1) * 3 + 1), hInstance, NULL);

		  pMessenger->hwndMessageEditor = CreateWindow(TEXT("edit"), NULL,
			   WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER | ES_LEFT |
			   ES_MULTILINE | ES_AUTOVSCROLL, 0, 0, 0, 0, 
			   hwnd, 
			   (HMENU) ((messengerWindowIndex-1) * 3 + 2), hInstance, NULL);

          pMessenger->hwndMessageButton = CreateWindow(TEXT("button"), TEXT("send"),
			   WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 0, 0, 0, 0, 
			   hwnd, 
			   (HMENU) ((messengerWindowIndex-1) * 3 + 3), hInstance, NULL);   

#if DEBUG_MODE
	      wsprintf(tempBuffer, TEXT("pMessenger->socket = %d"), pMessenger->socket);//////////////////
		  MessageBox(hwnd, tempBuffer, TEXT("Message"), 0);	  
#endif		  
     

		  hdc = GetDC(hwnd);

		  GetTextMetrics(hdc, &tm);
		  cxChar = tm.tmAveCharWidth;
		  cxCaps = (tm.tmPitchAndFamily & 1 ? 3:2) * cxChar / 2;
		  cyChar = tm.tmHeight + tm.tmExternalLeading;

		  ReleaseDC(hwnd, hdc);
		}
		else if (connectionWindow)
        {
			hInstance = (HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE);
			hwndTalkIp = CreateWindow(TEXT ("static"), TEXT("IP"),
				WS_CHILD | WS_VISIBLE | SS_RIGHT, 0, 0, 0, 0, hwnd, 
				(HMENU) ID_TALK_IP, hInstance, NULL);

			hwndTalkIpEdit = CreateWindow(TEXT ("edit"), NULL,
				WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL,
				0, 0, 0, 0, hwnd, 
				(HMENU) ID_TALK_IP_EDIT, hInstance, NULL);

			hwndTalkPort = CreateWindow(TEXT ("static"), TEXT("port"),
				WS_CHILD | WS_VISIBLE | SS_RIGHT, 0, 0, 0, 0, hwnd, 
				(HMENU) ID_TALK_PORT, hInstance, NULL);

            hwndTalkPortEdit = CreateWindow(TEXT ("edit"), NULL,
				WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL,
				0, 0, 0, 0, hwnd, 
				(HMENU) ID_TALK_PORT_EDIT , hInstance, NULL);

			hwndConnectionButton = CreateWindow(TEXT ("button"), 
				TEXT("Connect"), WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
				0, 0, 0, 0, hwnd, 
				(HMENU) ID_CONNECT_BUTTON, hInstance, NULL);

			
			hdc = GetDC(hwnd);

			GetTextMetrics(hdc, &tm);
			cxChar = tm.tmAveCharWidth;
		    cxCaps = (tm.tmPitchAndFamily & 1 ? 3:2) * cxChar / 2;
		    cyChar = tm.tmHeight + tm.tmExternalLeading;

		    ReleaseDC(hwnd, hdc); 
         }
		 else {} 
		
		 hBrushStatic = CreateSolidBrush (GetSysColor(COLOR_BTNHIGHLIGHT));

         return 0 ;
          
     case WM_SIZE :
        
		if (messengerWindow)	
		{	
			cxClient = LOWORD(lParam);
			cyClient = HIWORD(lParam);

#if DEBUG_MODE
			wsprintf(tempBuffer, TEXT("messengerWindowIndex (WM_SIZE)= %d"), messengerWindowIndex);//////////////////
		    MessageBox(hwnd, tempBuffer, TEXT("Message"), 0);
#endif

			MoveWindow (pMessenger->hwndMessageButton,
               7 * cxClient / 16, 
               11 * cyClient /14,
               3 * cxClient / 16, 7 * cyChar / 4, FALSE) ;

  		    MoveWindow (pMessenger->hwndMessageDisplay,
               2 * cxClient / 16, 
               2 * cyClient / 14,
               12 * cxClient /16, 5 * cyClient / 14, FALSE) ;  

		    MoveWindow (pMessenger->hwndMessageEditor,
               2 * cxClient / 16, 
               8 * cyClient / 14,
               12 * cxClient /16, 2 * cyClient / 14, FALSE) ;  
	
	        pMessenger->idFocus = (messengerWindowIndex-1) * 3 + 2;
          
            SetFocus (pMessenger->hwndMessageEditor) ;
        } 
		if (connectionWindow)
        {
			cxClient = LOWORD(lParam);
			cyClient = HIWORD(lParam);

			MoveWindow(hwndConnectionButton, 
				7 * cxClient / 16,
				12 * cyClient /14, 
				3 * cxClient /16,
				7 * cyChar / 4, TRUE);

			MoveWindow(hwndTalkIp, 
				3 * cxClient / 16,
				2 * cyClient /14, 
				2 * cxClient /16,
				cyChar, TRUE);

			MoveWindow(hwndTalkPort, 
				3 * cxClient / 16,
				4 * cyClient /14, 
				2 * cxClient /16,
				cyChar, TRUE); 

            MoveWindow(hwndTalkIpEdit, 
				5 * cxClient / 16,
				2 * cyClient /14, 
				7 * cxClient /16,
				3 * cyChar / 2, TRUE); 

			MoveWindow(hwndTalkPortEdit, 
				5 * cxClient / 16,
				4 * cyClient /14, 
				7 * cxClient /16,
				3 * cyChar / 2, TRUE);
     
			SetFocus(hwndTalkIpEdit);
		} 
		
        return 0 ;
          
     case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
		

    	EndPaint(hwnd, &ps);
        return 0;
     
     	  
	 case WM_CHAR:
		  switch (wParam)  
		  {
		  case '\t':
			  if (connectWindowOnScreen)
              { 
                  SetFocus(hwndTalkPortEdit);
                  wsprintf(tempBuffer, TEXT("tab key pressed"));
		          MessageBox(hwnd, tempBuffer, TEXT("Message"), 0); 
              }
			  break;
			
		  default:	  
		      DestroyWindow(hwnd);
			  break;
          } 
		  return 0;
     
     case WM_CREATE_MESSAGE_WINDOW:

		 wait(mutex_messengerWindowIndex);

		 hInstance = (HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE);
		 s = (int) wParam;
		 
		 
#if DEBUG_MODE
		 wsprintf(tempBuffer, TEXT("messengerWindowIndex (WM_CREATE_M_WINDOW)= %d"), messengerWindowIndex);
		 MessageBox(hwnd, tempBuffer, TEXT("Message"), 0);
#endif

		 if(s == 1)
		 	 pMessenger++;

		 messengerWindow = TRUE;     
		 pMessenger->hwndMessageWindow = CreateWindow(szWinName, TEXT("Messenger"),
			 WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_BORDER | WS_MINIMIZEBOX,
			 CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			 NULL, NULL, hInstance, NULL);

		 SetWindowPos(pMessenger->hwndMessageWindow, NULL, 50, 50, 
			 ClientAreaWidth, ClientAreaHeight, SWP_NOMOVE);

		 ShowWindow(pMessenger->hwndMessageWindow, SW_SHOWNORMAL);

		 UpdateWindow(pMessenger->hwndMessageWindow);

		 
			 pMessenger++;

		 messengerWindow = FALSE;

		 ++messengerWindowIndex;

		 signal(mutex_messengerWindowIndex);

         return 0;
     
     case WM_SET_FOCUS:
         
		 s = (int) wParam;
		 SetFocus ((messenger[s].hwndMessageEditor));
		 wParam = TRUE;
	
		 
		 return 0;

     

     case WM_COMMAND:
		  
		  
		switch(LOWORD(wParam) % 3)
          {          
          case 0:
			  for (i = 1; i < sockets.size(); i++)
            {
				if ((LOWORD(wParam) == ((i-1) * 3 + 3))
                      && (HIWORD(wParam) == BN_CLICKED))
					  break;
			}
			
			if (i == sockets.size())
					  break;

#if DEBUG_MODE
            MessageBox(hwnd, TEXT("Works here!"), TEXT("Message"), 0);
#endif

			iLength = GetWindowTextLength (messenger[i].hwndMessageEditor);
			iLength = GetWindowText(messenger[i].hwndMessageEditor, messageA, iLength+1);

			
#if DEBUG_MODE
			wsprintf(tempBuffer, TEXT("message = (%s)"), messageA);
		    MessageBox(hwnd, tempBuffer, TEXT("Message"), 0);

			
			wsprintf(tempBuffer, TEXT("i = %d"), i);
		    MessageBox(hwnd, tempBuffer, TEXT("Message"), 0);

			wsprintf(tempBuffer, TEXT("socket (messenger) = %d"), messenger[i].socket);
		    MessageBox(hwnd, tempBuffer, TEXT("Message"), 0);
#endif
			EditPrintf(messenger[i].hwndMessageDisplay, messageA);
            EditPrintf(messenger[i].hwndMessageDisplay, "\r\n");
  		    SetWindowText(messenger[i].hwndMessageEditor, "");

			if ((returnValue = send(messenger[i].socket, messageA, 
				(strlen(messageA))+1, 0)) == -1)
			{
				MessageBox(hwnd, TEXT("Can not send message"), TEXT("Message"), 0);
			}
			

			return 0;

		  default:
             break; 
		}

		if (LOWORD(wParam) == IDM_INSTANT_MESSENGER) 
        {
          wait(mutex_messengerWindowIndex);

          if(!connectWindowOnScreen)
          {    
			hInstance = (HINSTANCE) GetWindowLong (hwnd, GWL_HINSTANCE);
			connectionWindow = TRUE;
            messengerWindow = FALSE;

			hwndConnectionWindow = CreateWindow(szWinName, 
				TEXT("connection window"),
				WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_BORDER | WS_MINIMIZEBOX,
				CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
				NULL, NULL, hInstance, NULL);

			SetWindowPos(hwndConnectionWindow, NULL, 50, 50, 
				ClientAreaWidth, ClientAreaHeight, SWP_NOMOVE);

			ShowWindow(hwndConnectionWindow, SW_SHOWNORMAL);

			UpdateWindow(hwndConnectionWindow);

			SetFocus(hwndTalkIpEdit);
			connectionWindow = FALSE;
			connectWindowOnScreen =	TRUE;

			
		  }
		  else
          {
			  SetFocus(hwndTalkIpEdit);
		  }

		  signal(mutex_messengerWindowIndex);

		  return 0;
		}
		else if (LOWORD(wParam) == ID_CONNECT_BUTTON)     //Connect button clicked
        {
			  wait(mutex_messengerWindowIndex);
			  
			  iLength = GetWindowTextLength (hwndTalkIpEdit);

#if DEBUG_MODE
              wsprintf(tempBuffer, TEXT("length of ipaddress = %d"), iLength);
		      MessageBox(hwnd, tempBuffer, TEXT("Message"), 0);
#endif

              iLength = GetWindowText (hwndTalkIpEdit,
				  ipaddress, iLength+1);

#if DEBUG_MODE
              wsprintf(tempBuffer, TEXT("length of ipaddress copied = %d"), iLength);
		      MessageBox(hwnd, tempBuffer, TEXT("Message"), 0);
#endif

			  iLength = GetWindowTextLength (hwndTalkPortEdit);
              iLength = GetWindowText (hwndTalkPortEdit,
				  port, iLength+1);

#if DEBUG_MODE
              MessageBox(hwnd, ipaddress, TEXT("IP Address"), 0);

              MessageBox(hwnd, port, TEXT("Port number"), 0);
#endif

			  portNumber = stringToInt(port);


              

			  if (WSAStartup (MAKEWORD(2, 0), &WSAData))
			  {
		         MessageBox(hwnd, TEXT("Startup error.\r\n"),
				    TEXT("Erro Message"), 0);
		
		         return 0;
			  } 
	          

  	          if ((talker = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
			  {
		          MessageBox(hwnd, TEXT("Socket creation error.\r\n"),
		          TEXT("Message"), 0);
		       
			      return 0;
		
			  }
	          

              if (setsockopt(talker, SOL_SOCKET, SO_REUSEADDR,
			        &yes, sizeof(int)) == -1)
			  {
		           MessageBox(hwnd, TEXT("setsockopt() error.\r\n"),
					  TEXT("Message"), 0);
		
				   return 0;
		
			  }
	          

              myaddr.sin_family = AF_INET;
	          myaddr.sin_addr.S_un.S_addr = inet_addr(ipaddress);
	          myaddr.sin_port = htons(portNumber);
              memset(&(myaddr.sin_zero), '\0', 8);
    
  	          if (connect(talker, (SOCKADDR*)&myaddr, 
		        sizeof(myaddr)) == -1)
			  { 
		           MessageBox(hwnd, TEXT("connect() error.\r\n"),
		           TEXT("Message"), 0);
		  
			       return 0;
			  } 

			  signal(mutex_messengerWindowIndex);
 
			  wait(mutex_fds);

			  sockets.push_back(talker);
			  FD_SET(talker, &master_read);
			  FD_SET(talker, &master_write);

			  signal(mutex_fds);

			  if (talker > max)
				  max = talker;

			  

			  SendMessage(hwnd, WM_CREATE_MESSAGE_WINDOW, (WPARAM) sockets.size() - 1, 0);

#if DEBUG_MODE
			  wsprintf(tempBuffer, TEXT("talkerSocket = %d"), talker);
		      MessageBox(hwnd, tempBuffer, TEXT("Message"), 0);
#endif

              DestroyWindow(hwndConnectionWindow);
			  hwndConnectionButton = NULL; 
		      hwndTalkIp = NULL;
	          hwndTalkPort = NULL;
			  hwndTalkIpEdit = NULL;
			  hwndTalkPortEdit = NULL; 
			  hwndConnectionWindow = NULL;

			  connectWindowOnScreen = FALSE;


             
			  return 0;

		}        
		else {}


		break;

	 case WM_CTLCOLORSTATIC:
		  i = GetWindowLong ((HWND) lParam, GWL_ID);
		  
		  SetBkColor ((HDC) wParam, GetSysColor(COLOR_BTNHIGHLIGHT));
		  return (LRESULT) hBrushStatic;

     case WM_SYSCOLORCHANGE:
		  DeleteObject (hBrushStatic);
		  hBrushStatic = CreateSolidBrush (GetSysColor (COLOR_BTNHIGHLIGHT));
		  return 0;

     case WM_DESTROY :
		  if (hwnd == hwndConnectionWindow)
          {
			DestroyWindow(hwnd);
            connectWindowOnScreen = FALSE;
          }
		  else 
            PostQuitMessage (0) ;
		  
          return 0 ;
     } 
     return DefWindowProc (hwnd, message, wParam, lParam) ;
}

int validIP(TCHAR s[], HWND hwnd)
{
	int number;
	int digitCount = 0, charCount = 0, groupCount = 0;
	TCHAR * temp;
    
    
#if DEBUG_MODE
	MessageBox(hwnd, TEXT("inside validIP"), TEXT("TEST"), 0);
#endif
    
	while(*s != '\0' && groupCount < 4)
    {
#if DEBUG_MODE
		MessageBox(hwnd, TEXT("inside while loop"), TEXT("TEST"), 0);
		MessageBox(hwnd, s, TEXT("TEST"), 0);
#endif		

		temp[0] = s[0];
		
		

		while((*temp != '.' && digitCount < 3) && (*temp != '\0'))
        {			
			digitCount++;
			charCount++;
			temp++;
			s++;
			*temp = *s;
		}
		
		groupCount++;
		if(groupCount <= 3)
        {
			if (*temp = '\0' || (*temp != '.'))
				return 0;

			charCount++;
		}
		else
		{
			if(*temp != '\0')
				if (isalpha(*temp) || isdigit(*temp))
					return 0;
		}
		*temp = '\0';
		number = stringToInt(temp);
		if(number < 0 || number > 255)
			return 0;

		*temp = NULL;
		s++;
		digitCount = 0;
	} //while
	
	if (groupCount != 4)
		return 0;

	return charCount;

} //validIP


int stringToInt(TCHAR n[])
{
	int i = 0, count = 0;
	int multiplier = 1;
	int temp, j; 
	int sub, total = 0;

	while(n[i] != '\0')
    {
		count++;
		i++;		
    }
	temp = count - 1;

	for (i = 0; i < count; i++)
    {
		for (j = 0; j < temp; j++)
		{
			multiplier *= 10;
        }
		temp--;
		sub =  charToInt(n[i]) * multiplier;
		total += sub;
		multiplier = 1;
    }

	return total;
}

int charToInt(TCHAR c)
{
	int digit;
	switch (c)
    {
	case '0':
		digit = 0;
		break;
    case '1':
		digit = 1;
		break;
	case '2':
		digit = 2;
		break;
	case '3':
		digit = 3;
		break;
	case '4':
		digit = 4;
		break;
	case '5':
		digit = 5;
		break;
	case '6':
		digit = 6;
		break;
	case '7':
		digit = 7;
		break;
	case '8':
		digit = 8;
		break;
	case '9':
		digit = 9;
		break;

    default: 
		break;
	}
	return digit;
}

LRESULT CALLBACK ListenPortProc (HWND hwnd, UINT message, 
								 WPARAM wParam, LPARAM lParam) 
{	
	int id = GetWindowLong(hwnd, GWL_ID);
	
	switch(message)
    {
	case WM_KEYDOWN:
		if(wParam == VK_TAB)
			SetFocus (GetDlgItem(GetParent(hwnd), id == 2? 3:2));
		break;
	}
	return CallWindowProc(OldListen[id==2? 0:1], hwnd, message, 
		            wParam, lParam);
}

LRESULT CALLBACK MessageDisplayProc (HWND hwnd, UINT message, 
									 WPARAM wParam, LPARAM lParam) 
{
	int id = GetWindowLong(hwnd, GWL_ID);
	
	switch(message)
    {
	case WM_KEYDOWN:
		if(wParam == VK_TAB)
			SetFocus (GetDlgItem(GetParent(hwnd), id == 5? 19:5));
		if(wParam == '\x1B')
        { 			
			SendMessage(GetParent(hwnd), WM_CHAR, wParam, lParam);
			return 0;
		}
			
		break;
	}
	return CallWindowProc(OldListen[id==4? 0:1], hwnd, message, 
		            wParam, lParam);
}

LRESULT CALLBACK TalkPortProc (HWND hwnd, UINT message, 
							   WPARAM wParam, LPARAM lParam)
{
	int id = GetWindowLong(hwnd, GWL_ID);
	
	switch(message)
    {
	case WM_KEYDOWN:
		if(wParam == VK_TAB)
			SetFocus (GetDlgItem(GetParent(hwnd), 
			(((id - 9) + (GetKeyState(VK_SHIFT) < 0? 3:1)) % 4) + 9));

		break;	
	}
	return CallWindowProc(OldTalk[id - 9], hwnd, message, 
		            wParam, lParam);
}

void EditPrintf (HWND hwndEdit, TCHAR * szFormat, ...)
{
	TCHAR szBuffer[1024];
	va_list pArgList;

	va_start (pArgList, szFormat);
	wvsprintf (szBuffer, szFormat, pArgList);
	va_end (pArgList);



	SendMessage(hwndEdit, EM_SETSEL, (WPARAM) -1, (LPARAM) -1);
    SendMessage(hwndEdit, EM_REPLACESEL, FALSE, (LPARAM) szBuffer);
	SendMessage(hwndEdit, EM_SCROLLCARET, 0, 0);
}

void wait(int s)
{
	while (s <= 0)
    ;

	--s;
}

void signal(int s)
{
	while (s < 1)
	   ++s;
}