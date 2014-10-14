#include <stdio.h>
#include <syslog.h>

#include "controller.h"

int main(int argc, char *argv[])
{
	int status;
	// int uid = 33; // www-data

	Controller controller;

	status = controller.setup_reader();
	if (status) {
		fprintf(stderr, "Failed init bcm2835\n");
		syslog(LOG_DAEMON|LOG_ERR,"Failed init bcm2835\n");
		return 1;
	}

	// setuid(uid);

	status = controller.setup_db();
	if (status) {
		fprintf(stderr, "DB: error opening: %s\n", controller.get_db_error().c_str());
		syslog(LOG_DAEMON|LOG_ERR, "DB: error opening\n");
		return 2;
	}

	controller.main_loop();

	return 0;
}

