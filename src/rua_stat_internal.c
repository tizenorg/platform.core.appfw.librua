#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <db-util.h>
/* For multi-user support */
#include <tzplatform_config.h>

#include "db-schema.h"
#include "rua_stat_internal.h"

int __rua_stat_insert(sqlite3 *db, char *caller, char *rua_stat_tag)
{
	int r;
	char query[QUERY_MAXLEN];
	sqlite3_stmt *stmt = NULL;

	sqlite3_snprintf(QUERY_MAXLEN, query,
		"INSERT INTO rua_panel_stat (caller_panel, rua_stat_tag, score) VALUES (?,?,?)");

	r = sqlite3_prepare(db, query, sizeof(query), &stmt, NULL);
	if (r != SQLITE_OK) {
		LOGE("sqlite3_prepare error(%d , %d, %s)", r, sqlite3_extended_errcode(db), sqlite3_errmsg(db));
		goto out;
	}

	r = sqlite3_bind_text(stmt, 1, caller, strlen(caller), SQLITE_STATIC);
	if (r != SQLITE_OK) {
		LOGE("caller bind error(%d) \n", r);
		goto out;
	}

	r = sqlite3_bind_text(stmt, 2, rua_stat_tag, strlen(rua_stat_tag), SQLITE_STATIC);
	if (r != SQLITE_OK) {
		LOGE("rua_stat_tag bind error(%d) \n", r);
		goto out;
	}

	r = sqlite3_bind_int(stmt, 3, WIN_SCORE);
	if (r != SQLITE_OK) {
		LOGE("arg bind error(%d) \n", r);
		goto out;
	}

	r = sqlite3_step(stmt);
	if (r != SQLITE_DONE) {
		LOGE("step error(%d) \n", r);
		goto out;
	}

out:
	if (stmt)
		sqlite3_finalize(stmt);

	return r;
}

int __rua_stat_lose_score_update(sqlite3 *db, char *caller, char *rua_stat_tag)
{
	int r;
	char query[QUERY_MAXLEN];
	sqlite3_stmt *stmt = NULL;

	sqlite3_snprintf(QUERY_MAXLEN, query,
		"UPDATE rua_panel_stat SET score = score * %f WHERE caller_panel = ? AND rua_stat_tag != ?",
		LOSE_SCORE_RATE);

	LOGD("lose score update sql : %s", query);
	r = sqlite3_prepare(db, query, sizeof(query), &stmt, NULL);
	if (r != SQLITE_OK) {
		LOGE("sqlite3_prepare error(%d , %d, %s)", r, sqlite3_extended_errcode(db), sqlite3_errmsg(db));
		goto out;
	}

	r = sqlite3_bind_text(stmt, 1, caller, strlen(caller), SQLITE_STATIC);
	if (r != SQLITE_OK) {
		LOGE("caller bind error(%d) \n", r);
		goto out;
	}

	r = sqlite3_bind_text(stmt, 2, rua_stat_tag, strlen(rua_stat_tag), SQLITE_STATIC);
	if (r != SQLITE_OK) {
		LOGE("rua_stat_tag bind error(%d) \n", r);
		goto out;
	}

	r = sqlite3_step(stmt);
	if (r != SQLITE_DONE) {
		LOGE("step error(%d) \n", r);
		goto out;
	}

out:
	if (stmt)
		sqlite3_finalize(stmt);

	return r;
}

