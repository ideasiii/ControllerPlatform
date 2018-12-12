/*  ---------------------------------------------------------------  */
/*           The HMM-Based Speech Synthesis System (HTS)             */
/*                       HTS Working Group                           */
/*                                                                   */
/*                  Department of Computer Science                   */
/*                  Nagoya Institute of Technology                   */
/*                               and                                 */
/*   Interdisciplinary Graduate School of Science and Engineering    */
/*                  Tokyo Institute of Technology                    */
/*                                                                   */
/*                     Copyright (c) 2001-2006                       */
/*                       All Rights Reserved.                        */
/*                                                                   */
/*  Permission is hereby granted, free of charge, to use and         */
/*  distribute this software and its documentation without           */
/*  restriction, including without limitation the rights to use,     */
/*  copy, modify, merge, publish, distribute, sublicense, and/or     */
/*  sell copies of this work, and to permit persons to whom this     */
/*  work is furnished to do so, subject to the following conditions: */
/*                                                                   */
/*    1. The source code must retain the above copyright notice,     */
/*       this list of conditions and the following disclaimer.       */
/*                                                                   */
/*    2. Any modifications to the source code must be clearly        */
/*       marked as such.                                             */
/*                                                                   */
/*    3. Redistributions in binary form must reproduce the above     */
/*       copyright notice, this list of conditions and the           */
/*       following disclaimer in the documentation and/or other      */
/*       materials provided with the distribution.  Otherwise, one   */
/*       must contact the HTS working group.                         */
/*                                                                   */
/*  NAGOYA INSTITUTE OF TECHNOLOGY, TOKYO INSTITUTE OF TECHNOLOGY,   */
/*  HTS WORKING GROUP, AND THE CONTRIBUTORS TO THIS WORK DISCLAIM    */
/*  ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL       */
/*  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT   */
/*  SHALL NAGOYA INSTITUTE OF TECHNOLOGY, TOKYO INSTITUTE OF         */
/*  TECHNOLOGY, HTS WORKING GROUP, NOR THE CONTRIBUTORS BE LIABLE    */
/*  FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY        */
/*  DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,  */
/*  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTUOUS   */
/*  ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR          */
/*  PERFORMANCE OF THIS SOFTWARE.                                    */
/*                                                                   */
/*  ---------------------------------------------------------------  */
/*    hts_engine.c: a compact HMM-based speech synthesis engine      */
/*  ---------------------------------------------------------------  */

/*  hts_engine libraries */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include"hts_engine.h"

int hts_engine(char* command[], double f0r, double sr, int spk)
{
	char **argv = &command[0];

	FILE *labfp = stdin; // label 具有欲合成   文本中每個聲母、韻母及靜音停頓的文脈相關語言參數
	FILE *lf0fp = NULL, *mcepfp = NULL, *durfp = NULL, *rawfp = NULL, *tracefp = NULL;

	ModelSet ms;
	TreeSet ts;
	PStream mceppst, lf0pst;
	globalP gp;
	VocoderSetup vs;

	/* default value for control parameter */
	gp.RATE = 16000;
	gp.FPERIOD = 80;
	gp.RHO = 0.0;
	gp.ALPHA = 0.42;
	gp.F0_STD = 1.0;
	gp.F0_MEAN = 0.0;
	gp.UV = 0.5;
	gp.BETA = 0.4;
	gp.LENGTH = 0.0;
//   gp.LENGTH	= atof(*(argv+4));
	gp.algnst = 0;
	gp.algnph = 0;

	gp.F0_STD = f0r;
	gp.RHO = sr;

	/* initialise TreeSet and ModelSet */
	InitTreeSet(&ts);
	InitModelSet(&ms);

	/* delta window handler for log f0 */
	//lf0pst.dw.fn = (char **) HTS_Calloc(argc, sizeof (char *));
	lf0pst.dw.fn = (char **) HTS_Calloc(38, sizeof(char *));
	lf0pst.dw.num = 0;

	/* delta window handler for mel-cepstrum */
	//mceppst.dw.fn = (char **) HTS_Calloc(argc, sizeof (char *));
	mceppst.dw.fn = (char **) HTS_Calloc(38, sizeof(char *));
	mceppst.dw.num = 0;

	char chName[50];

	// files for decision trees
	sprintf(chName, "model/tree-dur_%d.inf", spk);
	ts.fp[DUR] = HTS_Getfp(chName, "r");
	sprintf(chName, "model/tree-lf0_%d.inf", spk);
	ts.fp[LF0] = HTS_Getfp(chName, "r");
	sprintf(chName, "model/tree-mcp_%d.inf", spk);
	ts.fp[MCP] = HTS_Getfp(chName, "r");

	// files for model
	sprintf(chName, "model/dur_%d.pdf", spk);
	ms.fp[DUR] = HTS_Getfp(chName, "rb");
	sprintf(chName, "model/lf0_%d.pdf", spk);
	ms.fp[LF0] = HTS_Getfp(chName, "rb");
	sprintf(chName, "model/mcp_%d.pdf", spk);
	ms.fp[MCP] = HTS_Getfp(chName, "rb");

	// files for dynamic windows
	mceppst.dw.fn[mceppst.dw.num++] = const_cast<char*>("model/mcp.win1");
	mceppst.dw.fn[mceppst.dw.num++] = const_cast<char*>("model/mcp.win2");
	mceppst.dw.fn[mceppst.dw.num++] = const_cast<char*>("model/mcp.win3");

	lf0pst.dw.fn[lf0pst.dw.num++] = const_cast<char*>("model/lf0.win1");
	lf0pst.dw.fn[lf0pst.dw.num++] = const_cast<char*>("model/lf0.win2");
	lf0pst.dw.fn[lf0pst.dw.num++] = const_cast<char*>("model/lf0.win3");
	labfp = HTS_Getfp(*(argv), "r");
	lf0fp = HTS_Getfp(*(argv + 1), "wb");
	rawfp = HTS_Getfp(*(argv + 2), "wb");
	tracefp = HTS_Getfp(*(argv + 3), "w");
	/* load decision trees for duration, log f0 and mel-cepstrum */
	LoadTreeSet(&ts, DUR);
	LoadTreeSet(&ts, LF0);
	LoadTreeSet(&ts, MCP);

	/* load model files for duration, log f0 and mel-cepstrum */
	LoadModelSet(&ms);

	/* check model */
	if (lf0pst.dw.num != ms.lf0stream)
		HTS_Error(1, const_cast<char*>("hts_engine: #window for log f0 is not matched to acoustic model.\n"));
	if (ms.mcepvsize % mceppst.dw.num != 0)
		HTS_Error(1, const_cast<char*>("hts_engine: #window for mcep is not matched to acoustic model.\n"));

	/* initialise MLSA filter */
	InitVocoder(rawfp, ms.mcepvsize - 1, &vs, gp.RATE, gp.FPERIOD);

	/* generate speech */
	HTS_Process(labfp, rawfp, lf0fp, mcepfp, durfp, tracefp, &mceppst, &lf0pst, &gp, &ms, &ts, &vs);

	/* free memory */
	ClearVocoder(rawfp, &vs);
	ClearModelSet(&ms);
	ClearTreeSet(&ts, MCP);
	ClearTreeSet(&ts, LF0);
	ClearTreeSet(&ts, DUR);

	HTS_Free(mceppst.dw.fn);
	HTS_Free(lf0pst.dw.fn);

	fclose(labfp);
	return 0;
}

/* OutLabel: output label with frame number or time */
void OutLabel(FILE *durfp, UttModel *um, globalP *gp)
{
	Model *m;
	int i = 0, j;

	for (m = um->mhead; m != um->mtail; m = m->next)
	{
		j = m->totaldur;
		/* in HTK & HTS format */
		fprintf(durfp, "%d %d %s\n", (int) (i * gp->FPERIOD * 1e+7 / gp->RATE),
				(int) ((i + j) * gp->FPERIOD * 1e+7 / gp->RATE), m->name);
		i += j;
	}

	return;
}

