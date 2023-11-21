/********************************************************************************
 *   Copyright (c) : Université de Lyon 1, CNRS/IN2P3, UMR5822,                 *
 *                   IP2I, F-69622 Villeurbanne Cedex, France                   *
 *   Contibutor(s) :                                                            *
 *      Jérémie Dudouet jeremie.dudouet@cnrs.fr [2023]                          *
 *                                                                              *
 *    This software is governed by the CeCILL-B license under French law and    *
 *    abiding by the  rules of distribution of free  software.  You can use,    *
 *    modify  and/ or  redistribute  the  software under  the  terms of  the    *
 *    CeCILL-B license as circulated by CEA, CNRS and INRIA at the following    *
 *    URL \"http://www.cecill.info\".                                           *
 *                                                                              *
 *    As a counterpart to the access  to the source code and rights to copy,    *
 *    modify  and redistribute granted  by the  license, users  are provided    *
 *    only with a limited warranty  and the software's author, the holder of    *
 *    the economic  rights, and the  successive licensors have  only limited    *
 *    liability.                                                                *
 *                                                                              *
 *    In this respect, the user's attention is drawn to the risks associated    *
 *    with loading,  using, modifying  and/or developing or  reproducing the    *
 *    software by the user in light of its specific status of free software,    *
 *    that  may mean that  it is  complicated to  manipulate, and  that also    *
 *    therefore  means that it  is reserved  for developers  and experienced    *
 *    professionals having in-depth  computer knowledge. Users are therefore    *
 *    encouraged  to load  and test  the software's  suitability  as regards    *
 *    their  requirements  in  conditions  enabling the  security  of  their    *
 *    systems  and/or data to  be ensured  and, more  generally, to  use and    *
 *    operate it in the same conditions as regards security.                    *
 *                                                                              *
 *    The fact that  you are presently reading this means  that you have had    *
 *    knowledge of the CeCILL-B license and that you accept its terms.          *
 ********************************************************************************/

#ifndef CXRadInterface_H
#define CXRadInterface_H

#include <math.h>
#include <stdio.h>

#include "TString.h"

constexpr int int_ceil(float f) {
    const int i = static_cast<int>(f);
    return f > i ? i + 1 : i;
}

//for 8192 channels
//#define RW_MAXCH 8192      /* Maximum channels on cube axis */
//                           /*    (must be multiple of 8)    */
//#define RW_LB2 43819       /* length required for buf2, depends on RW_LB1   */
//                           /* RW_LB2 >= RW_LB1/RW_CHNK_S                    */
//#define RW_LB1 1402208     /* length required for buf1, depends on RW_MAXCH */
//                           /* RW_LB1 > x(x+1)(x+2)/(6*128),                 */
//		                     /* where x = RW_MAXCH/8                          */
//                           /* must also be a multiple of RW_CHNK_S          */

static constexpr int RW_CHNK_S    = 32;       // size of update chunk, in units of 1/4 MB
static constexpr int RW_MAXCH     = 8192;     // Maximum channels on cube axis, (must be multiple of 8)

static constexpr int RW_LB2 = int_ceil((float)(int_ceil((float)((RW_MAXCH/8)*((RW_MAXCH/8)+1)*((RW_MAXCH/8)+2)/6)/128))/RW_CHNK_S);
static constexpr int RW_LB1 =  RW_LB2*RW_CHNK_S;

static constexpr int RW_DB1 = 90;         // depth of buf1
static constexpr int RW_DB2 = 180;        // depth of buf2 not to exceed RW_SCR_RECL byte records in scr file
static constexpr int RW_SCR_RECL = 32768; // Record length of scr file, must be >= RW_DB2*2*(RW_DB1+1) + 4
static constexpr int RW_MAXMULT = 40;

static constexpr int MAXCHS     = 8192;  /*    (must be multiple of 8)    */
static constexpr int MAXGAM     = 1500;   /* max number of gammas in scheme */

#define MCLEN(mcptr) (mcptr[0] + (mcptr[0] >= 7*32 ? mcptr[1]*32+2 : 1))

/************************************************
 *  3D CUBE FILE FORMAT
 *  1024byte file header
 *  4096byte data records
 *
 *      each data record contains:
 *      variable number of bit-compressed 8x8x4 mini-cubes
 *      8bytes header, 4088 bytes of data
 ************************************************/

