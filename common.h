// Note: this file is #included from *EVERY* cpp file. Don't add stuff here
// that doesn't need to be everywhere.

#define ntr(x) x

#define LOCK_MUTEX(x) x.lock()
#define UNLOCK_MUTEX(x) x.unlock()
//#define LOCK_MUTEX(x) { //qDebug() << "Locking mutex " #x " at line " << __LINE__ << " in file " << __FILE__ << " in object " << (void*)this << ", thread " << (void*)QThread::currentThread(); x.lock(); }
//#define UNLOCK_MUTEX(x) { //qDebug() << "Unlocking mutex " #x " at line " << __LINE__ << " in file " << __FILE__ << " in object " << (void*)this << ", thread " << (void*)QThread::currentThread(); x.unlock(); }


// clang doesn't understand -Wterminate
#ifdef __clang__
#define HIDE_COMPILE_WARNINGS _Pragma("GCC diagnostic push") \
	_Pragma("GCC diagnostic ignored \"-Weffc++\"") \
	_Pragma("GCC diagnostic ignored \"-Wnon-virtual-dtor\"")
#else
#define HIDE_COMPILE_WARNINGS _Pragma("GCC diagnostic push") \
	_Pragma("GCC diagnostic ignored \"-Weffc++\"") \
	_Pragma("GCC diagnostic ignored \"-Wnon-virtual-dtor\"") \
	_Pragma("GCC diagnostic ignored \"-Wterminate\"")
#endif

#define UNHIDE_COMPILE_WARNINGS _Pragma("GCC diagnostic pop")

HIDE_COMPILE_WARNINGS

#include "QsLog.h"

UNHIDE_COMPILE_WARNINGS
