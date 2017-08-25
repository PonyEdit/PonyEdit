#ifndef GLOBAL_H
#define GLOBAL_H

#include "QsLog.h"

// Note: this file is #included from *EVERY* cpp file. Don't add stuff here that doesn't
// need to be everywhere; it will slow compilation down.

#define ntr( x ) x

#define LOCK_MUTEX( x ) x.lock()
#define UNLOCK_MUTEX( x ) x.unlock()

// #define LOCK_MUTEX(x) { //qDebug() << "Locking mutex " #x " at line " << __LINE__ << " in file " << __FILE__ << " in
// object " << (void*)this << ", thread " << (void*)QThread::currentThread(); x.lock(); }
// #define UNLOCK_MUTEX(x) { //qDebug() << "Unlocking mutex " #x " at line " << __LINE__ << " in file " << __FILE__ << "
// in object " << (void*)this << ", thread " << (void*)QThread::currentThread(); x.unlock(); }

#endif  // GLOBAL_H
