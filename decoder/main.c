
/******************************************************************

    iLBC Speech Coder ANSI-C Source Code

    iLBC_test.c

    Copyright (C) The Internet Society (2004).
    All Rights Reserved.

 ******************************************************************/

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "iLBC_define.h"
#include "iLBC_encode.h"
#include "iLBC_decode.h"

/* Runtime statistics */
#include <time.h>

#define ILBCNOOFWORDS_MAX   (NO_OF_BYTES_30MS/2)

/*----------------------------------------------------------------*
 *  Decoder interface function
 *---------------------------------------------------------------*/

short decode(/* (o) Number of decoded samples */
        iLBC_Dec_Inst_t *iLBCdec_inst, /* (i/o) Decoder instance */
        short *decoded_data, /* (o) Decoded signal block*/
        short *encoded_data, /* (i) Encoded bytes */
        short mode /* (i) 0=PL, 1=Normal */
        ) {
    int k;
    float decblock[BLOCKL_MAX], dtmp;

    /* check if mode is valid */

    if (mode < 0 || mode > 1) {
        printf("\nERROR - Wrong mode - 0, 1 allowed\n");
        exit(3);
    }

    /* do actual decoding of block */

    iLBC_decode(decblock, (unsigned char *) encoded_data,
            iLBCdec_inst, mode);

    /* convert to short */



    for (k = 0; k < iLBCdec_inst->blockl; k++) {
        dtmp = decblock[k];

        if (dtmp < MIN_SAMPLE)
            dtmp = MIN_SAMPLE;
        else if (dtmp > MAX_SAMPLE)
            dtmp = MAX_SAMPLE;
        decoded_data[k] = (short) dtmp;
    }

    return (iLBCdec_inst->blockl);
}

/*---------------------------------------------------------------*
 *  Main program to test iLBC encoding and decoding
 *
 *  Usage:
 *    exefile_name.exe <infile> <bytefile> <outfile> <channel>
 *
 *    <infile>   : Input file, speech for encoder (16-bit pcm file)
 *    <bytefile> : Bit stream output from the encoder
 *    <outfile>  : Output file, decoded speech (16-bit pcm file)
 *    <channel>  : Bit error file, optional (16-bit)
 *                     1 - Packet received correctly
 *                     0 - Packet Lost
 *
 *--------------------------------------------------------------*/

int main(int argc, char* argv[]) {

    FILE *efileid, *ofileid, *cfileid;
    short data[BLOCKL_MAX];
    short encoded_data[ILBCNOOFWORDS_MAX]={0};
    short decoded_data[BLOCKL_MAX];
    int len;
    short pli, mode;
    int blockcount = 0;
    /* Create structs */
    iLBC_Enc_Inst_t Enc_Inst;
    iLBC_Dec_Inst_t Dec_Inst;
    int i = 0;


    /* get arguments and open files */

    mode = 30;

    efileid = stdin; 
    ofileid = stdout;
    cfileid = NULL;

    /* Initialization */

    initDecode(&Dec_Inst, mode, 1);

    /* loop over input blocks */
    while (fread(encoded_data, sizeof (short),ILBCNOOFWORDS_MAX, efileid) ==
            ILBCNOOFWORDS_MAX) {

        pli = 1;

        blockcount += 1;
        /* decoding */

        fprintf(stderr, "--- Decoding block %i ---\n ", blockcount);
        len = decode(&Dec_Inst, decoded_data, encoded_data, pli);
        /* write output file */
        fwrite(decoded_data, sizeof (short), len, ofileid);
        fflush(stderr);
    }

    /* close files */
    fclose(efileid);
    fclose(ofileid);

    return (0);
}

