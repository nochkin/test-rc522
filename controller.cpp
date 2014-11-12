#include "controller.h"

using namespace mpc_rfid;

int Controller::setup_db(const std::string &db_filename) {
	return tagdb.open(db_filename);
}

int Controller::setup_reader(interface_t interface, uint8_t parameter) {
	switch (interface) {
		case IF_SPI:
			return rfreader.init_spi(parameter);
			break;
		case IF_I2C:
			return rfreader.init_i2c(parameter);
			break;
		default:
			return -1;
	}
}

int Controller::setup_mpclient() {
	return mpclient.connect();
}

void Controller::main_loop() {
	while(1)
	{
		uint8_t status = rfreader.read_tag();
		// status = TAG_OK;
		if (status == TAG_OK) {
			std::string mytag = rfreader.get_tag_str();
			printf("tag: %s\n", mytag.c_str());
			syslog(LOG_DAEMON|LOG_INFO, "Tag: %s\n", mytag.c_str());
			tagdb.add_new(mytag);
			tag_t mytag_info = tagdb.get_taginfo(mytag);
			if (!mytag_info.playfile.empty()) {
				printf("playfile: %s\n", mytag_info.playfile.c_str());
				syslog(LOG_DAEMON|LOG_INFO, "playfile: %s\n", mytag_info.playfile.c_str());
				mpclient.add_and_play(mytag_info.playfile);
			}
		}
		rfreader.halt();
		usleep(200000);
	}
}

std::string Controller::get_db_error()const {
	return tagdb.get_error();
}

