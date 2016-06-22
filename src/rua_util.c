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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <grp.h>
#include <pwd.h>

#include <tzplatform_config.h>
#include <sys/stat.h>
#include <db-util.h>

#include "rua_util.h"

#define DBPATH_LEN_MAX	4096

char *_rua_util_get_db_path(uid_t uid, char *db_name)
{
	struct group *grpinfo = NULL;
	char db_path[DBPATH_LEN_MAX] = {0, };
	struct passwd *userinfo = getpwuid(uid);
	if (userinfo == NULL) {
		LOGE("getpwuid(%d) returns NULL !", uid);
		return NULL;
	}
	grpinfo = getgrnam("users");
	if (grpinfo == NULL) {
		LOGE("getgrnam(users) returns NULL !");
		return NULL;
	}
	/* Compare git_t type and not group name */
	if (grpinfo->gr_gid != userinfo->pw_gid) {
		LOGE("UID [%d] does not belong to 'users' group!", uid);
		return NULL;
	}
	snprintf(db_path, sizeof(db_path), "%s/.applications/dbspace/%s", userinfo->pw_dir, db_name);
	LOGD("db path %s", db_path);
	return db_path;
}

int _rua_util_open_db(sqlite3 **db, int flags, uid_t uid, char *db_name)
{
	int r;
	char *db_path = _rua_util_get_db_path(uid, db_name);
	r = db_util_open_with_options(db_path, db, flags, NULL);
	if (r) {
		LOGE("db util open error(%d/%d/%d/%s)", r,
			sqlite3_errcode(*db),
			sqlite3_extended_errcode(*db),
			sqlite3_errmsg(*db));
		return -1;
	}
	return r;
}

int _rua_util_check_uid(uid_t target_uid)
{
	uid_t uid = getuid();
	if (uid > BASE_UID && uid != target_uid) {
		LOGE("Invalid UID : %d, target UID : %d", uid, target_uid);
		return -1;
	}
	return 0;
}