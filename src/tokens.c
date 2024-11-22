#include <stdio.h>
#include "types.h"

static inline void print_expected_tokens(FILE *out, kw_t *exl) {
	myfprintf(out, "[%s", keywords[*exl]);
	for (exl++; *exl; exl++) myfprintf(out, ", %s", keywords[*exl]);
	myfprintf(out, "]");
}

static inline size_t token_error(control_t cc, token const *tt, size_t len, size_t i, kw_t *exl) {
	(void)len;
	kw_t t;
	long line, col;
	if (i < len) { t = tt[i].t; line = tt[i].lineno; col = tt[i].colno; }
	else { t = KW_EOF; line = -1; col = -1; }
	myfprintf(cc.ferrs, "Error: (token %zu) expected one of ", i);
	print_expected_tokens(cc.ferrs, exl);
	myfprintf(cc.ferrs, ", but got %s (%zu:%zu)\n", keywords[t], line, col);
	return i+1;
}

static inline size_t term(control_t cc, token const *tt, size_t len, size_t i, kw_t ex) {
	if (i < len && tt[i].t == ex) return i+1;
	return token_error(cc, tt, len, i, (kw_t[]){ex, 0});
}

static inline size_t term_any(control_t cc, token const *tt, size_t len, size_t i, kw_t *exl) {
	if (i < len) {
		for (kw_t t = tt[i].t; *exl; exl++)
			if (t == *exl) return i+1;
	}
	return token_error(cc, tt, len, i, exl);
}

static inline size_t parse_classdef(control_t cc, token const *tt, size_t len, size_t i);
static inline size_t parse_namespace(control_t cc, token const *tt, size_t len, size_t i);
static inline size_t parse_func(control_t cc, token const *tt, size_t len, size_t i);
static inline size_t parse_balanced_expr(control_t cc, token const *tt, size_t len, size_t i);
static inline size_t parse_loop(control_t cc, token const *tt, size_t len, size_t i);
static inline size_t parse_for(control_t cc, token const *tt, size_t len, size_t i);
static inline size_t parse_do(control_t cc, token const *tt, size_t len, size_t i);
static inline size_t parse_while(control_t cc, token const *tt, size_t len, size_t i);
static inline size_t parse_loop_block(control_t cc, token const *tt, size_t len, size_t i);
static inline size_t parse_loop_stmt(control_t cc, token const *tt, size_t len, size_t i);
static inline size_t parse_loop_body(control_t cc, token const *tt, size_t len, size_t i);

static inline size_t parse_classdef(control_t cc, token const *tt, size_t len, size_t i) {
	myfprintf(cc.ftree, "class start %zu\n", i);
	i = term_any(cc, tt, len, i, (kw_t[]){KW_class, KW_struct, 0});
	if (i < len && tt[i].t == ':')
		i++; // inheritance colon
	if (i < len && tt[i].t == ';')
		i++;
	else if (i < len && tt[i].t == '{') {
		i++;
		while (i < len && tt[i].t != '}') {
			if (tt[i].t == ':' || tt[i].t == ';') i++;
			else if (tt[i].t == '(')
				i = parse_func(cc, tt, len, i);
			else if (tt[i].t == KW_class || tt[i].t == KW_struct)
				i = parse_classdef(cc, tt, len, i);
			else
				i = token_error(cc, tt, len, i, (kw_t[]){':', ';', '(', KW_class, KW_struct, 0});
		}
		i = term(cc, tt, len, i, '}');
	}
	else i = token_error(cc, tt, len, i, (kw_t[]){'{', ';', 0});
	myfprintf(cc.ftree, "class end %zu\n", i);
	return i;
}

static inline size_t parse_balanced_expr(control_t cc, token const *tt, size_t len, size_t i) {
	if (tt[i].t == '(') {
		// implied '('
		i = parse_balanced_expr(cc ,tt, len, i+1);
		i = term(cc, tt, len, i, ')');
	}
	else if (tt[i].t == '{') {
		// implied '{'
		i = parse_balanced_expr(cc, tt, len, i+1);
		i = term(cc, tt, len, i, '}');
	}
	// inside is optional => no "else error"
	return i;
}

static inline size_t parse_func(control_t cc, token const *tt, size_t len, size_t i) {
	myfprintf(cc.ftree, "func start %zu\n", i);
	i = term(cc, tt, len, i, '(');
	i = term(cc, tt, len, i, ')');
	i = term(cc, tt, len, i, '{');
	while (i < len && tt[i].t != '}') {
		myfprintf(cc.ftree, "func stmt start %zu\n", i);
		if (tt[i].t == ';') i++;
		else if (tt[i].t == KW_for || tt[i].t == KW_do || tt[i].t == KW_while)
			i = parse_loop(cc, tt, len, i);
		else if (tt[i].t == '(')
			i = parse_balanced_expr(cc, tt, len, i);
		else
			i = token_error(cc, tt, len, i, (kw_t[]){';', KW_for, KW_do, KW_while, 0});
		myfprintf(cc.ftree, "func stmt end %zu\n", i);
	}
	i = term(cc, tt, len, i, '}');
	myfprintf(cc.ftree, "func end %zu\n", i);
	return i;
}

