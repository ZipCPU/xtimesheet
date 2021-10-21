////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	xtimesheet.cpp
//
// Project:	Xtimesheet, a very simple text-based timesheet tracking program
//
// Purpose:	This is the main program for a GUI managing automatic
//		updates and changes to my hourly timesheet files.
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
#include <signal.h>

#include <gtk/gtk.h>
#include <gtkmm.h>

#include "gladef.h"
#include "sm_splash.cpp"
#include "timecard.h"

extern long	timezone; // seconds west of UTC

class	XTIMESHEET : public TIMECARD {
public:
	time_t		m_last_start, m_today;
	bool		m_currently_working;
	char		*m_fname, *m_name;
	unsigned	m_sumunits, m_daily_s;
	double		m_hourly_rate;

	XTIMESHEET(void) {
		m_last_start = 0;
		m_currently_working = false;
		m_today = get_midnight(time(NULL));
		m_fname = NULL;
		m_name = NULL;
		m_sumunits = m_daily_s = 0;
		m_hourly_rate = 225.0;
	}

	void	load(const char *fname) {
		m_hourly_rate = 225.0;

		if (m_fname)
			delete[] m_fname;
		m_fname = new char[strlen(fname)+2];
		strcpy(m_fname, fname);

		assert(access(fname, R_OK)==0);
		assert(access(fname, W_OK)==0);

		reload();
	}

	void	reload(void) {
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
				char	*ptr;
				ptr = &line[8];
				while(isspace(*ptr))
					ptr++;
				if (strlen(ptr) > 1) {
					if (m_name)
						delete[] m_name;
					m_name = new char[strlen(ptr)+2];
					strcpy(m_name, ptr);
					while(isspace(m_name[strlen(m_name)-1]))
						m_name[strlen(m_name)-1]='\0';
				}
			} else if (parse(line, lnstart, lnstop)) {
				time_t		midnight;
				if (lnstart > 24*3600) {
					midnight = get_midnight(lnstart);
					if (midnight != thisday) {
						m_sumunits  += (m_daily_s+180)/60/6;
						m_daily_s = 0;
						thisday = midnight;
					}
				}

				m_daily_s += lnstop-lnstart;
			}
		}
		fclose(fp);

		if (m_today != thisday) {
			m_sumunits  += (m_daily_s+180)/60/6;
			m_daily_s = 0;
		}

		delete[] line;
	}

	void	toggle(void) {
		if (!m_currently_working) {
			reload();

			m_currently_working = true;
			time(&m_last_start);
		} else {
			time_t	now = time(NULL);
			log(m_last_start, now);
			m_currently_working = false;

			m_daily_s += (now - m_last_start);
			m_last_start = 0;
		}
	}

	void	log(time_t t_start, time_t t_stop) {
		TIMECARD::log(m_fname, t_start, t_stop);
	}

};

class	APPDATA {
public:
	XTIMESHEET	*m_xts;
	Gtk::ApplicationWindow	*m_xts_main;
	Gtk::Entry		*m_taskname, *m_taskfile, *m_totalcost, *m_today,
				*m_sumhours, *m_hourlyrate;
	Gtk::ToggleButton	*m_working_btn;
	Gtk::ProgressBar	*m_daily_prg;
	Gtk::Image		*m_splash;
	bool			m_terminate_now;

	APPDATA(void) {
		m_xts        = NULL;
		m_xts_main   = NULL;
		m_taskname   = NULL;
		m_taskfile   = NULL;
		m_totalcost  = NULL;
		m_today      = NULL;
		m_sumhours   = NULL;
		m_hourlyrate = NULL;
		m_working_btn= NULL;
		m_daily_prg  = NULL;
		m_terminate_now = false;
	}

