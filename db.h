#ifndef _DB_H
#define _DB_H

#include <string>
#include <sqlite3.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>

#define DB_CREATE_TAGS	"create table if not exists rfid_tags(tag text primary key not null,tagname text not null unique,mytime int not null,playfile text default '')"
#define DB_INSERT_TAG	"insert or ignore into rfid_tags (mytime,tag,tagname) values (?,?,?)"
#define DB_UPDATE_TIME	"update rfid_tags set mytime=? where tag=?"
#define DB_UPDATE_FILE	"update rfid_tags set playfile=? where tag=?"
#define DB_UPDATE_NAME	"update rfid_tags set tagname=? where tag=?"
#define DB_SELECT_TAG	"select tagname,mytime,playfile from rfid_tags where tag=?"

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
			int open(const std::string &db_filename="rc522.db");
			int close();
			std::string get_error();
			int add_new(std::string tag);
			int update_playfile(std::string tag, std::string playfile);
			int update_tagname(std::string tag, std::string tagname);
			tag_t get_taginfo(std::string tag);
		private:
			sqlite3 *mydb;
			sqlite3_stmt *stmt;
			std::string my_filename;

			int create();
			int update_text(std::string sql, std::string tag, std::string textfield);
			int sql_update(std::string sql, unsigned long args_i[], const char *args_s[], uint8_t args[], uint8_t argn);
			int sql_run(std::string sql, unsigned long args_i[], const char *args_s[], uint8_t args[], uint8_t argn);
	};
}

#endif
