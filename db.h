#ifndef _DB_H
#define _DB_H

#include <string>
#include <sqlite3.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>

#define DB_CREATE_TAGS	"create table rfid_tags(tag text primary key not null, timestamp int not null, playfile text default '')"
#define DB_INSERT_TAG	"insert or ignore into rfid_tags (tag, timestamp) values (?,?)"
#define DB_UPDATE_TIME	"update rfid_tags set timestamp=? where tag=?"
#define DB_UPDATE_FILE	"update rfid_tags set playfile=? where tag=?"

class DB
{
	public:
		DB();
		DB(std::string db_filename);
		int open();
		int close();
		std::string get_error();
		int add_new(std::string tag);
	private:
		std::string db_filename;
		sqlite3 *db;

		int create();
		int run_sql(std::string sql, unsigned long *args_i[], const char *args_s[], uint8_t args);
};

#endif