	void	set_values(void) {
		char	buf[128];
		m_taskfile->set_text(m_xts->m_fname);
		if (m_xts->m_name)
			m_taskname->set_text(m_xts->m_name);
		else
			m_taskname->set_text("No name");

		sprintf(buf, "$ %.2f", m_xts->m_hourly_rate);
		m_hourlyrate->set_text(buf);

		sprintf(buf, "%.1f", (m_xts->m_sumunits+((m_xts->m_daily_s+180)/360)) / 10.0);
		m_sumhours->set_text(buf);

		double	 v = m_xts->m_sumunits * m_xts->m_hourly_rate / 10.0;
		sprintf(buf, "%.2f", v);
		m_totalcost->set_text(buf);

		v = m_xts->m_daily_s / 3600.0;
		sprintf(buf, "%.1f", v);
		m_today->set_text(buf);

		tick();

		m_splash->set(Gdk::Pixbuf::create_from_inline(sizeof(sm_splash), sm_splash));
	}

	void	tick(void) {
		char	buf[128];
		time_t	daily_s = m_xts->m_daily_s;
		unsigned	sumunits;
		double	f;


		if (m_terminate_now) {
			close();
			return;
		}

		if (m_xts->m_currently_working)
			daily_s = m_xts->m_daily_s + (time(NULL)-m_xts->m_last_start);

		sumunits = m_xts->m_sumunits + ((daily_s+180)/360);
		sprintf(buf, "%.1f", sumunits / 10.0);
		m_sumhours->set_text(buf);

		double	 v = sumunits * m_xts->m_hourly_rate / 10.0;
		sprintf(buf, "%.2f", v);
		m_totalcost->set_text(buf);


		f = (double)daily_s; f = f / 8.0 / 3600.0;
		if (f > 1.0) f = 1.0;
		m_daily_prg->set_fraction(f);
		f = daily_s / 3600.0;
		sprintf(buf, "%4.1f", f);
		m_daily_prg->set_text(buf);

		f = daily_s / 3600.0;
		sprintf(buf, "%.1f", f);
		m_today->set_text(buf);

		if (m_xts->m_currently_working) {
			unsigned hrs, mns, len;
			len = (time(NULL)-m_xts->m_last_start);
			hrs = len / 3600;
			mns = (len-hrs*3600)/60;
			if ((mns>0)||(hrs>0)) {
				sprintf(buf, "Working (%d:%02d)", hrs, mns);
				m_working_btn->set_label(buf);
			} else m_working_btn->set_label("Working");
		}
	}

	void	on_toggle(void) {
		m_xts->toggle();

		if (m_xts->m_currently_working) {
			m_working_btn->set_label("Working");
			Gdk::RGBA	clr("red");
			m_working_btn->override_background_color(clr);
		} else {
			m_working_btn->set_label("Press to Start Working");
			Gdk::RGBA	clr("green");
			m_working_btn->override_background_color(clr);
		}

		tick();
	}

	void	on_show(void) {
		set_values();
	}

	bool	on_close(GdkEventAny *e) {
		close();
		return true;
	}

	bool	close(void) {
		// If we are currently working, then we should log our last
		// working interval before closing.  Therefore, let's toggle
		// the button and log off.
		if (m_xts->m_currently_working)
			m_xts->toggle();
		gtk_main_quit();
		return true;
	}

	void	load(const char *fname) {
		m_xts->load(fname);
		set_values();
	}
};

extern "C" {
	void	cb_on_toggle(GtkToggleButton *w, gpointer d);
	gboolean	cb_on_close(GtkWidget *w, gpointer d);
};

int	on_tick(APPDATA *ad) {
	if (ad)
		ad->tick();
	return 1;
}

void	cb_on_toggle(GtkToggleButton *w, gpointer d) {
	// APPDATA *ad = (APPDATA *)d;
	// ad->on_toggle();
}

gboolean	cb_on_close(GtkWidget *w, gpointer d) {
	// APPDATA *ad = (APPDATA *)d;
	// ad->on_close();

	return true;
}

