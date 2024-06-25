// Windows API Headers
#pragma once

#include <functional>
#include "common.h"

namespace m0st4fa {

	/**
	 * @brief Represents a server.
	 */
	class Server : public ConnectionInformation {

		std::vector<std::pair<int, sockaddr_storage>> connectedSockets;
		m0st4fa::Sockets fileDescriptors{};

		static std::string _format_server(const std::string_view);
		int _set_up_listening_socket() const;

		using FnType = std::function<void(const int, std::string_view)>;
	protected:

		std::string _format(const std::string_view) const override;
		std::string _format(const std::string_view, const std::string_view) const override;
		std::string _format(const std::string_view, const std::string_view, const std::string_view) const override;
		std::string _receive_line(const int, int&);
		int _accept_connection();
		void _close_connection(const int);
		void _broadcast(const size_t, FnType, const std::string_view) const;

	public:

		Server(const int myPort = 3490) : ConnectionInformation() {
			this->setDeviceAddress(myPort);
			this->assignSocket();
			std::cout << _format("Server Information: {}", (this->operator std::string().data())) << "\n";
		};

		int start(std::function<void(const int, std::string_view)>);

	};

}

