#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

static int getnext(FILE *in)
{
    int c, i;
    again:
    c = fgetc(in);
    if (EOF == c)
        return -1;
    if (!isalnum(c))
        goto again;
    if (islower(c))
        c &= ~(0x20);
    if ((c < '0') || (c > 'F') || ((c > '9') && (c < 'A')))
        return -2;
    if ((c >= '0') && (c <= '9'))
        i = c - '0';
    else
        i = (c - 'A') + 10;
    return i;
}

static int undump(FILE *in, FILE *out)
{
    int i, r;

    while (1) {
        i = getnext(in);
        if (-1 == i)
            return 0;
        if (i < 0)
            return i;
        r = i << 4;
        i = getnext(in);
        if (i < 0)
            return i;
        r |= i;
        if (fputc(r, out) == EOF)
            return -3;
    }
}

static struct option long_options[] = {
    {"input", required_argument, 0, 'i'},
    {"output", required_argument, 0, 'o'},
    {"help", no_argument, 0, 'h'},
    {"version", no_argument, 0, 'v'},
    {0, 0, 0, 0}
};

static void help(void)
{
    puts("unhexdump [-i|--input INPUT_FILE] [-o|--output OUTPUT_FILE]");
    puts("");
    puts("Turns a hex dump into binary.");
    puts("Defaults to stdin and stdout.");
}

static void version(void)
{
    puts("unhexdump 0.01 by Chris Barts <chbarts@gmail.com> 2022");
}

int main(int argc, char *argv[])
{
    int option_index = 0, exitv, c;
    FILE *in = stdin, *out = stdout;
    char *inf = "stdin", *outf = "stdout";

    if (1 == argc) {
        switch (undump(stdin, stdout)) {
        case 0:
            return 0;
        case -1:
            fprintf(stderr, "Unexpected EOF on stdin\n");
            exit(EXIT_FAILURE);
        case -2:
            fprintf(stderr, "Invalid character on stdin\n");
            exit(EXIT_FAILURE);
        }
    }

    while ((c = getopt_long(argc, argv, "i:o:hv", long_options, &option_index)) != -1) {
        switch (c) {
            case 0:
                switch (option_index) {
                    case 0:
                         inf = optarg;
                         break;
                    case 1:
                         outf = optarg;
                         break;
                    case 2:
                         help();
                         return 0;
                    case 3:
                         version();
                         return 0;
                }

                break;
            case 'i':
                inf = optarg;
                break;
            case 'o':
                outf = optarg;
                break;
            case 'h':
                help();
                return 0;
            case 'v':
                version();
                return 0;
            case '?':
                help();
                exit(EXIT_FAILURE);
            default:
                help();
                exit(EXIT_FAILURE);
        }
    }

    if ((strcmp("stdin", inf) != 0) && (in = fopen(inf, "rb")) == NULL) {
        perror("unhexdump couldn't open input");
        exit(EXIT_FAILURE);
    }

    if ((strcmp("stdout", outf) != 0) && (out = fopen(outf, "wb")) == NULL) {
        perror("unhexdump couldn't open output");
        fclose(in);
        exit(EXIT_FAILURE);
    }

    switch (undump(in, out)) {
    case 0:
        exitv = EXIT_SUCCESS;
        break;
    case -1:
        exitv = EXIT_FAILURE;
        fprintf(stderr, "Unexpected EOF on %s\n", inf);
        break;
    case -2:
        exitv = EXIT_FAILURE;
        fprintf(stderr, "Invalid character on %s\n", inf);
        break;
    default:
        exitv = EXIT_FAILURE;
        fprintf(stderr, "Error writing to %s\n", outf);
        break;
    }

    if (stdin != in)
        fclose(in);

    if (stdout != out)
        fclose(out);

    exit(exitv);
}
