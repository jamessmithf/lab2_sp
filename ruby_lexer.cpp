#include <bits/stdc++.h>
using namespace std;

struct Token { string type; string value; int line; int col; };

static const string RESET = "\x1b[0m";
static const string MAGENTA = "\x1b[95m";
static const string BLUE = "\x1b[94m";
static const string GREEN = "\x1b[92m";
static const string GREY = "\x1b[90m";
static const string YELLOW = "\x1b[93m";
static const string ERR_BG = "\x1b[41m";
static const string CYAN = "\x1b[96m";
static const string CYAN_BOLD = "\x1b[1;96m";

static unordered_set<string> KEYWORDS = {
    "BEGIN","END","alias","and","begin","break","case","class","def","defined?",
    "do","else","elsif","end","ensure","for","if","in","module","next","nil",
    "not","or","redo","rescue","retry","return","self","super","then","undef",
    "unless","until","when","while","yield","true","false"
};

static unordered_set<string> VALID_OPERATORS = {
    "=", "+", "-", "*", "/", "%", "<", ">", "&", "|", "^", "~", "!", "?", ":", ".",
    "==", "===", "!=", "<=", ">=", "<<", ">>", "&&", "||", "+=", "-=", "*=", "/=", "%=",
    "::", "..", "...", "->", "=~", "!~", "**", "?:",
    "<<=", ">>=", "&=", "|=", "^="
};

inline bool is_ascii_letter(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}
inline bool is_identifier_start(unsigned char c) {
    return (is_ascii_letter(c) || c == '_' || c >= 128);
}
inline bool is_identifier_part(unsigned char c) {
    return is_identifier_start(c) || (c >= '0' && c <= '9');
}
inline bool is_digit(char c) { return c >= '0' && c <= '9'; }
inline bool is_hex_digit(char c) {
    return (c >= '0' && c <= '9') ||
           (c >= 'A' && c <= 'F') ||
           (c >= 'a' && c <= 'f');
}
inline bool is_op_char(char c) {
    static const string s = "=+-*/%<>&|^~!:?.";
    return s.find(c) != string::npos;
}

vector<Token> tokenize(const string &src) {
    vector<Token> out;
    int n = (int)src.size();
    int pos = 0;
    int line = 1, line_start = 0;
    auto col = [&](int p){ return p - line_start + 1; };

    while (pos < n) {
        char c = src[pos];
        if (c == '\n') {
            out.push_back({"NEWLINE", "\n", line, col(pos)});
            pos++; line++; line_start = pos; continue;
        }
        if (c == ' ' || c == '\t' || c == '\r') {
            int st = pos;
            while (pos < n && (src[pos] == ' ' || src[pos] == '\t' || src[pos] == '\r')) pos++;
            out.push_back({"SKIP", src.substr(st, pos-st), line, col(st)});
            continue;
        }
        if (c == '#') { int st = pos; while (pos < n && src[pos] != '\n') pos++; out.push_back({"COMMENT", src.substr(st,pos-st), line, col(st)}); continue; }
        if (c == '"' || c == '\'') {
            char q = c; int st = pos; pos++; bool escaped=false, closed=false;
            while (pos < n) {
                char ch = src[pos];
                if (escaped) { escaped=false; pos++; continue; }
                if (ch == '\\') { escaped=true; pos++; continue; }
                if (ch == q) { pos++; closed=true; break; }
                if (ch == '\n') break;
                pos++;
            }
            string val = src.substr(st, pos-st);
            if (!closed) out.push_back({"ERROR", val, line, col(st)}); else out.push_back({"STRING", val, line, col(st)});
            continue;
        }
        if (c == '0' && pos+1 < n && (src[pos+1]=='x' || src[pos+1]=='X')) {
            int st = pos; pos += 2; int hexstart = pos;
            while (pos < n && is_hex_digit(src[pos])) pos++;
            if (pos == hexstart) { out.push_back({"ERROR", src.substr(st,pos-st), line, col(st)}); continue; }
            if (pos < n && is_identifier_start((unsigned char)src[pos])) { int idstart = pos; while (pos < n && is_identifier_part((unsigned char)src[pos])) pos++; out.push_back({"ERROR", src.substr(st,pos-st), line, col(st)}); continue; }
            out.push_back({"NUMBER", src.substr(st,pos-st), line, col(st)}); continue;
        }
        if (is_digit(c) || (c == '.' && pos+1 < n && is_digit(src[pos+1]))) {
            int st = pos; bool seen_dot=false, seen_e=false;
            if (c == '.') { seen_dot=true; pos++; }
            while (pos < n) {
                char ch = src[pos];
                if (is_digit(ch)) { pos++; continue; }
                if (!seen_dot && ch == '.') { if (pos+1 < n && src[pos+1]=='.') break; seen_dot=true; pos++; continue; }
                if (!seen_e && (ch=='e' || ch=='E')) { seen_e=true; pos++; if (pos<n && (src[pos]=='+'||src[pos]=='-')) pos++; int expstart=pos; while (pos<n && is_digit(src[pos])) pos++; if (pos==expstart) break; continue; }
                break;
            }
            if (pos < n && is_identifier_start((unsigned char)src[pos])) { int idstart=pos; while (pos<n && is_identifier_part((unsigned char)src[pos])) pos++; out.push_back({"ERROR", src.substr(st,pos-st), line, col(st)}); continue; }
            out.push_back({"NUMBER", src.substr(st,pos-st), line, col(st)}); continue;
        }
        if (is_identifier_start((unsigned char)c)) {
            int st = pos; pos++; while (pos < n && is_identifier_part((unsigned char)src[pos])) pos++;
            if (pos < n && (src[pos]=='?' || src[pos]=='!' || src[pos]=='=')) { pos++; }
            string val = src.substr(st,pos-st);
            if (KEYWORDS.count(val)) out.push_back({"KEYWORD", val, line, col(st)}); else out.push_back({"IDENT", val, line, col(st)});
            continue;
        }
        if (is_op_char(c)) {
            int st = pos; while (pos < n && is_op_char(src[pos])) pos++;
            string op = src.substr(st,pos-st);
            if (VALID_OPERATORS.count(op)) out.push_back({"OP", op, line, col(st)}); else out.push_back({"ERROR", op, line, col(st)});
            continue;
        }
        if (string("[](){},;").find(c) != string::npos) { out.push_back({"PUNC", string(1,c), line, col(pos)}); pos++; continue; }
        out.push_back({"ERROR", string(1,c), line, col(pos)}); pos++;
    }
    return out;
}

