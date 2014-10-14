#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#include <unistd.h>
#include <string>

#include "reader.h"
#include "db.h"

class Controller
{
	public:
		Controller();
		int setup_db();
		int setup_reader();
		void main_loop();
		void close();
		std::string get_db_error();
	private:
		// Reader rfreader;
		DB tagdb;
};

#endif
