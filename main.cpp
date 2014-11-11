#include <stdio.h>
#include <syslog.h>
#include <getopt.h>

#include "config.h"
#include "controller.h"

using namespace mpc_rfid;

int usage(char *exec_name)
{
	printf("Usage:\n");
	printf("%s [-c mpc-rfid.cfg]\n", exec_name);
	return 2;
}

int main(int argc, char *argv[])
{
	Controller controller;
	std::string cfg_file = "mpc-rfid.cfg";

	int c;
	while ((c = getopt(argc, argv, "c:h")) != EOF) {
		switch (c) {
			case 'c':
				cfg_file = optarg;
				break;
			case 'h':
				return usage(argv[0]);
			default:
				return 1;
		}
	}

	if (Config::get().load_config(cfg_file)) {
		fprintf(stderr, "Failed to load %s\n", cfg_file.c_str());
		syslog(LOG_DAEMON|LOG_ERR, "Failed to load %s\n", cfg_file.c_str());
		return 101;
	}

	interface_t reader_if = IF_NOT_SET;
	uint8_t param_if = 0;
	std::string reader_if_str = Config::get().config().rc522_interface;
	if (reader_if_str == "spi") {
		reader_if = IF_SPI;
		param_if = Config::get().config().rc522_spi_cs_int;
	} else if (reader_if_str == "i2c") {
		reader_if = IF_I2C;
		param_if = Config::get().config().rc522_i2c_address;
	}
	if (controller.setup_reader(reader_if, param_if)) {
		fprintf(stderr, "Failed to init bcm2835\n");
		syslog(LOG_DAEMON|LOG_ERR, "Failed to init bcm2835\n");
		return 102;
	}

	int uid = Config::get().config().uid;
	if (uid > 0) {
		setuid(uid);
	}

	std::string db_filename = Config::get().config().db_path;
	if (controller.setup_db(db_filename)) {
		fprintf(stderr, "DB: failed to open %s: %s\n", db_filename.c_str(), controller.get_db_error().c_str());
		syslog(LOG_DAEMON|LOG_ERR, "DB: failed to open %s: %s\n", db_filename.c_str(), controller.get_db_error().c_str());
		return 103;
	}

	if (controller.setup_mpclient()) {
		fprintf(stderr, "MPClient: error connecting\n");
		syslog(LOG_DAEMON|LOG_ERR, "MPClient: error connecting\n");
		return 104;
	}

	controller.main_loop();

	return 0;
}

