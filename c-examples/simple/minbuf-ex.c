/*
 * c_examples/simple/minbuf_ex.c - an example of how to call C ASN.1-BER
 *               encoders and decoders generated by snacc
 *               using the MinBuf buffer.
 *
 * AUTHOR: Mike Sample
 * DATE:   Mar 92
 *
 * $Header: /baseline/SNACC/c-examples/simple/minbuf-ex.c,v 1.2 2003/12/17 19:05:03 gronej Exp $
 * $Log: minbuf-ex.c,v $
 * Revision 1.2  2003/12/17 19:05:03  gronej
 * SNACC baseline merged with PER v1_7 tag
 *
 * Revision 1.1.2.1  2003/11/05 14:58:57  gronej
 * working PER code merged with esnacc_1_6
 *
 * Revision 1.1.1.1  2000/08/21 20:36:07  leonberp
 * First CVS Version of SNACC.
 *
 * Revision 1.5  1995/07/24  20:46:59  rj
 * changed `_' to `-' in file names.
 *
 * Revision 1.4  1995/02/18  15:12:55  rj
 * cosmetic changes
 *
 * Revision 1.3  1994/09/01  01:02:38  rj
 * more portable .h file inclusion.
 *
 * Revision 1.2  1994/08/31  08:59:36  rj
 * first check-in. for a list of changes to the snacc-1.1 distribution please refer to the ChangeLog.
 *
 */

#include "snacc.h"
#include "asn-incl.h"

#include <sys/file.h>
#include <sys/stat.h>
#if HAVE_FCNTL_H 
#include <fcntl.h>
#endif
#include <stdio.h>
#include <unistd.h>

#include "p-rec.h"
#include "min-buf.h"

int
main PARAMS ((argc, argv),
    int argc _AND_
    char *argv[])
{
    int fd;
    GenBuf buf, encBuf;
    char *encData;
    int encBufSize;
    AsnLen decodedLen;
    int     val;
    PersonnelRecord pr;
    int      size;
    char    *origData;
    struct stat sbuf;
    jmp_buf env;
    int  decodeErr;
    char *filename;

    if (argc != 2) {
        filename = "pr.ber";
    } else {
        filename = argv[1];
    }

    fd = open(filename, O_RDONLY, 0);
    if (fd < 0) {
        fprintf(stderr, "Usage: %s <BER data file name>\n", argv[0]);
        fprintf(stderr, "   Decodes the given PersonnelRecord BER data "
                "file\n");
        fprintf(stderr, "   and re-encodes it to stdout\n");
        exit(1);
    }

    if (fstat(fd, &sbuf) < 0) {
        perror("main: fstat");
        exit(1);
    }

    size = sbuf.st_size;
    origData = (char*)malloc(size);
    if (read(fd, origData, size) != size) {
        perror("main: read");
        exit(1);
    }

    close(fd);

    /* set up min buf  */
    GenBufFromMinBuf(&buf, origData);

    decodedLen = 0;
    decodeErr = FALSE;
    if ((val = setjmp(env)) == 0) {
        BDecPersonnelRecord(&buf, &pr, &decodedLen, env);
    } else {
        decodeErr = TRUE;
        fprintf(stderr, "ERROR - Decode routines returned %d\n", val);
    }

    if (decodeErr)
        exit(1);

    fprintf(stderr, "decodedValue PersonnelRecord ::= ");
    PrintPersonnelRecord(stderr, &pr, 0);
    fprintf(stderr, "\n\n");

    /*
     * setup a new buffer set up for writing.
     * make sure size is big enough to hold the encoded
     * value (may be larger than decoded value if encoding
     * with indef lengths - so add 512 slush bytes)
     */
    encBufSize = size + 512;
    encData = (char*) malloc(encBufSize);

    /*
     * set 'buffer' up for writing by setting ptr
     * byte after last byte of the block
     */
    GenBufFromMinBuf(&encBuf, encData + encBufSize);
    (void)BEncPersonnelRecord(&encBuf, &pr);

    free(encData);
    free(origData);
    return 0;
}
