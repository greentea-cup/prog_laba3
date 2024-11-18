import inspect
from sys import stderr

ignored_funcs = ["term", "term_any", "term_seq", "_x", "_y", "main", "<module>"]

def token_error(tt, rr, l, i, ex):
    start, end = _y(tt, rr, l, i)
    exs = ", ".join(ex)
    got = _x(tt, rr, l, i)
    if got == "continue":
        print("Errorneous continue (token %d) at (%d:%d)" % (i, start, end), file=stderr)
        return i + 1
    elif got in ["for", "do", "while"]:
        print("Errorneous loop (token %d) at (%d:%d)" % (i, start, end), file=stderr)
        return i + 1
    fmt_args = (i, start, end, exs if len(ex) == 1 else f"one of [{exs}]", got)
    error_text = "Error at token %d (%d:%d): expected %s, got %s" % fmt_args
    print(error_text, file=stderr)
    print("===Token error start===", file=stderr)
    print(", ".join([fr.function.split("_", 1)[-1] for fr in inspect.stack()[1:] if fr.function not in ignored_funcs]), file=stderr)
    print("=== Token error end ===", file=stderr)
    return i + 1

def _x(tt, rr, l, i):
    if i < l:
        return tt[i]
    return "<EOF>"

def _y(tt, rr, l, i):
    if i < l:
        return rr[i]
    # return rr[l-1][1], rr[l-1][1]
    return -1, -1

def term_any(tt, rr, l, i, expected_list):
    if (i < l) and (tt[i] in expected_list):
        i += 1
    else:
        i = token_error(tt, rr, l, i, expected_list)
    return i

def term(tt, rr, l, i, expected):
    return term_any(tt, rr, l, i, [expected])

def term_seq(tt, rr, l, i, seq):
    for expected in seq:
        if isinstance(expected, str):
            i = term(tt, rr, l, i, expected)
        else:
            i = term_any(tt, rr, l, i, expected)
    return i

def parse_classdef(tt, rr, l, i):
    print("class start", i)
    i = term_any(tt, rr, l, i, ["class", "struct"])
    if (i < l) and tt[i] == ":":
        i += 1 # inheritance colon
    if (i < l) and tt[i] == ";":
        i = term(tt, rr, l, i, ";")
    elif (i < l) and tt[i] == "{":
        i = term(tt, rr, l, i, "{")
        # while (i < l) and (tt[i] in [":", "(", ";", "class", "struct"]):
        while (i < l) and tt[i] != "}":
            if tt[i] in [":", ";"]:
                i += 1
            elif tt[i] == "(":
                i = parse_func(tt, rr, l, i)
            elif tt[i] in ["class", "struct"]:
                i = parse_classdef(tt, rr, l, i)
            else:
                i = token_error(tt, rr, l, i, [":", ";", "(", "class", "struct"])
                # i += 1
        i = term(tt, rr, l, i, "}")
    else:
        i = token_error(tt, rr, l, i, ["{", ";"])
    print("class end", i)
    return i

def parse_balanced_expr(tt, rr, l, i):
    if tt[i] == "(":
        i = term(tt, rr, l, i, "(")
        i = parse_balanced_expr(tt, rr, l, i)
        i = term(tt, rr, l, i, ")")
    elif tt[i] == "{":
        i = term(tt, rr, l, i, "{")
        i = parse_balanced_expr(tt, rr, l, i)
        i = term(tt, rr, l, i, "}")
    # inside is optional => no "else error"
    return i

def parse_func(tt, rr, l, i):
    print("func start", i)
    i = term_seq(tt, rr, l, i, ["(", ")", "{"])
    # while (i < l) and (tt[i] in [";", "for", "do", "while"]):
    while (i < l) and (tt[i] != "}"):
        print("func stmt start", i)
        if tt[i] == ";":
            i += 1
        elif tt[i] in ["for", "do", "while"]:
            i = parse_loop(tt, rr, l, i)
        else:
            i = token_error(tt, rr, l, i, [";", "for", "do", "while"])
        print("func stmt end", i)
    i = term(tt, rr, l, i, "}")
    print("func end", i)
    return i

