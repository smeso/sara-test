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

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>

#define SUPPOSED_PAGESIZE 4096
#define BUFSIZE 4096

#define GET_PAGE_ADDR(ADDR)					\
	(char *)(((unsigned long) ADDR) & ~(PAGESIZE-1))

#define RUN_TEST(NAME) do {							\
	pid_t child;								\
	int ret = 0;								\
	if ((child = fork()) == -1) {						\
		printf("cannot fork for '%s': %s\n", #NAME, strerror(errno));	\
		return 1;							\
	} else if (child == 0) {						\
		return NAME();							\
	} else {								\
		waitpid(child, &ret, 0);					\
		if (WIFEXITED(ret) && WEXITSTATUS(ret) == 1)			\
			printf("%25s:\tVULNERABLE\n", #NAME);			\
		else if (WIFEXITED(ret) && WEXITSTATUS(ret) == 0)		\
			printf("%25s:\tOK\n", #NAME);				\
		else								\
			printf("%25s:\tERROR\n", #NAME);			\
	}									\
} while (0)

extern int PAGESIZE;

void get_perms(const void *m, pid_t pid, char perms[5]);
int count_wx_mappings(pid_t pid);
int is_wx(void *m);
int is_x(void *m);
void *do_mmap(size_t len, int prot, int flags, int fd);
void do_mprotect(const void *addr, size_t len, int prot);
int try_wx(void *m, size_t size);
int try_x(void *m, size_t size);
