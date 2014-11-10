#include "db.h"

using namespace mpc_rfid;

DB::DB()
{
	mydb = NULL;
}

int DB::open(const std::string &db_filename)
{
	my_filename = db_filename;
	int rc = sqlite3_open_v2(my_filename.c_str(), &mydb, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	if (rc == SQLITE_OK) {
		return create();
	}

	return rc;
}

int DB::close()
{
	return sqlite3_close(mydb);
}

std::string DB::get_error()const
{
	return std::string(sqlite3_errmsg(mydb));
}

int DB::create()
{
	char *err_msg = 0;
	return sqlite3_exec(mydb, DB_CREATE_TAGS, NULL, NULL, &err_msg);
}

int DB::add_new(std::string tag)
{
	static char const *args_s[] = {
		"",
		tag.c_str(),
		tag.c_str()
	};
	unsigned long args_i[] = {
		(unsigned long)time(NULL),
		0
	};
	uint8_t args[] = {
		SQLITE_INTEGER,
		SQLITE_TEXT,
		SQLITE_TEXT
	};

	int rc = sql_update(DB_INSERT_TAG, args_i, args_s, args, 3);
	if (rc == SQLITE_OK) {
		rc = sql_update(DB_UPDATE_TIME, args_i, args_s, args, 2);
		if (rc != SQLITE_OK) {
			printf("DB: error update_time: %s\n", get_error().c_str());
		}
	} else {
		printf("DB: error insert_tag: %s\n", get_error().c_str());
	}

	return rc;
}

int DB::update_playfile(std::string tag, std::string playfile)
{
	return update_text(DB_UPDATE_FILE, tag, playfile);
}

int DB::update_tagname(std::string tag, std::string tagname)
{
	return update_text(DB_UPDATE_NAME, tag, tagname);
}

tag_t DB::get_taginfo(std::string tag)
{
	static char const *args_s[] = {
		tag.c_str()
	};
	unsigned long args_i[] = {
		0
	};
	uint8_t args[] = {
		SQLITE_TEXT
	};

	tag_t tag_info;
	int rc = sql_run(DB_SELECT_TAG, args_i, args_s, args, 1);
	while (1) {
		rc = sqlite3_step(stmt);
		if (rc == SQLITE_DONE) break;
		else if (rc == SQLITE_ROW) {
			tag_info.tag = tag;
			tag_info.tagname = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
			tag_info.mytime = sqlite3_column_int(stmt, 1);
			tag_info.playfile = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
		} else {
			printf("step(%i): %s\n", rc, get_error().c_str());
		}
	}
	rc = sqlite3_finalize(stmt);

	return tag_info;
}

int DB::update_text(std::string sql, std::string tag, std::string textfield)
{
	static char const *args_s[] = {
		textfield.c_str(),
		tag.c_str()
	};
	unsigned long args_i[] = {
		0
	};
	uint8_t args[] = {
		SQLITE_TEXT,
		SQLITE_TEXT
	};

	int rc = sql_update(sql.c_str(), args_i, args_s, args, 2);
	if (rc != SQLITE_OK) {
		printf("DB: error update_text: %s\n", get_error().c_str());
	}

	return rc;
}

int DB::sql_update(std::string sql, unsigned long args_i[], const char *args_s[], uint8_t args[], uint8_t argn)
{
	int rc = sql_run(sql, args_i, args_s, args, argn);
	if (rc == SQLITE_OK) {
		rc = sqlite3_step(stmt);
		if (rc == SQLITE_DONE) {
			rc = sqlite3_finalize(stmt);
		}
	}

	return rc;
}

int DB::sql_run(std::string sql, unsigned long args_i[], const char *args_s[], uint8_t args[], uint8_t argn)
{
	int rc = sqlite3_prepare_v2(mydb, sql.c_str(), strlen(sql.c_str()), &stmt, NULL);
	if (rc == SQLITE_OK) {
		for (uint8_t ii=0; ii<argn; ii++) {
			switch (args[ii]) {
				case SQLITE_TEXT:
					sqlite3_bind_text(stmt, ii+1, args_s[ii], strlen(args_s[ii]), 0);
					break;
				case SQLITE_INTEGER:
					sqlite3_bind_int(stmt, ii+1, args_i[ii]);
					break;
			}
		}
	}

	return rc;
}

