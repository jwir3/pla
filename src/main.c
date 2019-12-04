/*
 * Copyright 2011-2015 Thierry FOURNIER
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License.
 *
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "pla.h"
#include "load.h"
#include "render.h"
#include "render_txt.h"
#include "utils.h"

typedef struct arg_t {
	char** id;
	const char* in_file;
	const char* out_file;
	int mode;
	time_t start;
	time_t end;
	struct disp d;
	int noid;
	// char **oid;
	int nid;
} arg_t;

void usage(void) {
	fprintf(stderr,
		"\n"
		"pla -i <filename> -o <filename> [-f (eps|png|svg|pdf|csv|tex)]\n"
		"    [-s yyyymmdd] [-e yyyymmdd] [-id task_id]\n"
		"    [-res] [-did] [-m <margin>]\n"
		"\n"
		"    -res: display resources\n"
		"    -did: display id\n"
		"    -m  : margin size. Default 150\n"
		"\n"
		"You can use the LANG environment variable to control languages\n"
		"for the purposes of month names:\n\n"
		"LANG=fr_FR pla -i <filename> -o <filename>\n"
	);
}

int auto_select_mode(arg_t* args) {
	char* p;
	p = strrchr(args->out_file, '.');
	if (p == NULL) {
		return 1;
	} else if (strcasecmp(p, ".png") == 0) {
		return 1;
	} else if (strcasecmp(p, ".eps") == 0) {
		return 2;
	} else if (strcasecmp(p, ".svg") == 0) {
		return 3;
	} else if (strcasecmp(p, ".pdf") == 0) {
		return 4;
	} else if (strcasecmp(p, ".csv") == 0) {
		return 5;
	} else if (strcasecmp(p, ".tex") == 0) {
		return 6;
	}

	fprintf(stderr, "Unknown file extension, output format expected. See -f\n");
	usage();
	exit(1);
}

arg_t* get_arguments(int argc, char* argv[]) {
	int i;
	char *err;
	arg_t* args = malloc(sizeof(arg_t));
	args->in_file = NULL;
	args->out_file = NULL;
	args->mode = 0;
	args->start = -1;
	args->end = -1;
	args->id = NULL;
	args->d.display_res = 0;
	args->d.display_id = 0;
	args->d.margin = 150.0f;
	args->noid = 0;
	// args->oid = NULL;
	args->nid = 0;

	/* argument parser */
	for (i=1; i<argc; i++) {

		/* input file */
		if (strcmp(argv[i], "-i") == 0) {
			i++;
			if (i == argc) {
				fprintf(stderr, "\nargument -i expect filename\n");
				usage();
				exit(1);
			}
			args->in_file = argv[i];
		}

		/* output file */
		else if (strcmp(argv[i], "-o") == 0) {
			i++;
			if (i == argc) {
				fprintf(stderr, "\nargument -o expect filename or -\n");
				usage();
				exit(1);
			}
			args->out_file = argv[i];
		}

		/* start date for render */
		else if (strcmp(argv[i], "-s") == 0) {
			i++;
			if (i == argc) {
				fprintf(stderr, "\nargument -s expect start date in format yyymmdd\n");
				usage();
				exit(1);
			}
			args->start = convert_yyymmdd(argv[i]);
			if (args->start == -1) {
				fprintf(stderr, "\nargument -s: invalid date\n");
				usage();
				exit(1);
			}
		}

		/* end date for render */
		else if (strcmp(argv[i], "-e") == 0) {
			i++;
			if (i == argc) {
				fprintf(stderr, "\nargument -e expect end date in format yyymmdd\n");
				usage();
				exit(1);
			}
			args->end = convert_yyymmdd(argv[i]);
			if (args->end == -1) {
				fprintf(stderr, "\nargument -e: invalid date\n");
				usage();
				exit(1);
			}
			args->end += 86400;
		}

		/* format */
		else if (strcmp(argv[i], "-f") == 0) {
			i++;
			if (i == argc) {
				fprintf(stderr, "\nargument -f expect format (eps, png, pdf, svg, csv or tex)\n");
				usage();
				exit(1);
			}

		  if (strcasecmp(argv[i], "png") == 0)
				args->mode = 1;
			else if (strcasecmp(argv[i], "eps") == 0)
				args->mode = 2;
			else if (strcasecmp(argv[i], "svg") == 0)
				args->mode = 3;
			else if (strcasecmp(argv[i], "pdf") == 0)
				args->mode = 4;
			else if (strcasecmp(argv[i], "csv") == 0)
				args->mode = 5;
			else if (strcasecmp(argv[i], "tex") == 0)
				args->mode = 5;
		}

		/* task id */
		else if (strcmp(argv[i], "-id") == 0) {
			i++;
			if (i == argc) {
				fprintf(stderr, "\nargument -id expect id\n");
				usage();
				exit(1);
			}

			/* add id */
			args->id = realloc(args->id, (args->nid+1)*sizeof(char *));
			args->id[args->nid] = argv[i];
			args->nid++;
		}

		/* margin size */
		else if (strcmp(argv[i], "-m") == 0) {
			i++;
			if (i == argc) {
				fprintf(stderr, "\nargument -m expect margin\n");
				usage();
				exit(1);
			}
			args->d.margin = strtod(argv[i], &err);
			if (*err != '\0') {
				fprintf(stderr, "\nargument -m is not numeric (%s)\n", argv[i]);
				usage();
				exit(1);
			}
		}

		/* display resource */
		else if (strcmp(argv[i], "-res") == 0) {
			args->d.display_res = 1;
		}

		/* display id */
		else if (strcmp(argv[i], "-did") == 0) {
			args->d.display_id = 1;
		}

		/* help */
		else if (strcmp(argv[i], "-h") == 0) {
			usage();
			exit(0);
		}

		/* help */
		else {
			fprintf(stderr, "\nunknown argument\n");
			usage();
			exit(0);
		}
	}

	return args;
}

