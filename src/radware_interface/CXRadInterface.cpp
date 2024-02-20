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

#include "CXRadInterface.h"
#include "CXBashColor.h"

#include <sys/stat.h>

/* ======================================================================= */
int file_error(const char *error_type, const TString &filename)
{
    /* write error message, cannot perform operation error_type on file filename */
    gbash_color->ErrorMessage(Form("Cannot %s file %s\n", error_type, filename.Data()));
    return 0;
} /* file_error */

/* ======================================================================= */
FILE *open_readonly(const TString &filename)
{
    /* open old file for READONLY (if possible)
     filename  = file name
     provided for VMS compatibility */

    FILE *file;
    if (!(file = fopen(filename, "r"))) file_error("open", filename);
    return file;
} /* open_readonly */

/* ======================================================================= */
void swapb8(char *buf)
{
    char c;
    c = buf[7]; buf[7] = buf[0]; buf[0] = c;
    c = buf[6]; buf[6] = buf[1]; buf[1] = c;
    c = buf[5]; buf[5] = buf[2]; buf[2] = c;
    c = buf[4]; buf[4] = buf[3]; buf[3] = c;
} /* swapb8 */
void swapb4(char *buf)
{
    char c;
    c = buf[3]; buf[3] = buf[0]; buf[0] = c;
    c = buf[2]; buf[2] = buf[1]; buf[1] = c;
} /* swapb4 */
void swapb2(char *buf)
{
    char c;
    c = buf[1]; buf[1] = buf[0]; buf[0] = c;
} /* swapb2 */

void swap4(int *in)
{
    char *c, tmp;

    c = (char *) in;
    tmp = *c;
    *c = *(c+3);
    *(c+3) = tmp;
    tmp = *(c+1);
    *(c+1) = *(c+2);
    *(c+2) = tmp;
}

void swap2(unsigned short *in)
{
    char *c, tmp;

    c = (char *) in;
    tmp = *c;
    *c = *(c+1);
    *(c+1) = tmp;
}

/* ======================================================================= */
int get_file_rec(FILE *fd, void *data, int maxbytes, int swap_bytes)
{
    /* read one fortran-unformatted style binary record into data */
    /* for unix systems, swap_bytes controls how get_file_rec deals with swapping the bytes of the record length tags at the start and end of the records.
     * Set swap_bytes to
       0 to try to automatically sense if the bytes need swapping
       1 to force the byte swap, or
      -1 to force no byte swap */
    /* returns number of bytes read in record, or number of bytes * -1 if bytes need swapping, or 0 for error */
    int  reclen, j1, j2;
    if (fread(&reclen, 4, 1, fd) != 1) return 0;
    if (reclen == 0) return 0;
    j1 = reclen;
    if ((swap_bytes == 1) ||
        (swap_bytes == 0 && reclen >= 65536)) swapb4((char *) &reclen);
    if (reclen > maxbytes) goto ERR1;
    if (fread(data, reclen, 1, fd) != 1 ||
        fread(&j2, 4, 1, fd) != 1) goto ERR2;
    /* if (j1 != j2) goto ERR2; */
    if (reclen == j1) return reclen;
    return (-reclen);

ERR1:
    gbash_color->ErrorMessage(Form("ERROR: record is too big for get_file_rec\n"
                                   "       max size = %d, record size = %d.\n",maxbytes, reclen));
    return 0;
ERR2:
    gbash_color->ErrorMessage(Form("ERROR during read in get_file_rec.\n"));
    return 0;
} /* get_file_rec */

int read_tab_file(const TString &filnam, int *nclook, int *lookmin, int *lookmax, short *looktab, int dimlook)
{
    /* read lookup table from .tab file = filnam
     into array looktab of dimension dimlook
     Nclook = number of channels read
     lookmin, lookmax = min/max values in lookup table
     returns 1 for open/read error
     default file extension = .tab */

    char cbuf[800];
    int  j, rl, swap = -1;
    FILE *file;

    /* OPEN (ILU,FILE=FILNAM,FORM='UNFORMATTED',STATUS='OLD')
     READ (ILU) NCLOOK,LOOKMIN,LOOKMAX
     READ (ILU) (LOOKTAB(I),I=1,NCLOOK) */

    if (!(file = open_readonly(filnam))) return 1;
    rl = get_file_rec(file, cbuf, 800, 0);
    if (rl != 12  && rl != -12) {
        file_error("read lookup-table from", filnam);
        fclose(file);
        return 1;
    }
    if (rl < 0) {
        /* file has wrong byte order - swap bytes */
        cout<<"*** Swapping bytes read from file " << filnam <<endl;
        swap = 1;
        for (j = 0; j < 3; ++j) {
            swapb4(cbuf + 4*j);
        }
    }
    memcpy(nclook,  cbuf,     4);
    memcpy(lookmin, cbuf + 4, 4);
    memcpy(lookmax, cbuf + 8, 4);
    if (*nclook < 2 || *nclook > dimlook) {
        file_error("read lookup-table from", filnam);
        fclose(file);
        return 1;
    }

    rl = get_file_rec(file, looktab, 2*dimlook, swap);
    fclose(file);
    if (rl != 2*(*nclook) && rl != -2*(*nclook)) {
        file_error("read lookup-table from", filnam);
        return 1;
    }
    if (rl < 0) {
        /* file has wrong byte order - swap bytes */
        for (j = 0; j < *nclook; ++j) {
            swapb2((char *) (looktab + j));
        }
    }

    return 0;
} /* read_tab_file */

