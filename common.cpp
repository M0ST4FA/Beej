#include "common.h"
#include <string>

namespace m0st4fa {

	std::string toString(const sockaddr_storage* addr) {
		const size_t ipAddrSz = 1000;
		char ipAddr[ipAddrSz] = {};
		inet_ntop(addr->ss_family, addr, ipAddr, ipAddrSz);

		unsigned short port = ntohs(((sockaddr_in*)addr)->sin_port);

		return std::format("IP: {}, port: {}", ipAddr, port);
	};

	int setupWinsock() {

		// Set up the WinSock library
		WORD wVersionRequested;
		WSADATA wsaData;
		int err;

		/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
		wVersionRequested = MAKEWORD(2, 2);

		err = WSAStartup(wVersionRequested, &wsaData);
		if (err != 0) {
			/* Tell the user that we could not find a usable */
			/* Winsock DLL.                                  */
			printf("WSAStartup failed with error: %d\n", err);
			return 1;
		}

		return 0;
	}

	/**
	 * @brief Populates the address of this device according to hints.
	 * @returns The value returned by `getaddrinfo` (important for error checking).
	 */
	int ConnectionInformation::setDeviceAddress(const unsigned int myPort) {

		// Get my address
		int rv = getaddrinfo(nullptr, std::to_string(myPort).c_str(), &mMyHints, &this->mMyInfo);

		if (rv != 0) {
			std::cout << _format("Could not obtain the localhost address: {}.\n", gai_strerrorA(rv));
			std::abort();
		};

		return rv;
	}

	/**
	 * @brief Binds the first address of our server (that, presumably it got by calling setDeviceAddress before) to a socket. It aborts the process in case of any error.
	 * @returns The value returned by `bind` (important for error checking).
	 */
	int ConnectionInformation::assignSocket()
	{
		addrinfo* p = nullptr;
		int rv = 0;

		// Bind the first name we get to a socket
		for (p = this->mMyInfo; p != nullptr; this->mMyInfo->ai_next) {

			// Get a new socket
			pMySockFd = socket(this->mMyInfo->ai_family, this->mMyInfo->ai_socktype, 0);

			int e = errno;

			if (pMySockFd == -1) {
				std::cout << _format("Failed to create listening socket for address {}: {}.\n" , toString((const sockaddr_storage*)p->ai_addr).data(), strerror(e));
				continue;
			}

			// Bind the socket
			rv = bind(pMySockFd, this->mMyInfo->ai_addr, this->mMyInfo->ai_addrlen);
			e = errno;
			if (rv == -1) {
				std::cout << _format("Failed to bind listening socket '{}': {}.\n" , std::to_string(pMySockFd).data(), strerror(e));
				continue;
			}

			// If we reach here, we've created a socket and bound it to a port
			break; // We're done
		}

		if (p == NULL) {
			std::cout << _format("Failed to create or bind a listening socket!");
			std::abort();
		}

		this->mMyBoundName = p;

		return rv;
	}

	/**
	 * @brief Initializes connection with the server. It prints the server's address and sends "Hello world!" to the server.
	 * @param[in] connectedAddr The address of the connected client.
	 * @param[in] sockFd The descriptor of the socket bound for the connection with the client.
	 * @param[in] buf The buffer to use to receive the information (this is in order not to keep allocating buffers over and over again.)
	 * @param[in] bufSz The size of the `buf`.
	 * @returns The return value of the call to `send`.
	 */
	int ConnectionInformation::_initialize_connection(const sockaddr_storage* const connectedAddr, int sockFd) const
	{
		std::cout << _format("Accepted connection from {}.\n", toString(connectedAddr).data());

		std::string msg = "Hello world!\r\n> "; // Temporary variable to store messages
		int sendRv = send(sockFd, msg.c_str(), msg.size() + 1, 0);

		int e = errno;
		if (sendRv == -1) {
			std::cout << _format("Cannot send message \"{}\" to peer: {}.\n", msg, strerror(errno));
			std::abort();
		}

		return 0;
	}

	/**
	 * @brief Receives entire sentences from telnet application. It returns after the application sends `\r\lf`.
	 * @param[in] sockFd The socket from which to receive data.
	 * @param[in] buf The buffer to use to receive the information (this is in order not to keep allocating buffers over and over again.)
	 * @param[in] bufSize The size of `buf`.
	 * @return The received string.
	 */
	std::string_view ConnectionInformation::_receive_sentence(const int sockFd, char* const buf, const size_t bufSize) const
	{

		int msgSz = 0;
		int nextChar = 0;
		while (nextChar < bufSize) {
			int rd = ::recv(sockFd, buf + msgSz, bufSize - msgSz, 0); // rd = received data
			msgSz += rd;

			int err = errno;
			if (rd == -1) {
				std::cout << _format("An error returned while receiving data: {}.\n", strerror(err));
				return std::string_view("Close");
			}

			if (rd == 0) {
				std::cout << "[server] Client terminated session!\n";
				return std::string_view("Close");
			}

			char* const lastCharLoc = buf + msgSz - 1;
			if (*lastCharLoc == '\n') {
				*(lastCharLoc - 1) = 0;
				return std::string_view(buf, msgSz - 1);
			}

		}

		return std::string_view(buf);
	}

	ConnectionInformation::operator std::string() const
	{
		char buf[1001];

		inet_ntop(this->mMyBoundName->ai_family, (const void*)this->mMyBoundName->ai_addr, buf, 1000);

		return std::format("IP: {}, port: {}", buf, this->getBoundPort());
	}
}
