/*
 *  RUA
 *
 * Copyright (c) 2000 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Jayoun Lee <airjany@samsung.com>
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
 *
 */

#include <stdio.h>
#include <getopt.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "rua.h"

int __add_history(char *pkgname)
{
	int ret = 0;
	struct rua_rec rec;
	char apppath[FILENAME_MAX] = "";

	snprintf(apppath, "/opt/apps/%s", pkgname, FILENAME_MAX);
	memset(&rec, 0, sizeof(rec));
	rec.pkg_name = pkgname;
	rec.app_path = apppath;

	ret = rua_init();
	if (ret) {
		fprintf(stderr, "error rua_init()\n");
		return -1;
	}

	ret = rua_add_history(&rec);

	rua_fini();

	return ret;

}

int main(int argc, char* argv[])
{
	int ret = 0;

	if (argc != 2)
		return 0;
	ret = __add_history(argv[1]);
	return 0;
}


