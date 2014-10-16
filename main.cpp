#include <stdio.h>
#include <syslog.h>
#include <getopt.h>

#include "config.h"
#include "controller.h"

int main(int argc, char *argv[])
{
	int status = 0;
	Controller controller;
	int c;
	std::string cfg_file = "rc522-mpc.cfg";

	while ((c = getopt(argc, argv, "c:")) != EOF) {
		switch (c) {
			case 'c':
				cfg_file = optarg;
				break;
			default:
				return 1;
				break;
		}
	}

	status = Config::getInstance().load_config(cfg_file);
	if (status) {
		fprintf(stderr, "Failed to load %s\n", cfg_file.c_str());
		syslog(LOG_DAEMON|LOG_ERR, "Failed to load config\n");
		return 101;
	}

	status = controller.setup_reader();
	if (status) {
		fprintf(stderr, "Failed to init bcm2835\n");
		syslog(LOG_DAEMON|LOG_ERR, "Failed to init bcm2835\n");
		return 102;
	}

	int uid = Config::getInstance().config().uid;
	if (uid > 0) {
		setuid(uid);
	}

	status = controller.setup_db();
	if (status) {
		fprintf(stderr, "DB: filed to open: %s\n", controller.get_db_error().c_str());
		syslog(LOG_DAEMON|LOG_ERR, "DB: filed to open\n");
		return 103;
	}

	status = controller.setup_mpclient();
	if (status) {
		fprintf(stderr, "MPClient: error connecting\n");
		syslog(LOG_DAEMON|LOG_ERR, "MPClient: error connecting\n");
		return 104;
	}

	controller.main_loop();

	return 0;
}

