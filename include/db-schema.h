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

#ifndef __RUA_SCHEMA_H__
#define __RUA_SCHEMA_H__

#define CREATE_RUA_HISTORY_TABLE " \
PRAGMA journal_mode = WAL; \
\
CREATE TABLE IF NOT EXISTS rua_history ( \
	pkg_name TEXT, \
	app_path TEXT, \
	arg TEXT, \
	launch_time INTEGER, \
	PRIMARY KEY(pkg_name) \
);"

#define CREATE_RUA_STAT_TABLE " \
CREATE TABLE if not exists rua_panel_stat ( \
	caller_panel TEXT NOT NULL, \
	rua_stat_tag TEXT NOT NULL, \
	score INTEGER DEFAULT 0, \
	PRIMARY KEY(rua_stat_tag, caller_panel) \
);"

/* table index */
enum {
	RUA_COL_PKGNAME,
	RUA_COL_APPPATH,
	RUA_COL_ARG,
	RUA_COL_LAUNCHTIME
};

enum {
	RUA_STAT_COL_CALLER_PANEL = 0x00,
	RUA_STAT_COL_RUA_STAT_TAG,
	RUA_SATA_COL_SCORE
};

#endif /* __RUA_SCHEMA_H__ */
