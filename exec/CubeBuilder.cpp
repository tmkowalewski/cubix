/***************************************************************************
   incub8r3: Replay program for compressed triple coincidence cubes.
   Allows use of non-linear gains through look-up tables
     to convert gain-matched ADC outputs to cube channels.
   Uses disk buffering to .scr "scratch" file.
   This version for 1/6 cubes.
   D.C Radford     Sept  1997
****************************************************************************/
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#ifdef VMS
#include <unixio.h>
#else
/* #include <sys/mtio.h> */
#endif
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <chrono>

#include "TTree.h"
#include "TString.h"
#include "TMath.h"
#include "TChain.h"
#include "TTreeFormula.h"

#include "CXRadInterface.h"
#include "CXBashColor.h"
#include "CXProgressBar.h"
#include "CXRadReader.h"

using namespace std;

int dbinfo[10];

/* lookup table stuff */
char *tbuf;           /* tape buffer */
short luch[16384];    /* look-up table, maps ADC channels to cube channels */
int lumx[RW_MAXCH];   /* look-up table, maps 3d ch to linear minicube */
int lumy[RW_MAXCH];
int lumz[RW_MAXCH];

/* look up minicube addr */
#define LUMC(x,y,z) lumx[x] + lumy[y] + lumz[z]
/* look up addr in minicube */
#define LUIMC(x,y,z) (x&7) + ((y&7)<<3) + ((z&3)<<6)

/* increment buffers */

unsigned short buf1[RW_LB1][RW_DB1];
unsigned short buf2[RW_LB2][RW_DB2][RW_DB1+1];

int nbuf1[RW_LB1];       /* number in each row of buf1 */
int nbuf2[RW_LB2];       /* number in each row of buf2 */
int scrptr[RW_LB2];      /* last record # written to scr file  for each row of buf2*/

FILE *open_3cube (const TString &name, int length, FILE *mesg);
FILE *open_scr (const TString &name, int length, FILE *mesg);

TString fCubeFileName="";
TString fTabFile="";
Int_t fScratchSize = 512;
Float_t fCompressionFactor=1;
Int_t fCubeSize=4096;

TString fTreeFileName="";
TString fTreeName="";
TString fEGammaName="";
TString fGammaMultName="";
TString fCut="";

ULong64_t fNEvents=0;
ULong64_t fCurrentEntry = 0;
ULong64_t fNumberOfFilledEvents=0;
TChain *fChain = nullptr;
TBranch *b_EGamma = nullptr;
TBranch *b_MultGamma = nullptr;

Int_t fGammaMult;
Float_t fEGamma[500];

TTreeFormula *fTreeFormula = nullptr;

GlobalData gd;

