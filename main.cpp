#include <stdio.h>
#include <syslog.h>

#include "controller.h"

int main(int argc, char *argv[])
{
	int status = 0;
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

	status = controller.setup_mpclient();
	if (status) {
		fprintf(stderr, "MPClient: error connecting\n");
		syslog(LOG_DAEMON|LOG_ERR, "MPClient: error connecting\n");
		return 3;
	}

	controller.main_loop();

	return 0;
}

