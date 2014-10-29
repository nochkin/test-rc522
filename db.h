#ifndef _DB_H
#define _DB_H

#include <string>
#include <sqlite3.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>

namespace mpc_rfid {
	struct tag_t {
		std::string tag;
		std::string tagname;
		unsigned long mytime;
		std::string playfile;
	};

	class DB
	{
		public:
			DB();
			~DB();
			int open(const std::string &db_filename="rc522.db");
			std::string get_error()const;
			int add_new(std::string &tag);
			int update_playfile(std::string &tag, std::string &playfile);
			int update_tagname(std::string &tag, std::string &tagname);
			tag_t get_taginfo(std::string &tag);
		private:
			sqlite3 *mydb;
			sqlite3_stmt *stmt;
			std::string my_filename;

			int close();
			int create();
			int update_text(const std::string &sql, std::string &tag, std::string &textfield);
			int sql_update(const std::string &sql, unsigned long args_i[], const char *args_s[], uint8_t args[], uint8_t argn);
			int sql_run(const std::string &sql, unsigned long args_i[], const char *args_s[], uint8_t args[], uint8_t argn);
	};
}

#endif
