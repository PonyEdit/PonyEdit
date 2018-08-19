#!/bin/sh

UNCRUSTIFY="uncrustify-0.67"
OLD_PWD=$(pwd)

cd ~

[ ! -e "extras" ] && mkdir extras
cd extras


[ ! -e "$UNCRUSTIFY.tar.gz" ] && wget "https://github.com/uncrustify/uncrustify/archive/$UNCRUSTIFY.tar.gz"
[ ! -e "uncrustify-$UNCRUSTIFY" ] && tar -xvf "$UNCRUSTIFY.tar.gz" && rm -rf uncrustify-build

[ ! -e "uncrustify-build" ] && mkdir uncrustify-build
cd uncrustify-build

[ ! -e "Makefile" ] && cmake "../uncrustify-$UNCRUSTIFY"
[ ! -e "uncrustify" ] && make

export PATH=$PATH:$(pwd)

cd $OLD_PWD
