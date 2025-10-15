# lab2_sp
Sytem Programming - Lab 2. Lexical Analysis

Python bash:
cd /path/to/folder/with/files
python3 ruby_lexer.py ruby_code.rb              (list of tokens)
python3 ruby_lexer.py ruby_code.rb --color      (colored output)

C++ bash:
cd /path/to/folder/with/files
g++ -std=c++17 -O2 ruby_lexer.cpp -o ruby_lexer (compilation)
./ruby_lexer ruby_code.rb                       (list of tokens)
./ruby_lexer --color ruby_code.rb               (colored output)
cat ruby_code.rb | ./ruby_lexer --color         (from stdin)
