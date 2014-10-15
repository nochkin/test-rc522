#include "controller.h"

Controller::Controller() {
}

int Controller::setup_db() {
	return this->tagdb.open();
}

int Controller::setup_reader() {
	return this->rfreader.init_spi();
}

int Controller::setup_mpclient() {
	return this->mpclient.connect();
}

void Controller::main_loop() {
	uint8_t status;

	while(1)
	{
		status = this->rfreader.read_tag();
		// status = TAG_OK;
		if (status == TAG_OK) {
			std::string mytag = this->rfreader.get_tag_str();
			printf("tag: %s\n", mytag.c_str());
			this->tagdb.add_new(mytag);
			tag_t mytag_info = this->tagdb.get_taginfo(mytag);
			if (!mytag_info.playfile.empty()) {
				printf("playfile: %s\n", mytag_info.playfile.c_str());
				this->mpclient.add_and_play(mytag_info.playfile);
			}
		}
		this->rfreader.halt();
		usleep(200000);
	}
}

void Controller::close() {
	this->tagdb.close();
}

std::string Controller::get_db_error() {
	return this->tagdb.get_error();
}

