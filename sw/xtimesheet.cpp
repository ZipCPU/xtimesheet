////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	xtimesheet.cpp
// {{{
// Project:	Xtimesheet, a very simple text-based timesheet tracking program
//
// Purpose:	This is the main program for a GUI managing automatic
//		updates and changes to my hourly timesheet files.
//
// Creator:	Dan Gisselquist, Ph.D.
//		Gisselquist Technology, LLC
//
////////////////////////////////////////////////////////////////////////////////
// }}}
// Copyright (C) 2017-2022, Gisselquist Technology, LLC
// {{{
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
// }}}
// License:	GPL, v3, as defined and found on www.gnu.org,
// {{{
//		http://www.gnu.org/licenses/gpl.html
//
////////////////////////////////////////////////////////////////////////////////
//
// }}}
__attribute__((unused))
static const char *cpyright = "(C) 2022 Gisselquist Technology, LLC: " __FILE__;

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <assert.h>
#include <signal.h>

#include <string>
#include <unordered_map>
#include <iostream>

#include <gtk/gtk.h>
#include <gtkmm.h>

#include "gladef.h"
#include "sm_splash.cpp"
#include "timecard.h"

extern long	timezone; // seconds west of UTC

typedef	std::string		STRING;
typedef	std::unordered_map<STRING,STRING> TASKTBL;
typedef	std::pair<STRING,STRING> TBLVALUE;
typedef	TASKTBL::iterator	TBLINDEX;

TASKTBL	task_tbl;

// #define	DBGPRINTF	printf
#define	DBGPRINTF	null
static	void	null(...) {}

void	tbl_register_fname(const char *choice, const char *fname) {
	// {{{
	TBLINDEX	last;
	STRING		s_choice(choice), s_fname(fname);

	last = task_tbl.find(s_choice);
	if (last == task_tbl.end()) {
		task_tbl.insert(TBLVALUE(s_choice, s_fname));
		return;
	}

	if (last->second.compare(fname) != 0)
		last->second = s_fname;
}
// }}}

const char *tbl_lookup_fname(const char *choice) {
	// {{{
	STRING	ch;
	TBLINDEX	idx;

	idx = task_tbl.find(STRING(choice));
	if (idx == task_tbl.end())
		return NULL;
	return idx->second.c_str();
}
// }}}

class	XTIMESHEET : public TIMECARD {
// {{{
public:
	time_t		m_last_start, m_today;
	bool		m_currently_working;
	char		*m_fname, *m_name;
	unsigned	m_sumunits, m_daily_s;
	double		m_hourly_rate;

	// XTIMESHEET
	// {{{
	XTIMESHEET(void) {
		m_last_start = 0;
		m_currently_working = false;
		m_today = get_midnight(time(NULL));
		m_fname = NULL;
		m_name = NULL;
		m_sumunits = m_daily_s = 0;
		m_hourly_rate = 225.0;
	}
	// }}}

	// load
	// {{{
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
	// }}}

	// get_float_char -- will be either '.' or ',', depending on locale
	// {{{
	char get_float_char() {
		char buffer [4];
		
		sprintf (buffer, "%.1f", 0.0f);
		return buffer[1];
		// return retval;
	}
	// }}}

	// replace_char
	// {{{
	void replace_char(char *line) {
		char * t; // first copy the pointer to not change the original
		int index = 0;

		if (line != NULL) {
			for (t = line; *t != '\0'; t++) {
				if (*t == '.') {
					*t = get_float_char();
					break;
				}
				index++;
			}
		}
	}
	// }}}

	// reload
	// {{{
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
				char *rate = &line[5];
				replace_char(rate);
				m_hourly_rate = atof(rate);
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
				tbl_register_fname(m_name, m_fname);
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
	// }}}

	// toggle
	// {{{
	void	toggle(void) {
		if (!m_currently_working) {
			reload();

			m_currently_working = true;
			time(&m_last_start);
			TIMECARD::note_start(m_fname, m_last_start);
		} else {
			time_t	now = time(NULL);
			log(m_last_start, now);
			m_currently_working = false;

			m_daily_s += (now - m_last_start);
			m_last_start = 0;
		}
	}
	// }}}

