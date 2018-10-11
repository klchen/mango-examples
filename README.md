# mango examples
Some code examples to answer frequently asked questions.

The examples will be expanded when someone asks a question. Maybe even spontaneously at random occasions.
The library will never be complete or ready thus it will unlikely to be publicly available to save people from a lot of pain.

# macOS issues

The cmake build script workins on macOS is sketchy; the /usr/local for include and library paths must be correctly setup.
When linking some modules, the compiler will go into "insane mode" and complain that some system headers have incorrect UTF8/unicode characters, etc.

I have no strength to fight these problems so macOS users are on their own.
A simple "c++ -O3 -std=c++14 foo.cpp -lmango -o foo.binary" works just fine the problem is probably somewhere in cmake/macOS/xcode toolchain. I give up.
