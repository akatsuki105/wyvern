
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>



#define HASH_STAT	    /* gives statistics about the hash compares */
#define BACKSKIP_FULL       /* full backSkip table - enables RESCAN. If */
                            /* not defined, backSkip only uses max 128kB */

const char version[] = "\0$VER: c64pack 1.1 1-Jan-98\n";


static int escBits = 3, escMask, reservedBytes = 2;


/* Includes loading address */
static unsigned char headerLZ[] =
{
0x01,0x08,0x0B,0x08,0xEF,0x00,0x9E,0x32,
0x30,0x36,0x31,0x00,0x00,0x00,0x78,0xE6,
0x01,0xA2,0xBC,0xBD,0x32,0x08,0x9D,0xF6,
0x00,0xCA,0xD0,0xF7,0xA0,0xAA,0xCA,0xBD,
0xAA,0xAA,0x9D,0x00,0xFF,0x8A,0xD0,0xF6,
0xCE,0x23,0x08,0xCE,0x20,0x08,0x88,0xD0,
0xED,0x4C,0x36,0x01,0x00,0x8D,0xAA,0xAA,
0xE6,0xF9,0xF0,0x01,0x60,0xE6,0xFA,0x60,
0xAD,0xAA,0xAA,0xEE,0x04,0x01,0x18,0xF0,
0x01,0x60,0xEE,0x05,0x01,0x60,0xA9,0x37,
0x85,0x01,0xA5,0xF9,0x85,0x2D,0xA5,0xFA,
0x85,0x2E,0x58,0x4C,0xAA,0xAA,0xB0,0xEE,
0x20,0x03,0x01,0x48,0x29,0xE0,0xAA,0x68,
0x29,0x1F,0x05,0xF7,0x86,0xF7,0xAA,0x8A,
0x20,0xF8,0x00,0x20,0x03,0x01,0xAA,0x29,
0xE0,0xC5,0xF7,0xD0,0xF2,0xA0,0x00,0x8A,
0x29,0x1F,0x4A,0x90,0x3E,0x4A,0xB0,0x26,
0xAA,0x20,0x03,0x01,0xE0,0x00,0xD0,0x02,
0xAA,0x98,0xE8,0x20,0xF8,0x00,0xCA,0xD0,
0xFA,0x88,0x30,0xD7,0x48,0x98,0xAA,0x68,
0xA0,0x00,0x91,0xF9,0xC8,0xD0,0xFB,0xE6,
0xFA,0xCA,0x10,0xF6,0x30,0xC5,0xA8,0x20,
0x03,0x01,0xC9,0x00,0x10,0x0D,0x0A,0xAA,
0x98,0x4A,0xA8,0x8A,0x6A,0xAA,0x20,0x03,
0x01,0x90,0xCF,0x4A,0xAA,0xF0,0x97,0x98,
0x2A,0x85,0x2E,0x20,0x03,0x01,0x65,0xF9,
0x85,0x2D,0xA5,0xFA,0xE5,0x2E,0x85,0x2E,
0xA0,0x00,0xB1,0x2D,0x91,0xF9,0xC8,0xCA,
0x10,0xF8,0x98,0x18,0x65,0xF9,0x85,0xF9,
0x90,0x89,0xE6,0xFA,0x4C,0x36,0x01
};


int SavePackLZ(unsigned char *data, int size, char *target,
	       int start, int exec, int escape, int endAddr)
{
    FILE *fp = NULL;
    unsigned char *header = headerLZ;

    if(!data)
	return 10;

    if(!target)
	fp = stdout;

    if(0x801 + sizeof(headerLZ) - 2 + size > 0xfe00)
    {
	fprintf(stderr, "Packed file's max size is 0x%04x (0x%04x)!\n",
		0xfe00-0x801-(sizeof(headerLZ)-2), size);
	return 10;
    }

    header[0x81c -0x7ff] = (size>>8) + 1;
    header[0x81f -0x7ff] = (0x801 + (sizeof(headerLZ)-2) + size - 0x100) & 0xff;
    header[0x820 -0x7ff] = ((0x801 + (sizeof(headerLZ)-2) + size - 0x100)>>8);
    header[0x822 -0x7ff] = (endAddr - 0x100) & 0xff;
    header[0x823 -0x7ff] = ((endAddr - 0x100) >> 8);

    header[0x833 -0x7ff] = escape;
    header[0x835 -0x7ff] = (start & 0xff);
    header[0x836 -0x7ff] = (start >> 8);

    header[0x840-0x7ff] = (endAddr - size) & 0xff;
    header[0x841-0x7ff] = ((endAddr - size) >> 8);

    /* header[0x84e -0x7ff] = 0x37; */
    /* header[0x859 -0x7ff] = cli; */
    header[0x85b -0x7ff] = (exec & 0xff);
    header[0x85c -0x7ff] = (exec >> 8);

    header[0x864 -0x7ff] = header[0x877 -0x7ff] = escMask;
    header[0x868 -0x7ff] = header[0x880 -0x7ff] = escMask ^ 0xff;

    if(fp || (fp = fopen(target, "wb")))
    {
	fwrite(header, 1, sizeof(headerLZ), fp);
	fwrite(data, size, 1, fp);
	if(fp != stdout)
	    fclose(fp);
    }
    else
    {
	fprintf(stderr, "Could not open %s for writing\n", target);
	return 10;
    }
    return 0;
}