/***************************************************************************
   setup_replay: reads in data file that describes cube, opens and initializes cube, scr and log
*/
int setup_replay(void)
{
    char cbuf[256];
    TString title, tnam, snam, cnam;
    TString lnam, scrs;
    int  nclook, lmin, lmax, i, ovrtprompt = 1;

    cout<<"This is a replay program for compressed-format RadWare triple-coincidence cubes."<<endl;
    cout<<"Allows use of non-linear gains through look-up tables"<<endl;
    cout<<"to convert gain-matched ADC outputs to cube channels."<<endl;
    cout<<"This version is for 1/6 cubes."<<endl;
    cout<<"Author       D.C Radford  Sept 1997"<<endl;
    cout<<"Sub-author   J.Dudouet    Jan  2019"<<endl;

    if(fCubeFileName.EndsWith(".cub")) fCubeFileName.ReplaceAll(".cub","");

    title = fCubeFileName;
    snam = fCubeFileName.Copy().Append(".scr");
    cnam = fCubeFileName.Copy().Append(".cub");
    tnam = fCubeFileName.Copy().Append(".log");
    lnam = fTabFile.Data();
    scrs = TString::Itoa(fScratchSize,10);

    gd.SwapTapeBytes = 0;
    gd.ScrSize = fScratchSize;

    cout << " -- Scratch file name          = " << snam << endl;
    cout << " -- Cube file name             = " << cnam << endl;
    cout << " -- Look-up table file name    = " << lnam << endl;
    cout << " -- Scratch File Size (MBytes) = " << scrs << endl;

    /* test values */
    if (gd.ScrSize < 1) {
        gbash_color->ErrorMessage("scratch file size too small");
        return 1;
    }

    if ((gd.LogFile=fopen(tnam,"r"))) {
        fclose(gd.LogFile);
        if (ovrtprompt) {
            gbash_color->WarningMessage(Form("Log file (%s) already exists",tnam.Data()));
            gbash_color->InfoMessage("Enter new name or <Return> to overwrite:");
            if (fgets(cbuf,256,stdin) && cbuf[0] != '\n') {
                cbuf[strlen(cbuf)-1]=0;
                tnam = cbuf;
                if(!tnam.EndsWith(".log")) tnam.Append(".log");
                return 10;
            }
        }
        else
            gbash_color->InfoMessage(Form("  Overwriting Log file (%s).",tnam.Data()));
    }
    if (!(gd.LogFile=fopen(tnam,"w"))) {
        gbash_color->ErrorMessage(Form("Could not open %s for writing.",tnam.Data()));
        gbash_color->InfoMessage("Enter new log file name: ");
        if (fgets(cbuf,256,stdin) && cbuf[0] != '\n') {
            cbuf[strlen(cbuf)-1]=0;
            tnam = cbuf;
            if(!tnam.EndsWith(".log")) tnam.Append(".log");
            return 11;
        }
    }

    /* read in lookup table */
    // if not tab given, assuming a 1keV/channel binning
    if(lnam=="") {
        gen_binning(&nclook,&lmin,&lmax,luch,16384,fCubeSize*fCompressionFactor,1);
    }
    else if (read_tab_file(lnam,&nclook,&lmin,&lmax,luch,16384)) {
        fclose(gd.LogFile);
        gbash_color->ErrorMessage("error reading lookup table");
        return 2;
    }
    gd.adcmax = nclook;
    gbash_color->InfoMessage(Form("No. of values in lookup table = %d",lmax));
    if (lmax > RW_MAXCH) {
        gbash_color->ErrorMessage(Form("number of values in lookup file is too large (max = %d)",RW_MAXCH));
        fclose(gd.LogFile);
        return 3;
    }
    gd.length = lmax;

    /* Inform the log file */
    fprintf(gd.LogFile,"Replay log for program Incub8r3,  %s\n\n",datim());
    fprintf(gd.LogFile,"\n              Scratch file name = %s\n"
                       "                  Cube file name = %s\n"
                       "         Look-up table file name = %s\n"
                       "      Scratch File Size (MBytes) = %s\n",
            snam.Data(),cnam.Data(),lnam.Data(),scrs.Data());
    fprintf(gd.LogFile," No. of values in lookup table = %d\n\n",lmax);
    fprintf(gd.LogFile,"-------------------------------------------\n\n");

    /* calculate look-up tables to convert (x,y,z) cube address
          to linearized minicube address.  x<y<z
       lum{x,y,z}[ch] returns the number of subcubes to skip over
          to get to the subcube with ch in it.
    */
    for (i=0;i<8;i++) {
        lumx[i] = 0;
        lumy[i] = 0;
        lumz[i] = i/4;
    }
    for (i=8;i<gd.length;i++) {
        lumx[i] = (i/8)*2;
        lumy[i] = lumy[i-8] + lumx[i];
        lumz[i] = lumz[i-8] + lumy[i];
    }

    /* open cube and scratch */
    if (!(gd.CubeFile=open_3cube(cnam, gd.length, gd.LogFile))) {
        fclose(gd.LogFile);
        return 4;
    }
    if (!(gd.ScrFile = open_scr(snam, gd.ScrSize, gd.LogFile))) {
        fclose(gd.CubeFile);
        fclose(gd.LogFile);
        return 5;
    }

    /* clear buf1 and buf2 */
    for(i=0;i<RW_LB1;i++)
        nbuf1[i] = 0;

    for(i=0;i<RW_LB2;i++) {
        nbuf2[i] = 0;
        scrptr[i] = 0;
    }
    fflush(gd.LogFile);

    return 0;
}

/***************************************************************************
   open_3cube: returns file descriptor on success or NULL on failure
     name - file name on disk
     length - number of channels on an axis
     mesg - stream to print error messages (in addition to stdout)
*/
FILE *open_3cube (const TString &name, int length, FILE *mesg)
{
    FILE     *file;
    int      i, nummc;
    FHead3D  head;
    Record3D rec;

    if (!name) return NULL;
    if (!(file=fopen(name,"r+"))) {
        if (!(file=fopen(name,"w+"))) {
            gbash_color->ErrorMessage("Cannot open file %s.\n", name.Data());
            return NULL;
        }
    }
    rewind(file);

    nummc = (length + 7)/8;
    nummc = nummc*(nummc+1)*(nummc+2)/3; /* minicubes in cube */
    gd.nummc = nummc;

    switch (fread(&head,1,1024,file)) {
    default:
        fprintf(stderr,"  WARNING -- partial header on cube. Junking it.\n");
    case 0:
        gd.numrecs = (nummc + 4087)/4088; /* note, an empty minicube is 1 byte */
        strncpy(head.id, "Incub8r3/Pro4d  ", 16);
        head.numch = length;
        head.bpc = 4;
        head.cps = 6;
        head.numrecs = gd.numrecs;
        memset (head.resv, 0, 992);

        fprintf(mesg,"  Creating new cube: %d channels per side...\n",length);
        printf("  Creating new cube: %d channels per side...\n",length);
        rewind(file);
        if (!fwrite(&head,1024,1,file)) {
            fprintf(stderr, "\007  ERROR -- Could not write cube header... aborting.\n");
            fclose(file);
            return NULL;
        }
        /* initialize cube */
        for (i=0;i<4088;i++)
            rec.d[i] = 0;

        for (i=0; i<gd.numrecs; i++) { /* loop through records */
            rec.h.minmc = i*4088;
            rec.h.nummc = 4088;
            rec.h.offset = 8;
            if(!fwrite(&rec,4096,1,file)) {
                fprintf(stderr, "\007  ERROR -- Could not write record %d... "
                                "aborting.\n",i);
                fclose(file);
                return NULL;
            }
        }
        fflush(file);
        printf("  ...Done creating new cube.\n");
        break;

    case 1024:
        fprintf(mesg,"  Checking existing cube file...\n");
        printf("  Checking existing cube file...\n");
        if (strncmp(head.id,"Incub8r3/Pro4d  ",16) ||
                head.bpc != 4 ||
                head.cps != 6) {
            fprintf(stderr,"\007  ERROR -- Invalid header... aborting.\n");
            fclose(file);
            return NULL;
        }
        else if (head.numch != length) {
            fprintf(stderr,"\007  ERROR -- Different axis length in cube (%d)..\n",
                    head.numch);
            fclose(file);
            return NULL;
        }
        gd.numrecs = head.numrecs;
        printf("  ...Okay, %d records.\n", gd.numrecs);
    }

    fprintf(mesg,
            "Axis length of 3d cube is %d.\n"
            "3d cube has %d minicubes and %d records.\n",
            length, nummc, head.numrecs);

    printf("Axis length of 3d cube is %d.\n"
           "3d cube has %d minicubes and %d records.\n",
           length, nummc, head.numrecs);

    gd.CubeFileName= name;
    return file;
}

