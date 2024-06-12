#include "include/interface.h"

int main()
{

	// setup winsock and discard error code :)
	m0st4fa::setupWinsock();

	m0st4fa::Server server{3490};

	server.acceptConnections();

	return 0;
}
