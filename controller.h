#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#include <unistd.h>
#include <string>

#include "reader.h"
#include "db.h"
#include "mpclient.h"

using namespace mpc_rfid;

namespace mpc_rfid {
	class Controller
	{
		public:
			Controller();
			int setup_reader(interface_t, uint8_t parameter);
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
}

#endif
