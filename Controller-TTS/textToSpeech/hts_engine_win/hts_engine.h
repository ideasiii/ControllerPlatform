#include <math.h>

#define HTS_MAXBUFLEN 1024

typedef bool hts_Boolean;
typedef enum
{
	DUR, LF0, MCP
} HTS_Mtype;
#define HTS_NUMMTYPE 3

#define INFTY   ((double) 1.0e+38)
#define INFTY2  ((double) 1.0e+19)
#define INVINF  ((double) 1.0e-38)
#define INVINF2 ((double) 1.0e-19)
#define LTPI    1.83787706640935     /* log(2*PI) */

#define WLEFT  0
#define WRIGHT 1

#define RANDMAX 32767 
#define   IPERIOD    1
#define   SEED       1
#define   B0         0x00000001
#define   B28        0x10000000
#define   B31        0x80000000
#define   B31_       0x7fffffff
#define   Z          0x00000000

#define   GAUSS      1
#define   PADEORDER  5
#define   IRLENG     96

typedef struct _globalP
{
	int RATE; /* sampring rate                              */
	int FPERIOD; /* frame period (point)                       */
	double RHO; /* variable for speaking rate control         */
	double ALPHA; /* variable for frequency warping parameter   */
	double F0_STD; /* variable for f0 control                    */
	double F0_MEAN; /* variable for f0 control                    */
	double BETA; /* variable for postfiltering                 */
	double UV; /* variable for U/V threshold                 */
	double LENGTH; /* total number of frame for generated speech */
	hts_Boolean algnst; /* use state level alignment for duration     */
	hts_Boolean algnph; /* use phoneme level alignment for duration   */

	// added by pear, for Legendre parameter
	int* sybound;
	int sysize;
	float** lgdpar;
	int total;
} globalP;

/* DWin: structure for regression window */
typedef struct _DWin
{
	int num; /* number of static + deltas */
	char **fn; /* delta window coefficient file */
	int **width; /* width [0..num-1][0(left) 1(right)] */
	double **coef; /* coefficient [0..num-1][length[0]..length[1]] */
	int maxw[2]; /* max width [0(left) 1(right)] */
	int max_L; /* max {maxw[0], maxw[1]} */
} DWin;

/* SMatrices: structure for matrices to generate sequence of speech parameter vectors */
typedef struct _SMatrices
{
	double **mseq; /* sequence of mean vector */
	double **ivseq; /* sequence of invarsed variance vector */
	double *g; /* for forward substitution */
	double **WUW; /* W' U^-1 W  */
	double *WUM; /* W' U^-1 mu */
} SMatrices;

/* PStream: structure for parameter generation setting */
typedef struct _PStream
{
	int vSize; /* vector size of observation vector (include static and dynamic features) */
	int order; /* vector size of static features */
	int T; /* length */
	int width; /* width of dynamic window */
	DWin dw; /* dynamic window */
	double **par; /* output parameter vector */
	SMatrices sm; /* matrices for parameter generation */
} PStream;

/* Model: structure for individual HMM */
typedef struct _Model
{
	char *name; /* the name of this HMM */
	int durpdf; /* duration pdf index for this HMM */
	int *lf0pdf; /* mel-cepstrum pdf indexes for each state of this HMM */
	int *mceppdf; /* log f0 pdf indexes for each state of this HMM */
	int *dur; /* duration for each state of this HMM */
	int totaldur; /* total duration of this HMM */
	double **lf0mean; /* mean vector of log f0 pdfs for each state of this HMM */
	double **lf0variance; /* variance (diag) elements of log f0 for each state of this HMM */
	double **mcepmean; /* mean vector of mel-cepstrum pdfs for each state of this HMM */
	double **mcepvariance; /* variance (diag) elements of mel-cepstrum for each state of this HMM */
	hts_Boolean *voiced; /* voiced/unvoiced decision for each state of this HMM */
	struct _Model *next; /* pointer to next HMM */
} Model;

/* UttMode: structure for utterance HMM */
typedef struct _UttModel
{
	Model *mhead;
	Model *mtail;
	int nModel; /* # of models for current utterance     */
	int nState; /* # of HMM states for current utterance */
	int totalframe; /* # of frames for current utterance     */
} UttModel;

/* ModelSet: structure for HMM set */
typedef struct _ModelSet
{
	int nstate; /* # of HMM states for individual HMM */
	int lf0stream; /* # of stream for log f0 modeling */
	int mcepvsize; /* vector size for mcep modeling */
	int *nlf0pdf; /* # of pdfs for each state position (log F0) */
	int *nmceppdf; /* # of pdfs for each state position (mcep) */
	int ndurpdf; /* # of pdfs for duration */
	double **durpdf; /* pdfs for duration */
	double ***mceppdf; /* pdfs for mcep     */
	double ****lf0pdf; /* pdfs for lf0      */
	FILE *fp[HTS_NUMMTYPE]; /* file pointer for mcep, logF0, and duration model */
} ModelSet;