/***************************************************************************
   open_scr: returns file descriptor on success or NULL on failure
     name - file name on disk
     length - size on Megabytes of scr file
     mesg - stream to print error messages (in addition to stdout)
*/
FILE *open_scr (const TString &name, int length, FILE *mesg)
{
    FILE *file;
    int b[256], i;

    if(!name || length < 1 || !(file=fopen(name,"w+")))
        return NULL;
    rewind(file);

    fprintf(mesg,"  Making sure we have enough scratch space...\n");
    printf("  Making sure we have enough scratch space...\n");
    for (i=0;i<length*1024;i++) /* writing to make sure we have space */
        if (!fwrite(b, 1024, 1, file)) {
            fprintf(stderr,"\007  ERROR -- Could not allocate scratch space on disk"
                           "(%dM).\n",length);
            fclose(file);
            return NULL;
        }

    gd.numscr = 0;
    rewind(file);
    fprintf(mesg,"  ...Done.\n");
    printf("  ...Done.\n");

    return file;
}

void dscr(void)
{
    int            numchunks, chunknum;
    Record3D       recIn, recOut;
    int            j, k, mc, mcl;
    unsigned char  *mcptrIn, *mcptrOut, cbuf[2048];
    int            minmc, maxmc;
    int            recnumIn, recnumOut, nbytes;
    int            addr13b, addr8b, addr21b;
    int            nmcperc=RW_CHNK_S*256;
    unsigned short inbuf[RW_DB2+1][RW_DB1+1];
    FHead3D        filehead;
    unsigned int   *chunk;
    FILE           *OutFile;
    TString        filname;

    while (!(chunk=(unsigned int*)malloc(RW_CHNK_S*256*1024))) {
        printf("Ooops, chunk malloc failed  ... please free up some memory and press return.\n");
        cin.get();
    }

    /* open new output file for incremented copy of cube */
    filname = gd.CubeFileName + ".tmp-increment-copy";
    if (!(OutFile=fopen(filname,"w+"))) {
        printf("\007  ERROR -- Cannot open new output cube file.\n");
        exit (-1);
    }
    fseek (gd.CubeFile, 0, SEEK_SET);
    if (!fread (&filehead, 1024, 1, gd.CubeFile)) {
        printf("\007  ERROR -- Cannot read file header, aborting...\n");
        exit(-1);
    }
    fseek (OutFile, 0, SEEK_SET);
    if (!fwrite (&filehead, 1024, 1, OutFile)) {
        printf("\007  ERROR -- Cannot write file header, aborting...\n");
        exit(-1);
    }

    /* a chunk is RW_CHNK_S/16 Mchannels, nmcperc minicubes */
    numchunks = (gd.nummc+nmcperc-1)/nmcperc;

    fprintf(gd.LogFile," ...Updating cube from scratch file:  %s\n", datim());
    fflush(gd.LogFile);
    printf( " ...Updating cube from scratch file:  %s\n", datim());
    printf("There are %d chunks to increment...\n",numchunks);

    CXProgressBar progress(numchunks);

    /* read in first record */
    recnumIn = 0;
    fseek(gd.CubeFile, 1024, SEEK_SET);
    if (!fread(&recIn, 4096, 1, gd.CubeFile)) {
        printf("\007  ERROR -- Corrupted cube, aborting...\n");
        exit(-1);
    }
    mcptrIn = recIn.d;

    /* init the first output record */
    recnumOut = 0;
    recOut.h.minmc = recIn.h.minmc;
    recOut.h.offset = 8;
    mcptrOut = recOut.d;

    /* loop through all the chunks in the file */
    for (chunknum=0; chunknum<numchunks; chunknum++) {

        minmc = chunknum*nmcperc;
        maxmc = minmc+nmcperc-1;
        if (maxmc>gd.nummc-1)
            maxmc = gd.nummc - 1;
        dbinfo[0]=11;
        dbinfo[2]=0;
        dbinfo[3]=0;

        if(gd.gotsignal) exit(-1);

//        printf("\r  ...chunk %d, recs %d %d   ",chunknum,recnumIn,recnumOut);
//        fflush(stdout);
        ++progress;
        /* loop through all the minicubes in the chunk */
        for (mc=minmc; mc<=maxmc; mc++) {
            dbinfo[2]=mc;

            if (mc > recIn.h.minmc + recIn.h.nummc - 1) {
                /* next compressed minicube starts in the next input record */
                if (!fread (&recIn, 4096, 1, gd.CubeFile)) {
                    printf("\007  ERROR -- Corrupted cube, aborting...\n");
                    exit(-1);
                }
                recnumIn++;
                mcptrIn = recIn.d + recIn.h.offset - 8;
                /* at this point our minicube should be at the start of the record */
                if (recIn.h.minmc != mc) {
                    printf("Severe ERROR 1 - fatal!\n PLEASE send a bug report"
                           " to radfordd@mail.phy,ornl.gov with this information\n"
                           "  and a copy of your get_event.c and incub8r3.h.\n");
                    printf("rec: %d  mc: %d  should be %d\n",
                           recnumIn, mc, recIn.h.minmc);
                    exit(-1);
                }
            }
            else if (mcptrIn > recIn.d + 4088 ) {
                printf("Severe ERROR 2 - fatal!\n PLEASE send a bug report"
                       " to radfordd@mail.phy,ornl.gov with this information\n"
                       "  and a copy of your get_event.c and incub8r3.h.\n");
                printf("rec: %d  mc: %d  should be > %d\n",
                       recnumIn, mc, recIn.h.minmc + recIn.h.nummc - 1);
                exit(-1);
            }

            mcl=MCLEN(mcptrIn);
            if (mcptrIn + mcl > recIn.d + 4088 ) {
                /* compressed minicube spills over into the next input record */
                nbytes = mcptrIn + mcl - (recIn.d + 4088);
                memcpy (cbuf, mcptrIn, mcl-nbytes);
                if (!fread (&recIn, 4096, 1, gd.CubeFile)) {
                    printf("\007  ERROR -- Corrupted cube, aborting...\n");
                    exit(-1);
                }
                if (nbytes != (recIn.h.offset-8)) {
                    printf("Severe ERROR 3 - fatal!\n PLEASE send a bug report"
                           " to radfordd@mail.phy,ornl.gov with this information\n"
                           "  and a copy of your get_event.c and incub8r3.h.\n");
                    printf("rec, offset, nbytes, mcl, mcptr: %d %d %d %d %ld\n"
                           "mc minmc nummc: %d %d %d\n",
                           recnumIn+1, recIn.h.offset, nbytes ,mcl,
                           (long int) (mcptrIn-recIn.d+8),
                           mc, recIn.h.minmc, recIn.h.nummc);
                    exit(-1);
                }
                if (recIn.h.minmc != mc+1) {
                    printf("Severe ERROR 4 - fatal!\n PLEASE send a bug report"
                           " to radfordd@mail.phy,ornl.gov with this information\n"
                           "  and a copy of your get_event.c and incub8r3.h.\n");
                    printf("rec: %d  mc: %d  should be %d\n",
                           recnumIn+1, mc+1, recIn.h.minmc);
                    exit(-1);
                }
                recnumIn++;
                memcpy (&cbuf[mcl-nbytes], recIn.d, nbytes);
                decompress3d (cbuf, &chunk[(mc-minmc)<<8]);
                mcptrIn = recIn.d + recIn.h.offset - 8;
            }
            else {
                decompress3d (mcptrIn, &chunk[(mc-minmc)<<8]);
                mcptrIn += mcl;
            }
        }

        /* increment the chunk from the buffers */
        addr8b = chunknum;

        /* first empty the corresponding parts of buf1 */
        dbinfo[0]=12;
        dbinfo[2]=0;
        dbinfo[3]=0;
        for (addr13b=addr8b*RW_CHNK_S;
             addr13b<(addr8b+1)*RW_CHNK_S; addr13b++) {
            addr21b = (addr13b - addr8b*RW_CHNK_S)<<16;
            for (j=0; j<nbuf1[addr13b]; j++)
                chunk[addr21b+buf1[addr13b][j]]++;
            nbuf1[addr13b] = 0;
        }

        /* next empty the corresponding parts of buf2 */
        dbinfo[0]=13;
        for (j=0; j<nbuf2[addr8b]; j++) {
            addr21b = buf2[addr8b][j][RW_DB1]<<16;
            for (k=0; k<RW_DB1; k++)
                chunk[addr21b+buf2[addr8b][j][k]]++;
        }
        nbuf2[addr8b] = 0;

        /* increment the chunk from the scratch file */
        dbinfo[0]=14;
        while (scrptr[addr8b]>0) {
            fseek(gd.ScrFile, ((long)scrptr[addr8b]-1)*RW_SCR_RECL, SEEK_SET);
            if (!fread(inbuf, RW_SCR_RECL, 1, gd.ScrFile)) {
                fprintf(stderr,"\007  ERROR -- Could not read scr file.. fatal.\n");
                exit(-1);
            }
            for (j=0; j<RW_DB2; j++) {
                addr21b = inbuf[j][RW_DB1]<<16;
                for (k=0; k<RW_DB1; k++)
                    chunk[addr21b+inbuf[j][k]]++;
            }
            memcpy(&(scrptr[addr8b]), &inbuf[RW_DB2][0], 4);
        }

        /* recompress and rewrite the chunk */
        /* loop through all the minicubes in the chunk */
        dbinfo[0]=15;
        for (mc=minmc; mc<=maxmc; mc++) {
            dbinfo[2]=mc;
            mcl = compress3d (&chunk[(mc-minmc)<<8], cbuf);
            if (mcptrOut + mcl > recOut.d + 4088) {
                /* the minicube spills over the end of the output record */
                if (mcptrOut + 2 > recOut.d + 4088) {
                    /* need at least first 2 bytes of minicube in current record */
                    /* so move whole minicube to next record */
                    recOut.h.nummc = mc - recOut.h.minmc;
                    if (!fwrite(&recOut, 4096, 1, OutFile)) {
                        printf("\007  ERROR -- Cannot write cube, aborting...\n");
                        exit(-1);
                    }
                    recOut.h.minmc = mc;
                    recOut.h.offset = 8;
                    memcpy (recOut.d, cbuf, mcl);
                    mcptrOut = recOut.d + recOut.h.offset - 8 + mcl;
                }
                else {
                    /* move only part of minicube to next record */
                    nbytes = mcptrOut + mcl - (recOut.d + 4088);
                    memcpy (mcptrOut, cbuf, mcl-nbytes);
                    recOut.h.nummc = mc - recOut.h.minmc + 1;
                    if (!fwrite(&recOut, 4096, 1, OutFile)) {
                        printf("\007  ERROR -- Cannot write cube, aborting...\n");
                        exit(-1);
                    }
                    recOut.h.minmc = mc + 1;
                    recOut.h.offset = nbytes + 8;
                    memcpy (recOut.d, cbuf+mcl-nbytes, nbytes);
                    mcptrOut = recOut.d + recOut.h.offset - 8;
                }
                recnumOut++;
            }
            else {
                memcpy (mcptrOut, cbuf, mcl);
                mcptrOut += mcl;
            }
        }
    } /* end of loop through chunks in the file */

    /* write out the last record */
    dbinfo[0]=16;
    recOut.h.nummc = gd.nummc - recOut.h.minmc;
    if (!fwrite(&recOut, 4096, 1, OutFile)) {
        printf("\007  ERROR -- Cannot write cube, aborting...\n");
        exit(-1);
    }
    recnumOut++;

    fseek (OutFile, 0, SEEK_SET);
    if (!fread (&filehead, 1024, 1, OutFile)) {
        printf("\007  ERROR -- Corrupted file header, aborting...\n");
        exit(-1);
    }
    filehead.numrecs = recnumOut;
    fseek (OutFile, 0, SEEK_SET);
    if (!fwrite (&filehead, 1024, 1, OutFile)) {
        printf("\007  ERROR -- Cannot write file header, aborting...\n");
        exit(-1);
    }
    fflush(OutFile);

    free(chunk);
    fclose(gd.CubeFile);
    gd.CubeFile = OutFile;
    if (rename(filname, gd.CubeFileName)) {
        printf("\007  ERROR -- Cannot rename file, aborting...\n");
        exit(-1);
    }

    fprintf(gd.LogFile,"   ...Done updating cube:  %s\n"
                       "  There are now %d records.\n", datim(), recnumOut);
    fflush(gd.LogFile);
    printf("\n   ...Done updating cube:  %s\n"
           "  There are now %d records.\n", datim(), recnumOut);
    dbinfo[0]=17;
}

