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
#include <unistd.h>
#include <sys/types.h>

#include <db-util.h>
#include <aul.h>

#include "rua_stat_internal.h"
#include "rua_stat.h"

int rua_stat_update_for_uid(char *caller, char *tag, uid_t uid)
{
	int r;
	bundle *b = NULL;

	if (caller == NULL || tag == NULL) {
		LOGE("invalid param");
		return -1;
	}

	b = bundle_create();
	if (b == NULL) {
		LOGE("bundle_create fail out of memory.");
		return -1;
	}
	bundle_add_str(b, AUL_SVC_K_RUA_STAT_CALLER, caller);
	bundle_add_str(b, AUL_SVC_K_RUA_STAT_TAG, tag);
	r = aul_update_rua_stat_for_uid(b, uid);
	LOGI("rua_add_history_for_uid result : %d ", r);
	bundle_free(b);
	return r;
}

int rua_stat_get_stat_tags(char *caller,
		int (*rua_stat_tag_iter_fn)(const char *rua_stat_tag, void *data),
		void *data)
{
	return rua_stat_get_stat_tags_for_uid(caller, rua_stat_tag_iter_fn, data, getuid());
}

int rua_stat_get_stat_tags_for_uid(char *caller,
		int (*rua_stat_tag_iter_fn)(const char *rua_stat_tag, void *data),
		void *data, uid_t uid)
{
	int r;
	sqlite3_stmt *stmt;
	char query[QUERY_MAXLEN];
	const unsigned char *ct;
	sqlite3 *db = NULL;

	r = _rua_stat_init(&db, RUA_STAT_DB_NAME, SQLITE_OPEN_READONLY, uid);
	if (r == -1) {
		LOGE("__rua_stat_init fail");
		return -1;
	}

	sqlite3_snprintf(QUERY_MAXLEN, query,
		"SELECT rua_stat_tag FROM rua_panel_stat WHERE caller_panel = ? ORDER BY score DESC");

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

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		ct = sqlite3_column_text(stmt, 0);
		if (ct == NULL || ct[0] == '\0')
			LOGW("sqlite3_column_text null");

		rua_stat_tag_iter_fn((const char *)ct, data);
	}

out:
	if (stmt)
		sqlite3_finalize(stmt);

	if (db)
		db_util_close(db);

	return r;
}
