#include <stdio.h>
#include <syslog.h>

#include "INIReader.h"
#include "controller.h"

int main(int argc, char *argv[])
{
	int status = 0;
	int uid = 0;
	Controller controller;

	std::string cfg_file = "rc522-mpc.cfg";

	INIReader ini(cfg_file);
	if (ini.ParseError() < 0) {
		fprintf(stderr, "Failed to load %s\n", cfg_file.c_str());
		syslog(LOG_DAEMON|LOG_ERR, "Failed to load config\n");
		return 1;
	}

	status = controller.setup_reader();
	if (status) {
		fprintf(stderr, "Failed to init bcm2835\n");
		syslog(LOG_DAEMON|LOG_ERR, "Failed to init bcm2835\n");
		return 2;
	}

	// setuid(uid);

	status = controller.setup_db();
	if (status) {
		fprintf(stderr, "DB: filed to open: %s\n", controller.get_db_error().c_str());
		syslog(LOG_DAEMON|LOG_ERR, "DB: filed to open\n");
		return 3;
	}

	status = controller.setup_mpclient();
	if (status) {
		fprintf(stderr, "MPClient: error connecting\n");
		syslog(LOG_DAEMON|LOG_ERR, "MPClient: error connecting\n");
		return 4;
	}

	controller.main_loop();

	return 0;
}