void check_input_arguments(arg_t* args) {
	int first_id = 0;

	if (args->in_file == NULL) {
		fprintf(stderr, "\nsource file expected\n");
		usage();
		exit(1);
	}

	if (args->out_file == NULL && first_id == 0) {
		fprintf(stderr, "output file expected\n");
		usage();
		exit(1);
	}
}

int find_smallest_date(struct task* t, struct list_head* base) {
	time_t start = -1;

	list_for_each_entry(t, base, c) {
		if (t->start == 0)
			continue;

		if (start == -1) {
			start = t->start;
			continue;
		}

		if (t->start < start) {
			start = t->start;
			continue;
		}
	}

	return start;
}

int main(int argc, char *argv[])
{
	struct list_head base = LIST_HEAD_INIT(base);
	struct list_head res = LIST_HEAD_INIT(res);
	struct task *t = NULL;
	time_t max;
	int i;

	arg_t* args = get_arguments(argc, argv);

	/* checks */
	check_input_arguments(args);

	/* load the data structures into memory */
	pla_load(&base, &res, args->in_file);

	/* Find the smallest date */
	args->d.start = find_smallest_date(t, &base);

	/* Find the largest date */
	max = 0;
	list_for_each_entry(t, &base, c) {

		if (t->start == 0)
			continue;

		if (t->start + t->duration > max)
			max = t->start + t->duration;
	}

	args->d.duration = max - args->d.start;
	args->d.base = &base;
	args->d.res = &res;

	/* if id s known */
	if (args->nid > 0) {
		args->start = -1;
		args->end = -1;

		for (i=0; i<args->nid; i++) {

			t = pla_task_get_by_id(&base, args->id[i]);
			if (t == NULL) {
				fprintf(stderr, "\nunknown id\n");
				usage();
				exit(1);
			}
			pla_task_update_date(t);

			if (args->start == -1)
				args->start = t->start;

			else if (args->start > t->start)
				args->start = t->start;

			if (args->end == -1)
				args->end = t->start + t->duration;

			else if (args->end < t->start + t->duration)
				args->end = t->start + t->duration;
		}
	}

	/* if start is set */
	if (args->start != -1)
		args->d.start = args->start;

	/* if end is set */
	if (args->end != -1)
		args->d.duration = args->end - args->d.start;

	/* auto select mode if needed */
	if (args->mode == 0) {
		args->mode = auto_select_mode(args);
	}

	switch (args->mode) {
	case 1:
	case 2:
	case 3:
	case 4:
		pla_draw(args->mode, args->out_file, &args->d);
		break;

	case 5:
	case 6:
		render_text(args->mode, args->out_file, &args->d);
		break;
	}
//	pla_store(&base, "out.pla");
	return 0;
}
