#include <semaphore.h>

int sem_timedwait(void** pp) {return 0;}
int main() {
    void* p;
    return sem_timedwait(&p);
}