char *datim (void)
{
    time_t tm;

    tm = time(NULL);
    return (ctime(&tm));
}

int compress3d (unsigned int *in, unsigned char *out)
{
    static int sum_limits[] = { 16,32,64,128,
                               256,512,1024,2048,
                               4096,8192,17000,35000,
                               80000,160000,320000,640000 };

    static int bitmask[] = { 0,1,3,7,15,31,63,127,255,511,1023,2047,
                            4095,8191,16383,32767 };
    int cube_length;
    int sum=0, i, j;
    int nplanes, noverflows, stmp, nbits;
    unsigned char *bitptr, *ovrptr;
    int test1, test2, test3;

    /* guess at depth of bit field */
    for (i=0;i<256;i+=16) {
        sum += in[i];
    }

    i = 7;
    j = 8;
    while (j>1) {
        j = j / 2;
        if (sum > sum_limits[i])
            i += j;
        else
            i -= j;
    }
    if (sum > sum_limits[i+1])
        nplanes = i + 1;
    else
        nplanes = i;

    while (1) {
        test1 = test2 = test3 = 0;
        for (i=0;i<256;i++) {
            test1 += in[i] >> nplanes;
            test2 += in[i] >> (nplanes+1);
            test3 += in[i] >> (nplanes+2);
        }
        if (test2 > 31) {
            if (test3 > 31) {
                nplanes += 3;
                continue;
            }
            nplanes += 2;
            noverflows = test3;
            break;
        }
        else if (test1 > 31) {
            nplanes += 1;
            noverflows = test2;
            break;
        }
        noverflows = test1;
        break;
    }

    if (nplanes > 30)
        fprintf(stderr,"Expecting core dump...\n");

    /* insert length of compressed cube */
    if (nplanes < 7) {
        out[0] = 32*nplanes + noverflows;
        bitptr = out+1;
        cube_length = 1;
    }
    else {
        out[0] = 224 + noverflows;
        out[1] = nplanes-7;
        bitptr = out+2;
        cube_length = 2;
    }
    ovrptr = bitptr + nplanes*32;
    cube_length += nplanes*32 + noverflows;

    /* now, compress */
    /* prepare bit planes and insert overflow bits... */
    while (nplanes>=8) {
        for (i=0; i<256; i++) {
            *bitptr++ = in[i]&bitmask[8];
            in[i] = in[i]>>8;
        }
        nplanes -= 8;
    }

    if (nplanes > 0) {
        stmp = 0;
        nbits = 0;
        for (i=0; i<256; i++) {
            /* insert nplanes number of bits */
            stmp = (stmp << nplanes) + (in[i] & bitmask[nplanes]);
            nbits += nplanes;
            if (nbits > 7) {
                *bitptr++ = stmp >> (nbits - 8);
                nbits -= 8;
                stmp &= bitmask[nbits];
            }

            /* append overflows */
            noverflows = in[i] >> nplanes;
            for(j=0; j<noverflows; j++)
                *ovrptr++ = i;
        }
    }
    else { /* just do overflows */
        for (i=0; i<256; i++) {
            for(j=0; j<(int)in[i]; j++) {
                *ovrptr++ = i;
            }
        }
    }

    return cube_length;
}