/* OutInfo: output trace information */
void OutInfo(FILE *tracefp, UttModel *um, int nstate, globalP *gp)
{
	Model *m;
	int i, j, t;

	/* output settings */
	fprintf(tracefp, "sampring frequency                     -> %d(Hz)\n", gp->RATE);
	fprintf(tracefp, "frame period                           -> %d(point) %.2f(msec)\n", gp->FPERIOD,
			1e+3 * gp->FPERIOD / gp->RATE);
	fprintf(tracefp, "use state alignment for duration       -> %d\n", gp->algnst);
	fprintf(tracefp, "use phoneme alignment for duration     -> %d\n", gp->algnph);
	fprintf(tracefp, "all-pass constant                      -> %f\n", gp->ALPHA);
	fprintf(tracefp, "postfiltering coefficient              -> %f\n", gp->BETA);
	fprintf(tracefp, "control duration parameter             -> %f\n", gp->RHO);
	fprintf(tracefp, "multilply f0                           -> %f\n", gp->F0_STD);
	fprintf(tracefp, "add f0                                 -> %f\n", gp->F0_MEAN);
	fprintf(tracefp, "voiced/unvoiced threshold              -> %f\n", gp->UV);
	fprintf(tracefp, "specified utterance length             -> %f(sec.)\n\n", gp->LENGTH);

	/* output sentence HMM and generated utterance information */
	fprintf(tracefp, "number of HMMs        -> %d\n", um->nModel);
	fprintf(tracefp, "number of HMM states  -> %d\n", um->nState);
	fprintf(tracefp, "length of this speech -> %1.3lf sec. (%d frames)\n",
			(double) (um->totalframe * gp->FPERIOD) / gp->RATE, um->totalframe);
	fprintf(tracefp, "\n");

	/* output each state information */
	for (m = um->mhead, j = 1, t = 0; m != um->mtail; m = m->next, j++)
	{
		fprintf(tracefp, "%d: %s \n", j, m->name); /* model name */
		fprintf(tracefp, "            duration -> %-3d\n", m->durpdf); /* model duration */
		for (i = 2; i < nstate + 2; i++)
		{
			/* leaf number */
			fprintf(tracefp, " %2d-state : spectrum -> %-3d   f0 -> %-3d   ", i, m->mceppdf[i], m->lf0pdf[i]);
			/* state duration (time) */
			fprintf(tracefp, " %1.3lf--%1.3lf(sec)", (double) (t * gp->FPERIOD) / gp->RATE,
					(double) ((t + m->dur[i]) * gp->FPERIOD) / gp->RATE);
			/* state duration (#frame) */
			fprintf(tracefp, " %3d(frame)", m->dur[i]);
			/* voiced/unvoiced */
			fprintf(tracefp, "   %s\n", ((m->voiced[i]) ? "voiced" : "unvoiced"));
			t += m->dur[i];
		}
	}

	return;
}

/* HTS_Process: parse label, determine state duration, generate sequence of speech parameter vector, and synthesize waveform */
void HTS_Process(FILE *labfp, FILE *rawfp, FILE *lf0fp, FILE *mcepfp, FILE *durfp, FILE *tracefp, PStream *mceppst,
		PStream *lf0pst, globalP *gp, ModelSet *ms, TreeSet *ts, VocoderSetup *vs)
{
	char buf[HTS_MAXBUFLEN];
	Tree *tree;
	int state;
	int start, end;
	int rate, nf;
	double f, mean, var, diffdur;
	hts_Boolean hastime;
	Model *m, *next = NULL;
	UttModel um;

	rate = gp->FPERIOD * 10000000 / gp->RATE;

	mean = var = diffdur = 0.0;

	m = (Model *) HTS_Calloc(1, sizeof(Model));
	um.mtail = um.mhead = m;
	um.totalframe = um.nState = um.nModel = 0;
	start = end = 0;

	/* parse label file */
	while (!feof(labfp))
	{
		HTS_GetToken(labfp, buf);
		if (!isalnum(buf[0]))
			break;
		if (isdigit(buf[0]))
			hastime = 1; /* label contain segmentation information */
		else
			hastime = 0;

		if (hastime)
		{
			if (gp->algnst)
			{ /* state-level segmentation */
				start = atoi(buf); /* start time */
				HTS_GetToken(labfp, buf);
				end = atoi(buf); /* end time */
				HTS_GetToken(labfp, buf);
				HTS_GetToken(labfp, buf);
			}
			else if (gp->algnph)
			{ /* phoneme-level segmentation */
				start = atoi(buf); /* start time */
				HTS_GetToken(labfp, buf);
				end = atoi(buf); /* end time */
				HTS_GetToken(labfp, buf);
			}
			else
			{ /* not use segmentation */
				do
				{
					HTS_GetToken(labfp, buf);
				}
				while (isdigit(buf[0]));
			}
		}

		/* allocate memory for current model */
		m->totaldur = 0;
		m->name = HTS_Strdup(buf); /* model name          */
		m->dur = (int *) HTS_Calloc(ms->nstate + 2, sizeof(int)); /* duration            */
		m->lf0pdf = (int *) HTS_Calloc(ms->nstate + 2, sizeof(int)); /* f0 (leaf number)    */
		m->lf0mean = (double **) HTS_Calloc(ms->nstate + 2, sizeof(double *)); /* f0 (mean)           */
		m->lf0variance = (double **) HTS_Calloc(ms->nstate + 2, sizeof(double *)); /* f0 (variance)       */
		m->voiced = (hts_Boolean *) HTS_Calloc(ms->nstate + 2, sizeof(hts_Boolean)); /* voiced/unvoiced     */
		m->mceppdf = (int *) HTS_Calloc(ms->nstate + 2, sizeof(int)); /* spectrum (leaf num) */
		m->mcepmean = (double **) HTS_Calloc(ms->nstate + 2, sizeof(double *)); /* spectrum (mean)     */
		m->mcepvariance = (double **) HTS_Calloc(ms->nstate + 2, sizeof(double *)); /* spectrum (variance) */

		m->dur -= 2;
		m->lf0pdf -= 2;
		m->lf0mean -= 2;
		m->lf0variance -= 2;
		m->voiced -= 2;
		m->mceppdf -= 2;
		m->mcepmean -= 2;
		m->mcepvariance -= 2;

		for (state = 2; state <= ms->nstate + 1; state++)
		{
			m->lf0mean[state] = (double *) HTS_Calloc(ms->lf0stream, sizeof(double));
			m->lf0variance[state] = (double *) HTS_Calloc(ms->lf0stream, sizeof(double));
			m->lf0mean[state]--;
			m->lf0variance[state]--;
		}

		/* determine state-level duration */
		if (hastime && gp->algnph)
		{ /* use phoneme-level segmentation */
			m->durpdf = SearchTree(m->name, ts->thead[DUR]->root);
			FindDurPDF(m, ms, gp->RHO, &diffdur);
			nf = 0;
			for (state = 2; state <= ms->nstate + 1; state++)
				nf += m->dur[state];

			fprintf(stderr, ">>>nf=%d %d\n", nf, (end - start) / rate);

			f = (double) (end - start) / (rate * nf);
			m->totaldur = 0;

			for (state = 2; state <= ms->nstate + 1; state++)
			{
				nf = (int) (f * m->dur[state] + 0.5);
				if (nf <= 0)
					nf = 1;
				fprintf(stderr, "%d: %d %f %d\n", state, m->dur[state], f, nf);
				m->dur[state] = nf;
				m->totaldur += m->dur[state];
			}
			um.totalframe += m->totaldur;
		}
		else if (hastime && gp->algnst)
		{ /* use state-level segmentation */
			m->dur[2] = (end - start) / rate;
			m->totaldur = m->dur[2];
			um.totalframe += m->dur[2];

			for (state = 3; state <= ms->nstate + 1; state++)
			{
				HTS_GetToken(labfp, buf);
				start = atoi(buf);
				HTS_GetToken(labfp, buf);
				end = atoi(buf);
				HTS_GetToken(labfp, buf);
				m->dur[state] = (end - start) / rate;
				m->totaldur += m->dur[state];
				um.totalframe += m->dur[state];
			}
		}
		else
		{ /* estimate state duration from state duration model (Gaussian) */
			m->durpdf = SearchTree(m->name, ts->thead[DUR]->root);
			if (gp->LENGTH == 0.0)
			{
				FindDurPDF(m, ms, gp->RHO, &diffdur);
				um.totalframe += m->totaldur;
			}
			else
			{ /* if total length of generated speech is specified */
				for (state = 2; state <= ms->nstate + 1; state++)
				{
					mean += ms->durpdf[m->durpdf][state];
					var += ms->durpdf[m->durpdf][state + ms->nstate];
				}
			}
		}

		/* find pdf for f0 */
		for (tree = ts->thead[LF0], state = 2; tree != ts->ttail[LF0]; tree = tree->next, state++)
		{
			m->lf0pdf[state] = SearchTree(m->name, tree->root);
			FindLF0PDF(state, m, ms, gp->UV);
		}

		/* find pdf for spectrum */
		for (tree = ts->thead[MCP], state = 2; tree != ts->ttail[MCP]; tree = tree->next, state++)
		{
			m->mceppdf[state] = SearchTree(m->name, tree->root);
			FindMcpPDF(state, m, ms);
		}

		m->next = (Model *) HTS_Calloc(1, sizeof(Model));
		m = um.mtail = m->next;

		um.nModel++;
		um.nState += ms->nstate;
	}

	/* Specified utterance length is too short */
	if (gp->LENGTH > 0.0 && gp->LENGTH * gp->RATE / gp->FPERIOD < um.nState)
		HTS_Error(1, "hts_engine: specified utterance length is too short.\n");

	/* if total length of utterance is specified, RHO (temporal factor) have to be computed */
	if (gp->LENGTH > 0.0)
	{
		gp->RHO = (((int) gp->LENGTH * gp->RATE / gp->FPERIOD) - mean) / var;
		/* compute state duration for each state */
		for (m = um.mhead; m != um.mtail; m = m->next)
		{
			FindDurPDF(m, ms, gp->RHO, &diffdur);
			um.totalframe += m->totaldur;
		}
	}

	/* output trace information */
	if (tracefp != NULL)
	{
		OutInfo(tracefp, &um, ms->nstate, gp);
		fclose(tracefp);
	}

	/* output segment information */
	if (durfp != NULL)
		OutLabel(durfp, &um, gp);

	/* generate speech parameter vector sequence and synthesize speech waveform */
	pdf2speech(rawfp, lf0fp, mcepfp, mceppst, lf0pst, gp, ms, &um, vs);

	/* free memory */
	for (m = um.mhead; m != um.mtail; m = next)
	{
		next = m->next;
		for (state = ms->nstate + 1; state >= 2; state--)
		{
			if (m->lf0variance != NULL)
				HTS_Free(++m->lf0variance[state]);
			if (m->lf0mean != NULL)
				HTS_Free(++m->lf0mean[state]);
		}

		m->dur += 2;
		m->lf0pdf += 2;
		m->lf0mean += 2;
		m->lf0variance += 2;
		m->voiced += 2;
		m->mceppdf += 2;
		m->mcepmean += 2;
		m->mcepvariance += 2;

		HTS_Free(m->mcepvariance);
		HTS_Free(m->mcepmean);
		HTS_Free(m->mceppdf);
		HTS_Free(m->voiced);
		HTS_Free(m->lf0variance);
		HTS_Free(m->lf0mean);
		HTS_Free(m->lf0pdf);
		HTS_Free(m->dur);
		HTS_Free(m->name);
		HTS_Free(m);
	}
	HTS_Free(next);

	return;

}

