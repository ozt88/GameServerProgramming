#include "stdafx.h"
#include "Exception.h"
#include "EduServer_IOCP.h"
#include "ClientSession.h"
#include "IocpManager.h"
#include "SessionManager.h"


OverlappedIOContext::OverlappedIOContext(ClientSession* owner, IOType ioType) 
: mSessionObject(owner), mIoType(ioType)
{
	memset(&mOverlapped, 0, sizeof(OVERLAPPED));
	memset(&mWsaBuf, 0, sizeof(WSABUF));
	mSessionObject->AddRef();
}

ClientSession::ClientSession() : mBuffer(BUFSIZE), mConnected(0), mRefCount(0)
{
	memset(&mClientAddr, 0, sizeof(SOCKADDR_IN));
	mSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
}


void ClientSession::SessionReset()
{
	mConnected = 0;
	mRefCount = 0;
	memset(&mClientAddr, 0, sizeof(SOCKADDR_IN));

	mBuffer.BufferReset();

	LINGER lingerOption;
	lingerOption.l_onoff = 1;
	lingerOption.l_linger = 0;

	/// no TCP TIME_WAIT
	if (SOCKET_ERROR == setsockopt(mSocket, SOL_SOCKET, SO_LINGER, (char*)&lingerOption, sizeof(LINGER)))
	{
		printf_s("[DEBUG] setsockopt linger option error: %d\n", GetLastError());
	}
	closesocket(mSocket);

	mSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
}

bool ClientSession::PostAccept()
{
	CRASH_ASSERT(LThreadType == THREAD_MAIN);

	OverlappedAcceptContext* acceptContext = new OverlappedAcceptContext(this);
	//TODO : AccpetEx를 이용한 구현.

	acceptContext->mWsaBuf.buf = nullptr;
	acceptContext->mWsaBuf.len = 0;

	DWORD dwBytes = 0;
	//char buffer[128] = {0, }; ///# 이거 스택상에서 할당된 버퍼를 아중에  AcceptEx시에 다른 곳에서 여기에 접근해서 데이터를 쓰는 경우 우째될까? => 접근 오류발생할거같네요..
	//기존에 있던 circularBuffer에 넣는 걸로 바꿨습니다.

	if(FALSE == _AcceptEx(
		*(GIocpManager->GetListenSocket()), mSocket, 
		mBuffer.GetBuffer(), 0, 
		sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, 
		&dwBytes, (LPOVERLAPPED)acceptContext))
	{
		DWORD errorId = WSAGetLastError();
		if(errorId != WSA_IO_PENDING)
		{
			printf("[DEBUG] AcceptEx error: %d\n", errorId);
			DeleteIoContext(acceptContext);
			return false;
		}
	}
	return true;
}

bool ClientSession::AcceptCompletion()
{
	CRASH_ASSERT(LThreadType == THREAD_IO_WORKER);
	
	if (1 == InterlockedExchange(&mConnected, 1))
	{
		/// already exists?
		CRASH_ASSERT(false);
		return true;
	}

	bool resultOk = true;
	//클라 소켓이 listen socket 속성 상속받기
	if(SOCKET_ERROR == setsockopt(mSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*) GIocpManager->GetListenSocket(), sizeof(SOCKET)))
	{
		printf_s("[DEBUG] SO_UPDATE_ACCEPT_CONTEXT error: %d\n", GetLastError());
		return false;
	}

	//네이글 off
	int opt = 1;
	if(SOCKET_ERROR == setsockopt(mSocket, IPPROTO_TCP, TCP_NODELAY, (const char*) &opt, sizeof(int)))
	{
		printf_s("[DEBUG] TCP_NODELAY error: %d\n", GetLastError());
		return false;
	}

	//커널버퍼 0 직접 매핑 -> zero recv byte
	opt = 0;
	if(SOCKET_ERROR == setsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, (const char*) &opt, sizeof(int)))
	{
		printf_s("[DEBUG] SO_RCVBUF change error: %d\n", GetLastError());
		return false;
	}


	int addrlen = sizeof(SOCKADDR_IN);
	if(SOCKET_ERROR == getpeername(mSocket, (SOCKADDR*) &mClientAddr, &addrlen))
	{
		printf_s("[DEBUG] getpeername error: %d\n", GetLastError());
		return false;
	}

	//TODO: CreateIoCompletionPort를 이용한 소켓 연결
	HANDLE handle = CreateIoCompletionPort(
		(HANDLE) mSocket, GIocpManager->GetComletionPort(), ( ULONG_PTR )this, 0);

	if(handle != GIocpManager->GetComletionPort())
	{
		printf_s("[DEBUG] CreateIoCompletionPort error: %d\n", GetLastError());
		return false;
	}


	//zero bytes recv
	if (false == PreRecv())
	{
		printf_s("[DEBUG] PreRecv error: %d\n", GetLastError());
		return false;
	}

	printf_s("[DEBUG] Client Connected: IP=%s, PORT=%d\n", inet_ntoa(mClientAddr.sin_addr), ntohs(mClientAddr.sin_port));
	return true;
}


