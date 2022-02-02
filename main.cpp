/*		\(^-^)/		*/

#include "Server.hpp"

bool *killSwitch;

int		main(int argc, char** argv) {
	Server server;

	if (server.check(argc, argv))
		return 1;
	server.init();
	server.start();
	return 0;
}
