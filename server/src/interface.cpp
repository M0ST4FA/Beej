#include "include/interface.h"

namespace m0st4fa {

	/**
	 * @brief Formats `msg` for standard server stream.
	 * @param[in] msg The message to be formatted.
	 * @returns A string formatted for standard server stream.
	 */
	std::string Server::_format_server(const std::string_view msg)
	{
		std::string temp = "[server] ";
		temp += msg;
		return temp;
	}

	/**
	 * @brief Implements formatting for standard server stream with 0 format arguments.
	 * @param[in] msg The message to be formatted.
	 * @returns A string formatted for standard server stream.
	 */
	std::string Server::_format(const std::string_view msg) const
	{
		return _format_server(msg);
	}

	/**
	 * @brief Implements formatting for standard server stream with 1 format arguments.
	 * @param[in] msg The message to be formatted.
	 * @param[in] arg1 First formatting argument.
	 * @returns A string formatted for standard server stream.
	 */
	std::string Server::_format(const std::string_view msg, const std::string_view arg1) const
	{
		return std::vformat(_format_server(msg).data(), std::make_format_args(arg1));
	}

	/**
	 * @brief Implements formatting for standard server stream with 1 format arguments.
	 * @param[in] msg The message to be formatted.
	 * @param[in] arg1 The first formatting argument.
	 * @param[in] arg2 The second formatting argument.
	 * @returns A string formatted for standard server stream.
	 */
	std::string Server::_format(const std::string_view msg, const std::string_view arg1, const std::string_view arg2) const
	{
		return std::vformat(_format_server(msg).data(), std::make_format_args(arg1, arg2));
	}

	/**
	* @brief Listens on the bound address (There must exist one before calling this) and accepts incoming	connections. It aborts the process in case listen returns -1;
	* @returns The value returned by `listen` (important for error checking).
	*/
	int Server::acceptConnections()
	{
		int listenRv = ::listen(pMySockFd, ConnectionInformation::BACK_LOG);

		int e = errno;
		if (listenRv == -1) {
			std::cerr << _format("Could not listen on socket {}: {}.\n", std::to_string(pMySockFd), strerror(e));
			exit(-1);
		}

		std::cout << _format("Listening on port {}\n", std::to_string(this->getBoundPort()));
		std::cout << _format("Waiting for incoming connections...\n");

		size_t acceptNum = 0;
		while (acceptNum > 5 ? std::cout << (std::cerr, _format("Reached maximum response limit. Closing server\n")) : std::cout << (std::cout, ""), acceptNum <= 5) {

			int sockSz = sizeof sockaddr;
			sockaddr_storage connectedAddr{};
			int newSockFd = ::accept(this->pMySockFd, (sockaddr*)&connectedAddr, &sockSz);

			e = errno;
			if (newSockFd == -1) {
				std::cerr << _format("Could not accept an incoming connection: {}", strerror(e));
				exit(-1);
			}

			acceptNum++;

#define BUF_SZ 10000
#define CLOSE_SOCKET() ::closesocket(newSockFd); delete[] buf

			char* buf = new char[BUF_SZ];
			memset(buf, 0, BUF_SZ);
			int sendRv = 0;
			int e = 0;

			sendRv = _initialize_connection(&connectedAddr, newSockFd);

			std::string_view recMsg;
			std::string_view sendMsg = "\r> ";
			while (true) {
				recMsg = this->_receive_sentence(newSockFd, buf, BUF_SZ);

				if (recMsg.contains("Close")) {
					sendMsg = "Closing! bye, bye...";
					send(newSockFd, sendMsg.data(), sendMsg.size() + 1, 0);

					CLOSE_SOCKET();
					std::cout << _format("Closing");
					std::exit(0);
				}

				std::cout << recMsg << "\n";
				sendRv = send(newSockFd, sendMsg.data(), sendMsg.size() + 1, 0);

				e = errno;
				if (sendRv == -1) {
					std::cerr << _format("Error while sending data to client: {}\n", strerror(errno));
					exit(-1);
				}
			}

			CLOSE_SOCKET();
		}

		return listenRv;
	}

}