int __rua_stat_win_score_update(sqlite3 *db, char *caller, char *rua_stat_tag)
{
	int r;
	char query[QUERY_MAXLEN];
	sqlite3_stmt *stmt = NULL;

	sqlite3_snprintf(QUERY_MAXLEN, query,
		"UPDATE rua_panel_stat SET score = score + %d WHERE caller_panel = ? AND rua_stat_tag = ?",
		WIN_SCORE);

	LOGD("win score update sql : %s", query);

	r = sqlite3_prepare(db, query, sizeof(query), &stmt, NULL);
	if (r != SQLITE_OK) {
		LOGE("sqlite3_prepare error(%d , %d, %s)", r, sqlite3_extended_errcode(db), sqlite3_errmsg(db));
		goto out;
	}

	r = sqlite3_bind_text(stmt, 1, caller, strlen(caller), SQLITE_STATIC);
	if (r != SQLITE_OK) {
		LOGE("caller bind error(%d) \n", r);
		goto out;
	}

	r = sqlite3_bind_text(stmt, 2, rua_stat_tag, strlen(rua_stat_tag), SQLITE_STATIC);
	if (r != SQLITE_OK) {
		LOGE("rua_stat_tag bind error(%d) \n", r);
		goto out;
	}

	r = sqlite3_step(stmt);
	if (r != SQLITE_DONE) {
		LOGE("step error(%d) \n", r);
		goto out;
	}

out:
	if (stmt)
		sqlite3_finalize(stmt);

	return r;
}

static int __exec(sqlite3 *db, char *query)
{
	int r;
	char *errmsg = NULL;

	if (db == NULL)
		return -1;

	r = sqlite3_exec(db, query, NULL, NULL, &errmsg);
	if (r != SQLITE_OK) {
		SECURE_LOGE("query(%s) exec error(%s)", query, errmsg);
		sqlite3_free(errmsg);
		return -1;
	}

	return 0;
}

static int __create_table(sqlite3 *db)
{
	int r;

	r = __exec(db, CREATE_RUA_STAT_TABLE);
	if (r == -1) {
		LOGE("create table error");
		return -1;
	}

	return 0;
}

static sqlite3 *__db_init(sqlite3 *db, char *root, int flags)
{
	int r;

	r = db_util_open_with_options(root, &db, flags, NULL);
	if (r) {
		LOGE("db util open error(%d/%d/%d/%s)", r,
			sqlite3_errcode(db),
			sqlite3_extended_errcode(db),
			sqlite3_errmsg(db));
		return NULL;

	}
	r = __create_table(db);
	if (r) {
		db_util_close(db);
		return NULL;
	}

	return db;
}

int _rua_stat_init(sqlite3 *db, int flags)
{
	char defname[FILENAME_MAX];
	const char *rua_stat_db_path = tzplatform_getenv(TZ_USER_DB);

	snprintf(defname, sizeof(defname), "%s/%s", rua_stat_db_path, RUA_STAT_DB_NAME);
	__db_init(db, defname, flags);

	if (db == NULL) {
		LOGW("__rua_stat_init error");
		return -1;
	}

	return 0;
}

int _rua_stat_fini(sqlite3 *db)
{
	if (db) {
		db_util_close(db);
		db = NULL;
	}
	return 0;
}

int rua_stat_db_update(char *caller, char *rua_stat_tag)
{
	int r;
	int affected_rows = 0;
	sqlite3 *db = NULL;

	LOGD("rua_stat_update start");

	r = _rua_stat_init(db, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE);
	if (r == -1) {
		LOGE("__rua_stat_init fail");
		return -1;
	}

	if (db == NULL) {
		LOGE("rua_stat is not initialized");
		return -1;
	}

	if (caller == NULL) {
		LOGE("caller is null");
		return -1;
	}

	if (rua_stat_tag == NULL) {
		LOGE("rua_stat_tag is null");
		return -1;
	}


	r = __rua_stat_lose_score_update(db, caller, rua_stat_tag);
	if (r != SQLITE_DONE) {
		LOGE("__rua_stat_lose_score_insert fail.");
		return -1;
	}

	r = __rua_stat_win_score_update(db, caller, rua_stat_tag);
	affected_rows = sqlite3_changes(db);
	if ((r != SQLITE_DONE) || (affected_rows == 0)) {
		r = __rua_stat_insert(db, caller, rua_stat_tag);

		if (r != SQLITE_DONE) {
			LOGE("__rua_stat_insert fail.");
			return -1;
		}
	}

	_rua_stat_fini(db);
	LOGD("rua_stat_update done");

	return r;
}

