#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "types.h"

char const *const keywords[] = {
	[KW_NOT_A_KW] = "<?>",
	[KW_continue] = "continue",
	[KW_for] = "for",
	[KW_do] = "do",
	[KW_while] = "while",
	[KW_namespace] = "namespace",
	[KW_struct] = "struct",
	[KW_class] = "class",
	[KW_oppar] = "(",
	[KW_clpar] = ")",
	[KW_opbr] = "{",
	[KW_clbr] = "}",
	[KW_colon] = ":",
	[KW_semicolon] = ";",
};

static inline bool is_id(int c) {
	return (
		(c >= 'A' && c <= 'Z') ||
		(c >= 'a' && c <= 'z') ||
		(c >= '0' && c <= '0') ||
		(c == '_'));
}

static inline size_t skip_whitespace(char const *src, size_t len, size_t i) {
	while (i < len && isspace(src[i])) i++;
	return i;
}

static inline size_t skip_mcomments(char const *src, size_t len, size_t i) {
	i = skip_whitespace(src, len, i);
	if ((i + 1 < len) && (src[i] == '/') && (src[i + 1] == '*')) {
		i += 2;
		while (i < len && !((src[i - 1] == '*') && (src[i] == '/'))) i++;
	}
	return i;
}

static inline size_t skip_ocomments(char const *src, size_t len, size_t i) {
	i = skip_whitespace(src, len, i);
	if (((i + 1) < len) && (src[i] == '/') && (src[i + 1] == '/')) {
		while ((i < len) && (src[i] != '\n')) i++;
	}
	return i;
}

static inline size_t skip_preproc(char const *src, size_t len, size_t i) {
	i = skip_whitespace(src, len, i);
	if ((i < len) && (src[i] == '#')) {
		while ((i < len) && (src[i] != '\n')) i++;
	}
	return i;
}

static inline size_t skip_strings(char const *src, size_t len, size_t i) {
	i = skip_whitespace(src, len, i);
	if (((i + 1) < len) && (src[i] == 'L') && ((src[i + 1] == '"') || (src[i + 1] == '\'')))
		i++;
	if ((i < len) && ((src[i] == '"') || (src[i] == '\''))) {
		char x = src[i];
		i += 2;
		while ((i < len) && (src[i - 1] != x)) i++;
	}
	return i;
}

static inline size_t skip_garbage(char const *src, size_t len, size_t i) {
	while (i < len) {
		size_t prev_i = i;
		i = skip_mcomments(src, len, i);
		i = skip_ocomments(src, len, i);
		i = skip_preproc(src, len, i);
		i = skip_strings(src, len, i);
		if (i == prev_i) return i;
	}
	return len;
}

static inline kw_t try_get_keyword(char const *src, size_t len, size_t start,
	size_t end) {
	if (end > len) return KW_NOT_A_KW;
	char const *s = src + start;
	size_t n = end - start;
#define CHECK_KW(x) if (n == strlen(#x) && !strncmp(#x, s, n)) return KW_ ## x
	CHECK_KW(continue);
	else CHECK_KW(for);
	else CHECK_KW(do);
	else CHECK_KW(while);
	else CHECK_KW(namespace);
	else CHECK_KW(struct);
	else CHECK_KW(class);
	// if (!strcmp("continue", s)) return KW_continue;
	// else if (!strcmp("for", s)) return KW_for;
	// else if (!strcmp("do", s)) return KW_do;
	// else if (!strcmp("while", s)) return KW_while;
	// else if (!strcmp("namespace", s)) return KW_namespace;
	// else if (!strcmp("struct", s)) return KW_struct;
	// else if (!strcmp("class", s)) return KW_class;
	return KW_NOT_A_KW;
}

static inline size_t parse_idkw(char const *src, size_t len, size_t i) {
	i++;
	while (i < len && is_id(src[i])) i++;
	return i;
}

token *tokens(char const *src, size_t len, size_t *out_len) {
	size_t i = 0, reslen = 0, rescap = 16;
	token *res = malloc(sizeof(token) * rescap);
	if (res == NULL) return NULL;
#define PUSH_TOKEN(...) {\
		if (reslen == rescap) {\
			token *tmp = realloc(res, sizeof(token) * rescap * 2);\
			if (tmp == NULL) goto OOM;\
			rescap *= 2; res = tmp;\
		}\
		res[reslen++] = (__VA_ARGS__);\
	}
	while (i < len && src[i] != '\0') {
		i = skip_garbage(src, len, i);
		if (i >= len) break;
		else if (is_id(src[i])) {
			size_t j = parse_idkw(src, len, i);
			kw_t t;
			if ((t = try_get_keyword(src, len, i, j)) != 0) {
				PUSH_TOKEN((token) {.t = t, .startpos = i, .endpos = j});
			}
			i = j;
		}
		else if (strchr("(){};:", src[i]) != NULL) {
			PUSH_TOKEN((token) {.t = src[i], .startpos = i, .endpos = i + 1});
			i++;
		}
		else i++;
	}
	*out_len = reslen;
	return res;
#undef PUSH_TOKEN
OOM:
	free(res);
	return NULL;
}

void lcmap_tokens(char const *src, token *tt, size_t tokens_len) {
	size_t j = 0, line = 1, col = 1;
	for (size_t i = 0; i < tokens_len; i++) {
		for (size_t s = tt[i].startpos; j < s; j++) {
			if (src[j] == '\n') {
				line++;
				col = 1;
			}
			else col++;
		}
		tt[i].lineno = line;
		tt[i].colno = col;
	}
}
