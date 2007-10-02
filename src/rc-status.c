/*
   rc-status
   Display the status of the services in runlevels
   Copyright 2007 Gentoo Foundation
   Released under the GPLv2
   */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "builtins.h"
#include "einfo.h"
#include "rc.h"
#include "rc-misc.h"
#include "strlist.h"

#define APPLET "rc-status"

static void print_level (char *level)
{
	printf ("Runlevel: %s%s%s\n",
			ecolor (ECOLOR_HILITE),
			level,
			ecolor (ECOLOR_NORMAL));
}

static void print_service (char *service)
{
	char status[10];
	int cols =  printf (" %s\n", service);
	rc_service_state_t state = rc_service_state (service);
	einfo_color_t color = ECOLOR_BAD;

	if (state & RC_SERVICE_STOPPING)
		snprintf (status, sizeof (status), "stopping ");
	else if (state & RC_SERVICE_STARTING) {
		snprintf (status, sizeof (status), "starting ");
		color = ECOLOR_WARN;
	} else if (state & RC_SERVICE_INACTIVE) {
		snprintf (status, sizeof (status), "inactive ");
		color = ECOLOR_WARN;
	} else if (state & RC_SERVICE_STARTED) {
		if (geteuid () == 0 && rc_service_daemons_crashed (service))
			snprintf (status, sizeof (status), " crashed ");
		else {
			snprintf (status, sizeof (status), " started ");
			color = ECOLOR_GOOD;
		}
	} else if (state & RC_SERVICE_SCHEDULED) {
		snprintf (status, sizeof (status), "scheduled");
		color = ECOLOR_WARN;
	} else
		snprintf (status, sizeof (status), " stopped ");
	ebracket (cols, color, status);
}

#include "_usage.h"
#define extraopts "[runlevel1] [runlevel2] ..."
#define getoptstring "alsu" getoptstring_COMMON
static const struct option longopts[] = {
	{"all",         0, NULL, 'a'},
	{"list",        0, NULL, 'l'},
	{"servicelist", 0, NULL, 's'},
	{"unused",      0, NULL, 'u'},
	longopts_COMMON
	{NULL,          0, NULL, 0}
};
static const char * const longopts_help[] = {
	"Show services from all run levels",
	"Show list of run levels",
	"Show service list",
	"Show services not assigned to any run level",
	longopts_help_COMMON
};
#include "_usage.c"

int rc_status (int argc, char **argv)
{
	char **levels = NULL;
	char **services = NULL;
	char *level;
	char *service;
	int opt;
	int i;
	int j;

	while ((opt = getopt_long(argc, argv, getoptstring, longopts, (int *) 0)) != -1)
		switch (opt) {
			case 'a':
				levels = rc_get_runlevels ();
				break;
			case 'l':
				levels = rc_get_runlevels ();
				STRLIST_FOREACH (levels, level, i)
					printf ("%s\n", level);
				rc_strlist_free (levels);
				exit (EXIT_SUCCESS);
			case 's':
				services = rc_services_in_runlevel (NULL);
				STRLIST_FOREACH (services, service, i)
					print_service (service);
				rc_strlist_free (services);
				exit (EXIT_SUCCESS);
			case 'u':
				services = rc_services_in_runlevel (NULL);
				levels = rc_get_runlevels ();
				STRLIST_FOREACH (services, service, i) {
					bool found = false;
					STRLIST_FOREACH (levels, level, j)
						if (rc_service_in_runlevel (service, level)) {
							found = true;
							break;
						}
					if (! found)
						print_service (service);
				}
				rc_strlist_free (levels);
				rc_strlist_free (services);
				exit (EXIT_SUCCESS);

			case_RC_COMMON_GETOPT
		}

	while (optind < argc)
		rc_strlist_add (&levels, argv[optind++]);

	if (! levels) {
		level = rc_runlevel_get ();
		rc_strlist_add (&levels, level);
		free (level);
	}

	STRLIST_FOREACH (levels, level, i) {
		print_level (level);
		services = rc_services_in_runlevel (level);
		STRLIST_FOREACH (services, service, j)
			print_service (service);
		rc_strlist_free (services);
	}

	rc_strlist_free (levels);

	return (EXIT_SUCCESS);
}
