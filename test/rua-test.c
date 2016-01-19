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

/* For multi-user support */
#include <tzplatform_config.h>

#include "rua.h"

static int __add_history(char *pkgname)
{
	int ret;
	struct rua_rec *rec;

	rec = (struct rua_rec *)calloc(1, sizeof(struct rua_rec));
	if (rec == NULL) {
		printf("Failed to allocate rua_rec\n");
		return -1;
	}

	rec->pkg_name = strdup(pkgname);
	rec->app_path = strdup(tzplatform_mkpath(TZ_SYS_RW_APP, pkgname));

	ret = rua_add_history(rec);

	free(rec->pkg_name);
	free(rec->app_path);
	free(rec);

	return ret;
}

int main(int argc, char* argv[])
{
	int ret = 0;

	if (argc != 2)
		return 0;
	ret = __add_history(argv[1]);
	return ret;
}