void dbuf2(int addr8b)
{
    char outbuf[RW_SCR_RECL];

    dbinfo[0]=8;
    dbinfo[4]=scrptr[addr8b];
    dbinfo[5]=gd.numscr;
    memcpy(outbuf, &(buf2[addr8b][0][0]), RW_DB2*2*(RW_DB1+1));
    memcpy(outbuf+RW_DB2*2*(RW_DB1+1), &(scrptr[addr8b]), 4);

    if (!fwrite(outbuf,RW_SCR_RECL,1,gd.ScrFile)) {
        fprintf(stderr,"\007  ERROR -- Could not write to scr file.. fatal.\n");
        exit(-1);
    }
    gd.numscr++;
    scrptr[addr8b] = gd.numscr;
    dbinfo[0]=9;

    if (gd.numscr*RW_SCR_RECL >= gd.ScrSize*1024*1024) {
        dbinfo[0]=10;
        /* scr full, inc cub */
        fprintf(gd.LogFile,"  Scratch File full...\n");
        printf("\n  Scratch File full...\n");
        dscr();
        gd.numscr = 0;
        rewind(gd.ScrFile);
    }
}

/*--------------------------------------------------------*/

void breakhandler(int dummy)
{
    gd.gotsignal = 1;
}

/*--------------------------------------------------------*/

