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
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <elf.h>

#include "utils.h"
#include "libsara/libsara.h"

#define TRAMPOLINE_EXEC EXTRA_BINS_PATH "/trampoline"
#define TRAMPOLINE_NOPIE_EXEC EXTRA_BINS_PATH "/trampoline_nopie"
#define TRAMPOLINE_EXSTACK_EXEC EXTRA_BINS_PATH "/trampoline_exstack"
#define TRANSFER_EXEC EXTRA_BINS_PATH "/transfer"
#define PROCATTR_EXEC EXTRA_BINS_PATH "/procattr"
#define FAKET_EXEC EXTRA_BINS_PATH "/fake_tramp"
#define THIS_FILE "/proc/self/exe"

char BSS[SUPPOSED_PAGESIZE];
char DATA[SUPPOSED_PAGESIZE] = {0};

int wx_mappings(void)
{
	if (count_wx_mappings(getpid()) > 0)
		return 1;
	return 0;
}

void __attribute__((noinline)) __attribute__((optimize("O0"))) __helper(void)
{
	int a = 2 + 4;
	return;
}

int nx_shellcode(void)
{
	void (*a)(void);
	unsigned char buffer[30];
	int ret;
	pid_t child;

	a = __helper;
	a();
	memcpy(buffer, a, sizeof(buffer));
	do_mprotect(buffer, sizeof(buffer), PROT_READ | PROT_WRITE);
	a = (void (*)(void)) buffer;
	if ((child = fork()) == -1) {
		printf("cannot fork: %s\n", strerror(errno));
		return 2;
	} else if (child == 0) {
		a();
		return 0;
	} else {
		waitpid(child, &ret, 0);
		if (WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
			return 1;
	}
	return 0;
}

int gcc_trampolines_working1(void)
{
	int ret;
	pid_t child;

	if ((child = fork()) == -1) {
		printf("cannot fork: %s\n", strerror(errno));
		return 2;
	} else if (child == 0) {
		execlp(TRAMPOLINE_EXEC, TRAMPOLINE_EXEC, "foobar",
		       (char *) NULL);
	} else {
		waitpid(child, &ret, 0);
		if (WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
			return 0;
	}
	return 2;
}

int gcc_trampolines_working2(void)
{
	int ret;
	pid_t child;

	if ((child = fork()) == -1) {
		printf("cannot fork: %s\n", strerror(errno));
		return 2;
	} else if (child == 0) {
		execlp(TRAMPOLINE_NOPIE_EXEC, TRAMPOLINE_NOPIE_EXEC,
		       "foobar", (char *) NULL);
	} else {
		waitpid(child, &ret, 0);
		if (WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
			return 0;
	}
	return 2;
}

int anon_mmap_wx(void)
{
	void *m;

	m = do_mmap(PAGESIZE, PROT_READ | PROT_WRITE | PROT_EXEC,
		    MAP_PRIVATE | MAP_ANONYMOUS, -1);
	if (is_wx(m))
		return 1;
	munmap(m, PAGESIZE);
	return 0;
}

int file_mmap_wx(void)
{
	int fd;
	void *m;

	fd = open(THIS_FILE, O_RDONLY);
	if (fd == -1) {
                printf("open failed: %s\n", strerror(errno));
                exit(2);
        }
        m = do_mmap(PAGESIZE, PROT_READ | PROT_WRITE | PROT_EXEC,
		    MAP_PRIVATE, fd);
	close(fd);
	if (is_wx(m))
		return 1;
	return 0;
}

int gnu_executable_stack(void)
{
	int wx_count = 0;
	pid_t child;

	if ((child = fork()) == -1) {
		printf("cannot fork: %s\n", strerror(errno));
		return 2;
	} else if (child == 0) {
		execlp(TRAMPOLINE_EXSTACK_EXEC, TRAMPOLINE_EXSTACK_EXEC,
		       (char *) NULL);
	} else {
		usleep(10000);
		wx_count = count_wx_mappings(child);
		kill(child, SIGKILL);
		if (wx_count > 0)
			return 1;
	}
	return 0;
}

int heap_mprotect(void)
{
	void *m;

	m = malloc(PAGESIZE);
	if (m == NULL) {
		printf("Out of memory.\n");
		exit(2);
	}
	if (try_x(m, PAGESIZE))
		return 1;
	return 0;
}

int stack_mprotect(void)
{
	char stack[SUPPOSED_PAGESIZE*3];

	if (try_x(stack+SUPPOSED_PAGESIZE, SUPPOSED_PAGESIZE))
		return 1;
	return 0;
}

int anon_mmap_mprotect(void)
{
	void *m;

	m = do_mmap(PAGESIZE, PROT_READ | PROT_WRITE,
		    MAP_PRIVATE | MAP_ANONYMOUS, -1);
	if (try_x(m, PAGESIZE))
		return 1;
	return 0;
}

int file_mmap_mprotect(void)
{
	int fd;
	void *m;

	fd = open(THIS_FILE, O_RDONLY);
	if (fd == -1) {
                printf("open failed: %s\n", strerror(errno));
                exit(2);
        }
	m = do_mmap(PAGESIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd);
	close(fd);
	if (try_x(m, PAGESIZE))
		return 1;
	return 0;
}

int text_mprotect(void)
{
	void *m = text_mprotect;

	if (try_wx(m, PAGESIZE))
		return 1;
	return 0;
}

int bss_mprotect(void)
{
	if (try_x(BSS, sizeof(BSS)))
		return 1;
	return 0;
}

int data_mprotect(void)
{
	if (try_x(DATA, sizeof(DATA))) {
		do_mprotect(DATA, sizeof(DATA), PROT_READ | PROT_WRITE);
		return 1;
	}
	return 0;
}

int fake_trampolines(void)
{
	execlp(FAKET_EXEC, FAKET_EXEC, (char *) NULL);
	return 0;
}

int fake_trampoline_heap(void)
{
	int ret;
	pid_t child;

	if ((child = fork()) == -1) {
		printf("cannot fork: %s\n", strerror(errno));
		return 2;
	} else if (child == 0) {
		execlp(FAKET_EXEC, FAKET_EXEC, "heap", (char *) NULL);
	} else {
		waitpid(child, &ret, 0);
		if (WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
			return 1;
	}
	return 0;
}

int transfer(void)
{
	int ret;
	pid_t child;

	if ((child = fork()) == -1) {
		printf("cannot fork: %s\n", strerror(errno));
		return 2;
	} else if (child == 0) {
		execlp(TRANSFER_EXEC, TRANSFER_EXEC, (char *) NULL);
	} else {
		waitpid(child, &ret, 0);
		if (WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
			return 0;
		else if (WIFEXITED(ret) && WEXITSTATUS(ret) == 1)
			return 1;
		else if (WIFEXITED(ret) && WEXITSTATUS(ret) == 2)
			return 2;
	}
	return 0;
}

int mmap_exec(void)
{
	int fd;
	void *m;

	if (add_wxprot_self_flags(SARA_MMAP))
		return 2;
	fd = open(THIS_FILE, O_RDONLY);
	if (fd == -1) {
                printf("open failed: %s\n", strerror(errno));
                exit(2);
        }
        m = do_mmap(PAGESIZE, PROT_READ | PROT_EXEC,
		    MAP_PRIVATE, fd);
	close(fd);
	return 1;
}

int main(int argc, char *argv[])
{
	pid_t child;

	PAGESIZE = getpagesize();

	printf("These tests should pass even with SARA disabled:\n");
	RUN_TEST(wx_mappings);
	RUN_TEST(nx_shellcode);
#if defined __x86_64__ || defined __i386__
	RUN_TEST(fake_trampoline_heap);
#else
	printf("%25s:\tNOT AVAILABLE\n", "fake_trampoline_heap");
#endif

	printf("\n");
	printf("These tests should pass with SARA fully enabled:\n");
	RUN_TEST(anon_mmap_wx);
	RUN_TEST(file_mmap_wx);
	RUN_TEST(gnu_executable_stack);
	RUN_TEST(heap_mprotect);
	RUN_TEST(stack_mprotect);
	RUN_TEST(anon_mmap_mprotect);
	RUN_TEST(file_mmap_mprotect);
	RUN_TEST(text_mprotect);
	RUN_TEST(bss_mprotect);
	RUN_TEST(data_mprotect);
	RUN_TEST(mmap_exec);
	RUN_TEST(transfer);
	RUN_TEST(gcc_trampolines_working1);
	RUN_TEST(gcc_trampolines_working2);
#if defined __x86_64__ || defined __i386__
	RUN_TEST(fake_trampolines);
#else
	printf("%25s:\tNOT AVAILABLE\n", "fake_trampolines");
#endif

	printf("\nTests for procattr interface:\n");
	if ((child = fork()) == -1) {
		printf("cannot fork for 'procattr': %s\n", strerror(errno));
		return 1;
	} else if (child == 0) {
		execlp(PROCATTR_EXEC, PROCATTR_EXEC, (char *) NULL);
	} else {
		waitpid(child, NULL, 0);
	}

	return 0;
}
