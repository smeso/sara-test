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

typedef void (*fptr)(void);

void f0(fptr f)
{
	(*f)();
}

void f1(void)
{
	int i;

	i = 0;
	i += 1;
        void f2(void) {
		i += 3;
        }
	f0(f2);
}

int main(int argc, char *argv[])
{
	f1();
	if (argc == 1)
		sleep(60);
	return 0;
}
