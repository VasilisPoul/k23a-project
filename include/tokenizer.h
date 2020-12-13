#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

typedef struct tokenizer_s tokenizer_t;
typedef char *(*next_token_f)(tokenizer_t *);

struct tokenizer_s {
    next_token_f next;
    bool feof;
    FILE *f;
    size_t buf_sz;
    char *buf;
};

tokenizer_t *tokenizer_new_from_filename(char *, next_token_f next);
tokenizer_t *tokenizer_new_from_string(char *string, next_token_f next);
void tokenizer_free(tokenizer_t *tok);

char *tokenizer_next(tokenizer_t *tok);


tokenizer_t *json_tokenizer_from_filename(char *filename);
tokenizer_t *json_tokenizer_from_string(char *string);
