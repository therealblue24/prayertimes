/* Turns tabs into spaces. Forcefully. */
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERROUT               \
    do {                     \
        if(f)                \
            fclose(f);       \
        free(in), free(out); \
        printf("error\n");   \
        fflush(stdout);      \
        return 1;            \
    } while(0)

int process_file(const char *file, char mode)
{
    uint8_t *in = NULL, *out = NULL;
    if(mode == 'v') {
        printf("\tdetab: %s... ", file);
        fflush(stdout);
    }
    FILE *f = fopen(file, "rb");
    if(!f) {
        ERROUT;
    }

    int rc = -1;
    rc = fseek(f, 0, SEEK_END);
    if(rc == -1) {
        ERROUT;
    }

    long sz = ftell(f);
    if(sz < 0) {
        ERROUT;
    }

    errno = 0;
    rewind(f);
    if(errno) {
        ERROUT;
    }

    in = malloc(sz);
    out = malloc(sz * 4);

    if(!in || !out) {
        /* Safe because free(NULL) does nothing. */
        free(in);
        free(out);
        ERROUT;
    }

    rc = fread(in, 1, sz, f);
    if(rc != sz) {
        ERROUT;
    }

    fclose(f);

    long j = 0;
    int hastab = 0;

    for(long i = 0; i < sz; i++) {
        if(in[i] == '\t') {
            hastab = 1;
            for(int k = 0; k < 4; k++) {
                out[j++] = ' ';
            }
        } else
            out[j++] = in[i];
    }

    free(in);

    if(!hastab) {
        free(out);
        fclose(f);
        printf("skipped\n");
        fflush(stdout);
        return 0;
    }

    f = fopen(file, "wb");
    if(!f) {
        ERROUT;
    }

    rc = fwrite(out, 1, j, f);

    if(rc != j) {
        ERROUT;
    }

    fclose(f);

    free(out);

    if(mode == 'v') {
        printf("%zu -> %zu\n", sz, j);
        fflush(stdout);
    }

    return 0;
}

int main(int argc, char *argv[])
{
    if(argc < 2) {
        fprintf(stderr, "%s: usage: %s <v|q> <files...>\n", argv[0], argv[0]);
        return 1;
    }
    char mode = argv[1][0];
    int i = 2;
    while(i < argc) {
        const char *arg = argv[i++];
        if(process_file(arg, mode)) {
            fprintf(stderr, "%s: failed to detab: %s\n", argv[0],
                    strerror(errno));
            return 1;
        }
    }

    static const char *plural[2] = {"", "s"};

    if(mode == 'v') {
        printf("OK (detab'd %d file%s)\n", argc - 2, plural[(argc - 2) > 1]);
    }

    return 0;
}