	void	log(time_t t_start, time_t t_stop) {
		TIMECARD::log(m_fname, t_start, t_stop);
	}

};
// }}}

class	APPDATA {
// {{{
protected:
	typedef	Glib::RefPtr<Gtk::ListStore>	TSKMODEL;

	TSKMODEL	list_model(void) {
		// {{{
		return TSKMODEL::cast_dynamic( m_taskchoice->get_model());
	}
	// }}}

public:
	XTIMESHEET	*m_xts;
	Gtk::ApplicationWindow	*m_xts_main;
	Gtk::ComboBoxText	*m_taskchoice;
	Gtk::FileChooserButton	*m_taskfile;
	Gtk::Entry		*m_totalcost, *m_today,
				*m_sumhours, *m_hourlyrate;
	Gtk::ToggleButton	*m_working_btn;
	Gtk::ProgressBar	*m_daily_prg;
	Gtk::Image		*m_splash;
	bool			m_terminate_now;

	// APPDATA
	// {{{
	APPDATA(void) {
		m_xts        = NULL;
		m_xts_main   = NULL;
		m_taskchoice = NULL;
		m_taskfile   = NULL;
		m_totalcost  = NULL;
		m_today      = NULL;
		m_sumhours   = NULL;
		m_hourlyrate = NULL;
		m_working_btn= NULL;
		m_daily_prg  = NULL;
		m_terminate_now = false;
	}
	// }}}

	// set_values
	// {{{
	void	set_values(void) {
		char	buf[128];

		DBGPRINTF("SET-VALUES\n");
		m_taskfile->set_title("Select a timecard");
		if (m_xts && m_xts->m_fname && m_xts->m_fname[0]) {
			TSKMODEL	model;
			Gtk::TreeModel::iterator p, here;

			DBGPRINTF("\tSetting filename to %s\n", m_xts->m_fname);
			m_taskfile->set_filename(m_xts->m_fname);

			model = list_model();
			p = model->children().begin();
			here = m_taskchoice->get_active();

			p = model->children().begin();

			if (!model->iter_is_valid(p)) {
				DBGPRINTF("\tADDING.1 \"%s\"\n", m_xts->m_name);
				m_taskchoice->prepend(strdup(m_xts->m_name));
				m_taskchoice->set_active(0);
			} else if (model->iter_is_valid(here) && here == p
				&& m_xts && m_xts->m_name && m_xts->m_name[0]
				&& m_taskchoice->get_active_text().compare(
						m_xts->m_name)==0) {
				// Already active, no action required
				DBGPRINTF("\tADDING.2: No action\n");
				// m_taskchoice->set_active(0);
			} else {
				bool	found = false;
				// Our task isn't active yet

				// p = model->children().begin();
				for(; model->iter_is_valid(p) && p; p++) {
					Glib::ustring	this_text;
					p->get_value(0, this_text);
					DBGPRINTF("\tADDING.3: this_text = %s\n", this_text.c_str());
					if (this_text == m_xts->m_name) {
						if (found) {
							DBGPRINTF("\tADDING.3: Removing \"%s\"\n", this_text.c_str());
							p = model->erase(p);
						} else {
							m_taskchoice->set_active(p);
							found = true;
						}
					}
				}

				if (!found) {
					DBGPRINTF("\tADDING.4 %s\n", m_xts->m_name);
					m_taskchoice->prepend(strdup(m_xts->m_name));
					m_taskchoice->set_active(0);
				}
			}

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
		} else
			m_taskfile->set_title("Select a timecard");

		tick();

		m_splash->set(Gdk::Pixbuf::create_from_inline(sizeof(sm_splash), sm_splash));
	}
	// }}}

	// tick -- count and record the time as it passes
	// {{{
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
	// }}}

