/*
 * Copyright (c) 2000 - 2016 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * @file    rua_stat.c
 * @author  Hyunho Kang (hhstark.kang@samsung.com)
 * @version 0.1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <db-util.h>

/* For multi-user support */
#include <tzplatform_config.h>

#include "rua_stat.h"
#include "db-schema.h"
#include "perf-measure.h"

#include <dlog.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "RUA"

#define RUA_STAT_DB_NAME	".rua_stat.db"
#define QUERY_MAXLEN	4096
#define WIN_SCORE 100
#define LOSE_SCORE_RATE 0.7f

static sqlite3 *_db = NULL;
static int __exec(sqlite3 *db, char *query);
static sqlite3 *__db_init(char *root, int flags);

int __rua_stat_init(int flags)
{
	char defname[FILENAME_MAX];
	const char *rua_stat_db_path = tzplatform_getenv(TZ_USER_DB);

	if (_db)
		return 0;

	snprintf(defname, sizeof(defname), "%s/%s", rua_stat_db_path, RUA_STAT_DB_NAME);
	_db = __db_init(defname, flags);

	if (_db == NULL) {
		LOGW("__rua_stat_init error");
		return -1;
	}

	return 0;
}

int __rua_stat_fini(void)
{
	if (_db) {
		db_util_close(_db);
		_db = NULL;
	}

	return 0;
}

int __rua_stat_insert(char *caller, char *rua_stat_tag)
{
	int r;
	char query[QUERY_MAXLEN];
	sqlite3_stmt *stmt = NULL;

	sqlite3_snprintf(QUERY_MAXLEN, query,
		"INSERT INTO rua_panel_stat (caller_panel, rua_stat_tag, score) VALUES (?,?,?)");

	r = sqlite3_prepare(_db, query, sizeof(query), &stmt, NULL);
	if (r != SQLITE_OK) {
		LOGE("sqlite3_prepare error(%d , %d, %s)", r, sqlite3_extended_errcode(_db), sqlite3_errmsg(_db));
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

int __rua_stat_lose_score_update(char *caller, char *rua_stat_tag)
{
	int r;
	char query[QUERY_MAXLEN];
	sqlite3_stmt *stmt = NULL;

	sqlite3_snprintf(QUERY_MAXLEN, query,
		"UPDATE rua_panel_stat SET score = score * %f WHERE caller_panel = ? AND rua_stat_tag != ?",
		LOSE_SCORE_RATE);

	LOGD("lose score update sql : %s", query);
	r = sqlite3_prepare(_db, query, sizeof(query), &stmt, NULL);
	if (r != SQLITE_OK) {
		LOGE("sqlite3_prepare error(%d , %d, %s)", r, sqlite3_extended_errcode(_db), sqlite3_errmsg(_db));
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

int __rua_stat_win_score_update(char *caller, char *rua_stat_tag)
{
	int r;
	char query[QUERY_MAXLEN];
	sqlite3_stmt *stmt = NULL;

	sqlite3_snprintf(QUERY_MAXLEN, query,
		"UPDATE rua_panel_stat SET score = score + %d WHERE caller_panel = ? AND rua_stat_tag = ?",
		WIN_SCORE);

	LOGD("win score update sql : %s", query);

	r = sqlite3_prepare(_db, query, sizeof(query), &stmt, NULL);
	if (r != SQLITE_OK) {
		LOGE("sqlite3_prepare error(%d , %d, %s)", r, sqlite3_extended_errcode(_db), sqlite3_errmsg(_db));
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

int rua_stat_update(char *caller, char *rua_stat_tag)
{
	int r;
	int affected_rows = 0;

	LOGD("rua_stat_update start");

	r = __rua_stat_init(SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE);
	if (r == -1) {
		LOGE("__rua_stat_init fail");
		return -1;
	}

	if (_db == NULL) {
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


	r = __rua_stat_lose_score_update(caller, rua_stat_tag);
	if (r != SQLITE_DONE) {
		LOGE("__rua_stat_lose_score_insert fail.");
		return -1;
	}

	r = __rua_stat_win_score_update(caller, rua_stat_tag);
	affected_rows = sqlite3_changes(_db);
	if ((r != SQLITE_DONE) || (affected_rows == 0)) {
		r = __rua_stat_insert(caller, rua_stat_tag);

		if (r != SQLITE_DONE) {
			LOGE("__rua_stat_insert fail.");
			return -1;
		}
	}

	__rua_stat_fini();
	LOGD("rua_stat_update done");

	return r;

}


int rua_stat_get_stat_tags(char *caller,
		int (*rua_stat_tag_iter_fn)(const char *rua_stat_tag, void *data),
		void *data)
{
	int r;
	sqlite3_stmt *stmt;
	char query[QUERY_MAXLEN];
	const unsigned char *ct;

	r = __rua_stat_init(SQLITE_OPEN_READONLY);
	if (r == -1) {
		LOGE("__rua_stat_init fail");
		return -1;
	}

	sqlite3_snprintf(QUERY_MAXLEN, query,
		"SELECT rua_stat_tag FROM rua_panel_stat WHERE caller_panel = ? ORDER BY score DESC");

	if (!_db)
		return -1;

	r = sqlite3_prepare(_db, query, sizeof(query), &stmt, NULL);
	if (r != SQLITE_OK) {
		LOGE("sqlite3_prepare error(%d , %d, %s)", r, sqlite3_extended_errcode(_db), sqlite3_errmsg(_db));
		goto out;
	}

	r = sqlite3_bind_text(stmt, 1, caller, strlen(caller), SQLITE_STATIC);
	if (r != SQLITE_OK) {
		LOGE("caller bind error(%d) \n", r);
		goto out;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		ct = sqlite3_column_text(stmt, 0);
		if (ct == NULL || ct[0] == '\0')
			LOGW("sqlite3_column_text null");

		rua_stat_tag_iter_fn((const char *)ct, data);
	}

out:
	if (stmt)
		sqlite3_finalize(stmt);

	__rua_stat_fini();

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

static int _set_rua_db_journal_wal(sqlite3 *db) {

	int r = __exec(db, "PRAGMA journal_mode = WAL");
	if (r != SQLITE_OK) {
		LOGE("failt to set wal %d", r);
		return r;
	}

	r = __exec(db, "PRAGMA synchronous = NORMAL");
	if (r != SQLITE_OK) {
		LOGE("failt to set sync normal %d", r);
		return r;
	}

	LOGD("__set_rua_db_journal_wal done");

	return r;
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

static sqlite3 *__db_init(char *root, int flags)
{
	int r;
	sqlite3 *db = NULL;
	char *sql_err_msg = NULL;

	r = sqlite3_open_v2(root, &db, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL);
	if (r) {
		LOGE("db util open error(%d/%d/%d/%s)", r,
			sqlite3_errcode(db),
			sqlite3_extended_errcode(db),
			sqlite3_errmsg(db));
		return NULL;

	}
	r = sqlite3_exec(db, "PRAGMA journal_mode=WAL;", NULL, NULL, &sql_err_msg);
	if (r != SQLITE_OK) {
		LOGE("fail to set journal_mode %s", sql_err_msg);
		return NULL;
	}

	r = __create_table(db);
	if (r) {
		db_util_close(db);
		return NULL;
	}

	r = _set_rua_db_journal_wal(db);
	if (r) {
		db_util_close(db);
		return NULL;
	}

	return db;
}