int SaveStandAlone(unsigned char *data, int size, char *target,
		   int start, int exec, int escape, int endAddr)
{
    FILE *fp = NULL;

    if(!data)
	return 10;

    if(!target)
	fp = stdout;

    // printf("IN_ADDR   = $%04x\n", endAddr - size);
    // printf("EXEC_ADDR = $%04x\n", exec);
    // printf("OUT_ADDR  = $%04x\n", start);
    // printf("MEM_CONF  = $%02x\n", 0x37);


    printf("ESCAPE    = $%02x\n", escape);
    printf("E_MASK    = $%02x\n", escMask);

    if(fp || (fp = fopen(target, "wb")))
    {
	unsigned char t[2];

  //  t[0] = (endAddr -size) & 0xff;
  //  t[1] = (endAddr -size) >> 8;

    t[0] = 0x50;
    fwrite(t, 1, 1, fp);    /* 'cp' file ID */

    t[0] = escape;
    t[1] = escMask;
    fwrite(t, 1, 2, fp);    /* write escape data */

    fwrite(data, size, 1, fp);
	if(fp != stdout)
	    fclose(fp);
    }
    else
    {
	fprintf(stderr, "Could not open %s for writing\n", target);
	return 10;
    }
    return 0;
}



#define F_NORLE   (1<<0)
#define F_STATS   (1<<1)
#define F_NOOPT   (1<<2)
#define F_AUTO    (1<<3)
#define F_ALONE   (1<<4)

#define F_ERROR  (1<<15)

#ifndef min
#define min(a,b) ((a<b)?a:b)
#endif



/* LZ77: yet yet yet improved encoding */
/* escape bits default = 3 */
/* eee00010                            - eof */
/* eee00000 nnneeeee                   - escape (n = new escape) */
/* eeelllp0 pppppppp                   - short LZ77 L=1..7 (L+1) bytes copied */

/* eee00001 llllllll                   - 0-RLE  (L+1) 0-bytes */
/* eeelll01 vvvvvvvv                   - short RLE  L=1..7 (L+1) bytes */
/* eeelll11 1lllllll vvvvvvvv          - long RLE  L=0..1023 (L+1) bytes */
/* eeeppp11 0llllllp pppppppp          - long LZ77 L=0..63 (L+1) bytes copied */


#define LRANGE		(1<<(15-escBits))
#define MINLZLEN	3
#define MAXLZLEN	64
#define MINRLELEN	3
#define MAXRLELEN	(1<<(13-escBits))
#define DEFAULT_LZLEN	LRANGE


#define OUT_SIZE 65536
static unsigned char outBuffer[OUT_SIZE];
static int outPointer = 0;


int FWriteBuf(void *buffer, int nsize, int nelems)
{
    int i = nsize*nelems;

    while(i--)
    {
	outBuffer[outPointer++ & 0xffff] = *(unsigned char *)buffer;
	buffer = (void *)((int)buffer + 1);
    }
    return nelems;
}


static int gained_escaped = 0;
static int gained_rle = 0, gained_0rle = 0, gained_srle = 0, gained_lrle = 0;
static int gained_lz = 0, gained_slz = 0, gained_llz = 0;

static int times_escaped = 0, times_normal = 0;
static int times_rle = 0, times_0rle = 0, times_srle = 0, times_lrle = 0;
static int times_lz = 0, times_slz = 0, times_llz = 0;



int OutputEof(int *escape)
{
    unsigned char temp[2];

    temp[0] = *escape | 2;
    FWriteBuf(temp, 1, 1);
    return 1;
}

int OutputNormal(int *escape, unsigned char *data, int newesc)
{
    unsigned char temp[2];

    times_normal++;
    if((data[0] & escMask) == *escape)
    {
	temp[0] = *escape;
	temp[1] = newesc | (data[0] & (escMask ^ 0xff));
	FWriteBuf(temp, 1, 2);

	*escape = newesc;
	gained_escaped += 1;
	times_escaped++;
	return 1;
    }
    FWriteBuf(data, 1, 1);
    return 1;
}


int LenRle(int rlelen, unsigned char data)
{
    int len = rlelen, out = 0;

    while(len)
    {
	if(!data && len>=2 && len<=256)
	    return out + 2;

	if(len==1)
	    return out + 1;

	if(len >= 2 && len <= (1<<(6-escBits)))
	    return out + 2;

	if(len < MINRLELEN)
	    return out + len;

	if(len >= MINRLELEN && len <= MAXRLELEN)
	    return out + 3;

	len -= MAXRLELEN;
	out += 3;
    }
    return out;
}