/* -------------------- End of "hts_engine.cc" -------------------- */
void HTS_Usage(void)
{
	fprintf(stderr, "\n");
	fprintf(stderr, "hts_engine - A HMM-based speech synthesis engine\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  usage:\n");
	fprintf(stderr, "       hts_engine [ options ] [ infile ] \n");
	fprintf(stderr, "  options:                                                          [def] [min--max]\n");
	fprintf(stderr, "       -td tree  : decision trees file for state duration           [N/A]\n");
	fprintf(stderr, "       -tf tree  : decision trees file for Log F0                   [N/A]\n");
	fprintf(stderr, "       -tm tree  : decision trees file for Mel-Cepstrum             [N/A]\n");
	fprintf(stderr, "       -md pdf   : model file for state duration                    [N/A]\n");
	fprintf(stderr, "       -mf pdf   : model file for Log F0                            [N/A]\n");
	fprintf(stderr, "       -mm pdf   : model file for Mel-Cepstrum                      [N/A]\n");
	fprintf(stderr, "       -df win   : window file for culcuration delta of log F0      [N/A]\n");
	fprintf(stderr, "       -dm win   : filename of delta coeffs for Mel-Cepstrum        [N/A]\n");
	fprintf(stderr, "       -od s     : filename of output duration                      [N/A]\n");
	fprintf(stderr, "       -of s     : filename of output F0                            [N/A]\n");
	fprintf(stderr, "       -om s     : filename of output mcep                          [N/A]\n");
	fprintf(stderr, "       -or s     : filename of output raw audio (generated speech)  [N/A]\n");
	fprintf(stderr, "       -ot s     : filename of output trace information             [N/A]\n");
	fprintf(stderr, "       -vs       : use state alignment for duration                 [0]\n");
	fprintf(stderr, "       -vp       : use phoneme alignment for duration               [0]\n");
	fprintf(stderr, "       -s  i     : sampring frequency                               [16000][  0--48000]\n");
	fprintf(stderr, "       -p  i     : frame period (point)                             [ 80 ][   0--2000]\n");
	fprintf(stderr, "       -a  f     : all-pass constant                                [0.42][ 0.0--1.0]\n");
	fprintf(stderr, "       -b  f     : postfiltering coefficient                        [0.00][-0.8--0.8]\n");
	fprintf(stderr, "       -r  f     : control duration parameter                       [0.00][-1.0--0.0]\n");
	fprintf(stderr, "       -fs f     : multilply f0                                     [1.00][ 0.0--5.0]\n");
	fprintf(stderr, "       -fm f     : add f0                                           [0.00][ 0.0--100.0]\n");
	fprintf(stderr, "       -u  f     : voiced/unvoiced threshold                        [0.50][ 0.0--1.0]\n");
	fprintf(stderr, "       -l  f     : length of generated speech (in second)           [N/A] [ 0.0--30.0]\n");
	fprintf(stderr, "  infile:\n");
	fprintf(stderr, "       label file\n");
	fprintf(stderr, "  note:\n");
	fprintf(stderr, "       option '-d' may be repeated to use multiple\n");
	fprintf(stderr, "       delta parameters\n");
	fprintf(stderr, "       if 'd' is an integer, delta coefficients are\n");
	fprintf(stderr, "       calculated based on a regression formula.\n\n");
	fprintf(stderr, "       generated mel-cepstrum and log F0 sequences are \n");
	fprintf(stderr, "       saved in natural endian, binary (float) format.\n");
	fprintf(stderr, "\n");

	exit(0);

	return;
}

/* HTS_Error: output error message */
void HTS_Error(const int error, char *message, ...)
{
	va_list arg;

	fflush(stdout);
	fflush(stderr);

	if (error > 0)
		fprintf(stderr, "\nError: ");
	else
		fprintf(stderr, "\nWarning: ");

	va_start(arg, message);
	vfprintf(stderr, message, arg);
	va_end(arg);

	fflush(stderr);

	if (error > 0)
		exit(error);

	return;
}

/* HTS_Getfp: wrapper for fopen */
FILE *HTS_Getfp(const char *name, const char *opt)
{
	FILE *fp = fopen(name, opt);

	if (fp == NULL)
		HTS_Error(2, "HTS_Getfp: >\"< Cannot open %s.\n", name);

	return (fp);
}

/* HTS_GetToken: parser */
void HTS_GetToken(FILE *fp, char *buff)
{
	char c;
	int i;
	hts_Boolean squote = 0, dquote = 0;

	c = fgetc(fp);

	while (isspace(c))
		c = fgetc(fp);

	if (c == '\'')
	{ /* single quote case */
		c = fgetc(fp);
		squote = 1;
	}

	if (c == '\"')
	{ /*double quote case */
		c = fgetc(fp);
		dquote = 1;
	}

	if (c == ',')
	{ /*special character ',' */
		strcpy(buff, ",");
		return;
	}

	i = 0;
	while (1)
	{
		buff[i++] = c;
		c = fgetc(fp);
		if (squote && c == '\'')
			break;
		if (dquote && c == '\"')
			break;
		if (!(squote || dquote || isgraph(c)))
			break;
	}

	buff[i] = 0;

	return;
}

/* ----- Routines for memory allocation/free ----- */
/* HTS_Calloc: wrapper for calloc */
char *HTS_Calloc(const size_t num, const size_t size)
{
	char *mem = (char *) calloc(num, size);

	if (mem == NULL)
		HTS_Error(1, "HTS_calloc: Cannot allocate memory.\n");

	return (mem);
}

/* HTS_Free: wrapper for free */
void HTS_Free(void *ptr)
{
	free(ptr);
	return;
}

/* HTS_Strdup: wrapper for strdup */
char *HTS_Strdup(const char *in)
{
	char *tmp = (char *) HTS_Calloc(strlen(in) + 1, sizeof(char));
	strcpy(tmp, in);
	return tmp;
}

/* HTS_AllocVector: allocate vector */
double *HTS_AllocVector(const int x)
{
	double *ptr = (double *) HTS_Calloc(x, sizeof(double));

	ptr--;

	return (ptr);
}

/* HTS_AllocMatrix: allocate matrix */
double **HTS_AllocMatrix(const int x, const int y)
{
	int i;
	double **ptr = (double **) HTS_Calloc(x, sizeof(double *));

	ptr--;

	for (i = 1; i <= x; i++)
		ptr[i] = HTS_AllocVector(y);

	return (ptr);
}

/* HTS_FreeVector: free vector */
void HTS_FreeVector(double *ptr)
{
	ptr++;

	HTS_Free((void *) ptr);

	return;
}

/* HTS_FreeMatrix: free matrix */
void HTS_FreeMatrix(double **ptr, const int x)
{
	int i;

	for (i = x; i > 0; i--)
		HTS_FreeVector(ptr[i]);

	ptr++;

	HTS_Free((void *) ptr);

	return;
}

/* ----- Routines for reading from binary file ----- */
/* HTS_ByteSwap: byte swap */
int HTS_ByteSwap(void *p, const int size, const int blocks)
{
	char *q, tmp;
	int i, j;

	q = (char *) p;

	for (i = 0; i < blocks; i++)
	{
		for (j = 0; j < (size / 2); j++)
		{
			tmp = *(q + j);
			*(q + j) = *(q + (size - 1 - j));
			*(q + (size - 1 - j)) = tmp;
		}
		q += size;
	}

	return i;
}

/* HTS_Fread: fread with byteswap */
int HTS_Fread(void *p, const int size, const int num, FILE *fp)
{
	const int block = fread(p, size, num, fp);
//#ifndef WORDS_BIGENDIAN
	HTS_ByteSwap(p, size, block);
//#endif /* !BIG_ENDIAN */

	return block;
}

