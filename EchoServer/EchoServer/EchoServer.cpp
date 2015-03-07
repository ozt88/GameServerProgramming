#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "pch.h"
#include "EchoServer.h"

int main(int argc, _TCHAR* argv[])
{
	//메시지용 윈도우 생성
	HWND wndHandle = MakeWindow();
	if(CheckError(wndHandle == NULL, "MakeWindow()"))
	{
		return 1;
	}

	//server 소캣 초기화
	WSADATA wsaData;
	DWORD ret = WSAStartup(( 2, 2 ), &wsaData);
	if(CheckError(ret != 0, "WSAStartup()"))
	{
		return 1;
	}

	SOCKET serverSocket = socket(PF_INET, SOCK_STREAM, 0);
	if(CheckError(serverSocket == INVALID_SOCKET, "socket()"))
	{
		return 1;
	}

	//server소캣에 Select로 ACCEPT, CLOSE 이벤트 등록
	ret = WSAAsyncSelect(serverSocket, wndHandle, WM_SOCKET, FD_ACCEPT | FD_CLOSE);
	if(CheckError(ret != 0, "listenSocket WSAAsyncSelect()"))
	{
		return 1;
	}

	//server 소캣 바인딩, 리슨
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(LISTEN_PORT);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

	ret = bind(serverSocket, (SOCKADDR*) &serveraddr, sizeof(serveraddr));
	if(CheckError(ret == SOCKET_ERROR, "bind()"))
	{
		return 1;
	}

	ret = listen(serverSocket, NUM_OF_BACK_LOG);
	if(CheckError(ret == SOCKET_ERROR, "listen()"))
	{
		return 1;
	}

	//메시지 루프
	MSG wndMessage;
	while(GetMessage(&wndMessage, wndHandle, 0, 0))
	{
		TranslateMessage(&wndMessage);
		DispatchMessage(&wndMessage);
	}

	closesocket(serverSocket);
	WSACleanup();
	return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SOCKET selectedSocket = NULL;
	int errorId = NULL;
	switch(uMsg)
	{
		case WM_CREATE:
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_SOCKET:
			selectedSocket = wParam;
			if(errorId = WSAGETSELECTERROR(lParam))
			{
				if(errorId != WSAEWOULDBLOCK)
				{
					CloseSocketWithReason(selectedSocket, CRT_SELECT);
					return 0;
				}
			}
			switch(WSAGETSELECTEVENT(lParam))
			{
				case FD_ACCEPT:
					OnAccept(selectedSocket, hwnd);
					break;
				case FD_READ:
					OnRead(selectedSocket);
					break;
				case FD_CLOSE:
					OnClose(selectedSocket);
					break;
				default:
					break;
			}
			return 0;

		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

//메시지 루프용 윈도우 만들기
HWND MakeWindow()
{
	WNDCLASS wndClass;
	DWORD ret;

	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = (WNDPROC) WndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = NULL;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = WIN_NAME;

	ret = RegisterClass(&wndClass);
	if(CheckError(ret == NULL, "RegisterClass()"))
	{
		return NULL;
	}

	HWND wndHandle = CreateWindowEx(0, WIN_NAME, "", 0, CW_USEDEFAULT, CW_USEDEFAULT,
							   CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, NULL, NULL);

	if(CheckError(wndHandle == NULL, "CreateWindow()"))
	{
		return NULL;
	}

	return wndHandle;
}


//에러 체크해서 적당히 프린트
bool CheckError(bool isError, char* checkFuncName)
{
	char buffer[BUF_SIZE] = {0, };
	if(!isError)
	{
		sprintf(buffer, "%s is Success.\n", checkFuncName);
	}
	else
	{
		sprintf(buffer, "%s is Failed.\n With Error %d.\n", checkFuncName, GetLastError());
	}
	printf(buffer);
	return isError;
}

void OnAccept(SOCKET selectedSocket, HWND hwnd)
{
	int addrSize;
	DWORD ret;
	SOCKADDR_IN clientAddr;
	SOCKET acceptedSocket;

	addrSize = sizeof(clientAddr);
	ZeroMemory(&clientAddr, addrSize);
	acceptedSocket = accept(selectedSocket, (SOCKADDR*) &clientAddr, &addrSize);
	if(acceptedSocket == INVALID_SOCKET)
	{
		int errorId = WSAGetLastError();
		if(errorId != WSAEWOULDBLOCK)
		{
			CloseSocketWithReason(acceptedSocket, CRT_ACCEPT);
			return;
		}
	}
	char* ip = inet_ntoa(clientAddr.sin_addr);
	int port = ntohs(clientAddr.sin_port);
	printf("Client Connected [%s:%d]\n", ip, port);

	ret = WSAAsyncSelect(acceptedSocket, hwnd, WM_SOCKET, FD_READ | FD_WRITE | FD_CLOSE);
	if(ret != 0)
	{
		CloseSocketWithReason(acceptedSocket, CRT_ACCEPT);
		return;
	}
}

void OnRead(SOCKET selectedSocket)
{
	size_t recvSize;
	char buffer[BUF_SIZE] = {0, };

	recvSize = recv(selectedSocket, buffer, BUF_SIZE, 0);
	if(recvSize == SOCKET_ERROR)
	{
		int errorId = WSAGetLastError();
		if(errorId != WSAEWOULDBLOCK)
		{
			CloseSocketWithReason(selectedSocket, CRT_RECV);
			return;
		}
	}

	if(send(selectedSocket, buffer, recvSize, 0) == SOCKET_ERROR)
	{
		int errorId = WSAGetLastError();
		if(errorId != WSAEWOULDBLOCK)
		{
			CloseSocketWithReason(selectedSocket, CRT_SEND);
			return;
		}
	}
}

void OnClose(SOCKET selectedSocket)
{
	CloseSocketWithReason(selectedSocket, CRT_NONE);
}

void CloseSocketWithReason(SOCKET closeSocket, int closedReason)
{
	SOCKADDR_IN clientAddr;
	int addrSize = sizeof(clientAddr);
	ZeroMemory(&clientAddr, addrSize);

	getpeername(closeSocket, (SOCKADDR*) &clientAddr, &addrSize);
	char* ip = inet_ntoa(clientAddr.sin_addr);
	int port = ntohs(clientAddr.sin_port);
	printf("Client Disconnected Reason:%d [%s:%d]\n", closedReason, ip, port);
	closesocket(closeSocket);
}
