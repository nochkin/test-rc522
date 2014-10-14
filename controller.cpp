#include "controller.h"

Controller::Controller() {
}

int Controller::setup_db() {
	return db.open();
}

int Controller::setup_reader() {
	return reader.init_spi();
}

void Controller::main_loop() {
	uint8_t status;

	while(1)
	{
		status = reader.read_tag();
		if (status == TAG_OK) {
			std::string mytag = reader.get_tag_str();
			printf("tag(%i): %s\n", status, mytag.c_str());
			db.add_new(mytag);
		}
		reader.halt();
		usleep(300000);
	}
}

void Controller::close() {
	db.close();
}

std::string Controller::get_db_error() {
	return db.get_error();
}

