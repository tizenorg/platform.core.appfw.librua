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

#include <stdio.h>
#include <getopt.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <glib.h>

/* For multi-user support */
#include <tzplatform_config.h>

#include "rua.h"
#include "rua_internal.h"
#include "rua_stat.h"
#include "rua_stat_internal.h"

static int __add_history()
{
	int ret;
	char *app_path;
	char *pkgname = "org.tizen.ruatester";

	app_path = (char *)tzplatform_mkpath(TZ_SYS_RW_APP, pkgname);
	ret = rua_add_history_for_uid(pkgname, app_path, NULL, 5001);
	return ret;
}

static int __delete_history_with_pkgname()
{
	int ret;
	char *app_path;
	char *pkgname = "org.tizen.ruatester";

	ret = rua_delete_history_with_pkgname_for_uid(pkgname, 5001);
	return ret;
}

static int __load_rua_history()
{
	char **table = NULL;
	int rows = 0;
	int cols = 0;
	struct rua_rec record;

	if (rua_history_load_db_for_uid(&table, &rows, &cols, 5001) || !table) {
		printf("fail to load rua history \n");
		return -1;
	}

	int row;
	for (row = 0; row < rows; ++row) {
		rua_history_get_rec(&record, table, rows, cols, row);
		printf("pkgname : %s, time : %d \n", record.pkg_name, record.launch_time);
	}

	rua_history_unload_db(&table);
	return 0;
}

static int __update_stat()
{
	int ret;
	char *app_path;
	char *pkgname = "org.tizen.ruatester";

	ret = rua_stat_update_for_uid("ruacaller", "org.tizen.ruatester", 5001);
	return ret;
}

static int __rua_stat_tag_iter_cb(const char *rua_stat_tag, void *data)
{
	printf("rua_stat_tag : %s \n", rua_stat_tag);

	return 0;
}

static int __get_stat_tags()
{
	int ret;
	ret = rua_stat_get_stat_tags_for_uid("ruacaller", __rua_stat_tag_iter_cb, NULL, 5001);
	return ret;
}

static gboolean run_test(int selected_number)
{
	gboolean go_to_loop = TRUE;

	switch (selected_number) {
	case 0:
		go_to_loop = FALSE;
		break;

	case 1:
		__add_history();
		break;

	case 2:
		__delete_history_with_pkgname();
		break;

	case 3:
		__load_rua_history();
		break;

	case 4:
		__update_stat();
		break;

	case 5:
		__get_stat_tags();
		break;

	default:
		break;
	}

	return go_to_loop;

}

int main()
{
	int ret = 0;
	int test_num;
	gboolean run_next = TRUE;

	while(run_next) {
		printf("==========================================\n");
		printf("    Basic test menu \n");
		printf("==========================================\n");
		printf(" 0.  EXIT\n");
		printf(" 1.  Add rua history to DEFAULT USER(5001)\n");
		printf(" 2.  Delete history with pkgname\n");
		printf(" 3.  Load RUA history\n");
		printf(" 4.  Update RUA stat\n");
		printf(" 5.  Get RUA stat tags\n");
		printf("------------------------------------------\n");
		scanf("%d", &test_num);
		run_next = run_test(test_num);
	}
	return ret;
}
