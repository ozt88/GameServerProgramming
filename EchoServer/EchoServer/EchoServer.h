#pragma once

#define LISTEN_PORT 9001
#define BUF_SIZE 4096
#define NUM_OF_BACK_LOG 100
#define WM_SOCKET (WM_USER + 1)
#define WIN_NAME "EchoServer"

enum ClosedReasonType
{
	CRT_NONE,
	CRT_ACCEPT,
	CRT_RECV,
	CRT_SEND,
	CRT_SELECT,
	CRT_MAX,
};
LRESULT CALLBACK	WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HWND				MakeWindow();
int					AcceptNewSocket();
bool				CheckError(bool isError, char* checkFuncName);
void				OnAccept(SOCKET selectedSocket, HWND hwnd);
void				OnClose(SOCKET selectedSocket);
void				OnRead(SOCKET selectedSocket);
void				CloseSocketWithReason(SOCKET closeSocket, int closedReason);