void decompress3d (unsigned char in[1024], unsigned int out[256])
{
    static int bitmask[] = { 0,1,3,7,15,31,63,127,255,511,1023,2047,
                            4095,8191,16383,32767 };
    int nplanes, noverflows, nbits, savenplanes;
    unsigned char *bitptr;
    int i, j, t;

    nplanes = in[0] >> 5;
    noverflows = in[0] & 31;
    if (nplanes == 7) {
        nplanes += in[1];
        bitptr = in+2;
    }
    else {
        bitptr = in+1;
    }
    /* printf("%d %d %d\n",nplanes,noverflows,*bitptr); */

    /* extract bit planes */
    savenplanes = nplanes;
    for (i=0; i<256; i++)
        out[i] = 0;
    j = 0;
    while (nplanes>=8) {
        for (i=0; i<256; i++)
            out[i] += ((unsigned int)*bitptr++)<<j;
        nplanes -= 8;
        j += 8;
    }

    if (nplanes > 0) {
        nbits = 0;
        for (i=0; i<256; i++) {
            if (nbits+nplanes < 9) {
                out[i] += ((*bitptr >> (8-nbits-nplanes)) & bitmask[nplanes])<<j;
                nbits += nplanes;
                if (nbits > 7) {
                    bitptr++;
                    nbits -= 8;
                }
            }
            else {
                t = nplanes-8+nbits;
                out[i] += (((*bitptr & bitmask[8-nbits]) << t)
                           + (*(bitptr+1) >> (8-t)))<<j;
                bitptr++;
                nbits = t;
            }
        }
    }

    /* extract overflows */
    for (i=0; i<noverflows; i++)
        out[*bitptr++] += 1 << savenplanes;
}

void gen_binning(int *nclook, int *lookmin, int *lookmax, short *looktab, int lookdim, int rangemax, float cfactor) {

    *nclook  = rangemax*cfactor;
    *lookmin = 1;
    *lookmax = rangemax;
    for(short i=0 ; i<lookdim ; i++) looktab[i] = i+1;

//    float wid_spec[16384];
//    float wa = 3.0f, wb = 1.0f, wc = 4.0f, chs = 2.0f;
//    float w, x, f1, f2, f3,  wa2, wb2, wc2;
//    int   iclo = 200, ichi = 5000;
//    int   i1, i2, i3, hi, nc, i, j, ich2;
//    int rl= 0;

//    cfactor = 2;

//    *nclook = rangemax*cfactor;

//    iclo = 0;
//    ichi = rangemax-1;

//    // channels per FWHM
//    chs = 1;

//    // fixed binning
//    wa2=1.;
//    wb2=0.;
//    wc2=0.;

//    for(int i=0 ; i<16384 ; i++) looktab[i] = 0;

//    char  buf[32];

//    int lo = iclo;
//    ich2 = 0;
//    while (lo <= ichi) {
//        ++ich2;
//        x = ((float) lo / cfactor + 0.5f) / 1e3f;
//        w = sqrt(wa2 + wb2 * x + wc2 * x * x) * cfactor;
//        int incr = (int) ((w / chs) + 0.5);
//        if(incr==0) incr = 1;
//        lo += incr;
//    }

//    cout << "No. of chs. in cube = " << ich2 << endl;
//    cin.get();

//    for (j = 0; j < *nclook; ++j) {
//        looktab[j] = 0;
//    }
//    lo = iclo;
//    for (i = 1; i < ich2+1; ++i) {
//        x = ((float) lo + 0.5f) / 1e3f;
//        w = sqrt(wa2 + wb2 * x/cfactor + wc2 * x/cfactor * x/cfactor) * cfactor;
//        hi = lo + (int) ((w / chs) + 0.5);
//        if (hi > *nclook) hi = *nclook;
//        for (j = lo; j < hi; ++j) {
//            looktab[j] = i;
//        }
//        wid_spec[i - 1] = (float) (hi - lo)/cfactor;
//        if ((lo = hi) == *nclook) break;
//    }

//    *lookmin = 1;
//    *lookmax = ich2;
}

/* ======================================================================= */
FILE *open_new_file(TString &filename, int /*force_open*/)
{
    /* safely open a new file
     filename: name of file to be opened
     force_open = 0 : allow return value NULL for no file opened
     force_open = 1 : require that a file be opened */

    FILE *file = NULL;
    if ((file = fopen(filename, "w+"))) return file;
    file_error("open or overwrite", filename);
    return nullptr;
} /* open_new_file */

/* ======================================================================= */
int put_file_rec(FILE *fd, void *data, int numbytes)
{
    /* write one fortran-unformatted style binary record into data */
    /* returns 1 for error */

#ifdef VMS  /* vms */
    int   j1;
    short rh[2];
    char  *buf;

    buf = data;
    j1 = numbytes;
    if (numbytes <= 2042) {
        rh[0] = numbytes + 2; rh[1] = 3;
    } else {
        rh[0] = 2044; rh[1] = 1;
        while (j1 > 2042) {
            if (fwrite(rh, 2, 2, fd) != 2 ||
                    fwrite(buf, 2042, 1, fd) != 1) return 1;
            rh[1] = 0; j1 -= 2042; buf += 2042;
        }
        rh[0] = j1 + 2; rh[1] = 2;
    }
    if (fwrite(rh, 2, 2, fd) != 2 ||
            fwrite(buf, j1, 1, fd) != 1) return 1;
    /* if numbytes is odd, write an extra (padding) byte */
    if (2*(numbytes>>1) != numbytes) {
        j1 = 0;
        fwrite(&j1, 1, 1, fd);
    }

#else /* unix */

    if (fwrite(&numbytes, 4, 1, fd) != 1 ||
            fwrite(data, numbytes, 1, fd) != 1 ||
            fwrite(&numbytes, 4, 1, fd) != 1) return 1;
#endif
    return 0;
} /*put_file_rec */

