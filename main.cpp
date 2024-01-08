#include <vector>
#include <sstream>
#include <chrono>
#include <iostream>
#include <chrono>
#include <mutex>
#include <thread>
#include <time.h>
#include <stdio.h>

// Describes min await duration for philosopher configuration.
#define AWAIT_DURATION_MIN 2000

// Describes max await duration for philosopher configuration.
#define AWAIT_DURATION_MAX 3000

// Describes all available mutexes.
pthread_mutex_t locks[5]; 

// Describes philosopher basic logic.
class Philosopher {
private:
    int id;

    std::thread thread;

    pthread_mutex_t left, right;

public:
    Philosopher(int id, pthread_mutex_t left, pthread_mutex_t right) : id(id), left(left), right(right), thread(&Philosopher::startRaw, this) {}

    // Stops current philosophers.
    void await() {
        printf("Awaits for %d\n", id);

        std::chrono::milliseconds duration(rand() % (AWAIT_DURATION_MAX - AWAIT_DURATION_MIN + 1) + AWAIT_DURATION_MIN);
        std::this_thread::sleep_for(duration);
    }

    // Locks both left and right mutexes.
    void lock() {
        pthread_mutex_lock(&left);
        pthread_mutex_lock(&right);
    }

    // Unlocks both left and right mutexes.
    void unlock() {
        pthread_mutex_unlock(&left);
        pthread_mutex_unlock(&right);
    }
    
    // Starts philosopher flow.
    void startRaw() {
        while (true) {
            lock();
            await();
            unlock();
        }
    }

    // Starts philosopher flow in a seperated thread.
    void start() {
        thread.join();
    }
};

int main() {
    srand(time(0)); 

    std::vector<Philosopher*> philosophers;

    for (int i = 0; i < 5; i++) {
        philosophers.push_back(new Philosopher(i, locks[i], locks[(i+1)%5]));
    }

    std::for_each(philosophers.begin(), philosophers.end(),
        [](Philosopher* philosopher)
        {
            philosopher->start();
        });

    return -1;
}