double finv(const double x)
{
	if (x >= INFTY2)
		return 0.0;
	if (x <= -INFTY2)
		return 0.0;
	if (x <= INVINF2 && x >= 0)
		return INFTY;
	if (x >= -INVINF2 && x < 0)
		return -INFTY;

	return (1.0 / x);
}

/*------ parameter generation functions */
/* Calc_WUW_and_WUM: calculate W'U^{-1}W and W'U^{-1}M */
void Calc_WUW_and_WUM(PStream *pst, const int m)
{
	int t, i, j, k;
	double WU;

	for (t = 1; t <= pst->T; t++)
	{
		/* initialize */
		pst->sm.WUM[t] = 0.0;
		for (i = 1; i <= pst->width; i++)
			pst->sm.WUW[t][i] = 0.0;

		/* calc WUW & WUM */
		for (i = 0; i < pst->dw.num; i++)
			for (j = pst->dw.width[i][0]; j <= pst->dw.width[i][1]; j++)
				if ((t + j > 0) && (t + j <= pst->T) && (pst->dw.coef[i][-j] != 0.0))
				{
					WU = pst->dw.coef[i][-j] * pst->sm.ivseq[t + j][i * pst->order + m];
					pst->sm.WUM[t] += WU * pst->sm.mseq[t + j][i * pst->order + m];

					for (k = 0; (k < pst->width) && (t + k <= pst->T); k++)
						if ((k - j <= pst->dw.width[i][1]) && (pst->dw.coef[i][k - j] != 0.0))
							pst->sm.WUW[t][k + 1] += WU * pst->dw.coef[i][k - j];
				}
	}

	return;
}

/* LDL_Factorization: Factorize W'*U^{-1}*W to L*D*L' (L: lower triangular, D: diagonal) */
void LDL_Factorization(PStream *pst)
{
	int t, i, j;

	for (t = 1; t <= pst->T; t++)
	{
		for (i = 1; (i < pst->width) && (t - i > 0); i++)
			pst->sm.WUW[t][1] -= pst->sm.WUW[t - i][i + 1] * pst->sm.WUW[t - i][i + 1] * pst->sm.WUW[t - i][1];

		for (i = 2; i <= pst->width; i++)
		{
			for (j = 1; (i + j <= pst->width) && (t - j > 0); j++)
				pst->sm.WUW[t][i] -= pst->sm.WUW[t - j][j + 1] * pst->sm.WUW[t - j][i + j] * pst->sm.WUW[t - j][1];
			pst->sm.WUW[t][i] /= pst->sm.WUW[t][1];
		}
	}

	return;
}

/* Forward_Substitution */
void Forward_Substitution(PStream *pst)
{
	int t, i;

	for (t = 1; t <= pst->T; t++)
	{
		pst->sm.g[t] = pst->sm.WUM[t];
		for (i = 1; (i < pst->width) && (t - i > 0); i++)
			pst->sm.g[t] -= pst->sm.WUW[t - i][i + 1] * pst->sm.g[t - i];
	}

	return;
}

/* Backward_Substitution */
void Backward_Substitution(PStream *pst, const int m)
{
	int t, i;

	for (t = pst->T; t > 0; t--)
	{
		pst->par[t][m] = pst->sm.g[t] / pst->sm.WUW[t][1];
		for (i = 1; (i < pst->width) && (t + i <= pst->T); i++)
			pst->par[t][m] -= pst->sm.WUW[t][i + 1] * pst->par[t + i][m];
	}

	return;
}

/* mlpg: generate sequence of speech parameter vector maximizing its output probability for given pdf sequence */
/*
 1. Calculate W'U^{-1}W and W'U^{-1}M
 2. Factorize W'*U^{-1}*W to L*D*L' (L: lower triangular, D: diagonal)
 3. Substitution ?
 */

void mlpg(PStream *pst)		// pst = given pdf sequence ?
{
	int m;
	const int M = pst->order;

	for (m = 1; m <= M; m++)
	{
		Calc_WUW_and_WUM(pst, m);
		LDL_Factorization(pst); /* LDL factorization */
		Forward_Substitution(pst); /* forward substitution   */
		Backward_Substitution(pst, m); /* backward substitution  */
	}
}

/* ReadDouble: read one double */
double ReadDouble(FILE *fp)
{
	double d = 0.0;

	if (fscanf(fp, "%lf", &d) != 1)
	{
		return 0.0;
	}

	return d;
}

/* ReadInt: read one integer from file */
int ReadInt(FILE *fp)
{
	int i = 0;

	if (fscanf(fp, "%d", &i) != 1)
	{
		return 0;
	}

	return i;
}

/* InitDWin: Initialise window coefficients */
void InitDWin(PStream *pst)
{
	int i, j;
	int fsize, leng;
	FILE *fp;

	/* memory allocation */
	pst->dw.width = (int **) HTS_Calloc(pst->dw.num, sizeof(int *));
	for (i = 0; i < pst->dw.num; i++)
		pst->dw.width[i] = (int *) HTS_Calloc(2, sizeof(int));
	pst->dw.coef = (double **) HTS_Calloc(pst->dw.num, sizeof(double *));

	/* set window coefficients */
	for (i = 0; i < pst->dw.num; i++)
	{
		fp = HTS_Getfp(pst->dw.fn[i], "rt");

		/* check the number of coefficients */
		fscanf(fp, "%d", &fsize);
		if (fsize < 1)
			HTS_Error(1, "InitDWIn: number of coefficients in %s is invalid", pst->dw.fn[i]);

		/* read coefficients */
		pst->dw.coef[i] = (double *) HTS_Calloc(fsize, sizeof(double));

		for (j = 0; j < fsize; j++)
		{
			fscanf(fp, "%lf", &(pst->dw.coef[i][j]));
		}

		fclose(fp);

		/* set pointer */
		leng = fsize / 2;
		pst->dw.coef[i] += leng;
		pst->dw.width[i][WLEFT] = -leng;
		pst->dw.width[i][WRIGHT] = leng;

		if (fsize % 2 == 0)
			pst->dw.width[i][WRIGHT]--;
	}

	pst->dw.maxw[WLEFT] = pst->dw.maxw[WRIGHT] = 0;

	for (i = 0; i < pst->dw.num; i++)
	{
		if (pst->dw.maxw[WLEFT] > pst->dw.width[i][WLEFT])
			pst->dw.maxw[WLEFT] = pst->dw.width[i][WLEFT];
		if (pst->dw.maxw[WRIGHT] < pst->dw.width[i][WRIGHT])
			pst->dw.maxw[WRIGHT] = pst->dw.width[i][WRIGHT];
	}

	/* calculate max_L to determine size of band matrix */
	if (pst->dw.maxw[WLEFT] >= pst->dw.maxw[WRIGHT])
		pst->dw.max_L = pst->dw.maxw[WLEFT];
	else
		pst->dw.max_L = pst->dw.maxw[WRIGHT];

	return;
}

/* FreeDWin: free regression window */
void FreeDWin(PStream *pst)
{
	int i;

	/* free window */
	for (i = pst->dw.num - 1; i >= 0; i--)
	{
		pst->dw.coef[i] += pst->dw.width[i][WLEFT];
		HTS_Free(pst->dw.coef[i]);
	}
	HTS_Free(pst->dw.coef);

	for (i = pst->dw.num - 1; i >= 0; i--)
		HTS_Free(pst->dw.width[i]);
	HTS_Free(pst->dw.width);

	return;
}

/* InitPStream: Initialise PStream for parameter generation */
void InitPStream(PStream *pst)
{
	pst->width = pst->dw.max_L * 2 + 1; /* band width of R */

	pst->sm.mseq = HTS_AllocMatrix(pst->T, pst->vSize);
	pst->sm.ivseq = HTS_AllocMatrix(pst->T, pst->vSize);
	pst->sm.WUW = HTS_AllocMatrix(pst->T, pst->width);
	pst->par = HTS_AllocMatrix(pst->T, pst->order);

	pst->sm.g = HTS_AllocVector(pst->T);
	pst->sm.WUM = HTS_AllocVector(pst->T);

	return;
}

/* FreePStream: Free PStream */
void FreePStream(PStream *pst)
{
	HTS_FreeVector(pst->sm.WUM);
	HTS_FreeVector(pst->sm.g);

	HTS_FreeMatrix(pst->par, pst->T);
	HTS_FreeMatrix(pst->sm.WUW, pst->T);
	HTS_FreeMatrix(pst->sm.ivseq, pst->T);
	HTS_FreeMatrix(pst->sm.mseq, pst->T);

	return;
}

