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

		int _initialize_connection(int, const sockaddr_storage* const) const;
		std::string_view _receive_sentence(const int, char* const, const size_t, int&) const;
		virtual std::string _format(const std::string_view msg) const = 0;
		virtual std::string _format(const std::string_view msg, const std::string_view arg1) const = 0;
		virtual std::string _format(const std::string_view msg, const std::string_view arg1, const std::string_view arg2) const = 0;

		int _closeSocket(int sockFd);

	public:

		static std::string_view receive(const int, const size_t, int&);
		static int send(const int, const std::string_view);
		static std::string formatFckingMSErrorMessages(const int);
		static constexpr unsigned int BACK_LOG = 10;

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
			this->closeCurrentConnection();
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
		int closeCurrentConnection();

	};

	class Sockets {

		pollfd* sockets;
		size_t length = 0;
		size_t capacity;

	public:
		Sockets(size_t initSz = 10) : capacity{initSz}, sockets { new pollfd[initSz] } {
		}

		/**
		 * @brief Gets the element at index `i`.
		 * @param[in] i The index of the element to be retrieved.
		 * @returns A copy of the element at index `i`.
		 */
		pollfd at(const size_t i) const {
			return this->sockets[i];
		};
		void add(const int, const int);
		int remove(const int);

		/**
		 * @brief Gets the underlying `pollfd` collection.
		 * @returns The underlying set of `pollfd` collection.
		 */
		pollfd* getSockets() const {
			return this->sockets;
		}

		/**
		 * @brief Gets the number of elements stored in the collection.
		 * @returns The number of elements stored in the collection.
		 */
		size_t getLength() const {
			return this->length;
		}

	};

}