/* Pattern: structure for pattern */
typedef struct _Pattern
{
	char *pat; /* pattern */
	struct _Pattern *next; /* link to next pattern */
} Pattern;

/* Question: structure for questions */
typedef struct _Question
{
	char *qName; /* name of this question */
	Pattern *phead; /* link to head of pattern list */
	Pattern *ptail; /* link to tail of pattern list */
	struct _Question *next; /* link to next question */
} Question;

/* Node: structure for node of decision tree */
typedef struct _Node
{
	int idx; /* index of this node */
	int pdf; /* index of pdf for this node  ( leaf node only ) */
	struct _Node *yes; /* link to child node (yes) */
	struct _Node *no; /* link to child node (no)  */
	struct _Node *next; /* link to next node  */
	Question *quest; /* question applied at this node */
} Node;

/* Tree: structure for each decision tree */
typedef struct _Tree
{
	int state; /* state position of this tree */
	Pattern *phead; /* link to head of pattern list for this tree */
	Pattern *ptail; /* link to tail of pattern list for this tree */
	struct _Tree *next; /* link to next tree */
	Node *root; /* root node of this decision tree */
	Node *leaf; /* leaf nodes of this decision tree */
} Tree;

/* TreeSet: structure for decision tree set */
typedef struct _TreeSet
{
	Question *qhead[HTS_NUMMTYPE]; /* question lists for mcep, logF0, and duration */
	Question *qtail[HTS_NUMMTYPE];
	Tree *thead[HTS_NUMMTYPE]; /* tree lists for mcep, logF0, and duration */
	Tree *ttail[HTS_NUMMTYPE];
	int nTrees[HTS_NUMMTYPE]; /* # of trees for mcep, logF0, and duration */
	FILE *fp[HTS_NUMMTYPE]; /* file pointers for mcep, logF0, and duration */
} TreeSet;

/* VocoderSetup: structure for setting of vocoder */
typedef struct _VocoderSetup
{
	int fprd;
	int iprd;
	int seed;
	int pd;
	unsigned long next;
	hts_Boolean gauss;
	double p1;
	double pc;
	double pj;
	double pade[21];
	double *ppade;
	double *c, *cc, *cinc, *d1;
	double rate;

	int sw;
	double r1, r2, s;

	int x;

	/* for postfiltering */
	int size;
	double *d;
	double *g;
	double *mc;
	double *cep;
	double *ir;
	int o;
	int irleng;
} VocoderSetup;

int hts_engine(char* command[], double f0r, double sr, int spk);

void HTS_Process(FILE *, FILE *, FILE *, FILE *, FILE *, FILE *, PStream *, PStream *, globalP *, ModelSet *, TreeSet *,
		VocoderSetup *);

void OutLabel(FILE *durfp, UttModel *um, globalP *gp);
void OutInfo(FILE *tracefp, UttModel *um, int nstate, globalP *gp);

/* ----- routines for file input/output -----*/
void HTS_Usage(void);
void HTS_Error(const int error, char *, ...);
FILE *HTS_Getfp(const char * name, const char *opt);
void HTS_GetToken(FILE *fp, char *buff);

/* ----- routines for memory allocation/free -----*/
char *HTS_Calloc(const size_t num, const size_t size);
void HTS_Free(void *ptr);
char *HTS_Strdup(const char * in);
double *HTS_AllocVector(const int x);
double **HTS_AllocMatrix(const int x, const int y);
void HTS_FreeVector(double *ptr);
void HTS_FreeMatrix(double **ptr, const int x);

/* ----- Routines for reading from binary file ----- */
int HTS_Fread(void *p, const int size, const int num, FILE *fp);

/* Function prototype for HMM-based speech synthesis */
void pdf2speech(FILE *, FILE *, FILE *, PStream *, PStream *, globalP *, ModelSet *, UttModel *, VocoderSetup *);

/* Function prototypes for tree handling */
void LoadModelSet(ModelSet *);
void ClearModelSet(ModelSet *);

void FindDurPDF(Model *, ModelSet *, const double, double *);
void FindLF0PDF(const int, Model *, ModelSet *, const double);
void FindMcpPDF(const int, Model *, ModelSet *);

void InitModelSet(ModelSet *);

/* Function prototypes for tree handling */
void LoadTreeSet(TreeSet *, const HTS_Mtype);
void ClearTreeSet(TreeSet *, const HTS_Mtype);

int SearchTree(char *, Node *);

void InitTreeSet(TreeSet *);

/* Function prototypes for vocoding */
void InitVocoder(FILE *, int, VocoderSetup *, const int, const int);
void ClearVocoder(FILE *, VocoderSetup *);
void Vocoder(double p, double *mc, const int m, FILE *rawfp, globalP *gp, VocoderSetup *vs);
