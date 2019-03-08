#include <stdio.h>
#include <glob.h>
#include <stdlib.h>
#include <string.h>


int check_or_logic(int *bools, int count) {
    int b = 0;
    for (int i = 0; i < count; i++) {
        if (bools[i] == 1) { 
            b++; break;
        }
    }
    return b;
}

int check_and_logic(int *bools, int count) {
    int b = 0;
    for (int i = 0; i < count; i++)
        b += bools[i];

    if (b == count) return 1;
    else return 0;
}

// compares substring within file with pattern provided
// returns 1 if substring == pattern
// returns 0 if substring != pattern
// returns -1 if EOF is reached while reading substring
int substring_cmp(char *pattern, int length, FILE *file) {
    char *substr = malloc(1 * length);
    fread(substr, 1, length, file);
    if (strlen(substr) < length) return -1;
    int same = strncmp(substr, pattern, length);
    if (same == 0) return 1;
    else return 0;
}

// reads one filename from line in file
// resulting filename will not include '\n'
// returns pointer to read filename 
char *read_fname(FILE* file) {
    int fname_len = 0;
    int cursor;
    // counts len of string preceding newline character for malloc
    while(1) {
        cursor = fgetc(file);
        if (cursor == '\n' || cursor == EOF) break;
        else {
            fname_len++;
        }
    }
    
    char *fname = malloc(fname_len);
    
    // return file position indicator to start of current line
    fseek(file, -(fname_len + 1), SEEK_CUR);

    // read filename
    int i = 0;
    while(1) {
        cursor = fgetc(file);
        if (cursor == '\n' || cursor == EOF) break;
        else {
            fname[i] = cursor;
            i++;
        } 
    }

    return fname;
}

// reads all pattern names from file and stores them in array
// returns pointer to that array
char **get_patternnames(int *out_count, FILE *file) {
    int line_count = 0;
    int ch = 0;
    while((ch = fgetc(file)) != EOF)
        if (ch == '\n') line_count++;
    
    rewind(file);
    
    char **patternnames = malloc(sizeof(char*) * line_count);    
    
    for (int i = 0; i < line_count; i++) {
        char *fname = read_fname(file);
        *(patternnames + i) = fname;
    }

    // additionaly set line_count to out_count
    // for further mallocing 'found_in'
    *out_count = line_count;

    return patternnames;
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf("Usage: logfind [-or] [pattern ...]\n");
        return 1;
    } else if (argc == 2) {
        if (strncmp(argv[1], "-or", 3) == 0) {
            printf("No patterns provided.\n");
            return 1;
        }
    }

    FILE *logfind = fopen(".logfind", "r");
    int pattern_count = 0;
    // patternames contains patterns going to be fed to glob()
    char **patternnames = get_patternnames(&pattern_count, logfind);    

    glob_t pglob;
    int matchc = 0;
    for (int i = 0; i < pattern_count; i++) {
        if (i == 0) glob(patternnames[i], GLOB_NOSORT, NULL, &pglob);
        else glob(patternnames[i], GLOB_NOSORT | GLOB_APPEND, NULL, &pglob);
        matchc += pglob.gl_matchc;
    }

    // which files passed
    int *found_in = malloc(sizeof(int) * matchc);
    memset(found_in, 0, sizeof(int) * matchc);

    // patterns to search in files found by glob()
    char **patterns = argv + 1;
    int ptrn_count = argc - 1;
    int or_logic = 0;

    if (argc > 2)
        if (strncmp(argv[1], "-or", 3) == 0) {
            patterns = argv + 2; // 1st is executable name, 2nd is -o parameter
            ptrn_count--;    
            or_logic++; // patterns are meant to be 'or' logic
                        // by default - 'and' logic
        }

    int cursor;
    int same = 0;

    for (int j = 0; j < matchc; j++) {
        FILE *file = fopen(pglob.gl_pathv[j], "r");
        if (!file) { // no such file exists;
            continue;
        }
                
        // will be used to handle 'or' or 'and' logic of pattern search
        int *found_ptrn = malloc(sizeof(int) * ptrn_count);
        memset(found_ptrn, 0, sizeof(int) * ptrn_count);

        while (1) {
            cursor = fgetc(file);

            if (cursor == EOF) break;
            for (int i = 0; i < ptrn_count; i++) {
                if (cursor == patterns[i][0]) {
                    int len = strlen(patterns[i] + 1);
                    same = substring_cmp((patterns[i] + 1), len, file);

                    if (same == 0) { // not identical
                                     // return back position indicator
                        fseek(file, -(len), SEEK_CUR);
                    } else if (same == 1){ // identical
                        if (found_ptrn[i] != 1) found_ptrn[i] = 1;
                    } else { // EOF reached, done with this file
                        continue;
                    }
                }
            }
        }

        if (or_logic) {
            if (check_or_logic(found_ptrn, ptrn_count))
                found_in[j] = 1;      
        } else {
            if (check_and_logic(found_ptrn, ptrn_count))
                found_in[j] = 1;
        }

        fclose(file);
    }

    for (int i = 0; i < matchc; i++) {
        if (found_in[i] == 1)
            printf("Found in %s\n", pglob.gl_pathv[i]);
    }
    
    free(found_in);
    globfree(&pglob);
    fclose(logfind);

    return 0;
}