static inline size_t parse_for(control_t cc, token const *tt, size_t len, size_t i) {
	i = term(cc, tt, len, i, KW_for);
	i = term(cc, tt, len, i, '(');
	if (tt[i].t == ':') {
		// range-based for
		// implied ':'
		i = term(cc, tt, len, i+1, ')');
	}
	else if (i+1 < len && tt[i].t == ';' && tt[i+1].t == ';') {
		// normal for
		// implied ';' ';'
		i = term(cc, tt, len, i+2, ')');
	}
	else i = token_error(cc, tt, len, i, (kw_t[]){';', ':', 0});
	i = parse_loop_body(cc, tt, len, i);
	return i;
}

static inline size_t parse_do(control_t cc, token const *tt, size_t len, size_t i) {
	i = term(cc, tt, len, i, KW_do);
	i = parse_loop_body(cc, tt, len, i);
	i = term(cc, tt, len, i, KW_while);
	i = term(cc, tt, len, i, '(');
	i = parse_balanced_expr(cc, tt, len, i);
	i = term(cc, tt, len, i, ')');
	i = term(cc, tt, len, i, ';');
	return i;
}

static inline size_t parse_while(control_t cc, token const *tt, size_t len, size_t i) {
	i = term(cc, tt, len, i, KW_while);
	i = term(cc, tt, len, i, '(');
	i = parse_balanced_expr(cc, tt, len, i);
	i = term(cc, tt, len, i, ')');
	i = parse_loop_body(cc, tt, len, i);
	return i;
}

static inline size_t parse_loop(control_t cc, token const *tt, size_t len, size_t i) {
	myfprintf(cc.ftree, "loop start (%s) %zu\n", keywords[tt[i].t], i);
	if (tt[i].t == KW_for)
		i = parse_for(cc, tt, len, i);
	else if (tt[i].t == KW_do)
		i = parse_do(cc, tt, len, i);
	else if (tt[i].t == KW_while)
		i = parse_while(cc, tt, len, i);
	else
		i = token_error(cc, tt, len, i, (kw_t[]){KW_for, KW_do, KW_while, 0});
	myfprintf(cc.ftree, "loop end %zu\n", i);
	return i;
}

static inline size_t parse_loop_block(control_t cc, token const *tt, size_t len, size_t i) {
	myfprintf(cc.ftree, "loop block start %zu\n", i);
	i = term(cc, tt, len, i, '{');
	while (i < len && tt[i].t != '}') {
		myfprintf(cc.ftree, "loop stmt start %zu\n", i);
		i = parse_loop_stmt(cc, tt, len, i);
		myfprintf(cc.ftree, "loop stmt end %zu\n", i);
	}
	i = term(cc, tt, len, i, '}');
	myfprintf(cc.ftree, "loop block end %zu\n", i);
	return i;
}

static inline size_t parse_loop_stmt(control_t cc, token const *tt, size_t len, size_t i) {
	if (tt[i].t == ';') i++;
	else if (tt[i].t == '(')
		i = parse_balanced_expr(cc, tt, len, i);
	else if (tt[i].t == KW_continue)
		// implied KW_continue
		i = term(cc, tt, len, i+1, ';');
	else i = token_error(cc, tt, len, i, (kw_t[]){';', '(', KW_continue, 0});
	return i;
}

static inline size_t parse_loop_body(control_t cc, token const *tt, size_t len, size_t i) {
    myfprintf(cc.ftree, "loop body start %zu\n", i);
    if (i < len && tt[i].t == '{')
		i = parse_loop_block(cc, tt, len, i);
    else
		i = parse_loop_stmt(cc, tt, len, i);
    myfprintf(cc.ftree, "loop body end %zu\n", i);
    return i;
}

static inline size_t parse_namespace(control_t cc, token const *tt, size_t len, size_t i) {
	myfprintf(cc.ftree, "namespace start %zu\n", i);
	i = term(cc, tt, len, i, KW_namespace);
	i = term(cc, tt, len, i, '{');
	i = parse_prog(cc, tt, len, i);
	i = term(cc, tt, len, i, '}');
	myfprintf(cc.ftree, "namespace end %zu\n", i);
	return i;
}

size_t parse_prog(control_t cc, token const *tt, size_t len, size_t i) {
	myfprintf(cc.ftree, "prog start %zu\n", i);
	while ((i < len) && (tt[i].t != '}')) {
		if (tt[i].t == ';') i++;
		else if (
			i+1 < len
			&& (tt[i].t == KW_class || tt[i].t == KW_class)
			&& (tt[i+1].t == '{' || tt[i+1].t == ';'))
			i = parse_classdef(cc, tt, len, i);
		else if (tt[i].t == KW_namespace)
			i = parse_namespace(cc, tt, len, i);
		else if (
			i+3 < len
			&& tt[i].t == '('
			&& tt[i+1].t == ')'
			&& tt[i+2].t == '{') {
			i = parse_func(cc, tt, len, i);
		}
		else
			i = token_error(cc, tt, len, i, (kw_t[]){';', '(', KW_class, KW_struct, KW_namespace, 0});
	}
	myfprintf(cc.ftree, "prog end %zu\n", i);
	return i;
}
