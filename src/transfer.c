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

#include <unistd.h>
#include "libsara/libsara.h"

int main(int argc, char *argv[])
{
	if (argc > 1) {
		if (get_wxprot_self_flags() & SARA_TRANSFER)
			return 0;
		else
			return 1;
	} else {
		if (get_wxprot_self_flags() & SARA_TRANSFER)
			return 2;
		add_wxprot_self_flags(SARA_TRANSFER);
		execlp(argv[0], argv[0], "foo", (char *) NULL);
	}

	return 2;
}
