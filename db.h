#ifndef _DB_H
#define _DB_H

#include <string>
#include <sqlite3.h>
#include <stdio.h>

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
};

#endif
