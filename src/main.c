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
	enum language lng;
	struct disp d;
	int noid;
	char **oid;
	int nid;
} arg_t;

void usage(void) {
	fprintf(stderr,
		"\n"
		"pla -i <filename> -o <filename> [-f (eps|png|svg|pdf|csv|tex)]\n"
		"    [-s yyyymmdd] [-e yyyymmdd] [-id task_id] [-oid task_id]\n"
		"    [-res] [-did] [-m <margin>] -l (en|fr)\n"
		"\n"
		"    -res: display resources\n"
		"    -did: display id\n"
		"    -m  : margin size. Default 150\n"
		"\n"
	);
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
	args->lng = french;
	args->id = NULL;
	args->d.display_res = 0;
	args->d.display_id = 0;
	args->d.margin = 150.0f;
	args->noid = 0;
	args->oid = NULL;
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
		/* language */
		else if (strcmp(argv[i], "-l") == 0) {
			i++;
			if (i == argc) {
				fprintf(stderr, "\nargument -l expect format (en or fr)\n");
				usage();
				exit(1);
			}
		  if (strcasecmp(argv[i], "en") == 0)
				args->lng = english;
			else if (strcasecmp(argv[i], "fr") == 0)
				args->lng = french;
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

		/* task id */
		else if (strcmp(argv[i], "-oid") == 0) {
			i++;
			if (i == argc) {
				fprintf(stderr, "\nargument -oid expect id\n");
				usage();
				exit(1);
			}

			/* add oid */
			args->oid = realloc(args->oid, (args->noid+1)*sizeof(char *));
			args->oid[args->noid] = argv[i];
			args->noid++;
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

int main(int argc, char *argv[])
{
	char *p;
	struct list_head base = LIST_HEAD_INIT(base);
	struct list_head res = LIST_HEAD_INIT(res);
	struct task *t;
	struct task *tt;
	struct res *r;
	struct res *rr;
	time_t max;
	int i;
	int ok;
	int first_id = 0;

	arg_t* args = get_arguments(argc, argv);

	/* checks */
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

	/* loda planning */
	pla_load(&base, &res, args->in_file);

	/* oid */
	if (args->noid > 0) {

		/* check childs */
		list_for_each_entry(t, &base, c)
			for (i=0; i<args->noid; i++)
				if (strcpy(t->id, args->oid[i])==0)
					list_for_each_entry(tt, &t->childs, _child)
						oid_add(&args->oid, &args->noid, tt->id);

		/* delete task */
		list_for_each_entry_safe(t, tt, &base, c) {
			ok = 0;
			for (i=0; i<args->noid; i++)
				if (t->id == args->oid[i])
					ok = 1;
			if (ok == 1)
				continue;
			list_del(&t->c);
		}

		/* remove res */
		list_for_each_entry_safe(r, rr, &res, c) {
			ok = 0;
			list_for_each_entry(t, &base, c)
				for (i=0; i<t->nres; i++)
					if (t->res[i] == r)
						ok = 1;
			if (ok == 0)
				list_del(&r->c);
		}
	}

	/* Find the smallest date */
	args->d.start = -1;
	list_for_each_entry(t, &base, c) {

		if (t->start == 0)
			continue;

		if (args->d.start == -1) {
			args->d.start = t->start;
			continue;
		}

		if (t->start < args->d.start) {
			args->d.start = t->start;
			continue;
		}
	}

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
		p = strrchr(args->out_file, '.');
		if (p == NULL)
			args->mode = 1;
		else if (strcasecmp(p, ".png") == 0)
			args->mode = 1;
		else if (strcasecmp(p, ".eps") == 0)
			args->mode = 2;
		else if (strcasecmp(p, ".svg") == 0)
			args->mode = 3;
		else if (strcasecmp(p, ".pdf") == 0)
			args->mode = 4;
		else if (strcasecmp(p, ".csv") == 0)
			args->mode = 5;
		else if (strcasecmp(p, ".tex") == 0)
			args->mode = 6;
		else {
			fprintf(stderr, "Unknown extension file, output format expected. see -f\n");
			usage();
			exit(1);
		}
	}

	switch (args->mode) {
	case 1:
	case 2:
	case 3:
	case 4:
		pla_draw(args->mode, args->out_file, &args->d, args->lng);
		break;

	case 5:
	case 6:
		render_text(args->mode, args->out_file, &args->d);
		break;
	}
//	pla_store(&base, "out.pla");
	return 0;
}