int OutputRle(int *escape, unsigned char *data, int rlelen)
{
    unsigned char temp[5];
    int len = rlelen, out = 0;

    while(len)
    {
	if(!data[0] && len>=2 && len<=256)
	{
	    /* Short 0 RLE */
	    temp[0] = *escape | 1;
	    temp[1] = len-1;
	    FWriteBuf(temp, 1, 2);

	    gained_rle += len - 2;
	    gained_0rle += len - 2;

	    times_rle++;
	    times_0rle++;
	    return out + 2;
	}
	if(len==1)
	    return out + OutputNormal(escape, data, *escape);

	if(len >= 2 && len <= (1<<(6-escBits)))
	{
	    /* Short RLE */
	    temp[0] = *escape | ((len-1)<<2) | 1;
	    temp[1] = data[0];
	    FWriteBuf(temp, 1, 2);

	    gained_rle += len - 2;
	    gained_srle += len - 2;

	    times_rle++;
	    times_srle++;
	    return out + 2;
	}
	if(len<MINRLELEN)
	{
	    while(len--)
		out += OutputNormal(escape, data, *escape);
	    return out;
	}

	if(len >= MINRLELEN && len <= MAXRLELEN)
	{
	    /* Run-length encoding */
	    temp[0] = *escape | (((len-1)>>7)<<2) | 3;
	    temp[1] = ((len-1) & 0x7f) | 128;
	    temp[2] = data[0];
	    FWriteBuf(temp, 1, 3);

	    gained_rle += len - 3;
	    gained_lrle += len - 3;
	    times_rle++;
	    times_lrle++;
	    return out + 3;
	}

	/* Run-length encoding */
	temp[0] = *escape | (((MAXRLELEN-1)>>7)<<2) | 3;
	temp[1] = ((MAXRLELEN-1) & 0x7f) | 128;
	temp[2] = data[0];
	FWriteBuf(temp, 1, 3);

	gained_rle += MAXRLELEN - 3;
	gained_lrle += MAXRLELEN - 3;
	times_rle++;
	times_lrle++;
	len -= MAXRLELEN;
	data += MAXRLELEN;
	out += 3;
    }
    return out;
}


int LenLz(int lzlen, int lzpos)
{
    if(lzlen >= 2 && lzlen <= (1<<(6-escBits)) && lzpos <= 512)
	return 2;

    if(lzlen >= 1 && lzlen<=MAXLZLEN)
	return 3;

    return lzlen;
}


int OutputLz(int *escape, int lzlen, int lzpos, char *data)
{
    unsigned char temp[5];

    if(lzlen >= 2 && lzlen <= (1<<(6-escBits)) && lzpos <= 512)
    {
	temp[0] = *escape | ((lzlen-1)<<2) | (((lzpos-1)>>8)<<1);
	temp[1] = ((lzpos-1) & 0xff) ^ 0xff;
	FWriteBuf(temp, 1, 2);

	gained_lz += lzlen - 2;
	gained_slz += lzlen - 2;
	times_lz++;
	times_slz++;
	return 2;
    }

    if(lzlen >= 1 && lzlen <= MAXLZLEN)
    {
	temp[0] = *escape | (((lzpos-1)>>9)<<2) | 3;
	temp[1] = (((lzpos-1)>>8)&1) | ((lzlen-1)<<1);
	temp[2] = ((lzpos-1) & 0xff) ^ 0xff;
	FWriteBuf(temp, 1, 3);

	gained_lz += lzlen - 3;
	gained_llz += lzlen - 3;
	times_lz++;
	times_llz++;
	return 3;
    }
    fprintf(stderr, "Error: lzlen too short/long (%d)\n", lzlen);
    return lzlen;
}



static unsigned short *rle, *elr, *lzlen, *lzpos;
static int *length, inlen;
static unsigned char *indata, *mode, *newesc;
unsigned short *backSkip;