/* ======================================================================= */
int wspec(TString &filnam, float *spec, int idim)
{
    /* write spectra in gf3 format
     filnam = name of file to be created and written
     spec = spectrum of length idim */

    char buf[32];
    int  c1 = 1, rl = 0;
    char namesp[8];
    FILE *file;

    if(!filnam.EndsWith(".spe")) filnam.Append(".spe");

    if (!(file = open_new_file(filnam, 0))) return 1;
    strncpy(namesp, filnam.Data(), 8);
    if (filnam.Length()-4 < 8) memset(&namesp[filnam.Length()-4], ' ', 8-(filnam.Length()-4));

    /* WRITE(1) NAMESP,IDIM,1,1,1 */
    /* WRITE(1) SPEC */
#define W(a,b) { memcpy(buf + rl, a, b); rl += b; }
    W(namesp,8); W(&idim,4); W(&c1,4); W(&c1,4); W(&c1,4);
#undef W
    if (put_file_rec(file, buf, rl) ||
            put_file_rec(file, spec, 4*idim)) {
        file_error("write to", filnam);
        fclose(file);
        return 1;
    }
    fclose(file);
    return 0;
} /* wspec */

/* ====================================================================== */
int setext(char *filnam, const char *cext, int filnam_len)
{
    /* set default extension of filename filnam to cext
     leading spaces are first removed from filnam
     if extension is present, it is left unchanged
     if no extension is present, cext is used
     returned value pointer to the dot of the .ext
     cext should include the dot plus a three-letter extension */

    int nc, iext;

    /* remove leading spaces from filnam */
    nc = strlen(filnam);
    if (nc > filnam_len) nc = filnam_len;
    while (nc > 0 && filnam[0] == ' ') {
        memmove(filnam, filnam+1, nc--);
        filnam[nc] = '\0';
    }
    /* remove trailing spaces from filnam */
    while (nc > 0 && filnam[nc-1] == ' ') {
        filnam[--nc] = '\0';
    }
    /* look for file extension in filnam
     if there is none, put it to cext */
    iext = 0;
    if (nc > 0) {
        for (iext = nc-1;
             (iext > 0 &&
              filnam[iext] != ']' &&
              filnam[iext] != '/' &&
              filnam[iext] != ':');
             iext--) {
            if (filnam[iext] == '.') return iext;
        }
        iext = nc;
    }
    strncpy(&filnam[iext], cext, filnam_len - iext - 1);
    return iext;
} /* setext */


/* ======================================================================= */
int inq_file(char *filename, int *reclen)
{
    /* inquire for file existence and record length in longwords
     returns 0 for file not exists, 1 for file exists */

    int  ext = 0;
    char jfile[800];
    struct stat statbuf;

    *reclen = 0;
    if (stat(filename, &statbuf)) return 0;

    strncpy(jfile, filename, 800);
    ext = setext(jfile, "    ", 800);
    if (!strcmp(&jfile[ext], ".mat") ||
            !strcmp(&jfile[ext], ".MAT") ||
            !strcmp(&jfile[ext], ".esc") ||
            !strcmp(&jfile[ext], ".ESC")) {
        *reclen = 2048;
    } else if (!strcmp(&jfile[ext], ".spn") ||
               !strcmp(&jfile[ext], ".SPN") ||
               !strcmp(&jfile[ext], ".m4b") ||
               !strcmp(&jfile[ext], ".M4B") ||
               !strcmp(&jfile[ext], ".e4k") ||
               !strcmp(&jfile[ext], ".E4K")) {
        *reclen = 4096;
    } else if (!strcmp(&jfile[ext], ".cub") ||
               !strcmp(&jfile[ext], ".CUB")) {
        *reclen = 256;
    } else if (!strcmp(&jfile[ext], ".2dp") ||
               !strcmp(&jfile[ext], ".2DP")) {
        if (statbuf.st_size <= 0) {
            *reclen = 0;
        } else {
            *reclen = (int) (0.5 + sqrt((float) (statbuf.st_size/4)));
        }
    }
    return 1;
} /* inq_file */

