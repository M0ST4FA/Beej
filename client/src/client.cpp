#include "include/client.h"
#include <string>

namespace m0st4fa {

	/**
	 * @brief Sets the address of the server to which we will be connecting.
	 * @param[in] serverAddr The address of the server to which we will be connecting.
	 * @param[in] serverPort The port of the server on the machine on which it is running.
	 * @returns The return value of `getaddrinfo`.
	 */
	int Client::_set_server_address(const std::string serverAddr, const int serverPort)
	{
		// Get my address
		int rv = getaddrinfo(serverAddr.c_str(), std::to_string(serverPort).c_str(), &mServerHints, &this->mServerInfo);
		if (rv != 0) {
			std::cout << std::format("getaddrinfo: {}\n", gai_strerrorA(rv));
		};

		return 0;
	}

	/**
	 * @brief Formats `msg` for standard client stream.
	 * @param[in] msg The message to be formatted.
	 * @returns A string formatted for standard client stream.
	 */
	std::string Client::_format_client(const std::string_view msg)
	{
		return "[client] " + std::string(msg);
	}

	/**
	 * @brief Implements formatting for standard client stream with 0 format arguments.
	 * @param[in] msg The message to be formatted.
	 * @returns A string formatted for standard client stream.
	 */
	std::string Client::_format(const std::string_view msg) const
	{
		return _format_client(msg);
	}

	/**
	 * @brief Implements formatting for standard client stream with 1 format arguments.
	 * @param[in] msg The message to be formatted.
	 * @param[in] arg1 First formatting argument.
	 * @returns A string formatted for standard client stream.
	 */
	std::string Client::_format(const std::string_view msg, const std::string_view arg1) const
	{
		return std::vformat(_format_client(msg).data(), std::make_format_args(arg1));
	}

	/**
	 * @brief Implements formatting for standard client stream with 1 format arguments.
	 * @param[in] msg The message to be formatted.
	 * @param[in] arg1 The first formatting argument.
	 * @param[in] arg2 The second formatting argument.
	 * @returns A string formatted for standard client stream.
	 */
	std::string Client::_format(const std::string_view msg, const std::string_view arg1, const std::string_view arg2) const
	{
		return std::vformat(_format_client(msg).data(), std::make_format_args(arg1, arg2));
	}

	int Client::connect()
	{
		int rv = ::connect(this->pMySockFd, this->mServerInfo->ai_addr, this->mServerInfo->ai_addrlen);

		int e = errno;
		if (rv != 0) {
			std::cout << _format("Error while connecting to server: {}", strerror(e));
			exit(1);
		}

		std::cout << _format("Connected to {}\n", m0st4fa::toString((const sockaddr_storage*)this->mServerInfo->ai_addr));

		return 0;
	}

	std::string_view Client::receive(const size_t byteN)
	{
		char* buf = new char[byteN] {};

		::recv(this->pMySockFd, buf, byteN, 0);

		return std::string_view{ buf };
	}

	int Client::send(const std::string_view msg)
	{

		::send(this->pMySockFd, msg.data(), msg.length(), 0);

		return 0;
	}
}