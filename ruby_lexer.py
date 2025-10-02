import re
import sys
import argparse
from collections import namedtuple

Token = namedtuple('Token', 'type value line column')

KEYWORDS = {
    'BEGIN','END','alias','and','begin','break','case','class','def','defined?',
    'do','else','elsif','end','ensure','for','if','in','module','next','nil',
    'not','or','redo','rescue','retry','return','self','super','then','undef',
    'unless','until','when','while','yield','true','false'
}

token_specification = [
    ('NUMBER',    r'0[xX][0-9A-Fa-f]+|\d+(?:\.\d*)?(?:[eE][+-]?\d+)?'),
    ('STRING',    r'"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])*\''),
    ('IDENT',     r'[^\W\d]\w*[!?=]?'),
    ('COMMENT',   r'\#.*'),
    ('NEWLINE',   r'\n'),
    ('SKIP',      r'[ \t\r]+'),
    ('OP',        r'===|==|!=|<=|>=|<<|>>|\|\||&&|\+=|-=|\*=|/=|%=|::|->'),
    ('PUNC',      r'[+\-*/%<>=\[\]\(\)\{\},;.:]'),
    ('MISMATCH',  r'.'),
]

tok_regex = '|'.join('(?P<%s>%s)' % pair for pair in token_specification)
master_re = re.compile(tok_regex, re.UNICODE)

def tokenize(code):
    line_num = 1
    line_start = 0
    pos = 0
    mo = master_re.match(code, pos)
    while mo is not None:
        kind = mo.lastgroup
        value = mo.group(kind)
        col = mo.start() - line_start + 1
        if kind == 'NEWLINE':
            yield Token(kind, value, line_num, col)
            pos = mo.end()
            line_num += 1
            line_start = pos
        elif kind == 'MISMATCH':
            yield Token('ERROR', value, line_num, col)
            pos = mo.end()
        else:
            if kind == 'IDENT' and value in KEYWORDS:
                yield Token('KEYWORD', value, line_num, col)
            else:
                yield Token(kind, value, line_num, col)
            pos = mo.end()
        mo = master_re.match(code, pos)

def colorize_token(t: Token) -> str:
    reset = '\x1b[0m'
    colors = {
        'KEYWORD': '\x1b[95m',
        'IDENT':   '',
        'NUMBER':  '\x1b[94m',
        'STRING':  '\x1b[92m',
        'COMMENT': '\x1b[90m',
        'OP':      '\x1b[93m',
        'PUNC':    '\x1b[93m',
        'ERROR':   '\x1b[41m',
    }
    col = colors.get(t.type)
    if col:
        return f"{col}{t.value}{reset}"
    else:
        return t.value

def main():
    parser = argparse.ArgumentParser(description='Simple Ruby lexer')
    parser.add_argument('file', nargs='?', help='Ruby file to lex (if omitted, read stdin)')
    parser.add_argument('--color', action='store_true', help='Colorize source output in terminal')
    args = parser.parse_args()

    if args.file:
        try:
            with open(args.file, 'r', encoding='utf-8') as f:
                src = f.read()
        except FileNotFoundError:
            print(f"Error: file not found: {args.file}", file=sys.stderr)
            sys.exit(2)
    else:
        src = sys.stdin.read()

    if args.color:
        for t in tokenize(src):
            sys.stdout.write(colorize_token(t))
    else:
        for t in tokenize(src):
            if t.type == 'SKIP':
                continue
            print(f"<{t.type}, {t.value!r}, {t.line}:{t.column}>")

if __name__ == '__main__':
    main()
