#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#include <unistd.h>
#include <string>

#include "reader.h"
#include "db.h"
#include "mpclient.h"

class Controller
{
	public:
		Controller();
		int setup_reader(interface_t);
		int setup_db();
		int setup_mpclient();
		void main_loop();
		void close();
		std::string get_db_error();
	private:
		Reader rfreader;
		DB tagdb;
		MPClient mpclient;
};

#endif