	// on_select -- switch tasks
	// {{{
	void	on_select(void) {
		bool		working = m_xts->m_currently_working;
		const	char	*fname;
		Glib::ustring	taskname;

		DBGPRINTF("APP:ON-SELECT");
		taskname = m_taskchoice->get_active_text();
		DBGPRINTF(" -- %s", taskname.c_str());

		if (m_xts && m_xts->m_name && m_xts->m_name[0]
				&& taskname.compare(m_xts->m_name)==0) {
			// User selected the currently selected task
			return;
		}

		fname = tbl_lookup_fname(taskname.c_str());
		if (!fname || !fname[0]) {
			DBGPRINTF("ON-SELECT: No file found (yet)\n");
			if (list_model()->iter_is_valid(list_model()->children().begin()))
				m_taskchoice->set_active(0);
			return;
		} if (m_xts->m_fname && m_xts->m_fname[0]
				&& strcmp(fname, m_xts->m_fname)==0) {
			DBGPRINTF("Task already loaded and active\n");
			return;
		}

		if (working) {
			// Stop the time card, write a close task to it
			m_xts->toggle();
		}


		DBGPRINTF("Loading: %s\n", fname);
		load(fname);

		if (working) {
			// Restart, now under the new time card
			m_xts->toggle();
		}

		// Re-sort to make this item the "top" or #1 item
		TSKMODEL	model = list_model();
		Gtk::TreeModel::iterator p = m_taskchoice->get_active(),
				first = model->children().begin();
		if (model->iter_is_valid(p)
				&& model->iter_is_valid(first)
				&& (p != first)) {
			DBGPRINTF("... choice swap\n");
			model->iter_swap(first, p);
			// model->erase(p);
			// m_taskchoice->prepend(m_xts->m_name);
			m_taskchoice->set_active(0);
			
		}
	}
	// }}}

