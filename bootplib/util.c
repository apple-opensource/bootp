/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

/*
 * util.c
 * - contains miscellaneous routines
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/syslog.h>
#include <errno.h>
#include <mach/boolean.h>
#include <string.h>
#include <ctype.h>

#include "util.h"

/* 
 * Function: nbits_host
 * Purpose:
 *   Return the number of bits of host address 
 */
int
nbits_host(struct in_addr mask)
{
    u_long l = iptohl(mask);
    int i;

    for (i = 0; i < 32; i++) {
	if (l & (1 << i))
	    return (32 - i);
    }
    return (32);
}

/*
 * Function: inet_nettoa
 * Purpose:
 *   Turns a network address (expressed as an IP address and mask)
 *   into a string e.g. 17.202.40.0 and mask 255.255.252.0 yields
 *   the string  "17.202.40/22".
 */
char *
inet_nettoa(struct in_addr addr, struct in_addr mask)
{
    uint8_t *		addr_p;
    int 		nbits = nbits_host(mask);
    int 		nbytes;
    static char 	sbuf[32];
    char 		tmp[8];

#define NBITS_PER_BYTE	8    
    sbuf[0] = '\0';
    nbytes = (nbits + NBITS_PER_BYTE - 1) / NBITS_PER_BYTE;
//    printf("-- nbits %d, nbytes %d--", nbits, nbytes);
    for (addr_p = (uint8_t *)&addr.s_addr; nbytes > 0; addr_p++) {

	snprintf(tmp, sizeof(tmp), "%d%s", *addr_p, nbytes > 1 ? "." : "");
	strlcat(sbuf, tmp, sizeof(sbuf));
	nbytes--;
    }
    if (nbits % NBITS_PER_BYTE) {
	snprintf(tmp, sizeof(tmp), "/%d", nbits);
	strlcat(sbuf, tmp, sizeof(sbuf));
    }
    return (sbuf);
}

/* 
 * Function: random_range
 * Purpose:
 *   Return a random number in the given range.
 */
long
random_range(long bottom, long top)
{
    long ret;
    long number = top - bottom + 1;
    long range_size = UINT32_MAX / number;
    if (range_size == 0)
	return (bottom);
    ret = (arc4random() / range_size) + bottom;
    return (ret);
}

/*
 * Function: timeval_subtract
 *
 * Purpose:
 *   Computes result = tv1 - tv2.
 */
void
timeval_subtract(struct timeval tv1, struct timeval tv2, 
		 struct timeval * result)
{
    result->tv_sec = tv1.tv_sec - tv2.tv_sec;
    result->tv_usec = tv1.tv_usec - tv2.tv_usec;
    if (result->tv_usec < 0) {
	result->tv_usec += USECS_PER_SEC;
	result->tv_sec--;
    }
    return;
}

/*
 * Function: timeval_add
 *
 * Purpose:
 *   Computes result = tv1 + tv2.
 */
void
timeval_add(struct timeval tv1, struct timeval tv2,
	    struct timeval * result)
{
    result->tv_sec = tv1.tv_sec + tv2.tv_sec;
    result->tv_usec = tv1.tv_usec + tv2.tv_usec;
    if (result->tv_usec > USECS_PER_SEC) {
	result->tv_usec -= USECS_PER_SEC;
	result->tv_sec++;
    }
    return;
}

/*
 * Function: timeval_compare
 *
 * Purpose:
 *   Compares two timeval values, tv1 and tv2.
 *
 * Returns:
 *   -1		if tv1 is less than tv2
 *   0 		if tv1 is equal to tv2
 *   1 		if tv1 is greater than tv2
 */
int
timeval_compare(struct timeval tv1, struct timeval tv2)
{
    struct timeval result;

    timeval_subtract(tv1, tv2, &result);
    if (result.tv_sec < 0 || result.tv_usec < 0)
	return (-1);
    if (result.tv_sec == 0 && result.tv_usec == 0)
	return (0);
    return (1);
}

/*
 * Function: fprint_data
 * Purpose:
 *   Displays the buffer as a series of 8-bit hex numbers with an ASCII
 *   representation off to the side.
 */
__private_extern__ void
fprint_data(FILE * f, const uint8_t * data_p, int n_bytes)
{
#define CHARS_PER_LINE 	16
    char		line_buf[CHARS_PER_LINE + 1];
    int			line_pos;
    int			offset;

    for (line_pos = 0, offset = 0; offset < n_bytes; offset++, data_p++) {
	if (line_pos == 0)
	    fprintf(f, "%04x ", offset);

	line_buf[line_pos] = isprint(*data_p) ? *data_p : '.';
	printf(" %02x", *data_p);
	line_pos++;
	if (line_pos == CHARS_PER_LINE) {
	    line_buf[CHARS_PER_LINE] = '\0';
	    printf("  %s\n", line_buf);
	    line_pos = 0;
	}
	else if (line_pos == (CHARS_PER_LINE / 2))
	    printf(" ");
    }
    if (line_pos) { /* need to finish up the line */
	for (; line_pos < CHARS_PER_LINE; line_pos++) {
	    printf("   ");
	    line_buf[line_pos] = ' ';
	}
	line_buf[CHARS_PER_LINE] = '\0';
	printf("  %s\n", line_buf);
    }
}

__private_extern__ void
print_data(const uint8_t * data_p, int n_bytes)
{
    fprint_data(stdout, data_p, n_bytes);
}

__private_extern__ void
fprint_bytes(FILE * out_f, u_char * data_p, int n_bytes)
{
    int i;

    if (out_f == NULL) {
	out_f = stdout;
    }
    for (i = 0; i < n_bytes; i++) {
	char * space;

	if (i == 0) {
	    space = "";
	}
	else if ((i % 8) == 0) {
	    space = "  ";
	}
	else {
	    space = " ";
	}
	fprintf(out_f, "%s%02x", space, data_p[i]);
    }
    fflush(out_f);
    return;
}

__private_extern__ void
print_bytes(u_char * data, int len)
{
    fprint_bytes(NULL, data, len);
}

/*
 * Function: create_path
 *
 * Purpose:
 *   Create the given directory hierarchy.  Return -1 if anything
 *   went wrong, 0 if successful.
 */
int
create_path(const char * dirname, mode_t mode)
{
    boolean_t		done = FALSE;
    const char *	scan;

    if (mkdir(dirname, mode) == 0 || errno == EEXIST)
	return (0);

    if (errno != ENOENT)
	return (-1);

    {
	char	path[PATH_MAX];

	for (path[0] = '\0', scan = dirname; done == FALSE;) {
	    const char * 	next_sep;
	    
	    if (scan == NULL || *scan != '/')
		return (FALSE);
	    scan++;
	    next_sep = strchr(scan, '/');
	    if (next_sep == 0) {
		done = TRUE;
		next_sep = dirname + strlen(dirname);
	    }
	    strncpy(path, dirname , next_sep - dirname);
	    path[next_sep - dirname] = '\0';
	    if (mkdir(path, mode) == 0 || errno == EEXIST)
		;
	    else
		return (-1);
	    scan = next_sep;
	}
    }
    return (0);
}

int
ether_cmp(struct ether_addr * e1, struct ether_addr * e2)
{
    int i;
    u_char * c1 = e1->octet;
    u_char * c2 = e2->octet;

    for (i = 0; i < sizeof(e1->octet); i++, c1++, c2++) {
	if (*c1 == *c2)
	    continue;
	return ((int)*c1 - (int)*c2);
    }
    return (0);
}