void segvhandler(int dummy)
{
    char cbuf[256];

    gd.gotsignal = 1;
    printf("\007\n ACKKK!!!  Segmentation Violation!!!  Debug info follows.\n"
           "%d %d %d %d %d %d %d %d %d %d\n",
           dbinfo[0],dbinfo[1],dbinfo[2],dbinfo[3],dbinfo[4],
            dbinfo[5],dbinfo[6],dbinfo[7],dbinfo[8],dbinfo[9]);
    while (1) {
        printf("  ... Kill? (N/Y)\n");
        if (!fgets(cbuf,256,stdin)) continue;
        if (cbuf[0] == 'y' || cbuf[0] == 'Y')
            exit (1);
        if (cbuf[0] == 'n' || cbuf[0] == 'N')
            break;
    }
}

/*--------------------------------------------------------*/

void sighandler()
{
    /* See signal(2) for details: 2 = SIGINT, 11 = SIGSEGV */
    /* Maybe this'll be different on different OSs so watch out. */
    signal(SIGINT, breakhandler);
    signal(SIGSEGV, segvhandler);
    gd.gotsignal = 0;
}

/*--------------------------------------------------------*/

int ReadConfFile(TString ConfFileName){

    cout<<"\e[1;94m"<<endl;
    cout<<"******************************"<<endl;
    cout<<"* Reading Configuration File *"<<endl;
    cout<<"******************************"<<endl;
    cout<<"\e[0m"<<endl;

    std::ifstream FileConf;
    FileConf.open(ConfFileName.Data(), std::ifstream::in);

    if(!FileConf){
        cout<<"ERROR, "<<ConfFileName<<" not found ==> EXIT"<<endl;
        return 1;
    }

    string line;
    TString Buffer;

    while(FileConf)
    {
        getline(FileConf,line);
        Buffer = line;

        if(Buffer.BeginsWith("#")) continue;
        else if(Buffer.Copy().ReplaceAll(" ","").ReplaceAll("\t","") == "") continue;
        else if(Buffer.BeginsWith("CubeFileName")){
            TObjArray *loa=Buffer.Tokenize(" ");
            fCubeFileName = ((TString)loa->At(1)->GetName());
            delete loa;
            cout<<"\e[1;92mRaw Cube Filename: "<<fCubeFileName<<"\e[0m"<<endl;
        }
        else if(Buffer.BeginsWith("TabFile")){
            TObjArray *loa=Buffer.Tokenize(" ");
            fTabFile = ((TString)loa->At(1)->GetName());
            delete loa;
            cout<<"\e[1;92mRaw Tab file: "<<fTabFile<<"\e[0m"<<endl;
        }
        else if(Buffer.BeginsWith("ScratchSize")){
            TObjArray *loa=Buffer.Tokenize(" ");
            fScratchSize = ((TString)loa->At(1)->GetName()).Atoi();
            delete loa;
            cout<<"\e[1;92mRaw Scratch-file size (MB): "<<fScratchSize<<"\e[0m"<<endl;
        }
        else if(Buffer.BeginsWith("CompressionFactor")){
            TObjArray *loa=Buffer.Tokenize(" ");
            fCompressionFactor = ((TString)loa->At(1)->GetName()).Atoi();
            delete loa;
            cout<<"\e[1;92mRaw Compression factor: "<<fCompressionFactor<<"\e[0m"<<endl;
        }
        else if(Buffer.BeginsWith("CubeSize")){
            TObjArray *loa=Buffer.Tokenize(" ");
            fCubeSize = ((TString)loa->At(1)->GetName()).Atoi();
            delete loa;
            cout<<"\e[1;92mRaw Cube size: "<<fCubeSize<<"\e[0m"<<endl;
        }
        else if(Buffer.BeginsWith("TreeFile")){
            TObjArray *loa=Buffer.Tokenize(" ");
            fTreeFileName = ((TString)loa->At(1)->GetName());
            delete loa;
            cout<<"\e[1;92mRaw Input TTree(s): "<<fTreeFileName<<"\e[0m"<<endl;
        }
        else if(Buffer.BeginsWith("TreeName")){
            TObjArray *loa=Buffer.Tokenize(" ");
            fTreeName = ((TString)loa->At(1)->GetName());
            delete loa;
            cout<<"\e[1;92mRaw Tree name: "<<fTreeName<<"\e[0m"<<endl;
        }
        else if(Buffer.BeginsWith("NEvents")){
            TObjArray *loa=Buffer.Tokenize(" ");
            fNEvents = (Int_t)((TString)loa->At(1)->GetName()).Atof();
            delete loa;
            cout<<"\e[1;92mRaw NEvents to process: "<<fNEvents<<"\e[0m"<<endl;
        }
        else if(Buffer.BeginsWith("EGamma")){
            TObjArray *loa=Buffer.Tokenize(" ");
            fEGammaName = ((TString)loa->At(1)->GetName());
            delete loa;
            cout<<"\e[1;92mRaw EGamma branch: "<<fEGammaName<<"\e[0m"<<endl;
        }
        else if(Buffer.BeginsWith("GammaMult")){
            TObjArray *loa=Buffer.Tokenize(" ");
            fGammaMultName = ((TString)loa->At(1)->GetName());
            delete loa;
            cout<<"\e[1;92mRaw Gamma multiplicity branch: "<<fGammaMultName<<"\e[0m"<<endl;
        }
        else if(Buffer.BeginsWith("Cut")){
            fCut = Buffer.Copy().Remove(0,4);
            cout<<"\e[1;92mRaw Cut: "<<fCut<<"\e[0m"<<endl;
        }
        else{
            cout<<"\e[1;91mUnkown keyword: "<<Buffer<<"\e[0m"<<endl;
            cout<<endl;
            return 2;
        }
    }

    cout<<endl;

    return 0;
}