// Color-mode: reconstruct original source, but print tokens with colors.
// Also: color IDENT as CYAN; if IDENT is followed immediately by PUNC "(" -> treat as function name (CYAN_BOLD).
void print_colorized(const vector<Token> &tokens) {
    for (size_t i = 0; i < tokens.size(); ++i) {
        const Token &t = tokens[i];
        if (t.type == "KEYWORD") cout << MAGENTA << t.value << RESET;
        else if (t.type == "IDENT") {
            // check next non-SKIP token to see if it's "(" => function call/definition
            bool is_func = false;
            if (i+1 < tokens.size()) {
                // skip SKIP but not NEWLINE
                size_t j = i+1;
                while (j < tokens.size() && tokens[j].type == "SKIP") ++j;
                if (j < tokens.size() && tokens[j].type == "PUNC" && tokens[j].value == "(") is_func = true;
            }
            if (is_func) cout << CYAN_BOLD << t.value << RESET; else cout << CYAN << t.value << RESET;
        }
        else if (t.type == "NUMBER") cout << BLUE << t.value << RESET;
        else if (t.type == "STRING") {
            string s = t.value;
            if (!s.empty() && s[0] == '"') {
                // color interpolation #{...}
                for (size_t p = 0; p < s.size();) {
                    if (p+1 < s.size() && s[p]=='#' && s[p+1]=='{') {
                        // find closing }
                        size_t j = p+2; int depth=1;
                        while (j < s.size() && depth > 0) {
                            if (s[j] == '{') depth++; else if (s[j] == '}') depth--;
                            j++;
                        }
                        string inner = s.substr(p+2, (j>0?j-1: j) - (p+2));
                        cout << YELLOW << "#{" << RESET;
                        cout << BLUE << inner << RESET;
                        cout << YELLOW << "}" << RESET;
                        p = j;
                    } else {
                        // chunk until next #{ or end
                        size_t k = p;
                        while (k < s.size() && !(k+1 < s.size() && s[k]=='#' && s[k+1]=='{')) k++;
                        cout << GREEN << s.substr(p, k-p) << RESET;
                        p = k;
                    }
                }
            } else {
                cout << GREEN << s << RESET;
            }
        }
        else if (t.type == "COMMENT") cout << GREY << t.value << RESET;
        else if (t.type == "OP") cout << YELLOW << t.value << RESET;
        else if (t.type == "PUNC") cout << YELLOW << t.value << RESET;
        else if (t.type == "ERROR") cout << ERR_BG << t.value << RESET;
        else if (t.type == "NEWLINE") cout << '\n';
        else if (t.type == "SKIP") cout << t.value;
        else cout << t.value;
    }
}

int main(int argc, char **argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    bool color = false; string filename;
    for (int i = 1; i < argc; ++i) {
        string a = argv[i];
        if (a == "--color") color = true;
        else if (filename.empty()) filename = a;
    }

    string src;
    if (!filename.empty()) {
        ifstream f(filename, ios::binary);
        if (!f) { cerr << "Error: cannot open file: " << filename << "\n"; return 2; }
        ostringstream ss; ss << f.rdbuf(); src = ss.str();
    } else { ostringstream ss; ss << cin.rdbuf(); src = ss.str(); }

    auto tokens = tokenize(src);

    if (color) {
        print_colorized(tokens);
    } else {
        for (const auto &t : tokens) {
            if (t.type == "SKIP") continue;
            string v = t.value;
            // escape display for newline tokens (we print newline token as \n in non-color mode)
            if (t.type == "NEWLINE") v = "\\n";
            // escape tabs
            string esc;
            for (char ch : v) {
                if (ch == '\n') esc += "\\n";
                else if (ch == '\t') esc += "\\t";
                else esc.push_back(ch);
            }
            cout << "<" << t.type << ", '" << esc << "', " << t.line << ":" << t.col << ">\n";
        }
    }
    return 0;
}
