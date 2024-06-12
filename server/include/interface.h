// Windows API Headers
#pragma once

#include "common.h"

namespace m0st4fa {

	/**
	 * @brief Represents a server.
	 */
	class Server : public ConnectionInformation {

		static std::string _format_server(const std::string_view);

	protected:

		std::string _format(const std::string_view) const override;
		std::string _format(const std::string_view, const std::string_view) const override;
		std::string _format(const std::string_view, const std::string_view, const std::string_view) const override;

	public:

		Server(const int myPort = 3490) : ConnectionInformation() {
			this->setDeviceAddress(myPort);
			this->assignSocket();
			std::cout << _format("Server Information: {}", (this->operator std::string().data())) << "\n";
		};

		int acceptConnections();

	};

}