/* pdf2speech: parameter generation and waveform synthesis */
void pdf2speech(FILE *rawfp, FILE *lf0fp, FILE *mcepfp, PStream *mceppst, PStream *lf0pst, globalP *gp, ModelSet *ms,
		UttModel *um, VocoderSetup *vs)
{
	int frame, mcepframe, lf0frame;
	int state, lw, rw, k, n;
	Model *m;
	hts_Boolean nobound, *voiced;
	double f0;
	float temp;

	lf0pst->vSize = ms->lf0stream;
	lf0pst->order = 1;
	mceppst->vSize = ms->mcepvsize;
	mceppst->order = mceppst->vSize / mceppst->dw.num;

	InitDWin(mceppst);
	InitDWin(lf0pst);

	mcepframe = 0;
	lf0frame = 0;

	/* voiced/unvoiced decision */
	voiced = (hts_Boolean *) HTS_Calloc(um->totalframe, sizeof(hts_Boolean));
	voiced--;

	for (m = um->mhead; m != um->mtail; m = m->next)
	{
		for (state = 2; state <= ms->nstate + 1; state++)
		{
			for (frame = 1; frame <= m->dur[state]; frame++)
			{
				voiced[++mcepframe] = m->voiced[state];
				if (m->voiced[state])
				{
					++lf0frame;
				}
			}
		}
	}

	/* set the number of frames for mcep and lf0 */
	mceppst->T = mcepframe;
	lf0pst->T = lf0frame;

	/* initialise parameter generation */
	InitPStream(mceppst);
	InitPStream(lf0pst);

	mcepframe = 1;
	lf0frame = 1;

	/* copy pdfs */
	for (m = um->mhead; m != um->mtail; m = m->next)
	{
		for (state = 2; state <= ms->nstate + 1; state++)
		{
			for (frame = 1; frame <= m->dur[state]; frame++)
			{
				/* copy pdf for mcep */
				for (k = 0; k < ms->mcepvsize; k++)
				{
					mceppst->sm.mseq[mcepframe][k + 1] = m->mcepmean[state][k];
					mceppst->sm.ivseq[mcepframe][k + 1] = finv(m->mcepvariance[state][k]);
				}
				/* copy pdfs for lf0 */
				for (k = 0; k < ms->lf0stream; k++)
				{
					lw = lf0pst->dw.width[k][WLEFT];
					rw = lf0pst->dw.width[k][WRIGHT];
					nobound = (hts_Boolean) 1;
					/* check current frame is voiced/unvoiced boundary or not */
					for (n = lw; n <= rw; n++)
						if (mcepframe + n <= 0 || um->totalframe < mcepframe + n)
							nobound = (hts_Boolean) 0;
						else
							nobound = (hts_Boolean) ((int) nobound & (int) voiced[mcepframe + n]);

					/* copy pdfs */
					if (voiced[mcepframe])
					{
						lf0pst->sm.mseq[lf0frame][k + 1] = m->lf0mean[state][k + 1];
						if (nobound || k == 0)
							lf0pst->sm.ivseq[lf0frame][k + 1] = finv(m->lf0variance[state][k + 1]);
						else
							/* the variances for dynamic feature are set to inf on v/uv boundary */
							lf0pst->sm.ivseq[lf0frame][k + 1] = 0.0;
					}
				}
				if (voiced[mcepframe])
					lf0frame++;
				mcepframe++;
			}
		}
	}

	/* parameter generation for mcep */
	mlpg(mceppst);

	/* parameter generation for lf0 */
	if (lf0frame > 0)
		mlpg(lf0pst);

	lf0frame = 1;

	/* synthesize speech waveforms */
	for (mcepframe = 1; mcepframe <= mceppst->T; mcepframe++)
	{
		/* f0 modification */
		if (voiced[mcepframe])
			f0 = gp->F0_STD * exp(lf0pst->par[lf0frame++][1]) + gp->F0_MEAN;
		else
			f0 = 0.0;

		/* synthesize waveforms by MLSA filter */
		if (rawfp != NULL)
			Vocoder(f0, &mceppst->par[mcepframe][1], mceppst->order - 1, rawfp, gp, vs);

		/* output mcep sequence */
		if (mcepfp != NULL)
		{
			for (k = 1; k <= mceppst->order; k++)
			{
				temp = (float) mceppst->par[mcepframe][k];
				fwrite(&temp, sizeof(float), 1, mcepfp);
			}
		}

		/* output f0 sequence */
		if (lf0fp != NULL)
		{
			temp = (float) f0;
			fwrite(&temp, sizeof(float), 1, lf0fp);
		}
	}

	/* close files */
	if (mcepfp != NULL)
		fclose(mcepfp);
	if (lf0fp != NULL)
		fclose(lf0fp);
	if (rawfp != NULL)
		fclose(rawfp);

	/* free memory */
	FreePStream(lf0pst);
	FreePStream(mceppst);

	HTS_Free(++voiced);

	FreeDWin(lf0pst);
	FreeDWin(mceppst);

	return;
}

/* LoadModelSet: load models */
void LoadModelSet(ModelSet *ms)
{
	int i, j, k, l;
	double vw, uvw;
	float temp;

	/*-------------------- load pdfs for duration --------------------*/
	/* read the number of states & the number of pdfs (leaf nodes) */

	/* read the number of HMM states */
	HTS_Fread(&ms->nstate, sizeof(int), 1, ms->fp[DUR]);
	if (ms->nstate < 0)
		HTS_Error(1, "LoadModelFiles: #HMM states must be positive value.\n");

	/* read the number of duration pdfs */
	HTS_Fread(&ms->ndurpdf, sizeof(int), 1, ms->fp[DUR]);
	if (ms->ndurpdf < 0)
		HTS_Error(1, "LoadModelFiles: #duration pdf must be positive value.\n");

	ms->durpdf = (double **) HTS_Calloc(ms->ndurpdf, sizeof(double *));
	ms->durpdf--;

	/* read pdfs (mean & variance) */
	for (i = 1; i <= ms->ndurpdf; i++)
	{
		ms->durpdf[i] = (double *) HTS_Calloc(2 * ms->nstate, sizeof(double));
		ms->durpdf[i] -= 2;
		for (j = 2; j < 2 * (ms->nstate + 1); j++)
		{
			HTS_Fread(&temp, sizeof(float), 1, ms->fp[DUR]);
			ms->durpdf[i][j] = (double) temp;
		}
	}

	/*-------------------- load pdfs for mcep --------------------*/
	/* read vector size for spectrum */
	HTS_Fread(&ms->mcepvsize, sizeof(int), 1, ms->fp[MCP]);
	if (ms->mcepvsize < 0)
		HTS_Error(1, "LoadModelFiles: vector size of mel-cepstrum part must be positive.\n");

	ms->nmceppdf = (int *) HTS_Calloc(ms->nstate, sizeof(int));
	ms->nmceppdf -= 2;

	/* read the number of pdfs for each state position */
	HTS_Fread(&ms->nmceppdf[2], sizeof(int), ms->nstate, ms->fp[MCP]);
	for (i = 2; i <= ms->nstate + 1; i++)
	{
		if (ms->nmceppdf[i] < 0)
			HTS_Error(1, "LoadModelFiles: #mcep pdf at state %d must be positive value.\n", i);
	}
	ms->mceppdf = (double ***) HTS_Calloc(ms->nstate, sizeof(double **));
	ms->mceppdf -= 2;

	/* read pdfs (mean, variance) */
	for (i = 2; i <= ms->nstate + 1; i++)
	{
		ms->mceppdf[i] = (double **) HTS_Calloc(ms->nmceppdf[i], sizeof(double *));
		ms->mceppdf[i]--;
		for (j = 1; j <= ms->nmceppdf[i]; j++)
		{
			ms->mceppdf[i][j] = (double *) HTS_Calloc(2 * ms->mcepvsize, sizeof(double));
			for (k = 0; k < 2 * ms->mcepvsize; k++)
			{
				HTS_Fread(&temp, sizeof(float), 1, ms->fp[MCP]);
				ms->mceppdf[i][j][k] = (double) temp;
			}
		}
	}

	/*-------------------- load pdfs for log F0 --------------------*/
	/* read the number of streams for f0 modeling */
	HTS_Fread(&ms->lf0stream, sizeof(int), 1, ms->fp[LF0]);
	if (ms->lf0stream < 0)
		HTS_Error(1, "LoadModelFiles: #stream for log f0 part must be positive value.\n");

	ms->nlf0pdf = (int *) HTS_Calloc(ms->nstate, sizeof(int));
	ms->nlf0pdf -= 2;

	/* read the number of pdfs for each state position */
	HTS_Fread(&ms->nlf0pdf[2], sizeof(int), ms->nstate, ms->fp[LF0]);
	for (i = 2; i <= ms->nstate + 1; i++)
	{
		if (ms->nlf0pdf[i] < 0)
			HTS_Error(1, "LoadModelFiles: #lf0 pdf at state %d must be positive.\n", i);
	}
	ms->lf0pdf = (double ****) HTS_Calloc(ms->nstate, sizeof(double ***));
	ms->lf0pdf -= 2;

	/* read pdfs (mean, variance & weight) */
	for (i = 2; i <= ms->nstate + 1; i++)
	{
		ms->lf0pdf[i] = (double ***) HTS_Calloc(ms->nlf0pdf[i], sizeof(double **));
		ms->lf0pdf[i]--;
		for (j = 1; j <= ms->nlf0pdf[i]; j++)
		{
			ms->lf0pdf[i][j] = (double **) HTS_Calloc(ms->lf0stream, sizeof(double *));
			ms->lf0pdf[i][j]--;
			for (k = 1; k <= ms->lf0stream; k++)
			{
				/* 4 -> mean, variance, voiced weight, and unvoiced weight */
				ms->lf0pdf[i][j][k] = (double *) HTS_Calloc(4, sizeof(double));
				for (l = 0; l < 4; l++)
				{
					HTS_Fread(&temp, sizeof(float), 1, ms->fp[LF0]);
					ms->lf0pdf[i][j][k][l] = (double) temp;
				}

				vw = ms->lf0pdf[i][j][k][2]; /* voiced weight */
				uvw = ms->lf0pdf[i][j][k][3]; /* unvoiced weight */
				if (vw < 0.0 || uvw < 0.0 || vw + uvw < 0.99 || vw + uvw > 1.01)
					HTS_Error(1, "LoadModelFiles: voiced/unvoiced weights must be within 0.99 to 1.01.\n");
			}
		}
	}

	return;
}

