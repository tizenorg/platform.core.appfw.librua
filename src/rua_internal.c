
#include <tzplatform_config.h>
#include <db-util.h>
#include <aul.h>

#include "rua_internal.h"
#include "db-schema.h"

static int __exec(sqlite3 *db, char *query)
{
	int r;
	char *errmsg = NULL;

	if (db == NULL)
		return -1;

	r = sqlite3_exec(db, query, NULL, NULL, &errmsg);

	if (r != SQLITE_OK) {
		sqlite3_free(errmsg);
		return -1;
	}

	return 0;
}

static int __create_table(sqlite3 *db)
{
	int r;

	r = __exec(db, CREATE_RUA_HISTORY_TABLE);
	if (r == -1)
		return -1;

	return 0;
}

static sqlite3 *__db_init()
{
	int r;
	sqlite3 *db = NULL;

	char defname[FILENAME_MAX];
	const char *rua_db_path = tzplatform_getenv(TZ_USER_DB);
	if (rua_db_path == NULL) {
		LOGE("fail to get rua_db_path");
		return NULL;
	}
	snprintf(defname, sizeof(defname), "%s/%s", rua_db_path, RUA_DB_NAME);

	r = db_util_open_with_options(defname, &db, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL);
	if (r) {
		db_util_close(db);
		return NULL;
	}

	r = __create_table(db);
	if (r) {
		db_util_close(db);
		return NULL;
	}

	return db;
}

int rua_db_delete_history(bundle *b)
{
	int r;
	sqlite3 *db = NULL;
	char query[QUERY_MAXLEN];

	char *pkg_name = NULL;
	char *app_path = NULL;
	char *errmsg = NULL;
	int result = 0;

	db = __db_init();
	if (db == NULL) {
		LOGE("Error db null");
		return -1;
	}

	if (b != NULL) {
		bundle_get_str(b, AUL_K_RUA_PKGNAME, &pkg_name);
		bundle_get_str(b, AUL_K_RUA_APPPATH, &app_path);
	}

	if (pkg_name != NULL)
		snprintf(query, QUERY_MAXLEN, "delete from rua_history where pkg_name = '%s';", pkg_name);
	else if (app_path != NULL)
		snprintf(query, QUERY_MAXLEN, "delete from rua_history where app_path = '%s';", app_path);
	else
		snprintf(query, QUERY_MAXLEN, "delete from rua_history;");

	LOGI("rua_delete_history_from_db : %s", query);
	r = sqlite3_exec(db, query, NULL, NULL, &errmsg);

	if (r != SQLITE_OK) {
		LOGE("fail to exec delete query %s : %s", query, errmsg);
		sqlite3_free(errmsg);
		result = -1;
	}

	if (db != NULL)
		db_util_close(db);

	return result;

}

int rua_db_add_history(struct rua_rec *rec)
{
	int r;
	int cnt = 0;
	char query[QUERY_MAXLEN];
	sqlite3_stmt *stmt;
	sqlite3 *db = NULL;

	db = __db_init();
	if (db == NULL) {
		LOGE("Error db null");
		return -1;
	}

	if (rec == NULL) {
		LOGE("Error rec null");
		db_util_close(db);
		return -1;
	}

	snprintf(query, QUERY_MAXLEN,
		"select count(*) from %s where pkg_name = '%s';", RUA_HISTORY,
		rec->pkg_name);

	r = sqlite3_prepare(db, query, sizeof(query), &stmt, NULL);
	if (r != SQLITE_OK) {
		LOGE("Error sqlite3_prepare fail");
		db_util_close(db);
		return -1;
	}

	r = sqlite3_step(stmt);
	if (r == SQLITE_ROW)
		cnt = sqlite3_column_int(stmt, 0);

	sqlite3_finalize(stmt);

	if (cnt == 0)
		/* insert */
		snprintf(query, QUERY_MAXLEN,
			"insert into %s ( pkg_name, app_path, arg, launch_time ) "
			" values ( \"%s\", \"%s\", \"%s\", %d ) ",
			RUA_HISTORY,
			rec->pkg_name ? rec->pkg_name : "",
			rec->app_path ? rec->app_path : "",
			rec->arg ? rec->arg : "", (int)time(NULL));
	else
		/* update */
		snprintf(query, QUERY_MAXLEN,
			"update %s set arg='%s', launch_time='%d' where pkg_name = '%s';",
			RUA_HISTORY,
			rec->arg ? rec->arg : "", (int)time(NULL), rec->pkg_name);

	r = __exec(db, query);
	if (r == -1) {
		LOGE("[RUA ADD HISTORY ERROR] %s\n", query);
		db_util_close(db);
		return -1;
	}

	db_util_close(db);
	return r;
}
