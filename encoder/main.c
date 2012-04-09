
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
 *  Encoder interface function






 *---------------------------------------------------------------*/

short encode(/* (o) Number of bytes encoded */
        iLBC_Enc_Inst_t *iLBCenc_inst,
        /* (i/o) Encoder instance */
        short *encoded_data, /* (o) The encoded bytes */
        short *data /* (i) The signal block to encode*/
        ) {
    float block[BLOCKL_MAX];
    int k;

    /* convert signal to float */

    for (k = 0; k < iLBCenc_inst->blockl; k++)
        block[k] = (float) data[k];

    /* do the actual encoding */

    iLBC_encode((unsigned char *) encoded_data, block, iLBCenc_inst);


    return (iLBCenc_inst->no_of_bytes);
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

    FILE *ifileid, *efileid, *cfileid;
    short data[BLOCKL_MAX];
    short encoded_data[ILBCNOOFWORDS_MAX], decoded_data[BLOCKL_MAX];
    int len;
    short pli, mode;
    int blockcount = 0;

    /* Create structs */
    iLBC_Enc_Inst_t Enc_Inst;

    mode = 30;
    ifileid = stdin;
    efileid = stdout;
    cfileid = NULL;

    fprintf(stderr, "\n");

    /* Initialization */

    initEncode(&Enc_Inst, mode);

    /* loop over input blocks */

    while (fread(data, sizeof (short), Enc_Inst.blockl, ifileid) ==
            Enc_Inst.blockl) {
        blockcount++;

        /* encoding */

        fprintf(stderr, "--- Encoding block %i --- ", blockcount);
        len = encode(&Enc_Inst, encoded_data, data);
        fprintf(stderr, "\r");

        /* write byte file */

        fwrite(encoded_data, sizeof (unsigned char), len, efileid);
        pli = 1;
	fflush(NULL);
    }
    /* close files */

    fclose(ifileid);
    fclose(efileid);
    return (0);
}