	// on_toggle -- start or stop working
	// {{{
	void	on_toggle(void) {
		DBGPRINTF("APP:ON-TOGGLE\n");
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
	// }}}

	// on_show
	// {{{
	void	on_show(void) {
		DBGPRINTF("ON-SHOW, set_values()\n");
		set_values();
	}
	// }}}

	// on_close
	// {{{
	bool	on_close(GdkEventAny *e) {
		close();
		return true;
	}
	// }}}

	// on_newfile
	// {{{
	void	on_newfile(void) {
		const char	 *new_fname;
		char		cbuf[64];
		const	char	PATTERN[] = "Project:";
		const unsigned	nP = strlen(PATTERN);
		FILE	*ftmp;
		Glib::ustring	aux, cvt;

		aux = m_taskfile->get_filename();
		aux = Glib::filename_to_utf8(aux);
		new_fname = aux.c_str();

		DBGPRINTF("NEW-FILE: %s\n", new_fname);
		if (0 != access(new_fname, W_OK)) {
			fprintf(stderr, "%s cannot be read\n", new_fname);
		} else if (NULL == (ftmp = fopen(new_fname, "r"))) {
			fprintf(stderr, "%s cannot be read\n", new_fname);
		} else if (nP != fread(cbuf, sizeof(char), nP, ftmp)) { 
			fclose(ftmp);
			fprintf(stderr, "%s is not a valid time card\n", new_fname);
		} else if (0 != strncasecmp(cbuf, PATTERN, strlen(PATTERN))) {
			fclose(ftmp);
			fprintf(stderr, "%s is not a valid time card\n", new_fname);
		} else {
			fclose(ftmp);
			load(new_fname);
		}
	}
	// }}}

	// close
	// {{{
	bool	close(void) {
		// If we are currently working, then we should log our last
		// working interval before closing.  Therefore, let's toggle
		// the button and log off.
		if (m_xts->m_currently_working)
			m_xts->toggle();
		gtk_main_quit();
		return true;
	}
	// }}}

	void	read_config(void) {
		// {{{
		char	cfg_file[PATH_MAX+2], *home, cfg_task[PATH_MAX],
			prefix[PATH_MAX], *tsk_name;
		FILE	*fcfg = NULL, *ftsk;

// printf("READ-CONFIG\n");
		home = getenv("HOME");
		if (!home)
			return;

// printf("READ-CONFIG: home = %s\n", home);

		strcpy(cfg_file, home);
		strcat(cfg_file, "/.xtimesheet");
		fcfg = fopen(cfg_file, "r");
		if (!fcfg)
			return;

// printf("READ-CONFIG: cfg_file = %s\n", cfg_file);

		while(fgets(cfg_task, sizeof(cfg_task), fcfg)) {
			char	*tsk_file = cfg_task, *endp;

			endp = &cfg_task[strlen(cfg_task)-1];
			while(endp > cfg_task && isspace(*endp))
				*endp-- = '\0';
			while(*tsk_file && isspace(*tsk_file))
				tsk_file++;
			if (tsk_file[0] == '#' || tsk_file[0] == '*')
				continue;
			if (tsk_file[0] == '/' && tsk_file[1] == '/')
				continue;

			ftsk = NULL;
			if (0 != access(tsk_file, R_OK))
				continue;
			if (NULL == (ftsk=fopen(tsk_file,"r")))
				continue;

			if ((fgets(prefix,sizeof(prefix), ftsk))
				&&(0==strncasecmp(prefix, "project:", 8))) {
				tsk_name = prefix+8;
				while(*tsk_name && isspace(*tsk_name))
					tsk_name++;
				endp = &tsk_name[strlen(tsk_name)-1];
				while(tsk_name < endp && isspace(*endp))
					*endp-- = '\0';
				if (endp > tsk_name) {
					tbl_register_fname(tsk_name, cfg_task);
					m_taskchoice->append(strdup(tsk_name));
				}
			} fclose(ftsk);
		}

		fclose(fcfg);
	}
	// }}}

	// load
	// {{{
	void	load(const char *fname) {
		DBGPRINTF("LOAD()::CALLING LOAD: %s\n", fname);
		m_xts->load(fname);
		DBGPRINTF("LOAD()::CALLING LOAD::SET-VALUES\n");
		set_values();
	}
	// }}}
};
// }}}

extern "C" {
	void	cb_on_toggle(GtkToggleButton *w, gpointer d);
	gboolean	cb_on_close(GtkWidget *w, gpointer d);
};

// on_tick
// {{{
int	on_tick(APPDATA *ad) {
	if (ad)
		ad->tick();
	return 1;
}
// }}}

// cb_on_toggle
// {{{
void	cb_on_toggle(GtkToggleButton *w, gpointer d) {
stderr, printf("ON-TOGGLE\n");
	// APPDATA *ad = (APPDATA *)d;
	// ad->on_toggle();
}
// }}}

// cb_on_close
// {{{
gboolean	cb_on_close(GtkWidget *w, gpointer d) {
	// APPDATA *ad = (APPDATA *)d;
	// ad->on_close();

	return true;
}
// }}}

// Catch a SIGTERM signal?
// to shutdown accounting in case of a system crash?
//
// sigaction(SIGTERM, act, &old);
//
APPDATA		*ad;
void	sig_handler(int v) {
	ad->m_terminate_now = true;
}

// usage
// {{{
void usage(void) {
	fprintf(stderr, "USAGE:  xtimesheet <timesheet.txt>\n"
"\tor\n"
"\txtimesheet -p project_name -r rate\n\n"
"\twhere:\n"
"\t\t- timesheet.txt is the name of an existing textfile.\n "
"\t\t\tThis file should have in it, as a minimum, a line beginning with 'Project: '\n"
"\t\t\tthat specifies the project name, and a second line beginning with\n"
"\t\t\t'Rate: ' that specifies an hourly rate for the project.\n"
"\t\t- second command creates timesheet.txt file automatically\n"
"\t\t\t-p project_name, where project name can be multiple words separated by space, e.g. \"test project\"\n"
"\t\t\t-r rate, in decimal format, e.g. 33.3.\n"
	);
}
// }}}

// get_file_extension
// {{{
char* get_file_extension(char* file_name) {
  //store the position of last '.' in the file name
  char *ptr = strrchr(file_name, '.');
  if (ptr != NULL) {
	return ++ptr;
  }
  return NULL;
}
// }}}

// project_to_filename
// {{{
void project_to_filename(char *file_name, char *proj, size_t len) {
	// s = source index, d = destination index
	size_t d=0;
	for(size_t s=0; proj[s] != '\0' && d < len - 1; s++) {
		if (isspace(proj[s]))
			file_name[d++] = '_';
		else if (isgraph(proj[s]))
			file_name[d++] = tolower(proj[s]);
	} 
	file_name[d] = '\0';
}
// }}}

// main
// {{{
int main(int argc, char **argv) {

	Gtk::Main kit(argc, argv);
	Glib::RefPtr<Gtk::Builder>	builder;
	// Glib::Error		*error = NULL;
	char file_name[PATH_MAX];
	file_name[0] = '\0';

	if (argc == 5) { //new project
		// {{{
		if (strcasecmp(argv[1], "-p")==0 && strcasecmp(argv[3], "-r")==0) {
			char *project_name = strdup(argv[2]);
			if (strlen(project_name) > 250) {
				fprintf(stderr, "WARNING: truncating filename to 250 characters.");
			}

			project_to_filename(file_name, project_name, 250);
			strcat(file_name, ".txt");
			if (access(file_name, R_OK)==0) { //check if file exists, throw error if true
				fprintf(stderr, "ERROR: project file with name %s already exists\n\n", file_name);
				usage();
				exit(-1);
			}
			//validate rate
			double r = atof(argv[4]);
			if (r == 0.0) {
				fprintf(stderr, "ERROR: rate %s is not valid\n", argv[4]);
				usage();
				exit(-1);
			}
			if (access(file_name, F_OK) == 0) { 
				fprintf(stderr, "ERR: File %s already exists!  Cowardly refusing to overwrite it.\n", file_name);
				exit(EXIT_FAILURE);
			}
			FILE *new_file = fopen(file_name, "w");
			if (new_file == NULL) {
				fprintf(stderr, "ERROR: unable to open file %s\n", file_name);
				exit(-1);
			}
			fprintf(new_file, "Project: %s\n", argv[2]);
			fprintf(new_file, "Rate: %s\n", argv[4]);
			fclose(new_file);
		} else {
			usage();
			exit(-1);
		}
		// }}}
	}

	if ((argc < 2)||(strcasecmp(argv[1], "--help")==0)) {
		usage();
		exit(-1);
	} 
	
	if (strlen(file_name) == 0) {
		strcpy(file_name, argv[1]);
	}
	char* ext = get_file_extension(file_name);
	if (ext != NULL && strcasecmp(ext, "txt") != 0) {
		fprintf(stderr, "ERROR: file name should end with .txt\n");
		exit(-1);
	}
	if (access(file_name, W_OK)!=0) {
		fprintf(stderr, "ERROR: Cannot access %s\n\n", file_name);
		usage();
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

	if (file_name) {
		char	full_path[PATH_MAX];
		realpath(file_name, full_path);
		strcpy(file_name, full_path);
	}
	ad->m_xts->load(file_name);

	/* Create new GtkBuilder object */
	/* Load UI from file.  If error occurs, report it and quit application.
	 */
	// if (access("timesheet.glade", R_OK)==0)
		// builder = Gtk::Builder::create_from_file("timesheet.glade");
	// else
		builder = Gtk::Builder::create_from_string(gladef);

	/* Get widget pointers from UI */
	builder->get_widget("xts_main",    ad->m_xts_main);
	builder->get_widget("taskchoice",  ad->m_taskchoice);
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
	ad->m_taskchoice->signal_changed().connect(sigc::mem_fun(ad, &APPDATA::on_select));
	ad->m_taskfile->signal_file_set().connect(sigc::mem_fun(ad, &APPDATA::on_newfile));

	// Set the image value
	ad->m_splash->set(Gdk::Pixbuf::create_from_inline(sizeof(sm_splash), sm_splash));

	// Clear our task choices
	ad->m_taskchoice->remove_all();

	// Add all task choices found in the ~/.xtimesheet file
	ad->read_config();

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
// }}}
