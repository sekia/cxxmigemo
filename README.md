cxxmigemo
===

cxxmigemo is a thin wrapper of [C/Migemo](https://github.com/koron/cmigemo) library written in C++11.

Usage
---

```c++
#include <iostream>

#include "cxxmigemo.h"

using namespace cxxmigemo;

int main(int argc, const char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <migemo-dictionary-dir>"
              << std::endl;
    return -1;
  }

  Migemo mgm;
  // Loads migemo dictionary and character mapping files under specified dir.
  mgm.LoadDictsInDirectory(argv[1]);

  // Generates regexp string matches possible japanese text.
  auto regexp =
      mgm.Query(reinterpret_cast<const unsigned char *>("konnnichihaSekai"));
  std::cout << regexp.get() << std::endl;

  return 0;
}
```

To compile, C/Migemo's library (libmigmeo.so) and header (migemo.h) are needed.

```
$ clang++ --std=c++11 -lmigemo -o cxxmigemo_sample cxxmigemo_sample.cc
$ ./cxxmigemo_sample ~/cmigemo/dict/utf-8.d/
(ｺﾝﾆﾁﾊ|コンニチハ|今日は|こんにちは|ｋｏｎｎｎｉｃｈｉｈａ|konnnichiha)(ｾｶｲ|セカイ|世界|せかい|Ｓｅｋａｉ|Sekai)
```

License
---

The MIT License (MIT)

Copyright (c) 2014 Koichi SATOH, all rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