void ClientSession::DisconnectRequest(DisconnectReason dr)
{
	/// 이미 끊겼거나 끊기는 중이거나
	if (0 == InterlockedExchange(&mConnected, 0))
		return ;
	
	OverlappedDisconnectContext* context = new OverlappedDisconnectContext(this, dr);

	//TODO: DisconnectEx를 이용한 연결 끊기 요청

	context->mWsaBuf.buf = nullptr;
	context->mWsaBuf.len = 0;
	if(FALSE == DisconnectEx(mSocket, (LPOVERLAPPED) context, NULL, 0))
	{
		if(WSA_IO_PENDING != WSAGetLastError())
		{
			printf_s("[DEBUG] DisconnectEx error: %d\n", GetLastError());
			DeleteIoContext(context);
		}
	}
	
}

void ClientSession::DisconnectCompletion(DisconnectReason dr)
{
	printf_s("[DEBUG] Client Disconnected: Reason=%d IP=%s, PORT=%d \n", dr, inet_ntoa(mClientAddr.sin_addr), ntohs(mClientAddr.sin_port));

	/// release refcount when added at issuing a session
	ReleaseRef();
}


bool ClientSession::PreRecv()
{
	if (!IsConnected())
		return false;

	OverlappedPreRecvContext* recvContext = new OverlappedPreRecvContext(this);
	
	//TODO: zero-byte recv 구현
	recvContext->mWsaBuf.buf = nullptr;
	recvContext->mWsaBuf.len = 0;
	DWORD recvBytes = 0;
	DWORD flags = 0;

	if(SOCKET_ERROR == WSARecv(
		mSocket, &recvContext->mWsaBuf, 
		1,&recvBytes, &flags, 
		(LPOVERLAPPED) recvContext, NULL))
	{
		DWORD errorId = WSAGetLastError();
		if(WSA_IO_PENDING != errorId)
		{
			printf("[DEBUG] PreRecv error : %d\n", errorId);
			DeleteIoContext(recvContext);
			return false;
		}
	}

	return true;
}

bool ClientSession::PostRecv()
{
	if (!IsConnected())
		return false;

	FastSpinlockGuard criticalSection(mBufferLock);

	if (0 == mBuffer.GetFreeSpaceSize())
		return false;

	OverlappedRecvContext* recvContext = new OverlappedRecvContext(this);

	DWORD recvbytes = 0;
	DWORD flags = 0;
	recvContext->mWsaBuf.len = (ULONG)mBuffer.GetFreeSpaceSize();
	recvContext->mWsaBuf.buf = mBuffer.GetBuffer();
	

	/// start real recv
	if (SOCKET_ERROR == WSARecv(mSocket, &recvContext->mWsaBuf, 1, &recvbytes, &flags, (LPWSAOVERLAPPED)recvContext, NULL))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			DeleteIoContext(recvContext);
			printf_s("ClientSession::PostRecv Error : %d\n", GetLastError());
			return false;
		}
			
	}

	return true;
}

void ClientSession::RecvCompletion(DWORD transferred)
{
	FastSpinlockGuard criticalSection(mBufferLock);

	mBuffer.Commit(transferred);
}

bool ClientSession::PostSend()
{
	if (!IsConnected())
		return false;

	FastSpinlockGuard criticalSection(mBufferLock);

	if ( 0 == mBuffer.GetContiguiousBytes() )
		return true;

	OverlappedSendContext* sendContext = new OverlappedSendContext(this);

	DWORD sendbytes = 0;
	DWORD flags = 0;
	sendContext->mWsaBuf.len = (ULONG) mBuffer.GetContiguiousBytes(); 
	sendContext->mWsaBuf.buf = mBuffer.GetBufferStart();

	/// start async send
	if (SOCKET_ERROR == WSASend(mSocket, &sendContext->mWsaBuf, 1, &sendbytes, flags, (LPWSAOVERLAPPED)sendContext, NULL))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			DeleteIoContext(sendContext);
			printf_s("ClientSession::PostSend Error : %d\n", GetLastError());

			return false;
		}
			
	}

	return true;
}

void ClientSession::SendCompletion(DWORD transferred)
{
	FastSpinlockGuard criticalSection(mBufferLock);

	mBuffer.Remove(transferred);
}


void ClientSession::AddRef()
{
	CRASH_ASSERT(InterlockedIncrement(&mRefCount) > 0);
}

void ClientSession::ReleaseRef()
{
	long ret = InterlockedDecrement(&mRefCount);
	CRASH_ASSERT(ret >= 0);
	
	if (ret == 0)
	{
		//클라이언트 해제시 세션 풀에 반환
		GSessionManager->ReturnClientSession(this);
	}
}


void DeleteIoContext(OverlappedIOContext* context)
{
	if (nullptr == context)
		return;

	context->mSessionObject->ReleaseRef();

	delete context;

	
}

