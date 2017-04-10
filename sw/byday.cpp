////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	byday.cpp
//
// Project:	Xtimesheet, a very simple text-based timesheet tracking program
//
// Purpose:	Produce daily totals of hours worked
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
__attribute__((unused))
static const char *cpyright = "(C) 2017 Gisselquist Technology, LLC: " __FILE__;


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <assert.h>

#include "timecard.h"

extern long	timezone; // seconds west of UTC

const char	*daystr[] = {
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday"
};

class	DAILYSHEET : public TIMECARD {
public:
	time_t		m_last_start, m_today;
	bool		m_currently_working;
	char		*m_fname, *m_name;
	unsigned	m_sumunits, m_daily_s;
	double		m_hourly_rate;

	DAILYSHEET(void) {
		m_last_start = 0;
		m_currently_working = false;
		m_today = get_midnight(time(NULL));
		m_fname = NULL;
		m_name = NULL;
		m_sumunits = m_daily_s = 0;
		m_hourly_rate = 225.0;
	}

	void	load(const char *fname, bool latex) {
		m_hourly_rate = 225.0;

		if (m_fname)
			delete[] m_fname;
		m_fname = new char[strlen(fname)+2];
		strcpy(m_fname, fname);

		assert(access(fname, R_OK)==0);
		assert(access(fname, W_OK)==0);

		const	unsigned MXLEN=4096;
		FILE		*fp;
		char	*line;
		time_t	thisday = 0, lnstart=0, lnstop=0;

		m_today = get_midnight(time(NULL));
		m_sumunits = m_daily_s = 0;

		line = new char[MXLEN];
		fp = fopen(m_fname, "r");
		while(fgets(line, MXLEN, fp)) {
			if (strncasecmp(line, "rate:", 5)==0) {
				m_hourly_rate = atof(&line[5]);
			} else if (strncasecmp(line, "project:", 8)==0) {
			} else if (parse(line, lnstart, lnstop)) {
				time_t		midnight;
				if (lnstart > 24*3600) {
					midnight = get_midnight(lnstart);
					if (midnight != thisday) {
						int nunits = (m_daily_s+180)/60/6;
						if (m_daily_s > 0) {
							struct tm datev;
							localtime_r(&thisday, &datev);
							dailysum(&thisday, nunits, latex);
							m_sumunits  += nunits;
							m_daily_s = 0;
						}
						thisday = midnight;
					}
				}

				m_daily_s += lnstop-lnstart;
			}
		}
		fclose(fp);

		if (m_today != thisday) {
			dailysum(&thisday, (m_daily_s+180)/360, latex);
			m_sumunits  += (m_daily_s+180)/60/6;
			m_daily_s = 0;
		}

		delete[] line;
	}

	void	dailysum(time_t *date, int nunits, bool latex) const {
		if (nunits <= 0)
			return;
		struct tm datev;
		localtime_r(date, &datev);

		if (latex) {
		printf("\\Fee{%04d/%02d/%02d, %s}{%.2f}{%.1f}{%.2f}\n",
			datev.tm_year+1900,
			datev.tm_mon+1,
			datev.tm_mday,
			daystr[datev.tm_wday],
			m_hourly_rate,
			nunits/10.,
			m_hourly_rate * nunits/10.0
			);
		} else {
		printf("%9s %04d/%02d/%02d: %4.1f\n",
			daystr[datev.tm_wday],
			datev.tm_year+1900,
			datev.tm_mon+1,
			datev.tm_mday,
			nunits/10.);
		}
	}

	void	last(bool latex) {
		if (m_daily_s > 0) {
			dailysum(&m_today, (m_daily_s+180)/360, latex);
			m_sumunits  += (m_daily_s+180)/60/6;
			m_daily_s = 0;
		}
	}
};

int main(int argc, char **argv) {
	DAILYSHEET	ds;
	bool		latex = false;

	for(int argn=1; argn<argc; argn++) {
		if (argv[argn][0] == '-') {
			if (tolower(argv[argn][1]) == 'l')
				latex = true;
		} else
			ds.load(argv[argn], latex);
	} ds.last(latex);

	if (latex)
		;
	else
		printf("Total: %.1f Hours\n", ds.m_sumunits/10.0);

	return (0);
}

