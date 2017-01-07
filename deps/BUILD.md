Compiling for OSX
=================

(Substitute directory names for your directories, of course.)

## OpenSSL

Download source from https://www.openssl.org/source/, and extract.

	./config
	make

	cp lib*.dylib lib*.a ../PonyEdit/deps/lib-osx
	rm ../PonyEdit/deps/include/openssl/*
	cp include/openssl/* ../PonyEdit/deps/include/openssl/


## libssh2

Download source from https://www.libssh2.org/, and extract.

	./configure --with-libssl-prefix=/Users/pento/Projects/openssl CPPFLAGS="-I/Users/pento/Projects/openssl/include/ -D_REENTRANT" LDFLAGS="-L/Users/pento/Projects/openssl/"
	make

	cp src/.libs/libssh2*.dylib ../PonyEdit/deps/lib-osx/
	rm ../PonyEdit/deps/include/libssh2/*
	cp include/* ../PonyEdit/deps/include/libssh2/
