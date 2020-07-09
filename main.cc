#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <sys/time.h>
#include <FL/Fl.h>
#include <FL/Fl_Double_window.h>
#include <FL/Fl_Output.h>

enum
{
	Minute		= 1,
	Hour		= 60,
	Day		= 24*60,
};

static long		parsetime(char *s);
static void		fmtdisp(char *dst, size_t dstsz, long t, long endtime);
static void		fmttime(char *dst, size_t dstsz, long t);
static void		fmtcountdown(char *dst, size_t dstsz, long t);
static long		timenow(void);
static void		winclosed(Fl_Widget *wg, void *args);
static void		trigger(void);
static void		shutup(void);

static Fl_Window *win;

int
main(int argc, char **argv)
{
	if(argc != 2){
		printf("usage: alarm hh:mm\n");
		return 1;
	}
	
	if(0)
		SetThreadExecutionState(ES_DISPLAY_REQUIRED);
	
	auto now = timenow();
	auto wantend = parsetime(argv[1]);
	auto endtime = (now/Day)*Day + wantend;
	if(endtime%Day < now%Day)
		endtime += Day;

	win = new Fl_Double_Window(181, 33, "alarm");
	win->position((Fl::w() - win->w())/2, (Fl::h() - win->h())/2);
	win->callback(winclosed);

	auto disp = new Fl_Output(0, 0, win->w(), win->h());
	disp->color(fl_rgb_color(0, 0, 0));
	disp->textcolor(fl_rgb_color(200, 200, 200));
	disp->textsize(24);

	win->end();
	
	win->show();
	
	long triggered = 0;
	for(;;){
		long t = timenow();

		char buf[32];
		fmtdisp(buf, sizeof buf, t, endtime);
		disp->value(buf);

		fmtcountdown(buf, sizeof buf, endtime - t);
		win->label(buf);

		if(t >= endtime && !triggered){
			triggered = t;
			trigger();
		} else if(t > triggered + 10*Minute)
			shutup();
		
		Sleep(200);
		Fl::check();
	}
	return 0;
}

static long
parsetime(char *s)
{
	long h = 0, m = 0;
	if(sscanf(s, "%ld:%ld", &h, &m) != 2)
		sscanf(s, "%ld", &h);
	return h*Hour + m*Minute;
}

static void
fmtdisp(char *dst, size_t dstsz, long t, long endtime)
{
	char es[16], cs[16];
	fmttime(es, sizeof es, endtime);
	fmtcountdown(cs, sizeof cs, endtime - t);
	snprintf(dst, dstsz, "%s (%s)", es, cs);
}

static void
fmttime(char *dst, size_t dstsz, long t)
{
	auto h = (t/Hour) % 24;
	auto m = (t/Minute) % 60;
	snprintf(dst, dstsz, "%02ld:%02ld", h, m);
}

static void
fmtcountdown(char *dst, size_t dstsz, long t)
{
	const char *prefix = "T-";
	if(t < 0){
		prefix = "T+";
		t = -t;
	}
	char buf[16];
	fmttime(buf, sizeof buf, t);
	snprintf(dst, dstsz, "%s%s", prefix, buf);
}

static long
timenow()
{
	time_t t;
	time(&t);
	struct tm *lt = localtime(&t);
	return (long)lt->tm_yday*Day + (long)lt->tm_hour*Hour + (long)lt->tm_min*Minute;
}

static void
winclosed(Fl_Widget *wg, void *args)
{
	exit(0);
}

static void
trigger()
{
	PlaySound("alarm.wav", NULL, SND_LOOP|SND_ASYNC);
	win->iconize();
	win->show();
}

static void
shutup()
{
	PlaySound(NULL, NULL, 0);
}
