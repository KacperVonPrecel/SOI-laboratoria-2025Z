#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    char zero;
    char *filename;
    int size;
    int i;
    FILE *f;
    
    if (argc != 3) {
        printf("GENERATOR ERROR: Wrong number of arguments.\n");
        printf("Usage: ./gen <filename> <size_in_bytes>\n");
        return 1;
    }

    filename = argv[1];
    size = atoi(argv[2]);

    printf("DEBUG: Creating file '%s' of size %d B... ", filename, size);

    f = fopen(filename, "wb");
    if (!f) {
        printf("\nERROR: Cannot open file!\n");
        perror("System error");
        return 1;
    }

    zero = 0;
    for (i = 0; i < size; i++) {
        if (fputc(zero, f) == EOF) {
            printf("\nERROR: Failed to save file in byte of nr %d!\n", i);
            fclose(f);
            return 1;
        }
    }

    fclose(f);
    printf("OK (Success)\n");
    return 0;
}

