#ifndef _DB_H
#define _DB_H

#include <string>
#include <sqlite3.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>

#define DB_CREATE_TAGS	"create table if not exists rfid_tags(tag text primary key not null,mytime int not null,playfile text default '')"
#define DB_INSERT_TAG	"insert or ignore into rfid_tags (mytime,tag) values (?,?)"
#define DB_UPDATE_TIME	"update rfid_tags set mytime=? where tag=?"
#define DB_UPDATE_FILE	"update rfid_tags set playfile=? where tag=?"
#define DB_SELECT_FILE	"select playfile from rfid_tags where tag=?"

class DB
{
	public:
		DB();
		DB(std::string db_filename);
		int open();
		int close();
		std::string get_error();
		int add_new(std::string tag);
		std::string get_playfile(std::string tag);
	private:
		sqlite3 *mydb;
		std::string my_filename;

		int create();
		int run_sql(std::string sql, unsigned long args_i[], const char *args_s[], uint8_t args[], uint8_t argn);
};

#endif
