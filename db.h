#ifndef _DB_H
#define _DB_H

#include <string>
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#define DB_CREATE_TAGS	"create table rfid_tags(tag text primary key not null, timestamp int not null, playfile text default '')"
#define DB_INSERT_TAG	"insert into rfid_tags (tag, timestamp) values (?,?)"

class DB
{
	public:
		DB();
		DB(std::string db_filename);
		int open();
		int close();
		std::string get_error();
	private:
		std::string db_filename;
		sqlite3 *db;

		int create();
		int add_new(std::string tag);
};

#endif
