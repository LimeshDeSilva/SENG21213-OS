#include "mutex.h"
#include "thread.h"

static inline uint32_t atomic_xchg(volatile uint32_t *addr, uint32_t val) {
    uint32_t result;
    __asm__ __volatile__(
        "lock xchgl %0, %1"
        : "=r"(result), "+m"(*addr)
        : "0"(val)
        : "memory"
    );
    return result;
}

void mutex_init(mutex_t *m) {
    if (!m) return;
    m->locked = 0;
    m->owner_tid = -1;
}

void mutex_lock(mutex_t *m) {
    if (!m) return;
    while (atomic_xchg(&m->locked, 1) != 0) {
        // Yield CPU while waiting for lock release
        thread_yield();
    }
}

void mutex_unlock(mutex_t *m) {
    if (!m) return;
    atomic_xchg(&m->locked, 0);
}
