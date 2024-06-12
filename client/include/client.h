#include "common.h"

namespace m0st4fa {

	class Client : public ConnectionInformation {

		addrinfo mServerHints;
		addrinfo* mServerInfo = nullptr;

		int _set_server_address(const std::string, const int serverPort);
		static std::string _format_client(const std::string_view);

	protected:

		std::string _format(const std::string_view) const override;
		std::string _format(const std::string_view, const std::string_view) const override;
		std::string _format(const std::string_view, const std::string_view, const std::string_view) const override;

	public:
		Client(const std::string serverAddress = "localhost", const int serverPort = 3490, const int myPort = 3500) : ConnectionInformation() {

			this->setDeviceAddress(myPort);
			this->assignSocket();

			memset(&mServerHints, 0, sizeof(addrinfo));

			// Set server hints
			mServerHints.ai_family = AF_INET;
			mServerHints.ai_socktype = SOCK_STREAM;

			this->_set_server_address(serverAddress, serverPort);
			this->connect();

		}

		~Client() {
			freeaddrinfo(mServerInfo);
		}

		int connect();
		std::string_view receive(const size_t);
		int send(const std::string_view msg);

	};

}
