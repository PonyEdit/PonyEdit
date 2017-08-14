# Common Tasks

Here are some miscellaneous tasks that should probably be automated at some point, but in the mean time, will be documented.

## Updating Syntax Def Files

Requires a local install of the Qt files, assuming they're installed in `~/Qt/5.9.1/clang_64/`. If you don't have `cmake`, you'll need to `brew install cmake`.

In a temporary directory:

```
git clone https://anongit.kde.org/extra-cmake-modules.git
cd extra-cmake-modules && mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=~ ..
make && make install
cd ..
git clone https://anongit.kde.org/syntax-highlighting.git
cd syntax-highlighting
ECM_DIR=../extra-cmake-modules/build Qt5_DIR=~/Qt/5.9.1/clang_64/ cmake .
make
cp data/*.xml data/syntax/* <PonyEdit syntax defs directory>
cd ..
rm -rf extra-cmake-modules syntax-highlighting ~/share
```
