#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// Windows API Headers
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

// Other Headers
#include <iostream>
#include <vector>
#include <format>
#include <string>

namespace m0st4fa {

	int setupWinsock();

	std::string toString(const sockaddr_storage*);

	class ConnectionInformation {

		addrinfo mMyHints{};
		addrinfo* mMyInfo{ nullptr };
		addrinfo* mMyBoundName{ nullptr };

	protected:
		int pMySockFd = 0;

		int _initialize_connection(const sockaddr_storage* const, int) const;
		std::string_view _receive_sentence(const int, char* const, const size_t) const;
		virtual std::string _format(const std::string_view msg) const = 0;
		virtual std::string _format(const std::string_view msg, const std::string_view arg1) const = 0;
		virtual std::string _format(const std::string_view msg, const std::string_view arg1, const std::string_view arg2) const = 0;

	public:

		unsigned int BACK_LOG = 10;

		ConnectionInformation() {
			// Initialize to zero
			memset(this, 0, sizeof(ConnectionInformation));

			// Set hints
			mMyHints.ai_family = AF_INET;
			mMyHints.ai_socktype = SOCK_STREAM;
			mMyHints.ai_flags = AI_PASSIVE;

		}
		~ConnectionInformation() {
			::freeaddrinfo(mMyInfo);
			::closesocket(this->pMySockFd);
		}

		int setDeviceAddress(const unsigned int);

		int assignSocket();

		operator std::string() const;

		/**
		 * @return Bound port in host byte-order.
		 */
		int getBoundPort() const {
			sockaddr_in* addr = (sockaddr_in*)this->mMyBoundName->ai_addr;
			return ntohs(addr->sin_port);
		}

	};

}