/* Non-recursive version */
/* NOTE! IMPORTANT! the "length" array length must be inlen+1 */
int OptimizeLength(int optimize)
{
    int i;

    length[inlen] = 0;		/* one off the end, our 'target' */
    for(i=inlen-1;i>=0;i--)
    {
    	int r1 = 1 + length[i+1], r2, r3;

	if(!lzlen[i] && !rle[i])
	{
	    length[i] = r1;
	    mode[i] = 0;
	    continue;
	}

	/* If rle>MAXLZLEN, skip to the start of the rle-MAXLZLEN.. */
	if(rle[i] > MAXLZLEN && elr[i] > 1)
	{
	    int z = elr[i];

	    i -= elr[i];

	    r2 = LenRle(rle[i], indata[i]) + length[i+ rle[i]];
	    if(optimize)
	    {
		int ii, mini = rle[i], minv = r2;

		int bot = rle[i] - MAXLZLEN/2;
		if(bot < 2)
		    bot = 2;

		for(ii=mini-1;ii>=bot;ii--)
		{
		    int v = LenRle(ii, indata[i]) + length[i + ii];
		    if(v < minv)
		    {
			minv = v;
			mini = ii;
		    }
		}
		if(minv != r2)
		{
		    rle[i] = mini;
		    r2 = minv;
		}
	    }
	    length[i] = r2;
	    mode[i] = 2;

	    for(;z>=0;z--)
	    {
		length[i+z] = r2;
		mode[i+z] = 2;
	    }
	    continue;
	}
	r3 = r2 = r1 + 1000; /* r3 >= r2 > r1 */

	if(rle[i])
	{
	    r2 = LenRle(rle[i], indata[i]) + length[i+ rle[i]];

	    if(optimize)
	    {
		int ii, mini = rle[i], minv = r2;

#if 0
		int bot = rle[i] - MAXLZLEN/2;
		if(bot < 2)
		    bot = 2;

		for(ii=mini-1;ii>=bot;ii--)
		{
		    int v = LenRle(ii, indata[i]) + length[i + ii];
		    if(v < minv)
		    {
			minv = v;
			mini = ii;
		    }
		}
#else
		/* Does not really miss many 'minimums' this way,
		   at least not globally..
		   Makes the assumption that the Elias Gamma Code is
		   used, i.e. values of the form 2^n are 'optimal' */
		ii = 2;
		while(rle[i] > ii)
		{
		    int v = LenRle(ii, indata[i]) + length[i + ii];
		    if(v < minv)
		    {
			minv = v;
			mini = ii;
		    }
		    ii <<= 1;
		}
#endif
		if(minv != r2)
		{
/*printf("%05d RL %d %d\n", i, rle[i], mini);*/
		    rle[i] = mini;
		    r2 = minv;
		}
	    }
	}
	if(lzlen[i])
	{
	    r3 = LenLz(lzlen[i], lzpos[i]) + length[i + lzlen[i]];

	    if(optimize)
	    {
		int ii, mini = lzlen[i], minv = r3;

#if 0
		int topLen = LenLz(lzlen[i], lzpos[i])
		    - LenValue(lzlen[i]-1);
		int bot = 3;
		if(lzpos[i] <= 256)
		    bot = 2;

		for(ii=mini-1;ii>=bot;ii--)
		{
		    int v = topLen + LenValue(ii-1) + length[i + ii];
		    if(v < minv)
		    {
			minv = v;
			mini = ii;
		    }
		}
#else
		/* Does not really miss many 'minimums' this way,
		   at least not globally..
		   Makes the assumption that the Elias Gamma Code is
		   used, i.e. values of the form 2^n are 'optimal' */
		ii = 4;
		while(lzlen[i] > ii)
		{
		    int v = LenLz(ii, lzpos[i]) + length[i + ii];

		    if(v < minv)
		    {
			minv = v;
			mini = ii;
		    }
		    ii <<= 1;
		}
#ifdef BACKSKIP_FULL
		/*
		  Note:
		  2-byte optimization checks are no longer done
		  with the rest, because the equation gives too long
		  code lengths for 2-byte matches if extraLzPosBits>0.
		  */
		/* Two-byte rescan/check */
		if(backSkip[i] && backSkip[i] <= 512)
		{
		    /* There are previous occurrances (near enough) */
		    int v = LenLz(2, (int)backSkip[i]) + length[i + 2];

		    if(v < minv)
		    {
			minv = v;
			mini = 2;
			lzlen[i] = mini;
			r3 = minv;
			lzpos[i] = (int)backSkip[i];
		    }
		}
#endif /* BACKSKIP_FULL */
#endif
		if(minv != r3 && minv < r2)
		{
                    /*
		      printf("@%05d LZ %d %4x -> %d %4x\n",
		      i, lzlen[i], lzpos[i], mini, lzpos[i]);
		      */
		    lzlen[i] = mini;
		    r3 = minv;
		}
	    }
	}

	if(r2 <= r1)
	{
	    if(r2 <= r3)
	    {
		length[i] = r2;
		mode[i] = 2;
	    }
	    else
	    {
		length[i] = r3;
		mode[i] = 1;
	    }
	}
	else
	{
	    if(r3 <= r1)
	    {
		length[i] = r3;
		mode[i] = 1;
	    }
	    else
	    {
		length[i] = r1;
		mode[i] = 0;
	    }
	}
    }
    return length[0];
}



/*
    The algorithm in the OptimizeEscape() works as follows:
    1) Only unpacked bytes are processed, they are marked
       with mode 3. We proceed from the end to the beginning.
       Variable A (old/new length) is updated.
    2) At each unpacked byte, one and only one possible
       escape matches. A new escape code must be selected
       for this case. The optimal selection is the one which
       provides the shortest number of escapes to the end
       of the file,
	i.e. A[esc] = 1+min(A[0], A[1], .. A[states-1]).
       For other states A[esc] = A[esc];
       If we change escape in this byte, the new escape is
       the one with the smallest value in A.
    3) The starting escape is selected from the possibilities
       and mode 0 is restored to all mode 3 locations.

 */

int OptimizeEscape(int *startEscape, int *nonNormal)
{
    int i, j, states = (1<<escBits), minp = 0, minv = 0, other = 0;
    int a[256]; /* needs int */
    int b[256]; /* Remembers the # of escaped for each */

    for(i=0;i<256;i++)
	b[i] = a[i] = -1;

    if(states>256)
    {
	fprintf(stderr, "Escape optimize: only 256 states (%d)!\n",
		states);
	return 0;
    }

    /* Mark those bytes that are actually outputted */
    for(i=0;i<inlen;)
    {
	switch(mode[i])
	{
	case 1:
	    other++;
	    i += lzlen[i];
	    break;

	case 2:
	    other++;
	    i += rle[i];
	    break;

	case 0:
	default:
	    mode[i++] = 3; /* mark it used so we can identify it */
	    break;
	}
    }

    for(;i>=0;i--)
    {
	/* Using a table to skip non-normal bytes does not help.. */
	if(mode[i]==3)
	{
	    int k;

	    /* Change the tag values back to normal */
	    mode[i] = 0;

	    /*
		k are the matching bytes,
		minv is the minimum value,
		minp is the minimum index
	     */

	    k = (indata[i] >> (8-escBits));
	    newesc[i] = (minp << (8-escBits));
	    a[k] = 1 + minv;
	    b[indata[i]>>(8-escBits)] = b[minp] + 1;
	    if(k==minp)
	    {
		/* Minimum changed -> need to find a new minimum */
		/* a[k] may still be the minimum */
		minv++;
		for(k=0;k<states;k++)
		{
		    if(a[k] < minv)
		    {
			minv = a[k];
			minp = k;
			/* There may be others, but the first one is ok. */
			break;
		    }
		}
	    }
	}
    }

    /* Select the best value for the initial escape */
    if(startEscape)
    {
	i = inlen;	/* make it big enough */
	for(j=0;j<states;j++)
	{
	    if(a[j] <= i)
	    {
		*startEscape = (j<<(8-escBits));
		i = a[j];
	    }
	}
    }
    if(nonNormal)
	*nonNormal = other;
    return b[*startEscape>>(8-escBits)];
}


