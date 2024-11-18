from text import tokens
from tokens import parse_prog

def main():
    with open("./test1.cpp") as f:
        data = f.read()
    tt0 = list(tokens(data))
    ranges = [(t[0], t[1]) for t in tt0]
    lcmap = []
    i = 0
    line = col = 1
    for s, _ in ranges:
        while i < s:
            c = data[i]
            if c == '\n':
                line += 1
                col = 1
            else:
                col += 1
            i += 1
        lcmap.append((line, col))
    lcmap.append((0, 0))
    tt = [t[2] for t in tt0]
    with open("py_tokens", "w") as tokens_out:
        for i, (s, e, t) in enumerate(tt0):
            print(i, s, e, f"{lcmap[i][0]}:{lcmap[i][1]}", t, file=tokens_out)
    parse_prog(tt, lcmap, len(tt0), 0)

if __name__ == '__main__': main()