// Catch a SIGTERM signal?
// to shutdown accounting in case of a system crash?
//
// sigaction(SIGTERM, act, &old);
//
APPDATA		*ad;
void	sig_handler(int v) {
	ad->m_terminate_now = true;
}

int main(int argc, char **argv) {
	Gtk::Main kit(argc, argv);
	Glib::RefPtr<Gtk::Builder>	builder;
	// Glib::Error		*error = NULL;

	if ((argc < 2)||(argv[1][0]=='-')) {
		fprintf(stderr, "USAGE: xtimesheet <timesheet.txt>\n"
"\twhere timesheet.txt is the name of an existing textfile.  This file \n"
"\tshould have in it, as a minimum, a line beginning with \'Project: \'\n"
"\tthat specifies the project name, and a second line beginning with \n"
"\t'Rate: ' that specifies an hourly rate for the project.\n");
		exit(-1);
	} else if ((strcasecmp(&argv[1][strlen(argv[1])-4], ".txt")==0)
			&&(access(argv[1], W_OK)!=0)) {
		fprintf(stderr, "ERROR: Cannot access %s\n", argv[1]);
		exit(-1);
	}

	struct	sigaction sigact;
	sigact.sa_handler = sig_handler;
	sigact.sa_flags  = 0;
	sigaction(SIGTERM, &sigact, NULL);

	try {

	ad = new APPDATA();
	ad->m_xts = new XTIMESHEET();

	/* Init GTK+ */
	gtk_init(&argc, &argv);

	/* Parse any remaining arguments */
	for(int argn=1; argn<argc; argn++) {
		ad->m_xts->load(argv[argn]);
	}

	/* Create new GtkBuilder object */
	/* Load UI from file.  If error occurs, report it and quit application.
	 */
	// if (access("timesheet.glade", R_OK)==0)
		// builder = Gtk::Builder::create_from_file("timesheet.glade");
	// else
		builder = Gtk::Builder::create_from_string(gladef);

	/* Get widget pointers from UI */
	builder->get_widget("xts_main",    ad->m_xts_main);
	builder->get_widget("taskname",    ad->m_taskname);
	builder->get_widget("taskfile",    ad->m_taskfile);
	builder->get_widget("totalcost",   ad->m_totalcost);
	builder->get_widget("today",       ad->m_today);
	builder->get_widget("sumhours",    ad->m_sumhours);
	builder->get_widget("hourlyrate",  ad->m_hourlyrate);
	builder->get_widget("working_btn", ad->m_working_btn);
	builder->get_widget("daily_prg",   ad->m_daily_prg);
	builder->get_widget("splash",      ad->m_splash);

	/* Connect signals */
	// gtk_builder_connect_signals( builder, ad );
	ad->m_working_btn->signal_toggled().connect(sigc::mem_fun(ad, &APPDATA::on_toggle));
	ad->m_xts_main->signal_delete_event().connect(sigc::mem_fun(ad, &APPDATA::on_close));
	ad->m_xts_main->signal_show().connect(sigc::mem_fun(ad, &APPDATA::on_show));

	// Set the image value
	ad->m_splash->set(Gdk::Pixbuf::create_from_inline(sizeof(sm_splash), sm_splash));
	/* Destroy builder, since we don't need it anymore */
	// g_object_unref( G_OBJECT( builder ));
	// delete builder;

	/* Show window.  All other widgets are automatically shown by
	 * GtkBuilder
	 */
	ad->m_xts_main->show();

	ad->set_values();

	// Tick every three seconds (it was thirty, but we added a flag for
	// catching a signal.  This allows signals to be caught with a three
	// second delay or less)
	g_timeout_add(3000, (GSourceFunc)on_tick, ad);

	/* Start main loop */
	gtk_main();

	} catch (Glib::Error *e) {
		printf("Msg: %s\n", e->what().c_str());
	}

	return (0);
}


