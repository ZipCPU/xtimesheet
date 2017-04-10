////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	totalhrs.cpp
//
// Project:	Xtimesheet, a very simple text-based timesheet tracking program
//
// Purpose:	Caclulate the total number of hours from within a particular
//		timesheet file.
//
// Creator:	Dan Gisselquist, Ph.D.
//		Gisselquist Technology, LLC
//
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2017, Gisselquist Technology, LLC
//
// This program is free software (firmware): you can redistribute it and/or
// modify it under the terms of  the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTIBILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program.  (It's in the $(ROOT)/doc directory, run make with no
// target there if the PDF file isn't present.)  If not, see
// <http://www.gnu.org/licenses/> for a copy.
//
// License:	GPL, v3, as defined and found on www.gnu.org,
//		http://www.gnu.org/licenses/gpl.html
//
//
////////////////////////////////////////////////////////////////////////////////
//
//
#include <stdio.h>
#include <time.h>

#include "timecard.h"

#define	MXLEN	512

int main(int argc, char **argv) {
	TIMECARD	tc;
	time_t		midnight = 0, acc = 0;
	char	line[MXLEN];

	for(int argn=1; argn<argc; argn++) {
		if (access(argv[argn], R_OK)==0) {
			FILE	*fp = fopen(argv[argn], "r");
			while(fgets(line, MXLEN, fp)) {
				time_t	lnstart, lnstop;
				if (tc.parse(line, lnstart, lnstop)) {
					if (lnstart > 24*3600) {
						midnight = tc.get_midnight(lnstart);
					} else {
						lnstart += midnight;
						lnstop  += midnight;
					}

					acc += lnstop - lnstart;
					// printf("Adding %ld seconds ~= %.1f hours\n", lnstop-lnstart, (double)(lnstop-lnstart)/3600.0);
				}
			}

			fclose(fp);
		} else fprintf(stderr, "WARNING: Cannot access %s\n", argv[argn]);
	}

	long	hour_tenths = (long)(acc + 180)/60/6;
	printf("%.1f Hours\n", (double)hour_tenths/10.0);
	// printf("%ld seconds\n", acc);
}

