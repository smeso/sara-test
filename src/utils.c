/*
 *    sara-test - S.A.R.A.'s test suite
 *    Copyright (C) 2017  Salvatore Mesoraca <s.mesoraca16@gmail.com>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"

int PSIZE = SUPPOSED_PSIZE;

void get_perms(const void *m, pid_t pid, char perms[5])
{
	char path[25] = {0};
	FILE *fd;
	char buffer[BUFSIZE]  = {0};
	char *token;
	char addr[50] = {0};

	snprintf(path, 25, "/proc/%d/maps", pid);
	fd = fopen(path, "r");
	if (fd == NULL) {
		printf("proc open failed: %s\n", strerror(errno));
		exit(2);
	}
	snprintf(addr, 50, "%08lx-", (unsigned long) GET_PAGE_ADDR(m));
	snprintf(perms, 5, "none");
	while (fgets(buffer, BUFSIZE, fd) != NULL) {
		if (strncmp(buffer, addr, strlen(addr)) == 0) {
			token = strtok(buffer, " ");
			token = strtok(NULL, " ");
			snprintf(perms, 5, "%s", token);
			break;
		}
	}
	fclose(fd);
	if (strncmp(perms, "none", 5) == 0) {
		printf("proc error: page not found\n");
		exit(2);
	}
}

int count_wx_mappings(pid_t pid)
{
	int count = 0;
	char path[25] = {0};
	FILE *fd;
	char buffer[BUFSIZE]  = {0};
	char *token;

	snprintf(path, 25, "/proc/%d/maps", pid);
	fd = fopen(path, "r");
	if (fd == NULL) {
		printf("proc open failed: %s\n", strerror(errno));
		exit(2);
	}
	while (fgets(buffer, BUFSIZE, fd) != NULL) {
		token = strtok(buffer, " ");
		token = strtok(NULL, " ");
		if (token[1] == 'w' && token[2] == 'x') {
			++count;
		}
	}
	fclose(fd);
	return count;
}

int is_wx(void *m)
{
	char perms[5];
	get_perms(m, getpid(), perms);
	if (perms[1] == 'w' && perms[2] == 'x')
		return 1;
	return 0;
}

int is_x(void *m)
{
	char perms[5];
	get_perms(m, getpid(), perms);
	if (perms[2] == 'x')
		return 1;
	return 0;
}

void *do_mmap(size_t len, int prot, int flags, int fd)
{
	void *m = mmap(NULL, len, prot, flags, fd, 0);
	if (m == MAP_FAILED && errno != EPERM) {
		printf("mmap failed: %s\n", strerror(errno));
		exit(2);
	} else if (m == MAP_FAILED && errno == EPERM) {
		exit(0);
	}
	return m;
}

void do_mprotect(const void *addr, size_t len, int prot)
{
	int ret = mprotect(GET_PAGE_ADDR(addr), len, prot);
	if(ret != 0 && errno != EACCES) {
		printf("mprotect failed: %s\n", strerror(errno));
		exit(2);
	} else if(ret != 0 && errno == EACCES) {
		exit(0);
	}
}

int try_wx(void *m, size_t size)
{
	do_mprotect(m, size, PROT_READ | PROT_WRITE | PROT_EXEC);
	if (is_wx(m))
		return 1;
	return 0;
}

int try_x(void *m, size_t size)
{
	do_mprotect(m, size, PROT_READ | PROT_EXEC);
	if (is_x(m))
		return 1;
	return 0;
}