int PackLz77(int lzsz, int flags, int *startEscape, int endAddr)
{
    int i, j, p, outlen;
    int escape;

    unsigned short *lastPair;

#ifdef HASH_STAT
    unsigned long compares = 0, hashChecks = 0, hashEqual = 0;
#endif /* HASH_STAT */


    if(lzsz < 0 || lzsz > LRANGE)
    {
	fprintf(stderr, "Unsuitable LZ range %d. Must be from 0 to %d.\n",
		lzsz, LRANGE);
	return 20;
    }
    if(!lzsz)
	fprintf(stderr, "Warning: zero LZ range. Only RLE packing used.\n");

    length = (int *)calloc(sizeof(int), inlen + 1);
    mode   = (unsigned char *)calloc(sizeof(unsigned char), inlen);
    rle    = (unsigned short *)calloc(sizeof(unsigned short), inlen);
    elr    = (unsigned short *)calloc(sizeof(unsigned short), inlen);
    lzlen  = (unsigned short *)calloc(sizeof(unsigned short), inlen);
    lzpos  = (unsigned short *)calloc(sizeof(unsigned short), inlen);
    newesc = (unsigned char *)calloc(sizeof(unsigned char), inlen);
#ifdef BACKSKIP_FULL
    backSkip  = (unsigned short *)calloc(sizeof(unsigned short), inlen);
#else
    backSkip  = (unsigned short *)calloc(sizeof(unsigned short), 65536);
#endif /* BACKSKIP_FULL */
    lastPair  = (unsigned short *)calloc(sizeof(unsigned short), 256*256);


    /* error checking */
    if(!length || !mode || !rle || !elr || !lzlen || !lzpos || !newesc ||
	!lastPair || !backSkip
	)
    {
	fprintf(stderr, "Memory allocation failed!\n");
	goto errorexit;
    }

    /* Detect all RLE and LZ77 jump possibilities */
    for(p=0;p<inlen;p++)
    {
	if(!(p & 1023))
	{
	    fprintf(stderr, "\r%d ", p);
	    fflush(stderr);
	}

	/* check run-length code */
	if(rle[p] <= 0)
	{
	    /*
		There are so few RLE's and especially so few
		long RLE's that byte-by-byte is good enough.
	     */
	    unsigned char *a = indata + p;
	    int val = *a++;	/* if this is uchar, it goes to stack.. */
	    int top = inlen - p;
	    int rlelen = 1;

	    /* Loop for the whole RLE */
	    while(rlelen<top && *a++ == (unsigned char)val)
	    {
		rlelen++;
	    }
#ifdef HASH_STAT
	    compares += rlelen;
#endif /* HASH_STAT */

	    if(rlelen>=2)
	    {
		for(i=0;i<rlelen-1;i++)
		{
		    rle[p+i] = rlelen-i;
		    elr[p+i] = i;
		}
	    }
	}

	/* check LZ77 code */
	if(rle[p]<MAXLZLEN && p+rle[p]+1<inlen)
	{
	    int bot = p - lzsz, maxval, maxpos, rlep = rle[p];
	    unsigned char valueCompare = indata[p+2];

	    /*
		There's always 1 equal byte, although it may
		not be marked as RLE.
	     */
	    if(rlep <= 0)
		rlep = 1;
	    if(bot < 0)
		bot = 0;
	    bot += (rlep-1);

	    /*
		First get the shortest possible match (if any).
		If there is no 2-byte match, don't look further,
		because there can't be a longer match.
	     */
	    i = (int)lastPair[ (indata[p]<<8) | indata[p+1] ] -1;
	    if(i>=0 && i>=bot)
	    {
		/* Got a 2-byte match at least */
		maxval = 2;
		maxpos = p-i;

		/*
		    A..AB	rlep # of A's, B is something else..

		    Search for bytes that are in p + (rlep-1), i.e.
		    the last rle byte ('A') and the non-matching one
		    ('B'). When found, check if the rle in the compare
		    position (i) is long enough (i.e. the same number
		    of A's at p and i-rlep+1).

		    There are dramatically less matches for AB than for
		    AA, so we get a huge speedup with this approach.
		    We are still guaranteed to find the most recent
		    longest match there is.
		 */

		i = (int)lastPair[(indata[p+(rlep-1)]<<8) | indata[p+rlep]] -1;
		while(i>=bot /* && i>=rlep-1 */)
		{   /* bot>=rlep-1, i>=bot  ==> i>=rlep-1 */

		    /* Equal number of A's ? */
		    if(!(rlep-1) || rle[i-(rlep-1)]==rlep) /* 'head' matches */
		    {
			/* rlep==1 ==> (rlep-1)==0 */
			/* ivanova.run: 443517 rlep==1,
			   709846 rle[i+1-rlep]==rlep */

			/*
			    Check the hash values corresponding to the last
			    two bytes of the currently longest match and
			    the first new matching(?) byte. If the hash
			    values don't match, don't bother to check the
			    data itself.
			 */
#ifdef HASH_STAT
			hashChecks++;
#endif /* HASH_STAT */
			if(indata[i+maxval-rlep+1] == valueCompare)
			{
			    unsigned char *a = indata + i+2;	/* match  */
			    unsigned char *b = indata + p+rlep-1+2;/* curpos */
			    unsigned char *c = indata + inlen;	/* memtop */

			    /* the 2 first bytes ARE the same.. */
			    j = 2;
			    while(b!=c && *a++==*b++)
				j++;

#ifdef HASH_STAT
			    hashEqual++;
			    compares += j - 1;
#endif /* HASH_STAT */
			    if(j + rlep-1 > maxval)
			    {
				int tmplen = j+rlep-1, tmppos = p-i+rlep-1;

				if(tmplen > MAXLZLEN)
				    tmplen = MAXLZLEN;

				/* Accept only versions that really are shorter */
				if(tmplen*8 - LenLz(tmplen, tmppos) >
				   maxval*8 - LenLz(maxval, maxpos))
				{
				    maxval = tmplen;
				    maxpos = tmppos;
				    valueCompare = indata[p+maxval];
				}
				if(maxval == MAXLZLEN)
				    break;
			    }
			}
		    }
#ifdef BACKSKIP_FULL
		    if(!backSkip[i])
			break; /* No previous occurrances (near enough) */
		    i -= (int)backSkip[i];
#else
		    if(!backSkip[i & 0xffff])
			break; /* No previous occurrances (near enough) */
		    i -= (int)backSkip[i & 0xffff];
#endif /* BACKSKIP_FULL */
		}

		/*
		    If there is 'A' in the previous position also,
		    RLE-like LZ77 is possible, although rarely
		    shorter than real RLE.
		 */
		if(p && rle[p-1] > maxval)
		{
		    maxval = rle[p-1] - 1;
		    maxpos = 1;
		}
		/*
		    Last, try to find as long as possible match
		    for the RLE part only.
		 */
		if(maxval < MAXLZLEN && rlep > maxval)
		{
		    bot = p - lzsz;
		    if(bot < 0)
			bot = 0;

		    /* Note: indata[p] == indata[p+1] */
		    i = (int)lastPair[indata[p]*257] -1;
		    while(/* i>= rlep-2 &&*/ i>=bot)
		    {
			if(elr[i] + 2 > maxval)
			{
			    maxval = min(elr[i] + 2, rlep);
			    maxpos = p - i + (maxval-2);
			    if(maxval == rlep)
				break; /* Got enough */
			}
			i -= elr[i];
#ifdef BACKSKIP_FULL
			if(!backSkip[i])
			    break; /* No previous occurrances (near enough) */
			i -= (int)backSkip[i];
#else
			if(!backSkip[i & 0xffff])
			    break; /* No previous occurrances (near enough) */
			i -= (int)backSkip[i & 0xffff];
#endif /* BACKSKIP_FULL */
		    }
		}
		if(p+maxval > inlen)
		{
		    fprintf(stderr,
			    "Error @ %d, lzlen %d, pos %d - exceeds inlen\n",
			    p, maxval, maxpos);
		    maxval = inlen - p;
		}
		if(maxval > 2 || (maxval==2 && maxpos<= 512 /*256*/))
		{
		    if(maxpos < 0)
			fprintf(stderr, "Error @ %d, lzlen %d, pos %d\n",
				p, maxval, maxpos);
		    lzlen[p] = (maxval<MAXLZLEN)?maxval:MAXLZLEN;
		    lzpos[p] = maxpos;
		}
	    }
	}

	/* Update the two-byte history ('hash table') &
	   backSkip ('linked list') */
	if(p+1<inlen)
	{
	    int index = (indata[p]<<8) | indata[p+1];
	    int ptr = p - (lastPair[index]-1);

	    if(ptr > p || ptr > 0xffff)
		ptr = 0;

#ifdef BACKSKIP_FULL
	    backSkip[p] = ptr;
#else
	    backSkip[p & 0xffff] = ptr;
#endif /* BACKSKIP_FULL */
	    lastPair[index] = p+1;
	}
    }
    fprintf(stderr, "\rChecked: %d \n", p);
    fflush(stderr);

    /* Check the normal bytes / all ratio */
    if((flags & F_AUTO))
    {
	int mb, mv;

	fprintf(stderr, "Selecting the number of escape bits.. ");
	fflush(stderr);	/* for SAS/C */

	/*
	    Absolute maximum number of escaped bytes with
	    the escape optimize is 2^-n, where n is the
	    number of escape bits used.

	    This worst case happens only on equal-
	    distributed normal bytes (01230123..).
	    This is why the typical values are so much smaller.
	 */

	mb = 0;
	mv = 8*OUT_SIZE;
	for(escBits=1;escBits<5;escBits++)
	{
	    int escaped, other = 0, c;

	    escMask = (0xff00>>escBits) & 0xff;

	    /* Find the optimum path for selected escape bits (no optimize) */
	    OptimizeLength(0);

	    /* Optimize the escape selections for this path & escBits */
	    escaped = OptimizeEscape(&escape, &other);

	    /* Compare value: bits lost for escaping -- bits lost for prefix */
	    c = (escBits+3)*escaped + other*escBits;
	    if((flags & F_STATS))
	    {
		fprintf(stderr, " %d:%d", escBits, c);
		fflush(stderr);	/* for SAS/C */
	    }
	    if(c < mv)
	    {
		mb = escBits;
		mv = c;
	    }
	    else
	    {
		/* minimum found */
		break;
	    }
	    if(escBits==4 && (flags & F_STATS))
		fprintf(stderr, "\n");
	}
	if(mb==1)	/* Minimum was 1, check 0 */
	{
	    int escaped;

	    escBits = 0;
	    escMask = 0;

	    /* Find the optimum path for selected escape bits (no optimize) */
	    OptimizeLength(0);
	    /* Optimize the escape selections for this path & escBits */
	    escaped = OptimizeEscape(&escape, NULL);

	    if((flags & F_STATS))
	    {
		fprintf(stderr, " %d:%d", escBits, 3*escaped);
		fflush(stderr);	/* for SAS/C */
	    }
	    if(3*escaped < mv)
	    {
		mb = 0;
		/* mv = 3*escaped; */
	    }
	}
	if((flags & F_STATS))
	    fprintf(stderr, "\n");

	fprintf(stderr, "Selected %d-bit escapes\n", mb);
	fprintf(stderr, "Note: you get better results if you define \"-e%d\" yourself.\n",
			mb);
	fflush(stderr);	/* for SAS/C */
	escBits = mb;
	escMask = (0xff00>>escBits) & 0xff;
    }

    if(!(flags & F_NOOPT))
    {
	fprintf(stderr, "Optimizing LZ77 and RLE lengths...");
 	fflush(stderr);	/* for SAS/C */
    }

    /* Find the optimum path (optimize) */
    OptimizeLength((flags & F_NOOPT)?0:1);
    fprintf(stderr, "\n");

    OptimizeEscape(&escape, NULL);
    if(startEscape)
	*startEscape = escape;

    for(p=0;p<inlen;)
    {
	switch(mode[p])
	{
	case 0: /* normal */
	case 3:
	    length[p] = outPointer;
	    OutputNormal(&escape, indata+p, newesc[p]);
	    p++;
	    break;

	case 1: /* lz */
	    for(i=0;i<lzlen[p];i++)
		length[p+i] = outPointer;

	    length[p] = outPointer;
	    OutputLz(&escape, lzlen[p], lzpos[p], indata+p-lzpos[p]);
	    p += lzlen[p];
	    break;

	case 2: /* rle */
	    for(i=0;i<rle[p];i++)
		length[p+i] = outPointer;

	    OutputRle(&escape, indata+p, rle[p]);
	    p += rle[p];
	    break;
	}
    }
    OutputEof(&escape);

    i = inlen;
    for(p=0;p<inlen;p++)
    {
	int pos = (inlen - outPointer) + (int)length[p] - p;
	i = min(i, pos);
    }
    if(i<0)
	reservedBytes = -i + 2;
    else
	reservedBytes = 0;

    outlen = outPointer + sizeof(headerLZ);	/* unpack code */
    fprintf(stderr, "In: %d, out: %d, ratio: %f%%, gained: %f%% (%d escape bit%s)\n",
	    inlen, outlen, (double)outlen*100.0/(double)inlen,
	    100.0 - (double)outlen*100.0/(double)inlen, escBits, (escBits!=1)?"s":"" );

    fprintf(stderr, "Gained RLE: %d (%d+%d+%d), LZ: %d (%d+%d), escaped: %d bytes\n",
	   gained_rle, gained_0rle, gained_srle, gained_lrle,
	   gained_lz, gained_slz, gained_llz, -gained_escaped);

    fprintf(stderr, "Times  RLE: %d (%d+%d+%d), LZ: %d (%d+%d), esc': %d (%d normal)\n",
	   times_rle, times_0rle, times_srle, times_lrle,
	   times_lz, times_slz, times_llz, -times_escaped, times_normal);

    fprintf(stderr, "Hash Checks: %ld (%ld equal), RLE/LZ compares: %ld\n",
	    hashChecks, hashEqual, compares);

errorexit:
    if(rle)
	free(rle);
    if(elr)
	free(elr);
    if(lzlen)
	free(lzlen);
    if(lzpos)
	free(lzpos);
    if(length)
	free(length);
    if(mode)
	free(mode);
    if(newesc)
	free(newesc);
    if(lastPair)
	free(lastPair);
    if(backSkip)
	free(backSkip);
    return 0;
}



