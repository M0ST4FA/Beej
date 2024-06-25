#include "common.h"
#include <string>

namespace m0st4fa {

	/**
	 * @brief Converts an object of type sockaddr_storage to a string.
	 * @param[in] addr Pointer to the object to be converted.
	 * @returns A string representation of `addr`.
	 */
	std::string toString(const sockaddr_storage* addr) {
		const size_t ipAddrSz = 1000;
		char ipAddr[ipAddrSz] = {};
		inet_ntop(addr->ss_family, addr, ipAddr, ipAddrSz);

		unsigned short port = ntohs(((sockaddr_in*)addr)->sin_port);

		return std::format("IP: {}, port: {}", ipAddr, port);
	};

	/**
	 * @brief Initializes Winsock library. It does not work without this step!
	 */
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
			std::cout << _format("Could not obtain the localhost address: {}\n", gai_strerrorA(rv));
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
				std::cout << _format("Failed to create listening socket for address {}: {}\n" , toString((const sockaddr_storage*)p->ai_addr).data(), strerror(e));
				continue;
			}

			// Bind the socket
			rv = bind(pMySockFd, this->mMyInfo->ai_addr, this->mMyInfo->ai_addrlen);
			e = errno;
			if (rv == -1) {
				std::cout << _format("Failed to bind listening socket '{}': {}\n" , std::to_string(pMySockFd).data(), strerror(e));
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
	 * @param[in] sockFd The descriptor of the socket bound for the connection with the client.
	 * @param[in] connectedAddr The address of the connected client.
	 * @returns The return value of the call to `send`.
	 */
	int ConnectionInformation::_initialize_connection(int sockFd, const sockaddr_storage* const connectedAddr) const
	{
		std::cout << _format("Accepted connection from {}\n", toString(connectedAddr).data());

		std::string msg = std::format("Welcome {}!\r\n> ", sockFd); // Temporary variable to store messages
		int remainingBytes = send(sockFd, msg);

		int e = errno;
		if (remainingBytes > 0) {
			std::cout << _format("Cannot send message \"{}\" to peer: {}\n", msg, strerror(errno));
			std::abort();
		}

		return 0;
	}

	/**
	 * @brief Receives entire sentences from telnet application. It returns after the application sends `\r\lf`.
	 * @param[in] sockFd The socket from which to receive data.
	 * @param[in] buf The buffer to use to receive the information (this is in order not to keep allocating buffers over and over again.)
	 * @param[in] bufSize The size of `buf`.
	 * @param[in] nbyte The number of bytes have been received.
	 * @return The received string.
	 */
	std::string_view ConnectionInformation::_receive_sentence(const int sockFd, char* const buf, const size_t bufSize, int& nbyte) const
	{
		int err = 0;
		int msgSz = 0;
		int nextChar = 0;
		while (nextChar < bufSize) {
			int rd = ::recv(sockFd, buf + msgSz, bufSize - msgSz, 0); // rd = received data
			err = errno;

			msgSz += rd;


			// if there's an error
			if (rd == -1) {
				std::cout << _format("An error returned while receiving data: {}\n", strerror(err));
				std::exit(-1);
			}

			// if connection has been closed
			if (rd == 0) {
				nbyte = 0;
				return std::string_view{}; // it is impossible to close the connection and yet return any string
			}

			char* const lastCharLoc = buf + msgSz - 1;
			if (*lastCharLoc == '\n') {
				*(lastCharLoc - 1) = 0;
				return std::string_view(buf, msgSz - 1);
			}

		}

		return std::string_view{ buf };
	}

	/**
	 * @brief Close socket with file descriptor `sockFd` (if present; otherwise do nothing.)
	 * @returns `0` if socket was closed; `1` if there was no socket open; `-1` if an error occurred during socket closure.
	 */
	int ConnectionInformation::_closeSocket(int sockFd)
	{

		if (sockFd != 0) {
			return 1;
		}

		int rv = ::closesocket(sockFd);
		int e = errno;

		std::cout << this->_format("Error while closing socket: {}\n", strerror(e));

		return rv == 0 ? 0 : -1;

		return 0;
	}

	/**
	 * @brief Converts this object to a string.
	 * @returns The string representation of this object.
	 */
	ConnectionInformation::operator std::string() const
	{
		char buf[1001];

		inet_ntop(this->mMyBoundName->ai_family, (const void*)this->mMyBoundName->ai_addr, buf, 1000);

		return std::format("IP: {}, port: {}", buf, this->getBoundPort());
	}

	/**
	 * @brief Close current connection (if present; otherwise do nothing.)
	 * @returns `0` if socket was closed; `1` if there was no socket open; `-1` if an error occurred during socket closure.
	 */
	int ConnectionInformation::closeCurrentConnection()
	{
		int rv = this->_closeSocket(this->pMySockFd);
		this->pMySockFd = 0;

		return rv;
	}

	// TODO: THIS NEEDS A COMPLETE CHANGE AFTER ENCAPSULATING DATA TO BE ABLE TO GET DATA OF ANY LENGTH.
	// TODO: Make it use std::string instead to not have memory leaks.
	/**
	 * @brief Receives data from a particular socket.
	 * @param[in] sockFd The socket from which to receive data.
	 * @param[in] byteN The size of the data to be received.
	 * @param[in] nbytes The number of bytes that have been received.
	 * @returns The received data.
	 */
	std::string_view ConnectionInformation::receive(const int sockFd, const size_t byteN, int& nbytes)
	{
		char* buf = new char[byteN] {};
		nbytes = ::recv(sockFd, buf, byteN, 0);

		return std::string_view{ buf };
	}

	/**
	 * @brief Sends `msg` to connected server.
	 * @param[in] msg Message to be sent.
	 * @returns `0` if everything was sent successfully; otherwise, the number of bytes that remain to be sent (which implies an error has occurred while sending the data.)
	 */
	int ConnectionInformation::send(const int sockFd, const std::string_view msg)
	{

		size_t total = msg.length();
		int rv = 0;
		size_t remaining = total;

		while (remaining != 0) {
			rv = ::send(sockFd, msg.data(), msg.length(), 0);

			// if there was an error while sending data
			if (rv == -1) return remaining; // return how many characters remain to be sent

			remaining -= rv;
		}

		return 0;
	}

	std::string ConnectionInformation::formatFckingMSErrorMessages(const int errorCode)
	{
		LPTSTR errorString = NULL; // Pointer to store formatted message

		DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM;

		int result = FormatMessage(
			flags,
			NULL, // Source (use NULL for system messages)
			errorCode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR)&errorString,
			0,
			NULL
		);

		return std::string();
	}

	/**
	 * @brief Adds a file descriptor to the collection.
	 * @param[in] socketFd The file descriptor.
	 * @param[in] eventBitmap The events you want to poll for.
	 * @returns void
	 */
	void Sockets::add(const int socketFd, const int eventBitmap)
	{

		// if we're out of space on the heap
		if (length == capacity) {
			capacity *= 2; // duplicate the capacity
			pollfd* n = new pollfd[capacity]; // allocate new space for the objects

			// copy pollfd objects. I do this instead of using memcpy because I might implement some struct in the future, and this would copy the objects correctly (by calling constructures.)
			for (int i = 0; i <= length; i++)
				n[i] = this->sockets[i];

			delete[] this->sockets;
			this->sockets = n;
		}

		// construct new file descriptor object
		pollfd newFd{socketFd, eventBitmap};

		// add object
		this->sockets[length++] = newFd;
	}

	/**
	 * @brief Removes a file descriptor from the collection.
	 * @param[in] socketFd The file descriptor to be removed.
	 * @returns `0` if the file descriptor was removed; `-1` otherwise (e.g., it didn't actually exist.)
	 */
	int Sockets::remove(const int socketFd)
	{
		
		for (int i = 0; i <= this->length; i++) {
			pollfd& curr = this->sockets[i];

			if (curr.fd == socketFd) {
				curr = this->sockets[length - 1];
				this->length--;
				return 0; // because we found and eliminated the socket
			}

		}

		return -1; // if we didn't find the socket (and thus didn't eliminate it)
	}

}
