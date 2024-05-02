/**
 * 
 * 
 * ##Demo	Tcp Server
 * 
 **/
 
 
#include<iostream>
#include<WinSock2.h>
#include<mswsock.h>
using namespace std;
//#pragma comment(lib,"ws2_32.lib")
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <memory>

#include "../Src/IOSchedule/Reactor.hpp"
#include "../Src/Utility/ScopeGuard.hpp"
#include "../Src/Coroutine/Coroutine.hpp"
#include "../Src/Coroutine/Generator.hpp"
#include "../Src/Utility/Error.hpp"

 

int main() {
	/* ##: initialize wsock dyn-lib */
	WORD sockVersion = MAKEWORD(2,2);
	WSADATA wsaData; 
	if(WSAStartup(sockVersion,&wsaData)) 
		std::exit(EXIT_FAILURE);
	std::atexit(static_cast<void(*)()>([] {
		return (void) WSACleanup();
	}));
	/* ##: initialize wsock dyn-lib */
		
	/* ##: get listening socket handle */
	SOCKET listenFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(listenFD == INVALID_SOCKET) {
		std::perror("listen");
		Kelpa::Utility::WSAError();
		std::exit(EXIT_FAILURE); 
	}
	/* ##: get listening socket handle */
	
	/* ##: bind address and port */
	struct sockaddr_in serverAddr;
	std::memset(std::addressof(serverAddr), 0, sizeof serverAddr);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8888);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	
	
	if(bind(listenFD, reinterpret_cast<struct sockaddr *>(std::addressof(serverAddr)), sizeof serverAddr) == SOCKET_ERROR) {
		std::perror("bind");
		Kelpa::Utility::WSAError();
		std::exit(EXIT_FAILURE);
	}
	/* ##: bind address and port */

	/* ##: set listening */
	if(listen(listenFD, 64) == SOCKET_ERROR) {
		std::perror("listen");
		Kelpa::Utility::WSAError();	
		std::exit(EXIT_FAILURE);			
	}
	
	using namespace Kelpa;
	Kelpa::Utility::ScopeGuard Closer([&] {
		if(listenFD != INVALID_SOCKET)
			closesocket(listenFD);		
	});
	/* ##: set listening */
	
	/* ##: initilize reactor */
	IOSchedule::Reactor reactor;
	reactor.lometer.Schedule(
		Thread::Timer()
			.Build()
			.SetCallback([] { 
				std::puts("other timer callback");	
			})
			.Continue()
	);
	/* ##: initilize reactor */

	/* ##: set listening handle to non-block pattern */
	BOOL Yes { true };
	if(setsockopt(listenFD, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char const*>(std::addressof(Yes)), sizeof Yes) == SOCKET_ERROR) {
		std::perror("setsockopt");
		Kelpa::Utility::WSAError();			
		std::exit(EXIT_FAILURE);
	}
	/* ##: set listening handle to non-block pattern */
	
	/* ##: listening acceptence */
	reactor.Listen(IOSchedule::Event::READ, listenFD, [&] {
		SOCKET acceptFD {};
		if((acceptFD = accept(listenFD, nullptr, nullptr)) == INVALID_SOCKET) {
			std::perror("setsockopt");
			Kelpa::Utility::WSAError();	
			std::exit(EXIT_FAILURE);		
		}
	  	unsigned long SetNonBlocking = 1;  
	    if (ioctlsocket(acceptFD, FIONBIO, std::addressof(SetNonBlocking)) == SOCKET_ERROR) {  
			std::perror("ioctlsocket");
			Kelpa::Utility::WSAError();	
	        closesocket(acceptFD);  
	        acceptFD = INVALID_SOCKET;  
	        return;
	    }
	    
		if(setsockopt(acceptFD, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char const*>(std::addressof(Yes)), sizeof Yes) == SOCKET_ERROR) {
			std::perror("setsockopt");
			Kelpa::Utility::WSAError();			
			std::exit(EXIT_FAILURE);
		}
		/* ##: listening http request */
		reactor.Listen(IOSchedule::Event::READ, acceptFD, std::invoke(
		[&]() mutable  -> Coroutine::WeakFuture<> {
			char 			temp[1024] = {0};
			char 			buffer[4096] = {0};
			signed long  	length = 0;
			std::size_t  	total = 0;
			while(true) {
				std::puts("one request recved");
				std::memset(temp, 0, std::size(temp));
				std::memset(buffer, 0, std::size(buffer));
				length = 0;
				total = 0;
				
				/* ##: read all */
				while((length = recv(acceptFD, temp, std::size(temp), 0)) != SOCKET_ERROR) {
					if(total + length < std::size(buffer))
						std::memcpy(buffer + total, temp, length);
					total += length;
				}
				/* ##: parse http-request head line */
				if(length == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK) {
					char *p = std::strstr(buffer, "\r\n");
					buffer[p - buffer] = '\0';
					/* ##: send back to client peer */
					if(send(acceptFD, buffer, std::strlen(buffer) + 1, 0) == SOCKET_ERROR) {
						std::perror("send");
						Kelpa::Utility::WSAError();
						closesocket(acceptFD);
						std::exit(EXIT_FAILURE);
					}
					std::printf("%s\n", buffer);
					/* ##: ET pattern*/
					reactor.Cancel(acceptFD);					
				}
					
				else if(length == 0) {
					/* ##: Elegant closure of linkage */
					closesocket(acceptFD);
					co_return;
				} else {
					std::perror("recv");
					Kelpa::Utility::WSAError();
					closesocket(acceptFD);
					std::exit(EXIT_FAILURE);				
				}
				/* ##: wait for next request */
				co_await std::suspend_always();
			}	
		})
		);
	});
	std::getchar();
	return EXIT_SUCCESS;
}