Int_t InitTree()
{
    fChain = new TChain(fTreeName);
    fChain->Add(fTreeFileName);
    if(!fChain->GetEntry(0)) return 1;

    fChain->SetBranchAddress(fGammaMultName, &fGammaMult, &b_MultGamma);
    fChain->SetBranchAddress(fEGammaName, fEGamma, &b_EGamma);

    fChain->SetBranchStatus("*",false);
    fChain->SetBranchStatus(fGammaMultName);
    fChain->SetBranchStatus(fEGammaName);

    if(fCut != "None") {
         for(int i=0 ; i<fChain->GetListOfBranches()->GetEntries() ; i++) {
             TString BranchName = fChain->GetListOfBranches()->At(i)->GetName();
             if(fCut.Contains(BranchName))
                 fChain->SetBranchStatus(BranchName,1);
         }

         fTreeFormula = new TTreeFormula("form01",fCut,fChain);
         fChain->SetNotify(fTreeFormula);
     }

     if(fNEvents==0) fNEvents = fChain->GetEntries();

     return 0;
}

/*--------------------------------------------------------*/

int FillFromTree(int *elist){

    fChain->GetEntry(fCurrentEntry);

    if(fGammaMult<3)
        return 0;

    if(fTreeFormula) {
         for(Int_t j = 0; j < fTreeFormula->GetNdata(); ++j)  {
             if ( fTreeFormula->EvalInstance(j) )  {
                 return 0;
             }
         }
     }

    for(int i=0 ; i<fGammaMult ; i++) {
        elist[i] = TMath::Nint(fEGamma[i]*fCompressionFactor);
//        cout<<i<<"/"<<fGammaMult<<": "<<elist[i]<<" keV"<<endl;
    }
//    cout<<endl;

    fNumberOfFilledEvents++;

    return fGammaMult;
}

