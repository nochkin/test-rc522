#include "db.h"

static int mydb_callback(void *NotUsed, int argc, char **argv, char **azColName){
	for(int i=0; i<argc; i++)
	{
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}

DB::DB()
{
	my_filename = "/tmp/rc522.db";
	this->mydb = NULL;
}

DB::DB(std::string db_filename)
{
	this->my_filename = db_filename;
	this->mydb = NULL;
}

int DB::open()
{
	int rc;

	rc = sqlite3_open_v2(my_filename.c_str(), &this->mydb, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	if (rc == SQLITE_OK) {
		return create();
	}

	return rc;
}

int DB::close()
{
	return sqlite3_close(this->mydb);
}

std::string DB::get_error()
{
	std::string err_msg(sqlite3_errmsg(this->mydb));
	return err_msg;
}

int DB::create()
{
	char *err_msg = 0;
	int rc;

	rc = sqlite3_exec(this->mydb, DB_CREATE_TAGS, mydb_callback, 0, &err_msg);

	return rc;
}

int DB::add_new(std::string tag)
{
	int rc;
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

	rc = sql_update(DB_INSERT_TAG, args_i, args_s, args, 3);
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
	return this->update_text(DB_UPDATE_FILE, tag, playfile);
}

int DB::update_tagname(std::string tag, std::string tagname)
{
	return this->update_text(DB_UPDATE_NAME, tag, tagname);
}

tag_t DB::get_taginfo(std::string tag)
{
	tag_t tag_info;
	int rc;

	static char const *args_s[] = {
		tag.c_str()
	};
	unsigned long args_i[] = {
		0
	};
	uint8_t args[] = {
		SQLITE_TEXT
	};

	rc = this->sql_run(DB_SELECT_TAG, args_i, args_s, args, 1);
	while (1) {
		rc = sqlite3_step(this->stmt);
		if (rc == SQLITE_DONE) break;
		else if (rc == SQLITE_ROW) {
			tag_info.tag = tag;
			tag_info.tagname = reinterpret_cast<const char*>(sqlite3_column_text(this->stmt, 0));
			tag_info.mytime = sqlite3_column_int(this->stmt, 1);
			tag_info.playfile = reinterpret_cast<const char*>(sqlite3_column_text(this->stmt, 2));
		} else {
			printf("step(%i): %s\n", rc, get_error().c_str());
		}
	}
	rc = sqlite3_finalize(this->stmt);

	return tag_info;
}

int DB::update_text(std::string sql, std::string tag, std::string textfield)
{
	int rc;
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

	rc = sql_update(sql.c_str(), args_i, args_s, args, 2);
	if (rc != SQLITE_OK) {
		printf("DB: error update_text: %s\n", get_error().c_str());
	}

	return rc;
}

int DB::sql_update(std::string sql, unsigned long args_i[], const char *args_s[], uint8_t args[], uint8_t argn)
{
	int rc;
   
	rc = this->sql_run(sql, args_i, args_s, args, argn);
	if (rc == SQLITE_OK) {
		rc = sqlite3_step(this->stmt);
		if (rc == SQLITE_DONE) {
			rc = sqlite3_finalize(this->stmt);
		}
	}

	return rc;
}

int DB::sql_run(std::string sql, unsigned long args_i[], const char *args_s[], uint8_t args[], uint8_t argn)
{
	int rc;

	rc = sqlite3_prepare_v2(this->mydb, sql.c_str(), strlen(sql.c_str()), &this->stmt, NULL);
	if (rc == SQLITE_OK) {
		for (uint8_t ii=0; ii<argn; ii++) {
			switch (args[ii]) {
				case SQLITE_TEXT:
					sqlite3_bind_text(this->stmt, ii+1, args_s[ii], strlen(args_s[ii]), 0);
					break;
				case SQLITE_INTEGER:
					sqlite3_bind_int(this->stmt, ii+1, args_i[ii]);
					break;
			}
		}
	}

	return rc;
}

