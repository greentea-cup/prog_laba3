#ifndef TOKEN_T_H
#define TOKEN_T_H
#include <stddef.h>
#include <stdio.h>

#define myfprintf(x, ...) (x == NULL ? 0 : fprintf(x, __VA_ARGS__))

typedef struct {
	char *input;
	char *tree;
	char *tokens;
	char *errors;
	FILE *fin;
	FILE *ftree;
	FILE *ftok;
	FILE *ferrs;
} control_t;

typedef enum {
	KW_NOT_A_KW = 0,
	KW_continue,
	KW_for,
	KW_do,
	KW_while,
	KW_namespace,
	KW_struct,
	KW_class,
	KW_oppar = '(',
	KW_clpar = ')',
	KW_opbr = '{',
	KW_clbr = '}',
	KW_colon = ':',
	KW_semicolon = ';',

	KW_EOF = -1,
} kw_t;
extern char const *const keywords[];

typedef struct {
	kw_t t;
	size_t startpos, endpos;
	size_t lineno, colno;
} token;

size_t parse_prog(control_t cc, token const *tt, size_t len, size_t i);

token *tokens(char const *src, size_t len, size_t *out_len);

void lcmap_tokens(char const *src, token *tt, size_t tokens_len);
#endif