/* ClearModelSet: clear models */
void ClearModelSet(ModelSet *ms)
{
	int i, j, k;

	/* close */
	fclose(ms->fp[DUR]);
	fclose(ms->fp[LF0]);
	fclose(ms->fp[MCP]);

	/* free models for f0 */
	for (i = ms->nstate + 1; i >= 2; i--)
	{
		for (j = ms->nlf0pdf[i]; j >= 1; j--)
		{
			for (k = ms->lf0stream; k >= 1; k--)
			{
				HTS_Free(ms->lf0pdf[i][j][k]);
			}
			HTS_Free(++ms->lf0pdf[i][j]);
		}
		HTS_Free(++ms->lf0pdf[i]);
		fflush(stdout);
	}
	ms->lf0pdf += 2;
	ms->nlf0pdf += 2;
	HTS_Free(ms->lf0pdf);
	HTS_Free(ms->nlf0pdf);

	/* free models for spectrum */
	for (i = ms->nstate + 1; i >= 2; i--)
	{
		for (j = ms->nmceppdf[i]; j >= 1; j--)
		{
			HTS_Free(ms->mceppdf[i][j]);
		}
		HTS_Free(++ms->mceppdf[i]);
	}
	ms->mceppdf += 2;
	ms->nmceppdf += 2;
	HTS_Free(ms->mceppdf);
	HTS_Free(ms->nmceppdf);

	/* free models for duration */
	for (i = ms->ndurpdf; i >= 1; i--)
	{
		ms->durpdf[i] += 2;
		HTS_Free(ms->durpdf[i]);
	}
	HTS_Free(++ms->durpdf);

	return;
}

/* FindDurPDF: find duration pdf from pdf array */
void FindDurPDF(Model *m, ModelSet *ms, const double rho, double *diffdur)
{
	double data, mean, variance;
	int s;

	const int idx = m->durpdf;
	const int nstate = ms->nstate;

	for (s = 2; s <= nstate + 1; s++)
	{
		mean = ms->durpdf[idx][s];
		variance = ms->durpdf[idx][nstate + s];
		data = mean + rho * variance;

		m->dur[s] = (int) (data + *diffdur + 0.5);
		m->dur[s] = (m->dur[s] < 1) ? 1 : m->dur[s];

		m->totaldur += m->dur[s];
		*diffdur += data - (double) m->dur[s];
	}

	return;
}

/* FindLF0PDF : find required pdf for log F0 from pdf array */
void FindLF0PDF(const int s, Model *m, ModelSet *ms, const double uvthresh)
{
	int stream;
	double *weight;

	const int idx = m->lf0pdf[s];
	const int nstream = ms->lf0stream;

	for (stream = 1; stream <= nstream; stream++)
	{
		m->lf0mean[s][stream] = ms->lf0pdf[s][idx][stream][0];
		m->lf0variance[s][stream] = ms->lf0pdf[s][idx][stream][1];
		weight = ms->lf0pdf[s][idx][stream] + 2;

		if (stream == 1)
		{
			if (weight[0] > uvthresh)
				m->voiced[s] = 1;
			else
				m->voiced[s] = 0;
		}
	}

	return;
}

/* FindMcpPDF : find pdf for mel-cepstrum from pdf array */
void FindMcpPDF(const int s, Model *m, ModelSet *ms)
{
	const int idx = m->mceppdf[s];

	m->mcepmean[s] = ms->mceppdf[s][idx];
	m->mcepvariance[s] = ms->mceppdf[s][idx] + ms->mcepvsize;

	return;
}

/* InitModelSet: initialise model set */
void InitModelSet(ModelSet *ms)
{
	ms->fp[DUR] = NULL;
	ms->fp[LF0] = NULL;
	ms->fp[MCP] = NULL;

	return;
}

/* ------ Pattern Matching functions ------ */
/* DPMatch: recursive matching */
hts_Boolean DPMatch(char *str, char *pat, const int pos, const int max)
{
	if (pos > max)
		return 0;
	if (*str == '\0' && *pat == '\0')
		return 1;

	if (*pat == '*')
	{
		if (DPMatch(str + 1, pat, pos + 1, max) == 1)
			return 1;
		else
			return DPMatch(str + 1, pat + 1, pos + 1, max);
	}
	if (*str == *pat || *pat == '?')
	{
		if (DPMatch(str + 1, pat + 1, pos + 1, max + 1) == 1)
			return 1;
		else
		{
			if (*(pat + 1) == '*')
				return DPMatch(str + 1, pat + 2, pos + 1, max + 1);
		}
	}

	return 0;
}

/* PMatch: pattern matching function */
hts_Boolean PMatch(char *str, char *pat)
{
	int i, max = 0, nstar = 0, nques = 0;
	char buf[HTS_MAXBUFLEN];

	for (i = 0; i < (int) strlen(pat); i++)
	{
		switch (pat[i])
		{
		case '*':
			nstar++;
			break;
		case '?':
			nques++;
			max++;
			break;
		default:
			max++;
		}
	}

	if (nstar == 2 && nques == 0 && pat[0] == '*' && pat[i - 1] == '*')
	{
		/* only string matching is required */
		strncpy(buf, &pat[1], i - 2);
		buf[i - 2] = '\0';
		if (strstr(str, buf) != NULL)
			return 1;
		else
			return 0;
	}
	else
		return DPMatch(str, pat, 0, (int) (strlen(str) - max));
}

/* QMatch: check given string match given question */
hts_Boolean QMatch(char *str, Question *q)
{
	hts_Boolean flag = 0;
	Pattern *p;

	for (p = q->phead; p != q->ptail; p = p->next)
	{
		flag = PMatch(str, p->pat);
		if (flag)
			return 1;
	}

	return 0;
}

/* ------ decition tree handling functions ------*/
/* SearchTree: tree search */
int SearchTree(char *str, Node *node)
{
	while (node != NULL)
	{
		if (QMatch(str, node->quest))
		{
			if (node->yes->pdf > 0)
				return (node->yes->pdf);
			node = node->yes;
		}
		else
		{
			if (node->no->pdf > 0)
				return (node->no->pdf);
			node = node->no;
		}
	}
	return -1;
}

/* LoadQuestions: Load questions from file */
void LoadQuestions(FILE *fp, Question *q, const HTS_Mtype type)
{
	char buf[HTS_MAXBUFLEN];

	HTS_GetToken(fp, buf);
	q->qName = HTS_Strdup(buf);
	q->phead = q->ptail = (Pattern *) HTS_Calloc(1, sizeof(Pattern));

	HTS_GetToken(fp, buf);
	if (strcmp(buf, "{") == 0)
	{
		while (strcmp(buf, "}") != 0)
		{
			HTS_GetToken(fp, buf);
			q->ptail->pat = HTS_Strdup(buf);
			q->ptail->next = (Pattern *) HTS_Calloc(1, sizeof(Pattern));
			q->ptail = q->ptail->next;
			HTS_GetToken(fp, buf);
		}
	}

	return;
}

/* ParseTreePat: parse pattern specified for each tree */
void ParseTreePat(Tree *tree, char *buf)
{
	char *s, *l, *r;

	/* parse tree pattern */
	s = buf;
	if ((l = strchr(s, '{')) != NULL)
	{ /* pattern is specified */
		tree->phead = tree->ptail = (Pattern *) HTS_Calloc(1, sizeof(Pattern));

		/* manimupate pattern */
		s = l + 1;
		if (*s == '(')
			++s;

		r = strrchr(s, '}');
		if (l < r && *(r - 1) == ')')
			--r;
		*r = ',';

		/* parse pattern */
		while ((l = strchr(s, ',')) != NULL)
		{
			*l = '\0';
			tree->ptail->pat = HTS_Strdup(s);
			tree->ptail->next = (Pattern *) HTS_Calloc(1, sizeof(Pattern));
			tree->ptail = tree->ptail->next;

			s = l + 1;
		}
	}

	return;
}

/* IsTree: check given buffer is tree or not */
hts_Boolean IsTree(Tree *tree, char *buf)
{
	char *s, *l, *r;

	s = buf;
	if (((l = strchr(s, '[')) == NULL) || ((r = strrchr(s, ']')) == NULL))
	{
		return 0;
	}
	else
	{
		*r = '\0';
		s = l + 1;
		tree->state = atoi(s);
		ParseTreePat(tree, buf);
	}

	return 1;
}

