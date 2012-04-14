/*  AvKode: Multithreading classes

    Copyright (C) 2006 Allan Sandfeld Jensen <kde@carewolf.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef _AKODE_THREAD_H
#define _AKODE_THREAD_H

#include <pthread.h>
#include <semaphore.h>

namespace aKode {

class Condition;

class Mutex {
    friend class Condition;
public:
    Mutex() {
        pthread_mutex_init(&mutex, 0);
    }
    void lock() {
        pthread_mutex_lock(&mutex);
    }
    void unlock() {
        pthread_mutex_unlock(&mutex);
    }
private:
    pthread_mutex_t mutex;
};

class Condition {
public:
    Condition() {
        pthread_cond_init(&cond, 0);
    }
    void wait(Mutex *mutex) {
        pthread_cond_wait(&cond, &mutex->mutex);
    }
    void signal() {
        pthread_cond_signal(&cond);
    }
private:
    pthread_cond_t cond;
};

class Semaphore {
public:
    Semaphore(int i = 0) {
        sem_init(&sem, 0, i);
    }
    ~Semaphore() {
        sem_destroy(&sem);
    }
    void wait() {
        sem_wait(&sem);
    }
    void signal() {
        sem_post(&sem);
    }
private:
    sem_t sem;
};

} // namespace

#endif
