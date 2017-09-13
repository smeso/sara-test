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
#include <stdlib.h>
#include <string.h>

int i = 0;
void (*caller)(void);

#define RUN_TEST(I) I
#define RUN_TEST_HEAP(I) I

#ifdef __x86_64__
#undef RUN_TEST
#undef RUN_TEST_HEAP
void fun(void)
{
	__asm__ ("addl   $0x1,(%r10)\n");
}

struct libffi_trampoline_x86_64 {
	unsigned short mov1;
	unsigned long addr1;
	unsigned short mov2;
	unsigned long addr2;
	unsigned char stcclc;
	unsigned short jmp1;
	unsigned char jmp2;
} __attribute__((packed));

#define DEFINE_LIBFFI_TRAMPOLINE_x86_64(NAME, FUN, VAR)	\
	struct libffi_trampoline_x86_64 NAME = {	\
	.mov1 = 0xBB49,					\
	.mov2 = 0xBA49,					\
	.stcclc = 0xF8,					\
	.jmp1 = 0xFF49,					\
	.jmp2 = 0xE3,					\
	.addr1 = (unsigned long) FUN,			\
	.addr2 = (unsigned long) VAR};

struct gcc_trampoline_x86_64_type1 {
	unsigned short mov1;
	unsigned long addr1;
	unsigned short mov2;
	unsigned long addr2;
	unsigned short jmp1;
	unsigned char jmp2;
} __attribute__((packed));

#define DEFINE_GCC_TRAMPOLINE_x86_64_TYPE1(NAME, FUN, VAR)	\
	struct gcc_trampoline_x86_64_type1 NAME = {		\
	.mov1 = 0xBB49,						\
	.mov2 = 0xBA49,						\
	.jmp1 = 0xFF49,						\
	.jmp2 = 0xE3,						\
	.addr1 = (unsigned long) FUN,				\
	.addr2 = (unsigned long) VAR};

struct gcc_trampoline_x86_64_type2 {
	unsigned short mov1;
	unsigned int addr1;
	unsigned short mov2;
	unsigned long addr2;
	unsigned short jmp1;
	unsigned char jmp2;
} __attribute__((packed));

#define DEFINE_GCC_TRAMPOLINE_x86_64_TYPE2(NAME, FUN, VAR)	\
	struct gcc_trampoline_x86_64_type2 NAME = {		\
	.mov1 = 0xBB41,						\
	.mov2 = 0xBA49,						\
	.jmp1 = 0xFF49,						\
	.jmp2 = 0xE3,						\
	.addr1 = (unsigned int) FUN,				\
	.addr2 = (unsigned long) VAR};

#define RUN_TEST(I) do {					\
	int j = I;						\
	DEFINE_LIBFFI_TRAMPOLINE_x86_64(lf, fun, &I);		\
	DEFINE_GCC_TRAMPOLINE_x86_64_TYPE1(g1, fun, &I);	\
	DEFINE_GCC_TRAMPOLINE_x86_64_TYPE2(g2, fun, &I);	\
	caller = (typeof(caller)) &lf;				\
	caller();						\
	if (i != j+1)						\
		return j+1;					\
	caller = (typeof(caller)) &g1;				\
	caller();						\
	if (i != j+2)						\
		return j+2;					\
	caller = (typeof(caller)) &g2;				\
	caller();						\
	if (i != j+3)						\
		return j+3;					\
} while (0)

#define RUN_TEST_HEAP(I) do {					\
	int j = I;						\
	char *heap;						\
	DEFINE_GCC_TRAMPOLINE_x86_64_TYPE1(g1, fun, &I);	\
	heap = malloc(sizeof(g1));				\
	memcpy(heap, &g1, sizeof(g1));				\
	caller = (typeof(caller)) heap;				\
	caller();						\
	if (i != j)						\
		return j;					\
} while (0)
#endif /* __x86_64__ */

#ifdef __i386__
#undef RUN_TEST
#undef RUN_TEST_HEAP
void fun(void)
{
	__asm__ ("addl   $0x1,(%ecx)\n");
}
void fun2(void)
{
	__asm__ ("addl   $0x1,(%eax)\n");
}

struct gcc_trampoline_x86_32_type1 {
	unsigned char mov1;
	unsigned int addr1;
	unsigned char mov2;
	unsigned int addr2;
	unsigned short jmp;
} __attribute__((packed));

#define DEFINE_GCC_TRAMPOLINE_x86_32_TYPE1(NAME, FUN, VAR)	\
	struct gcc_trampoline_x86_32_type1 NAME = {		\
	.mov1 = 0xB9,						\
	.mov2 = 0xB8,						\
	.jmp = 0xE0FF,						\
	.addr1 = (unsigned long) VAR,				\
	.addr2 = (unsigned long) FUN};

struct libffi_trampoline_x86_32 {
	unsigned char mov;
	unsigned int addr1;
	unsigned char jmp;
	unsigned int addr2;
} __attribute__((packed));

#define DEFINE_LIBFFI_x86_32_TYPE1(NAME, VAR)		\
	struct libffi_trampoline_x86_32 NAME = {	\
	.mov = 0xB8,					\
	.jmp = 0xE9,					\
	.addr1 = (unsigned int) VAR,			\
	.addr2 = (unsigned int) 0};

struct gcc_trampoline_x86_32_type2 {
	unsigned char mov;
	unsigned int addr1;
	unsigned char jmp;
	unsigned int addr2;
} __attribute__((packed));
	
#define DEFINE_GCC_TRAMPOLINE_x86_32_TYPE2(NAME, VAR)		\
	struct gcc_trampoline_x86_32_type2 NAME = {	\
	.mov = 0xB9,					\
	.jmp = 0xE9,					\
	.addr1 = (unsigned int) VAR,			\
	.addr2 = (unsigned int) 0};

#define RUN_TEST(I) do {					\
	int j = I;						\
	DEFINE_GCC_TRAMPOLINE_x86_32_TYPE1(g1, fun, &I);	\
	DEFINE_LIBFFI_x86_32_TYPE1(lf, &I);			\
	DEFINE_GCC_TRAMPOLINE_x86_32_TYPE2(g2, &I);		\
	lf.addr2 = (unsigned int) (fun2 -			\
				   sizeof(lf) -			\
				   (unsigned long) &lf);	\
	g2.addr2 = (unsigned int) (fun -			\
				   sizeof(g2) -			\
				   (unsigned long) &g2);	\
	caller = (typeof(caller)) &g1;				\
	caller();						\
	if (i != j+1)						\
		return j+1;					\
	caller = (typeof(caller)) &lf;				\
	caller();						\
	if (i != j+2)						\
		return j+2;					\
	caller = (typeof(caller)) &g2;				\
	caller();						\
	if (i != j+3)						\
		return j+3;					\
} while (0)

#define RUN_TEST_HEAP(I) do {					\
	int j = I;						\
	char *heap;						\
	DEFINE_GCC_TRAMPOLINE_x86_32_TYPE1(g1, fun, &I);	\
	heap = malloc(sizeof(g1));				\
	memcpy(heap, &g1, sizeof(g1));				\
	caller = (typeof(caller)) heap;				\
	caller();						\
	if (i != j)						\
		return j;					\
} while (0)
#endif /* __i386__ */

int main(int argc, char *argv[])
{
	if (argc == 1)
		RUN_TEST(i);
	else
		RUN_TEST_HEAP(i);
	return 0;
}