def parse_for(tt, rr, l, i):
    i = term_seq(tt, rr, l, i, ["for", "("])
    if tt[i] == ":":
        # range-based for
        i = term_seq(tt, rr, l, i, [":", ")"])
    elif (i+1 < l) and (tt[i] == ";") and (tt[i+1] == ";"):
        # normal for
        i = term_seq(tt, rr, l, i, [";", ";", ")"])
    else:
        i = token_error(tt, rr, l, i, [";", ":"])
    i = parse_loop_body(tt, rr, l, i)
    return i

def parse_do(tt, rr, l, i):
    i = term(tt, rr, l, i, "do")
    i = parse_loop_body(tt, rr, l, i)
    i = term_seq(tt, rr, l, i, ["while", "("])
    i = parse_balanced_expr(tt, rr, l, i)
    i = term_seq(tt, rr, l, i, [")", ";"])
    return i

def parse_while(tt, rr, l, i):
    i = term_seq(tt, rr, l, i, ["while", "("])
    i = parse_balanced_expr(tt, rr, l, i)
    i = term(tt, rr, l, i, ")")
    i = parse_loop_body(tt, rr, l, i)
    return i

def parse_loop(tt, rr, l, i):
    print(f"loop start ({tt[i]})", i)
    if tt[i] == "for":
        i = parse_for(tt, rr, l, i)
    elif tt[i] == "do":
        i = parse_do(tt, rr, l, i)
    elif tt[i] == "while":
        i = parse_while(tt, rr, l, i)
    else:
        i = token_error(tt, rr, l, i, ["for", "do", "while"])
    print("loop end", i)
    return i

def parse_loop_block(tt, rr, l, i):
    print("loop block start", i)
    i = term(tt, rr, l, i, "{")
    while (i < l) and (tt[i] != "}"):
        print("loop stmt start", i)
        i = parse_loop_stmt(tt, rr, l, i)
        print("loop stmt end", i)
    i = term(tt, rr, l, i, "}")
    print("loop block end", i)
    return i

def parse_loop_stmt(tt, rr, l, i):
    if tt[i] == ";":
        i = term(tt, rr, l, i, ";")
    elif tt[i] == "(":
        i = parse_balanced_expr(tt, rr, l, i)
    elif tt[i] == "continue":
        i = term_seq(tt, rr, l, i, ["continue", ";"])
    else:
        i = token_error(tt, rr, l, i, [";", "(", "continue"])
    return i

def parse_loop_body(tt, rr, l, i):
    print("loop body start", i)
    if (i < l) and (tt[i] == "{"):
        i = parse_loop_block(tt, rr, l, i)
    else:
        i = parse_loop_stmt(tt, rr, l, i)
    print("loop body end", i)
    return i

def parse_namespace(tt, rr, l, i):
    print("namespace start", i)
    i = term_seq(tt, rr, l, i, ["namespace", "{"])
    i = parse_prog(tt, rr, l, i)
    i = term(tt, rr, l, i, "}")
    print("namespace end", i)
    return i

def parse_prog(tt, rr, l, i):
    print("prog start", i)
    while (i < l) and (tt[i] != "}"):
        if tt[i] == ";":
            i += 1
        elif (i+1 < l) and (tt[i] in ["class", "struct"]) and (tt[i+1] in ["{", ";"]):
            i = parse_classdef(tt, rr, l, i)
        elif tt[i] == "namespace":
            i = parse_namespace(tt, rr, l, i)
        elif (i+3 < l) and (tt[i] == "(") and (tt[i+1] == ")") and (tt[i+2] in ["{", ";"]):
            i = parse_func(tt, rr, l, i)
        else:
            i = token_error(tt, rr, l, i, [";", "class", "struct", "namespace", "("])
    print("prog end", i)
    return i