/* IsNum: check given buffer is number or not */
hts_Boolean IsNum(char *buf)
{
	int i;

	for (i = 0; i < (int) strlen(buf); i++)
		if (!(isdigit(buf[i]) || (buf[i] == '-')))
			return 0;

	return 1;
}

/* FindQuestion: find question from question list */
Question *FindQuestion(TreeSet *ts, const HTS_Mtype type, char *buf)
{
	Question *q;

	for (q = ts->qhead[type]; q != ts->qtail[type]; q = q->next)
		if (strcmp(buf, q->qName) == 0)
			break;

	if (q == ts->qtail[type])
		HTS_Error(1, "FindQuestion: cannot find question %s.\n", buf);

	return q;
}

/* name2num: convert name of node to node index number */
int name2num(char *buf)
{
	return (atoi(strrchr(buf, '_') + 1));
}

/* FindNode: find node for given node index */
Node *FindNode(Node *node, const int num)
{
	while (node)
	{
		if (node->idx == num)
			return node;
		else
			node = node->next;
	}
	return NULL;
}

/* LoadTree: Load trees */
void LoadTree(TreeSet *ts, FILE *fp, Tree *tree, const HTS_Mtype type)
{
	char buf[HTS_MAXBUFLEN];
	Node *node;

	HTS_GetToken(fp, buf);
	node = (Node *) HTS_Calloc(1, sizeof(Node));
	tree->root = tree->leaf = node;

	if (strcmp(buf, "{") == 0)
	{
		while (HTS_GetToken(fp, buf), strcmp(buf, "}") != 0)
		{
			node = FindNode(tree->leaf, atoi(buf));
			HTS_GetToken(fp, buf); /* load question at this node */

			node->quest = FindQuestion(ts, type, buf);
			node->yes = (Node *) HTS_Calloc(1, sizeof(Node));
			node->no = (Node *) HTS_Calloc(1, sizeof(Node));

			HTS_GetToken(fp, buf);
			if (IsNum(buf))
			{
				node->no->idx = atoi(buf);
			}
			else
			{
				node->no->pdf = name2num(buf);
			}
			node->no->next = tree->leaf;
			tree->leaf = node->no;

			HTS_GetToken(fp, buf);
			if (IsNum(buf))
			{
				node->yes->idx = atoi(buf);
			}
			else
			{
				node->yes->pdf = name2num(buf);
			}
			node->yes->next = tree->leaf;
			tree->leaf = node->yes;
		}
	}
	else
	{
		node->pdf = name2num(buf);
	}

	return;
}

/* LoadTreeSet: Load decision tree information */
void LoadTreeSet(TreeSet *ts, const HTS_Mtype type)
{
	char buf[HTS_MAXBUFLEN];
	Question *q;
	Tree *t;
	FILE *fp = ts->fp[type];

	q = (Question *) HTS_Calloc(1, sizeof(Question));
	ts->qhead[type] = q;
	ts->qtail[type] = NULL;

	t = (Tree *) HTS_Calloc(1, sizeof(Tree));
	t->next = NULL;
	t->root = t->leaf = NULL;
	t->phead = t->ptail = NULL;
	t->state = 0;

	ts->thead[type] = t;
	ts->ttail[type] = NULL;
	ts->nTrees[type] = 0;

	/* parse tree files */
	while (!feof(fp))
	{
		HTS_GetToken(fp, buf);
		/* parse questions */
		if (strcmp(buf, "QS") == 0)
		{
			LoadQuestions(fp, q, type);
			q->next = (Question *) HTS_Calloc(1, sizeof(Question));
			q = ts->qtail[type] = q->next;
			q->next = NULL;
		}
		/* parse trees */
		if (IsTree(t, buf))
		{
			LoadTree(ts, fp, t, type);
			t->next = (Tree *) HTS_Calloc(1, sizeof(Tree));
			t = ts->ttail[type] = t->next;
			t->next = NULL;
			t->root = t->leaf = NULL;
			t->phead = t->ptail = NULL;
			t->state = 0;

			/* increment # of trees */
			ts->nTrees[type]++;
		}
	}

	/* no tree information in tree file */
	if (ts->thead[type]->next == NULL)
	{
		if (type == DUR)
			HTS_Error(1, "LoadTreesFile: no trees for duration are loaded.\n");
		else if (type == LF0)
			HTS_Error(1, "LoadTreesFile: no trees for log f0 are loaded.\n");
		else
			HTS_Error(1, "LoadTreesFile: no trees for mel-cepstrum are loaded.\n");
	}

	return;
}

/* ClearNode: recursive function to free Node */
void ClearNode(Node *node)
{
	if (node->yes != NULL)
		ClearNode(node->yes);
	if (node->no != NULL)
		ClearNode(node->no);

	HTS_Free(node);

	return;
}

/* ClearTree: clear given tree */
void ClearTree(Tree *tree)
{
	ClearNode(tree->root);

	return;
}

/* ClearQuestion: clear loaded question */
void ClearQuestion(Question *q)
{
	Pattern *p, *next = NULL;

	for (p = q->phead; p != q->ptail; p = next)
	{
		HTS_Free(p->pat);
		next = p->next;
		HTS_Free(p);
	}
	HTS_Free(next);
	HTS_Free(q->qName);

	return;
}

/* ClearTreeSet: clear decision trees */
void ClearTreeSet(TreeSet *ts, const HTS_Mtype type)
{
	Question *q, *qnext = NULL;
	Tree *t, *tnext = NULL;

	/* close */
	fclose(ts->fp[type]);

	/* free questions */
	for (q = ts->qhead[type]; q != ts->qtail[type]; q = qnext)
	{
		ClearQuestion(q);
		qnext = q->next;
		HTS_Free(q);
	}
	HTS_Free(qnext);

	/* free trees */
	for (t = ts->thead[type]; t != ts->ttail[type]; t = tnext)
	{
		ClearTree(t);
		tnext = t->next;
		HTS_Free(t);
	}
	HTS_Free(tnext);

	return;
}

/* InitTreeSet: Initialise TreeSet */
void InitTreeSet(TreeSet *ts)
{
	ts->fp[DUR] = NULL;
	ts->fp[LF0] = NULL;
	ts->fp[MCP] = NULL;

	return;
}

/* movem: move */
void movem(double *a, double *b, const int nitem)
{
	long i = (long) nitem;

	if (a > b)
		while (i--)
			*b++ = *a++;
	else
	{
		a += i;
		b += i;
		while (i--)
			*--b = *--a;
	}

	return;
}

/* InitVocoder: initialise vocoder */
void InitVocoder(FILE *fp, const int m, VocoderSetup *vs, const int RATE, const int FPERIOD)
{
	if (fp == NULL)
		return;

	vs->fprd = FPERIOD;
	vs->iprd = IPERIOD;
	vs->seed = SEED;
	vs->pd = PADEORDER;

	vs->next = SEED;
	vs->gauss = GAUSS;

	vs->pade[0] = 1.0;
	vs->pade[1] = 1.0;
	vs->pade[2] = 0.0;
	vs->pade[3] = 1.0;
	vs->pade[4] = 0.0;
	vs->pade[5] = 0.0;
	vs->pade[6] = 1.0;
	vs->pade[7] = 0.0;
	vs->pade[8] = 0.0;
	vs->pade[9] = 0.0;
	vs->pade[10] = 1.0;
	vs->pade[11] = 0.4999273;
	vs->pade[12] = 0.1067005;
	vs->pade[13] = 0.01170221;
	vs->pade[14] = 0.0005656279;
	vs->pade[15] = 1.0;
	vs->pade[16] = 0.4999391;
	vs->pade[17] = 0.1107098;
	vs->pade[18] = 0.01369984;
	vs->pade[19] = 0.0009564853;
	vs->pade[20] = 0.00003041721;

	vs->rate = RATE;
	vs->c = (double *) HTS_Calloc(3 * (m + 1) + 3 * (vs->pd + 1) + vs->pd * (m + 2), sizeof(double));

	vs->p1 = -1;
	vs->sw = 0;
	vs->x = 0x55555555;

	/* for postfiltering */
	vs->mc = NULL;
	vs->o = 0;
	vs->d = NULL;
	vs->irleng = IRLENG;

	return;
}

/* ClearVocoder: clear vocoder */
void ClearVocoder(FILE *fp, VocoderSetup *vs)
{
	if (fp != NULL)
	{
		if (vs->d != NULL)
			HTS_Free(vs->d);
		HTS_Free(vs->c);
	}
}

