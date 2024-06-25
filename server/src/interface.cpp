#include <functional>
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
	 * @brief Creats and binds listening socket.
	 * @returns void
	 * @returns The value returned by `listen`.
	 */
	int Server::_set_up_listening_socket() const
	{

		int listenRv = ::listen(pMySockFd, ConnectionInformation::BACK_LOG);
		int e = errno;

		if (listenRv == -1) {
			std::cerr << _format("Could not listen on socket {}: {}.\n", std::to_string(pMySockFd), strerror(e));
			exit(-1);
		}

		std::cout << _format("Listening on port {}\n", std::to_string(this->getBoundPort()));
		std::cout << _format("Waiting for incoming connections...\n");

		return listenRv;
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
	 * @param[in] arg1 The first formatting argument.
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
	 * @brief Receives every character from socket `fd` until `\r\n` is sent.
	 * @param[in] fd The socket from which to receive data.
	 * @param[out] nbytes The number of received bytes. Use this to check whether the client has closed the connection or not.
	 * @returns The line received from socket `fd`.
	 */
	std::string Server::_receive_line(const int fd, int& nbytes)
	{
		std::string msg;
		nbytes = 0; // initialize to 0 as we have not read anything already. This is also useful for when connection is closed.

		while (!msg.ends_with("\r\n")) { // the second case for when the connection is closed, since nothing is sent by definition (and that's why nbytes is 0)
			int temp = 0;
			msg += ConnectionInformation::receive(fd, 200, temp);

			if (temp == 0)
				break; // in this case, `nbytes` must already be initialized to 0 from above.
			else nbytes += temp; // augment these new characters to `nbytes`
		}

		return msg;
	}

	/**
	 * @brief Accepts incoming connection.
	 * @returns The socket to be used to communicate with the new connection.
	 */
	int Server::_accept_connection()
	{
		connectedSockets.push_back(std::pair{ 0, sockaddr_storage{} });
		int length = sizeof sockaddr_storage;
		
		int newSocket = ::accept(this->pMySockFd, (sockaddr*)&this->connectedSockets.back().second, &length);
		int e = errno;

		connectedSockets.back().first = newSocket;

		if (newSocket == -1) {
			std::cout << _format("Error while accepting connection: {}\n", strerror(e));
			std::exit(-1);
		}

		int sendRv = _initialize_connection(newSocket, &connectedSockets.back().second);

		// handle errors while sending welcoming words
		if (sendRv > 0) {
			std::cout << _format("Error while sending the welcoming words to a new connection: {}\n", strerror(errno));
			std::exit(-1);
		}
		
		return newSocket;
	}

	/**
	 * @brief Closes the connection to `sockFd`.
	 */
	void Server::_close_connection(const int sockFd)
	{
		// tell everyone that `sockFd` has quit
		this->_broadcast(sockFd, [sockFd](int s, std::string_view msg) {
			m0st4fa::ConnectionInformation::send(s, std::to_string(sockFd) + " has disconnected.");
			}, "");

		// remove socket from being polled
		this->fileDescriptors.remove(sockFd);

		// TODO: You can make this more efficient if both have the same index (although will also require modification of `remove`
		// remove connection information of socket
		auto it = std::find_if(
			this->connectedSockets.begin(), this->connectedSockets.end(),
			[&sockFd](const std::pair<int, sockaddr_storage>& pair) {
				return pair.first == sockFd;
			});

		std::cout << _format("Removed connection {}\n", toString(&it->second));

		this->connectedSockets.erase(it, it);

	}

	/**
	 * @brief Calls function `fn` with the argument `msg` for each connection of the server. It is intended to broadcast `msg` to every connection of the server at the time of making the call.
	 * @param[in] sockIndex The index of the socket sending the data.
	 * @param[in] fn The function to be called for each socket connected to the server.
	 * @param[in] msg The message passed to `fn` as argument.
	 * @returns void
	 */
	void Server::_broadcast(const size_t sockIndex, FnType fn, const std::string_view msg) const
	{
		for (size_t j = 0; j < this->fileDescriptors.getLength(); j++) {
			int sock = this->fileDescriptors.at(j).fd;

			// this socket and the listening socket are skipped
			if (j != sockIndex && j != this->pMySockFd) {
				ConnectionInformation::send(sock, "\b\b");
				fn(sock, msg);
				ConnectionInformation::send(sock, "\r\n");
			}

			if (j != this->pMySockFd)
				ConnectionInformation::send(sock, "> "); // The client already supplies \r\n these when they return, so no need to add more
		}
	}

	/**
	* @brief Listens on the bound address (There must exist one before calling this) and accepts incoming	connections. It aborts the process in case listen returns -1;
	* @param[in] fn Function expected to take a socket descriptor and received data and returns nothing (void.)
	* @returns The value returned by `listen` (important for error checking).
	*/
	int Server::start(FnType fn)
	{
		int listenRv = _set_up_listening_socket();
		int e = errno;

		this->fileDescriptors.add(this->pMySockFd, POLLIN); // add the listening socket and poll for received data

		// get into the main loop
		while (true) {

			int pollrv = ::WSAPoll(this->fileDescriptors.getSockets(), this->fileDescriptors.getLength(), -1);
			e = WSAGetLastError();

			// report errors after poll returns
			if (pollrv == -1) {
				std::cout << _format("Error while polling for sockets: {}\n", ConnectionInformation::formatFckingMSErrorMessages(e));

				std::exit(-1);
			}

			// if timed out
			if (pollrv == 0) {
				std::cout << _format("Timed out while polling for sockets\n");
				continue;
			}

			// no error nor time out at this point. at this point, pollrv = the number of sockets with events.
			for (size_t i = 0; i < this->fileDescriptors.getLength(); i++) {
				pollfd curr = this->fileDescriptors.at(i);

				if (curr.revents == 0) // if no event has been detected for this socket
					continue;

				if (curr.fd == this->pMySockFd) // if this socket is the listening socket
					this->fileDescriptors.add(_accept_connection(), POLLIN); // accept incoming connection
				else { // if this socket is not the listening socket
					int recvBytes = 0;
					char* buf = new char[500];
					//std::string_view rd = _receive_sentence(curr.fd, buf, 500, recvBytes);
					std::string rd = _receive_line(curr.fd, recvBytes);
					std::string msg = std::format("{}: {}", curr.fd, rd.substr(0, rd.length() - 2));

					if (recvBytes == 0) // if no bytes have been received (the client has closed)
						_close_connection(curr.fd);
					else // if at least one byte has been received
						_broadcast(i, fn, msg);
					
					delete[] buf;
				}
				// else DO NOTHING

			}

		}

		return listenRv;
	}

}