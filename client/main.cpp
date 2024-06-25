#include "include/client.h"

constexpr size_t BUF_SIZE = 2000;

int main()
{

	// setup winsock and discard error code :)
	m0st4fa::setupWinsock();

	m0st4fa::Client client{ "localhost", 3490 };
	int nbytes = 1;

	std::cout << "The server says: " << client.receive(500, nbytes);
	client.send("I welcome you too!\r\n");
	
	std::string msg = "";
	char* buf = new char[BUF_SIZE];

	while (nbytes != 0) {
		std::cin.getline(buf, BUF_SIZE - 1);
		msg = buf;
		msg += "\r\n";

		if (msg.contains("StDn"))
			break;

		client.send(msg);
		std::cout << client.receive(500, nbytes);
	}

	return 0;
}
