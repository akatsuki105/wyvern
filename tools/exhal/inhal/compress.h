/*
	exhal / inhal (de)compression routines

	Copyright (c) 2013-2018 Devin Acker

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.

*/

#ifndef _COMPRESS_H
#define _COMPRESS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define DATA_SIZE 65536

    typedef struct
    {
        // Speed up compression somewhat by avoiding less common compression methods
        int fast;
        // Improve compression ratios by performing a shortest-path search
        int optimal;
    } pack_options_t;

    typedef struct
    {
        // Number of times each compression method occurred in the input
        int methoduse[7];
        // Size of compressed input
        size_t inputsize;
    } unpack_stats_t;

    size_t exhal_pack2(uint8_t *unpacked, size_t inputsize, uint8_t *packed, const pack_options_t *options);
    size_t exhal_pack(uint8_t *unpacked, size_t inputsize, uint8_t *packed, int fast);

#ifdef EXHAL_OLD_NAMES
#define pack(...) exhal_pack(__VA_ARGS__)
#define unpack(...)
#define unpack_from_file(...)
#endif

#ifdef __cplusplus
}
#endif

// end include guard
#endif