/* 3d cube file header */
typedef struct {
    char id[16];            /* "Incub8r3/Pro4d  " */
    int  numch;             /* number of channels on axis */
    int  bpc;               /* bytes per channel, = 4 */
    int  cps;               /* 1/cps symmetry compression, = 6 */
    int  numrecs;           /* number of 4kB data records in the file */
    char resv[992];         /* FUTURE flags */
} FHead3D;

/* 3Drecord header */
typedef struct {
    int minmc;               /* start minicube number, starts at 0 */
    unsigned short nummc;    /* number of minicubes stored in here */
    unsigned short offset;   /* offset in bytes to first full minicube */
} RHead3D;

typedef struct {
    RHead3D h;
    unsigned char d[4088];  /* the bit compressed data */
} Record3D;               /* see the compression alg for details */

typedef struct {
    FILE *CubeFile3d;
    int length;     /* length of axis on cube */
    int nummc;      /* number of minicubes */
    int *mcnum;     /* SuperRecord minicube number table */
    int numrecs;    /* number of 4kB data records in the file */
    int cubetype;   /* 0 for compressed 1/6 cube,
                 1 for compressed 1/2 cube,
                 2 for uncompressed 1/6 cube, or
                 3 for uncompressed 1/2 cube  */
    int numsr;
    int swapbytes;
} CubeData3D;

/* Some global data */
typedef struct {
    FILE *CubeFile;
    FILE *ScrFile;
    FILE *LogFile;
    TString CubeFileName;
    int  ScrSize;
    char TapeDev[256];
    int  SwapTapeBytes;
    int  length;     /* length of axis on cube */
    int  nummc;      /* number of minicubes */
    int  numscr;     /* number of records written to scrfile */
    int  adcmax;
    int  numrecs;    /* current number of records in cube file */
    int  gotsignal;  /* a signal was caught by signal handler */
} GlobalData;

/* Common Block Declarations */
typedef struct {
    int   numchs;
    float spec[6][MAXCHS], old_spec[6][MAXCHS], bspec[6][MAXCHS], background[MAXCHS], tmpspec[MAXCHS];
    float v_depth[MAXCHS], v_width;
    TString  name_gat, old_name_gat;

    int   lo_ch[MAXGAM], hi_ch[MAXGAM];
    int   npks_ch[MAXCHS];                     /* no. of gammas for each ch. */
    short pks_ch[MAXCHS][60];                  /* gammas w/ counts in ch. */
    int   fitchx[MAXCHS], fitchy[MAXCHS];

    TString  cubnam, levfile, fdname, fdgfile;
    TString  progname;

    float eff_sp[MAXCHS], energy_sp[MAXCHS];
    float elo_sp[MAXCHS], ehi_sp[MAXCHS], ewid_sp[MAXCHS];

    int   luch[MAXCHS+1], matchs ,le2pro2d;
    float pro2d[MAXCHS*(MAXCHS+1)/2], pro2d_temp[MAXCHS*(MAXCHS+1)/2];
    float e2pro2d[MAXCHS*(MAXCHS+1)/2], e2e2spec[MAXCHS], e2e2e2sum;

    float bf1, bf2, bf4, bf5;

    float pk_shape[MAXGAM][15];
    float pk_deriv[MAXGAM][15];
    float w_deriv [MAXGAM][15];

    short looktab[16384];
    int   nclook, lookmin, lookmax;

    /* stuff added for linear combinations of cubes: */
    float dpro2d[MAXCHS*(MAXCHS+1)/2];
    TString  cubenam1[5];
    float cubefact[5];
    int   many_cubes;
} CommonBlock;

/* ======================================================================= */

int file_error(const char *error_type, const TString &filename);
FILE *open_readonly(const TString &filename);
void swapb8(char *buf);
void swapb4(char *buf);
void swapb2(char *buf);
void swap4(int *in);
void swap2(unsigned short *in);
int get_file_rec(FILE *fd, void *data, int maxbytes, int swap_bytes);
int read_tab_file(const TString &filnam, int *nclook, int *lookmin, int *lookmax, short *looktab, int dimlook);
char *datim (void);

int compress3d (unsigned int *in, unsigned char *out);
void decompress3d (unsigned char in[1024], unsigned int out[256]);

int setext(char *filnam, const char *cext, int filnam_len);

FILE *open_new_file(TString &filename, int /*force_open*/);
int put_file_rec(FILE *fd, void *data, int numbytes);
int wspec(TString &filnam, float *spec, int idim);
int inq_file(char *filename, int *reclen);

void gen_binning(int *nclook, int *lookmin, int *lookmax, short *looktab, int lookdim, int rangemax, float compressed_factor);

#endif
