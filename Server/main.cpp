#include <iostream>

#include "Nocturn/Platform/logging.hpp"
#include "Nocturn/Platform/platform.hpp"

#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <stack>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>


// Need to link with Ws2_32.lib
#pragma comment( lib, "Ws2_32.lib" )
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

using namespace Nocturn;

constexpr uint32 CMaxClients      = 10;
constexpr uint32 CWorkerThreads	  = 2;
constexpr uint32 CMaxMessageLen   = 1024;
constexpr uint16 CMaxBufferLength = 0xffff; 

struct Buffer
{
	OVERLAPPED	overlapped{ };
	uint8_t		buffer[ CMaxBufferLength ]{ };
	bool		isRecvOrSend{ };
	bool		isHeaderOrBody{ };
	uint32		connectionIndex{ };
};

enum class Opcode : uint16_t
{
	ChatMessage = 0
	,	End
};

struct PacketHeader
{
	Opcode opcode;
	uint16 length;
};

struct PacketMessage : PacketHeader
{
	char message[ CMaxMessageLen ];
};

struct Client
{
	SOCKET ClientSocket{ };
	uint32 id{ };

	RStatus RecvAsync( uint32 recvLenth = 4, const bool isRecvHeaderOrBody = true ) const noexcept
	{
		std::unique_ptr<Buffer> buffer;

		WSABUF wsaBuf
		{
			.len = recvLenth,
			.buf = reinterpret_cast< char * >( buffer->buffer + recvLenth )
		};

		DWORD numberOfBytesRecvd;
		DWORD flags;
		// Very good explanation about Overlapped structure and about its purpose
		// https://stackoverflow.com/questions/28927832/what-does-context-information-means-when-talking-about-overlapped-i-o
		const auto Result = ::WSARecv( ClientSocket, &wsaBuf, 1, &numberOfBytesRecvd, &flags, &buffer->overlapped, nullptr );
		if( SOCKET_ERROR == Result )
		{
			return RFail;
		}

		return RSucces;
	}

	RStatus SendAsync( uint32 recvLength, const bool isRecvHeaderOrBody = true )
	{
		return RSucces;
	}
};

SOCKET               ListenSocket;
HANDLE               iocp;
Client               clients[ 10 ];
std::stack< uint32 > freeClients;
std::mutex			 freeClientsLock;

RStatus InitServer( )
{
	WSADATA wsaData;

	auto Result = ::WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
	if( Result != 0 )
	{
		Log( "WSAStartup failed with error: {}\n", Result );
		return RFail;
	}

	addrinfo hints{ };
	ZeroMemory( &hints, sizeof( hints ) );
	hints.ai_family	  = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags	  = AI_PASSIVE;

	addrinfo *result = nullptr;
	Result = ::getaddrinfo( nullptr, DEFAULT_PORT, &hints, &result );
	if( Result != 0 )
	{
		Log( "getaddrinfo failed with error: {}", Result );
		::WSACleanup( );
		return RFail;
	}

	ListenSocket = ::socket( result->ai_family, result->ai_socktype, result->ai_protocol );
	if( ListenSocket == INVALID_SOCKET )
	{
		Log( "socket failed with error: {}", WSAGetLastError( ) );
		::freeaddrinfo( result );
		::WSACleanup( );
		return RFail;
	}

	Result = ::bind( ListenSocket, result->ai_addr, static_cast< int32 >( result->ai_addrlen ) );
	if( Result == SOCKET_ERROR )
	{
		Log( "bind failed with error: {}", WSAGetLastError( ) );
		::freeaddrinfo( result );
		::closesocket( ListenSocket );
		::WSACleanup( );
		return RFail;
	}

	::freeaddrinfo( result );

	Result = ::listen( ListenSocket, SOMAXCONN );
	if( Result == SOCKET_ERROR )
	{
		Log( "listen failed with error: {}", ::WSAGetLastError( ) );
		::closesocket( ListenSocket );
		::WSACleanup( );
		return RFail;
	}

	for( uint32 i = 0; i < CMaxClients + 1; ++i )
	{
		if (i > 0)
			clients[i].id = i;
		freeClients.push( i );
	}

	// Creates an I/O completion port that is not yet associated with a file handle, allowing association at a later time
	iocp = ::CreateIoCompletionPort( INVALID_HANDLE_VALUE, nullptr, 0, 0 );
	if (nullptr == iocp)
	{
		Log( "Error occured when creating IOCP {}", ::GetLastError( ) );
		return RFail;
	}

	return RSucces;
}

void WaitConnection( )
{
	while( true )
	{
		const auto acceptedSocket = accept( ListenSocket, nullptr, nullptr );
		if( acceptedSocket == INVALID_SOCKET )
		{
			Log( "Accept client failed with error: {}", WSAGetLastError( ) );
			closesocket( ListenSocket );
			WSACleanup( );
			break;
		}

		uint32 newClientIndex = 0;
		{
			std::lock_guard lock(freeClientsLock);
			if( false == freeClients.empty( ) )
			{
				newClientIndex = freeClients.top( );
				freeClients.pop( );
			}
		}
		if( newClientIndex == 0 )
		{
			Log( "Cannot accept more clients!" );
			shutdown( acceptedSocket, SD_BOTH );
			closesocket( acceptedSocket );
		}
		else
		{
			const auto Result = CreateIoCompletionPort( reinterpret_cast< HANDLE >( acceptedSocket ), iocp, newClientIndex, CWorkerThreads );
			if (nullptr == Result)
			{
				Log( "Cannot associate the new client with AsyncIO." );
				shutdown( acceptedSocket, SD_BOTH );
				closesocket( acceptedSocket );

				{
					std::lock_guard lock( freeClientsLock );
					freeClients.push( newClientIndex );
				}
				continue;
			}

			Log( "Succesfully accepted the new client {}", newClientIndex );
			auto &currentClient = clients[ newClientIndex ];
			currentClient.ClientSocket = acceptedSocket;
		}
	}
}

RStatus WorkerRoutine( )
{
	DWORD numberOfBytesTransfered;
	uint64 completionKey;
	OVERLAPPED *overlaped;

	while( true )
	{
		const auto Result = ::GetQueuedCompletionStatus( iocp, &numberOfBytesTransfered, &completionKey, &overlaped, INFINITE );
		if( false == Result )
		{
			Log( "Failed to associate socket to the IOCP WsaError: {}", ::GetLastError( ) );
			return RFail;
		}

		auto buffer = reinterpret_cast< Buffer * >( overlaped );
		auto client = clients[ buffer->connectionIndex ];

		if( 0 == numberOfBytesTransfered )
		{
			// i think i have to shutdown and close socket - need to check
		}
		else
		{
			if( true == buffer->isRecvOrSend )
			{
				// Process header packet
				if( true == buffer->isHeaderOrBody )
				{
					auto packetHeader = reinterpret_cast< PacketHeader * >( buffer->buffer );
				}
			}
		}
	}
}

int main( )
{
	if( RSucces != InitServer( ) )
	{
		Log( "Failed to initialize server!" );
	}

	std::jthread WaitingThread( [ & ]
	{
		WaitConnection( );
	});

	WorkerRoutine( );

	return 0;
}