#include "include/interface.h"

static void fn(const int sockFd, std::string_view recvData) {
	m0st4fa::ConnectionInformation::send(sockFd, recvData);
};

int main()
{

	// setup winsock and discard error code :)
	m0st4fa::setupWinsock();

	m0st4fa::Server server{3490};

	server.start(fn);

	return 0;
}
