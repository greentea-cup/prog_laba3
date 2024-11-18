from string import ascii_letters, digits

keywords = ["continue", "for", "do", "while", "namespace", "struct", "class"]
def tokens(src):
    l = len(src)
    i = 0
    while src[i] != '\0':
        i = skip_garbage(src, l, i)
        if not (i < l):
            break
        if src[i] in first_id:
            j = parse_idkw(src, l, i)
            if src[i:j] in keywords:
                yield i, j, src[i:j]
            i = j
        elif src[i] in '(){};:':
            j = i+1
            yield i, j, src[i:j]
            i = j
        else:
            # j = i+1
            # yield i, j, f"<-{src[i:j]}->"
            # i = j
            i += 1
    pass

first_id = follow_id = "_" + digits + ascii_letters
def parse_idkw(src, l, i):
    if src[i] in first_id:
        i += 1
    else:
        text_error(src, l, i)
    while (i < l) and (src[i] in follow_id):
        i += 1
    return i

def skip_garbage(src, l, i):
    while i < l:
        prev_i = i
        # i = skip_whitespace(src, l, i)
        i = skip_mcomments(src, l, i)
        i = skip_ocomments(src, l, i)
        i = skip_preproc(src, l, i)
        i = skip_strings(src, l, i)
        if i == prev_i:
            return i
    return l

def skip_whitespace(src, l, i):
    while (i < l) and (src[i] in " \t\r\n"):
        i += 1
    return i

def skip_mcomments(src, l, i):
    i = skip_whitespace(src, l, i)
    if (i+1 < l) and (src[i:i+2] == '/*'):
        i += 2
        while (i < l) and (src[i-1:i+1] != '*/'):
            i += 1
    return i

def skip_ocomments(src, l, i):
    i = skip_whitespace(src, l, i)
    if (i+1 < l) and (src[i:i+2] == '//'):
        while (i < l) and (src[i] != '\n'):
            i += 1
    return i

def skip_preproc(src, l, i):
    i = skip_whitespace(src, l, i)
    if (i < l) and (src[i] == '#'):
        while (i < l) and (src[i] != '\n'):
            i += 1
    return i

def skip_strings(src, l, i):
    i = skip_whitespace(src, l, i)
    if (i+1 < l) and (src[i] == 'L') and (src[i+1] == '"' or src[i+1] == "'"):
        i += 1
    if (i < l) and (src[i] == '"' or src[i] == "'"):
        x = src[i]
        i += 2
        while (i < l) and (src[i-1] != x):
            i += 1
    return i

