/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All rights reserved.
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
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <db-util.h>
/* For multi-user support */
#include <tzplatform_config.h>

#include "db-schema.h"
#include "rua_stat_internal.h"
#include "rua_util.h"

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

int _rua_stat_init(sqlite3 **db, char *db_name, int flags, uid_t uid)
{
	int r;
	r = _rua_util_open_db(db, flags, uid, db_name);
	r = __create_table(*db);
	if (r) {
		db_util_close(*db);
		return -1;
	}

	if (*db == NULL) {
		LOGE("__rua_stat_init error");
		return -1;
	}
	return 0;
}

int rua_stat_db_update(char *caller, char *rua_stat_tag)
{
	int r;
	int affected_rows = 0;
	sqlite3 *db = NULL;

	LOGD("rua_stat_update start");

	r = _rua_stat_init(&db, RUA_STAT_DB_NAME, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, getuid());
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
	if (db)
		db_util_close(db);
	LOGD("rua_stat_update done");
	return r;
}