/*--------------------------------------------------------*/

int main (int argc, char **argv)
{
    char cbuf[512];
    int mc_addr, inmc_addr, addr13b, addr16b, addr8b;
    int ex, ey, ez;
    int i,j,k,l,tmp;
    int elist[RW_MAXMULT], gemult;
    unsigned long numincr = 0;
    FILE *file;

    if(argc !=2){
        cout<<"You need to give as input the configuration file"<<endl;
        exit(EXIT_FAILURE);
    }

    int err = ReadConfFile(argv[1]);
    if(err) exit(EXIT_FAILURE);

    err = InitTree();
    if(err) exit(EXIT_FAILURE);

    Int_t ErrVal = setup_replay();
    if(ErrVal>0){
        cout<<"Error in the setup (err val: "<<ErrVal<<") ==> EXIT"<<endl;
        exit(EXIT_FAILURE);
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    sighandler();   /* set up signal handling */

    snprintf(cbuf,sizeof(cbuf)-1,"Scan started:  %s\n\n", datim());
    printf("%s",cbuf);
    fflush(stdin);
    fprintf(gd.LogFile,"%s",cbuf);
    fflush(gd.LogFile);

    gbash_color->InfoMessage("Input Tree reading to build the database...");
    cout<<flush;
    CXProgressBar progress(fNEvents);

    dbinfo[0]=1;
    while ((gemult = FillFromTree(elist)) >= 0) {

        if(fCurrentEntry==fNEvents)
            break;

        if (gd.gotsignal){
            break;
        }

        ++progress;

        dbinfo[0]=2;
        dbinfo[1]=gemult;
        fCurrentEntry++;
        dbinfo[8]=fCurrentEntry;
        if (gemult < 3) continue;

        /* convert from ADC to cube channel numbers */
        dbinfo[0]=3;
        j = 0;
        for (i=0; i<gemult; i++) {
            if (elist[i] >= 0 && elist[i] < gd.adcmax && luch[elist[i]]>0)
                elist[j++] = luch[elist[i]]-1;
        }
        gemult = j;
        if (gemult < 3) continue;

        dbinfo[0]=4;
        dbinfo[1]=gemult;

        /* order elist */
        for (i=gemult-1; i>0; i--) {
            if(elist[i] < elist[i-1]) {
                tmp=elist[i]; elist[i]=elist[i-1]; elist[i-1]=tmp;
                j=i;
                while (j<gemult-1 && elist[j] > elist[j+1]) {
                    tmp=elist[j]; elist[j]=elist[j+1]; elist[j+1]=tmp;
                    j++;
                }
            }
        }
        dbinfo[0]=5;

        /* loop through all possible combinations */
        for(l=2;l<gemult;l++) {
            ez = elist[l];

            for(k=1;k<l;k++) {
                ey = elist[k];

                for(j=0;j<k;j++) {
                    ex = elist[j];
                    numincr++;

                    /* linear address (LUMC first 21bits, LUIMC last 8bits) */
                    dbinfo[0]=6;
                    dbinfo[2]=0;
                    dbinfo[3]=0;
                    dbinfo[4]=0;
                    dbinfo[5]=0;
                    mc_addr = LUMC (ex,ey,ez);
                    inmc_addr = LUIMC (ex,ey,ez);
                    dbinfo[2]=mc_addr;
                    dbinfo[3]=inmc_addr;
                    addr13b = mc_addr / 256;
                    addr16b = (mc_addr&255)*256 + inmc_addr;
                    dbinfo[4]=addr13b;
                    dbinfo[5]=nbuf1[addr13b];
                    buf1[addr13b][nbuf1[addr13b]++] = addr16b;

                    if (nbuf1[addr13b] == RW_DB1) {  /* dump buf1 to buf2 */
                        dbinfo[0]=7;
                        dbinfo[6]=0;
                        dbinfo[7]=0;
                        addr8b = addr13b / RW_CHNK_S;
                        dbinfo[6]=addr8b;
                        dbinfo[7]=nbuf2[addr8b];
                        memcpy(&(buf2[addr8b][nbuf2[addr8b]][0]),
                                &(buf1[addr13b][0]),
                                RW_DB1*2); /* copy whole row into buf2 */
                        buf2[addr8b][nbuf2[addr8b]++][RW_DB1] =
                                (addr13b-(addr8b*RW_CHNK_S));
                        nbuf1[addr13b] = 0;

                        if (nbuf2[addr8b] == RW_DB2) {
                            dbuf2(addr8b);  /* dump buf2 to scratch file */
                            nbuf2[addr8b] = 0;
                        }
                    }
                    dbinfo[0]=5;
                }
            }
        }
        dbinfo[0]=1;
    }

    cout << endl;

    gbash_color->InfoMessage("Building the cube...");
    dscr();

    snprintf(cbuf,sizeof(cbuf)-1,"Scan completed:  %s\n"
                 "    Total number of read events   = %llu\n"
                 "    Total number of filled events = %llu\n"
                 "    Total number of increments    = %lu\n",
            datim(),fCurrentEntry,fNumberOfFilledEvents,numincr);
    printf("%s",cbuf);
    fprintf(gd.LogFile,"%s",cbuf);

    fflush(stdout);
    fclose(gd.LogFile);

    gbash_color->InfoMessage("Creating 1D and 2D cube projections:");

    CXRadReader *radreader = new CXRadReader;
    radreader->ReadCube(fCubeFileName.Copy().Append(".cub"));
    radreader->AutoBuildProj(fCubeFileName.Copy().Append(".cub"),3);

    gbash_color->InfoMessage("Writting cube info in: " + fCubeFileName.Copy().Append(".conf"));
    ofstream conffile(fCubeFileName.Copy().Append(".conf"));
    conffile << "TotalProj " << fCubeFileName.Copy().Append(".spe")<<endl;
    conffile << "2DProj " << fCubeFileName.Copy().Append(".2dp")<<endl;
    if(fTabFile.Length()) conffile << "LUT " << fTabFile << endl;
    if(fCompressionFactor!=1) conffile << "CompressFact " << fCompressionFactor<<endl;
    conffile.close();

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);

    gbash_color->InfoMessage("Cube: " + fCubeFileName.Copy().Append(".cub") + " created in " +  duration.count() + " seconds");

    exit(EXIT_SUCCESS);
}