/* Vocoder: pulse/noise excitation and MLSA filster based waveform synthesis */
void Vocoder(double p, double *mc, const int m, FILE *rawfp, globalP *gp, VocoderSetup *vs)
{
	double inc, x, e1, e2;
	int i, j, k, xs;
	double a = gp->ALPHA, beta = gp->BETA;

	/* function for M-sequence random noise generation */
	int mseq(VocoderSetup *);

	/* functions for gaussian random noise generation */
	double nrandom(VocoderSetup *);
	unsigned long srnd(unsigned long);

	/* functions for MLSA filter */
	double mlsadf(double, double *, int, double, int, double *, VocoderSetup *);
	void mc2b(double *, double *, int, double);

	/* functions for postfiltering */
	double b2en(double *, int, double, VocoderSetup *);

	/* f0 -> pitch */
	if (p != 0.0)
		p = vs->rate / p;

	if (vs->p1 < 0)
	{
		if (vs->gauss & (vs->seed != 1))
			vs->next = srnd((unsigned) vs->seed);

		vs->p1 = p;
		vs->pc = vs->p1;
		vs->cc = vs->c + m + 1;
		vs->cinc = vs->cc + m + 1;
		vs->d1 = vs->cinc + m + 1;
	}

	mc2b(mc, vs->cc, m, a);

	/* postfiltering */
	if (beta > 0.0 && m > 1)
	{
		e1 = b2en(vs->cc, m, a, vs);
		vs->cc[1] -= beta * a * mc[2];
		for (k = 2; k <= m; k++)
			vs->cc[k] *= (1.0 + beta);
		e2 = b2en(vs->cc, m, a, vs);
		vs->cc[0] += log(e1 / e2) / 2;

		for (k = 0; k <= m; k++)
			mc[k] = vs->c[k];
	}

	for (k = 0; k <= m; k++)
		vs->cinc[k] = (vs->cc[k] - vs->c[k]) * (double) vs->iprd / (double) vs->fprd;

	if (vs->p1 != 0.0 && p != 0.0)
	{
		inc = (p - vs->p1) * (double) vs->iprd / (double) vs->fprd;
	}
	else
	{
		inc = 0.0;
		vs->pc = p;
		vs->p1 = 0.0;
	}

	for (j = vs->fprd, i = (vs->iprd + 1) / 2; j--;)
	{
		if (vs->p1 == 0.0)
		{
			if (vs->gauss)
				x = (double) nrandom(vs);
			else
				x = mseq(vs);
		}
		else
		{
			if ((vs->pc += 1.0) >= vs->p1)
			{
				x = sqrt(vs->p1);
				vs->pc = vs->pc - vs->p1;
			}
			else
				x = 0.0;
		}

		if (x != 0.0)
			x *= exp(vs->c[0]);

		x = mlsadf(x, vs->c, m, a, vs->pd, vs->d1, vs);
		xs = (short) x;

		fwrite(&xs, sizeof(short), 1, rawfp);

		fflush(stdout);

		if (!--i)
		{
			vs->p1 += inc;
			for (k = 0; k <= m; k++)
				vs->c[k] += vs->cinc[k];
			i = vs->iprd;
		}
	}

	vs->p1 = p;
	movem(vs->cc, vs->c, m + 1);

	return;
}

double mlsafir(const double x, double *b, const int m, const double a, const double aa, double *d)
{
	double y = 0.0;
	int i;

	d[0] = x;
	d[1] = aa * d[0] + a * d[1];

	for (i = 2; i <= m; i++)
		d[i] += a * (d[i + 1] - d[i - 1]);

	for (i = 2; i <= m; i++)
		y += d[i] * b[i];

	for (i = m + 1; i > 1; i--)
		d[i] = d[i - 1];

	return (y);
}

double mlsadf1(double x, double *b, const int m, const double a, const double aa, const int pd, double *d,
		VocoderSetup *vs)
{
	double v, out = 0.0, *pt;
	int i;

	pt = &d[pd + 1];

	for (i = pd; i >= 1; i--)
	{
		d[i] = aa * pt[i - 1] + a * d[i];
		pt[i] = d[i] * b[1];
		v = pt[i] * vs->ppade[i];
		x += (1 & i) ? v : -v;
		out += v;
	}

	pt[0] = x;
	out += x;

	return (out);
}

double mlsadf2(double x, double *b, const int m, const double a, const double aa, const int pd, double *d,
		VocoderSetup *vs)
{
	double v, out = 0.0, *pt;
	int i;

	pt = &d[pd * (m + 2)];

	for (i = pd; i >= 1; i--)
	{
		pt[i] = mlsafir(pt[i - 1], b, m, a, aa, &d[(i - 1) * (m + 2)]);
		v = pt[i] * vs->ppade[i];

		x += (1 & i) ? v : -v;
		out += v;
	}

	pt[0] = x;
	out += x;

	return (out);
}

double mlsadf(double x, double *b, int m, const double a, const int pd, double *d, VocoderSetup *vs)
{
	double aa = 1 - a * a;

	vs->ppade = &(vs->pade[pd * (pd + 1) / 2]);

	x = mlsadf1(x, b, m, a, aa, pd, d, vs);
	x = mlsadf2(x, b, m, a, aa, pd, &d[2 * (pd + 1)], vs);

	return (x);
}

double rnd(unsigned long *next)
{
	double r;

	*next = *next * 1103515245L + 12345;
	r = (*next / 65536L) % 32768L;

	return (r / RANDMAX);
}

double nrandom(VocoderSetup *vs)
{
	if (vs->sw == 0)
	{
		vs->sw = 1;
		do
		{
			vs->r1 = 2 * rnd(&vs->next) - 1;
			vs->r2 = 2 * rnd(&vs->next) - 1;
			vs->s = vs->r1 * vs->r1 + vs->r2 * vs->r2;
		}
		while (vs->s > 1 || vs->s == 0);

		vs->s = sqrt(-2 * log(vs->s) / vs->s);

		return (vs->r1 * vs->s);
	}
	else
	{
		vs->sw = 0;

		return (vs->r2 * vs->s);
	}
}

unsigned long srnd(unsigned long seed)
{
	return (seed);
}

int mseq(VocoderSetup *vs)
{
	int x0, x28;

	vs->x >>= 1;

	if (vs->x & B0)
		x0 = 1;
	else
		x0 = -1;

	if (vs->x & B28)
		x28 = 1;
	else
		x28 = -1;

	if (x0 + x28)
		vs->x &= B31_;
	else
		vs->x |= B31;

	return (x0);
}

/* mc2b: transform mel-cepstrum to MLSA digital fillter coefficients */
void mc2b(double *mc, double *b, int m, const double a)
{
	b[m] = mc[m];

	for (m--; m >= 0; m--)
		b[m] = mc[m] - a * b[m + 1];

	return;
}

/* b2bc: transform MLSA digital filter coefficients to mel-cepstrum */
void b2mc(double *b, double *mc, int m, const double a)
{
	double d, o;

	d = mc[m] = b[m];
	for (m--; m >= 0; m--)
	{
		o = b[m] + a * d;
		d = b[m];
		mc[m] = o;
	}

	return;
}

/* freqt: frequency transformation */
void freqt(double *c1, const int m1, double *c2, const int m2, const double a, VocoderSetup *vs)
{
	int i, j;
	double b;

	if (vs->d == NULL)
	{
		vs->size = m2;
		vs->d = (double *) HTS_Calloc(vs->size + vs->size + 2, sizeof(double));
		vs->g = vs->d + vs->size + 1;
	}

	if (m2 > vs->size)
	{
		HTS_Free(vs->d);
		vs->size = m2;
		vs->d = (double *) HTS_Calloc(vs->size + vs->size + 2, sizeof(double));
		vs->g = vs->d + vs->size + 1;
	}

	b = 1 - a * a;
	for (i = 0; i < m2 + 1; i++)
		vs->g[i] = 0.0;

	for (i = -m1; i <= 0; i++)
	{
		if (0 <= m2)
			vs->g[0] = c1[-i] + a * (vs->d[0] = vs->g[0]);
		if (1 <= m2)
			vs->g[1] = b * vs->d[0] + a * (vs->d[1] = vs->g[1]);
		for (j = 2; j <= m2; j++)
			vs->g[j] = vs->d[j - 1] + a * ((vs->d[j] = vs->g[j]) - vs->g[j - 1]);
	}

	movem(vs->g, c2, m2 + 1);

	return;
}

/* c2ir: The minimum phase impulse response is evaluated from the minimum phase cepstrum */
void c2ir(double *c, const int nc, double *h, const int leng)
{
	int n, k, upl;
	double d;

	h[0] = exp(c[0]);
	for (n = 1; n < leng; n++)
	{
		d = 0;
		upl = (n >= nc) ? nc - 1 : n;
		for (k = 1; k <= upl; k++)
			d += k * c[k] * h[n - k];
		h[n] = d / n;
	}

	return;
}

double b2en(double *b, const int m, const double a, VocoderSetup *vs)
{
	double en;
	int k;

	if (vs->o < m)
	{
		if (vs->mc != NULL)
			HTS_Free(vs->mc);

		vs->mc = (double *) HTS_Calloc((m + 1) + 2 * vs->irleng, sizeof(double));
		vs->cep = vs->mc + m + 1;
		vs->ir = vs->cep + vs->irleng;
	}

	b2mc(b, vs->mc, m, a);
	freqt(vs->mc, m, vs->cep, vs->irleng - 1, -a, vs);
	c2ir(vs->cep, vs->irleng, vs->ir, vs->irleng);
	en = 0.0;

	for (k = 0; k < vs->irleng; k++)
		en += vs->ir[k] * vs->ir[k];

	return (en);
}
