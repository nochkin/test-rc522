#include "controller.h"

Controller::Controller() {
}

int Controller::setup_db() {
	return this->tagdb.open();
}

int Controller::setup_reader() {
	return this->rfreader.init_spi();
}

void Controller::main_loop() {
	uint8_t status;

	while(1)
	{
		status = this->rfreader.read_tag();
		if (status == TAG_OK) {
			std::string mytag = this->rfreader.get_tag_str();
			printf("tag(%i): %s\n", status, mytag.c_str());
			this->tagdb.add_new(mytag);
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

