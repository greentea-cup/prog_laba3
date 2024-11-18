#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"

int main(int argc, char **argv) {
	// args: input tree tokens errors
	if (argc < 5) {
		fprintf(stderr, "Not enough arguments\n Provide paths for: input tree tokens erorrs\n");
		return EXIT_FAILURE;
	}
	control_t cc = {
		.input = argv[1],
		.tree = argv[2],
		.tokens = argv[3],
		.errors = argv[4],
		.fin = fopen(cc.input, "r"),
		.ftree = NULL,
		.ftok = NULL,
		.ferrs = NULL,
	};
	if (cc.fin == NULL) return EXIT_FAILURE;
	fseek(cc.fin, 0, SEEK_END);
	size_t len;
	{
		long len0 = ftell(cc.fin);
		if (len0 < 0) {
			// stdin is input stream
			// but if you put data from pipe it will go immediately
			// program should read piped-in data and ignore the rest
			fprintf(stderr, "Input streams are not supported yet\n");
			return EXIT_FAILURE;
		}
		len = (size_t)len0;
	}
	char *src = malloc((len + 1) * sizeof(char));
	fseek(cc.fin, 0, SEEK_SET);
	size_t readcount = fread(src, 1, len, cc.fin);
	src[readcount] = '\0';
	fclose(cc.fin);
	cc.fin = NULL;

	size_t tokens_len;
	token *tt = tokens(src, len, &tokens_len);
	if (tt == NULL) return EXIT_FAILURE;
	lcmap_tokens(src, tt, tokens_len);
	cc.ftree = fopen(cc.tree, "w");
	cc.ftok = fopen(cc.tokens, "w");
	cc.ferrs = fopen(cc.errors, "w");
	if (cc.ftree == NULL || cc.ftok == NULL || cc.ferrs == NULL) {
		fprintf(stderr, "Cannot open one of output files\n");
		free(tt);
		return EXIT_FAILURE;
	}	
	for (size_t i = 0; i < tokens_len; i++) {
		fprintf(cc.ftok,
			"%zu %zu %zu %zu:%zu %s\n",
			i, tt[i].startpos, tt[i].endpos,
			tt[i].lineno, tt[i].colno, tt[i].t == KW_EOF ? "<EOF>" : keywords[tt[i].t]
		);
	}
	fclose(cc.ftok);
	cc.ftok = NULL;
	parse_prog(cc, tt, tokens_len, 0);
	free(tt);
	fclose(cc.ftree);
	fclose(cc.ferrs);
	return EXIT_SUCCESS;
}
