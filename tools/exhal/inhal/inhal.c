/*
	inhal - HAL Laboratory compression tool

	Usage:
	inhal [-fast] infile romfile offset
	inhal -n [-fast] infile outfile
   
	Copyright (c) 2013 Devin Acker

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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "compress.h"

int main(int argc, char **argv)
{
    printf("inhal - " __DATE__ " " __TIME__ "\nby Devin Acker (Revenant)\n\n");

    if (argc < 4)
    {
        fprintf(stderr, "To insert compressed data into a ROM:\n"
                        "%s [options] infile romfile offset\n"

                        "To write compressed data to a new file:\n"
                        "%s [options] -n infile outfile\n\n"

                        "Compression options:\n"
                        "-fast  avoid less common compression methods (faster compression, but larger output)\n"
                        "-opt   perform shortest-path searching (smaller output, but slower compression)\n"
                        "\n"
                        "-1     fastest compression (same as -fast)\n"
                        "-2     fast compression (default)\n"
                        "-3     better compression (same as -fast -opt)\n"
                        "-4     best compression (same as -opt)\n"

                        "\nExample:\n%s -fast test.chr kirbybowl.sfc 0x70000\n"
                        "%s -n test.chr test-packed.bin\n\n"
                        "offset can be in either decimal or hex.\n",
                argv[0], argv[0], argv[0], argv[0]);
        exit(-1);
    }

    FILE *infile, *outfile;
    int fileoffset;
    int newfile = 0;
    pack_options_t options;

    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-n"))
        {
            newfile = 1;
        }
        else if (!strcmp(argv[i], "-fast"))
        {
            options.fast = 1;
        }
        else if (!strcmp(argv[i], "-opt"))
        {
            options.optimal = 1;
        }
        else if (!strcmp(argv[i], "-1"))
        {
            options.fast = 1;
            options.optimal = 0;
        }
        else if (!strcmp(argv[i], "-2"))
        {
            options.fast = 0;
            options.optimal = 0;
        }
        else if (!strcmp(argv[i], "-3"))
        {
            options.fast = 1;
            options.optimal = 1;
        }
        else if (!strcmp(argv[i], "-4"))
        {
            options.fast = 0;
            options.optimal = 1;
        }
    }

    if (options.fast)
        printf("Fast compression enabled.\n");
    if (options.optimal)
        printf("Optimal compression (shortest path) enabled.\n");

    // check for -n switch
    if (newfile)
    {
        fileoffset = 0;
        infile = fopen(argv[argc - 2], "rb");
        outfile = fopen(argv[argc - 1], "wb");
    }
    else
    {
        fileoffset = strtol(argv[argc - 1], NULL, 0);
        infile = fopen(argv[argc - 3], "rb");
        outfile = fopen(argv[argc - 2], "r+b");
    }

    if (!infile)
    {
        fprintf(stderr, "Error: unable to open input file\n");
        exit(-1);
    }
    if (!outfile)
    {
        fprintf(stderr, "Error: unable to open output file\n");
        exit(-1);
    }

    size_t inputsize, outputsize;
    uint8_t unpacked[DATA_SIZE];
    uint8_t packed[DATA_SIZE] = {0};

    // check size of input file
    fseek(infile, 0, SEEK_END);
    inputsize = ftell(infile);

    printf("Uncompressed size:  %lu bytes\n", (unsigned long)inputsize);

    if (inputsize > DATA_SIZE)
    {
        fprintf(stderr, "Error: File must be a maximum of 65,536 bytes!\n");
        exit(-1);
    }
    else if (!inputsize)
    {
        fprintf(stderr, "Error: Input file is empty!\n");
        exit(-1);
    }

    // read the file
    fseek(infile, 0, SEEK_SET);
    fread(unpacked, sizeof(uint8_t), inputsize, infile);
    if (ferror(infile))
    {
        perror("Error reading input file");
        exit(-1);
    }

    // compress the file
    clock_t time = clock();
    outputsize = exhal_pack2(unpacked, inputsize, packed, &options);
    time = clock() - time;

    if (outputsize)
    {
        // write the compressed data to the file
        fseek(outfile, fileoffset, SEEK_SET);
        fwrite((const void *)packed, 1, outputsize, outfile);
        if (ferror(outfile))
        {
            perror("Error writing output file");
            exit(-1);
        }

        printf("Compressed size:    %lu bytes\n", (unsigned long)outputsize);
        printf("Compression ratio:  %4.2f:1\n", (double)inputsize / outputsize);
        printf("Compression time:   %4.3f seconds\n\n", (double)time / CLOCKS_PER_SEC);

        printf("Inserted at 0x%06X - 0x%06lX\n", fileoffset, ftell(outfile) - 1);
    }
    else
    {
        fprintf(stderr, "Error: File could not be compressed because the resulting compressed data would\n"
                        "       have been larger than 64 kb.\n");
    }

    fclose(infile);
    fclose(outfile);
}