int main(int argc, char *argv[])
{
    int n, execAddr = -1, ea = -1, escape = 0, newlen, startAddr = 0;
    int flags = 0, lzlen = -1, buflen;
    char *fileIn = NULL, *fileOut = NULL;
    FILE *infp;
    unsigned char tmp[2];

    flags |= F_AUTO;
    flags |= F_ALONE;

    for(n=1;n<argc;n++)
    {
	if(argv[n][0]=='-')
	{
	    int i = 1;
	    char *val, *tmp, c;
	    long tmpval;

	    while(argv[n][i])
	    {
		switch(argv[n][i])
		{
		case 'n':	/* noopt, no rle/lzlen optimization */
		    flags |= F_NOOPT;
		    break;

		case 's':
		    flags |= F_STATS;
		    break;

		case 'a':	/* standalone version */
		    flags |= F_ALONE;
		    break;

		case 'h':
		case '?':
		    flags |= F_ERROR;
		    break;

		case 'r':
		case 'x':
		case 'e':
		    c = argv[n][i]; /* Remember the option */
		    if(argv[n][i+1])
		    {
			val = argv[n]+i+1;
		    }
		    else if(n+1 < argc)
		    {
			val = argv[n+1];
			n++;
		    }
		    else
		    {
			flags |= F_ERROR;
			break;
		    }

		    i = strlen(argv[n])-1;
		    tmpval = strtol(val, &tmp, 0);
		    if(*tmp)
		    {
			fprintf(stderr,
				"Error: invalid number: \"%s\"\n", val);
			flags |= F_ERROR;
			break;
		    }

		    switch(c)
		    {
		    case 'r':
			lzlen = tmpval;
			break;
		    case 'x':
			ea = tmpval;
			break;
		    case 'e':
			escBits = tmpval;
			if(escBits < 0 || escBits > 5)
			{
			    fprintf(stderr, "Escape bits must be 0..5!\n");
			    flags |= F_ERROR;
			    escBits = 2;
			}
			else
			    flags &= ~F_AUTO;
			escMask = (0xff00>>escBits) & 0xff;
			break;
		    }
		    break;

		default:
		    fprintf(stderr, "Error: Unknown option \"%c\"\n",
			    argv[n][i]);
		    flags |= F_ERROR;
		}
		i++;
	    }
	}
	else
	{
	    if(!fileIn)
	    {
		fileIn = argv[n];
	    }
	    else if(!fileOut)
	    {
		fileOut = argv[n];
	    }
	    else
	    {
		fprintf(stderr, "Only two filenames wanted!\n");
		flags |= F_ERROR;
	    }
	}
    }

    if((flags & F_ERROR))
    {
    fprintf(stderr, "c64pack 1.1.1 - Modified by Jeff F. for standalone data.\n");
	fprintf(stderr, "Usage: %s [-<flags>] [<infile> [<outfile>]]\n",
		argv[0]);
	fprintf(stderr,
		"\t x<val>    execution address\n"
		"\t e<val>    escape bits\n"
		"\t r<val>    lz search range\n"
		"\t n         no optimization\n"
		"\t s         stats\n"
    //    "\t a         standalone version\n"
        );
	return EXIT_FAILURE;
    }

    if(lzlen == -1)
	lzlen = DEFAULT_LZLEN;

    if(fileIn)
    {
	if(!(infp = fopen(fileIn, "rb")))
	{
	    fprintf(stderr, "Could not open %s for reading!\n", fileIn);
	    return 20;
	}
    }
    else
    {
	fprintf(stderr, "Reading from stdin\n");
	infp = stdin;
    }

    escMask  = (0xff00>>escBits) & 0xff;

    if ( !(flags & F_ALONE) )
       {
       fread(tmp, 1, 2, infp);
       startAddr = tmp[0] + 256*tmp[1];
       }

    /* Read in the data */
    inlen = 0;
    buflen = 0;
    indata = NULL;
    while( 1 )
    {
	if(buflen < inlen + LRANGE)
	{
	    unsigned char *tmp = realloc(indata, buflen + LRANGE);

	    if(!tmp)
	    {
		free(indata);
		return 20;
	    }
	    indata = tmp;
	    buflen += LRANGE;
	}
	newlen = fread(indata + inlen, 1, LRANGE, infp);
	if(newlen <= 0)
	    break;
	inlen += newlen;
    }
    if(infp != stdin)
	fclose(infp);


    if ( (startAddr < 0x400 || startAddr + inlen >= 0xffff) &&
         !(flags & F_ALONE) )
    {
	fprintf(stderr,
		"Only programs from 0x0400 to 0xfffe can be compressed\n");
	fprintf(stderr, "(the input file is from 0x%04x to 0x%04x)\n",
		startAddr, startAddr+inlen-1);
	if(indata)
	    free(indata);
	return 10;
    }

    if(startAddr==0x801)
    {
	for(n=0;n<30;n++)
	{
	    if(indata[n]==0x9e) /* SYS token */
	    {
		execAddr = 0;
		n++;
		while(indata[n]>='0' && indata[n]<='9')
		{
		    execAddr = execAddr * 10 + indata[n++] - '0';
		}
		break;
	    }
	}
    }

    if(ea!=-1)
    {
	if(execAddr!=-1 && ea!=execAddr)
	    fprintf(stderr, "Discarding exec address 0x%04x=%d\n", execAddr, execAddr);
	execAddr = ea;
    }

    if (!(flags & F_ALONE))
       {
       fprintf(stderr, "Load address 0x%04x=%d\n", startAddr, startAddr);
       fprintf(stderr, "Exec address 0x%04x=%d\n", execAddr, execAddr);
       }

    n = PackLz77(lzlen, flags, &escape, startAddr+inlen);
    if(!n)
    {
	int endAddr = startAddr + inlen;

	/* Move the end address for files that got expanded */
	if(0x801 + sizeof(headerLZ) + outPointer > endAddr)
	{
	    endAddr = 0x801 + sizeof(headerLZ) + outPointer;
	}
	/* 1 byte reserved for EOF */
	/* bytes reserved for temporary data expansion (escaped chars) */
	endAddr += 1 + reservedBytes;

	if(endAddr > 0x10000)
	{
	    fprintf(stderr, "**Error, wrap overrun! File will not decompress.\n");
	}

	if((flags & F_ALONE))
	{
	    SaveStandAlone(outBuffer, outPointer, fileOut,
			   startAddr, execAddr, escape, endAddr);

  //      fprintf(stderr, "%s uses the memory $f7-$197 "
  //          "and $%04x-$%04x (%d bytes reserved)\n",
  //          fileOut, startAddr, endAddr-1, reservedBytes);
	}
	else
	{
	    SavePackLZ(outBuffer, outPointer, fileOut,
			startAddr, execAddr, escape, endAddr);

	    fprintf(stderr, "%s uses the memory $f7-$197 "
		    "and $%04x-$%04x (%d bytes reserved)\n",
		    fileOut, startAddr, endAddr-1, reservedBytes);
	}
    }
    if(indata)
	free(indata);
    return n;
}

