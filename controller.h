#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#include <unistd.h>
#include <string>
#include <syslog.h>

#include "reader.h"
#include "db.h"
#include "mpclient.h"

namespace mpc_rfid {
	class Controller
	{
		public:
			int setup_reader(interface_t, uint8_t parameter);
			int setup_db(const std::string &db_filename);
			int setup_mpclient();
			void main_loop();
			std::string get_db_error()const;
		private:
			Reader rfreader;
			DB tagdb;
			MPClient mpclient;
	};
}

#endif
