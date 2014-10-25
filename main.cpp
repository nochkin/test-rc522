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
	std::string cfg_file = "mpc-rfid.cfg";

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

	status = Config::get().load_config(cfg_file);
	if (status) {
		fprintf(stderr, "Failed to load %s\n", cfg_file.c_str());
		syslog(LOG_DAEMON|LOG_ERR, "Failed to load config\n");
		return 101;
	}

	interface_t reader_if = IF_NOT_SET;
	std::string reader_if_str = Config::get().config().rc522_interface;
	if (reader_if_str == "spi") {
		reader_if = IF_SPI;
	} else if (reader_if_str == "i2c") {
		reader_if = IF_I2C;
	}
	status = controller.setup_reader(IF_SPI);
	if (status) {
		fprintf(stderr, "Failed to init bcm2835\n");
		syslog(LOG_DAEMON|LOG_ERR, "Failed to init bcm2835\n");
		return 102;
	}

	int uid = Config::get().config().uid;
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

