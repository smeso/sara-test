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

#include "utils.h"
#include "libsara/sara.h"

#define INITIAL_FLAGS (SARA_WXORX | \
		       SARA_OTHER | \
		       SARA_MMAP | \
		       SARA_COMPLAIN | \
		       SARA_VERBOSE)

int correct_settings(void)
{
	if (get_wxprot_self_flags() == INITIAL_FLAGS)
		return 0;
	return 1;
}

int verbosity_change(void)
{
	if (set_wxprot_self_flags(INITIAL_FLAGS-SARA_VERBOSE))
		return 2;
	if (get_wxprot_self_flags() == INITIAL_FLAGS)
		return 0;
	return 1;
}

int complain_change(void)
{
	if (set_wxprot_self_flags(INITIAL_FLAGS-SARA_COMPLAIN))
		return 2;
	if (get_wxprot_self_flags() == INITIAL_FLAGS)
		return 1;
	set_wxprot_self_flags(INITIAL_FLAGS & SARA_COMPLAIN);
	if (get_wxprot_self_flags() == INITIAL_FLAGS)
		return 1;
	return 0;
}

int full_change_no_force(void)
{
	uint16_t flags = SARA_MPROTECT | SARA_MMAP | SARA_VERBOSE;

	if (set_wxprot_self_flags(flags))
		return 2;
	if (get_wxprot_self_flags() != flags)
		return 1;
	set_wxprot_self_flags(INITIAL_FLAGS);
	if (get_wxprot_self_flags() == INITIAL_FLAGS)
		return 1;
	return 0;
}

int force_wxorx(void)
{
	uint16_t flags = SARA_FULL | SARA_VERBOSE;

	do_mprotect(force_wxorx, PSIZE, PROT_READ | PROT_WRITE | PROT_EXEC);
	if (count_wx_mappings(getpid()) == 0)
		return 2;
	if (set_wxprot_self_flags(flags))
		return 2;
	flags &= ~SARA_FORCE_WXORX;
	if (get_wxprot_self_flags() != flags)
		return 1;
	if (count_wx_mappings(getpid()) != 0)
		return 1;
	set_wxprot_self_flags(INITIAL_FLAGS);
	if (get_wxprot_self_flags() == INITIAL_FLAGS)
		return 1;
	return 0;
}

int main(int argc, char *argv[])
{
	PSIZE = getpagesize();

	RUN_TEST(correct_settings);
	if (correct_settings()) {
		printf("procattr tests disabled\n");
		return 1;
	}
	RUN_TEST(verbosity_change);
	RUN_TEST(complain_change);
	RUN_TEST(full_change_no_force);
	RUN_TEST(force_wxorx);

	return 0;
}
