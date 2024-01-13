#include <vector>
#include <sstream>
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <time.h>
#include <stdio.h>
#include <ncurses.h>

// Describes min await duration for philosopher configuration.
#define AWAIT_DURATION_MIN 2000

// Describes max await duration for philosopher configuration.
#define AWAIT_DURATION_MAX 3000

// Describes load shift await duration for window updater.
#define LOAD_DURATION_SHIFT 10

// Describes window width size.
#define WINDOW_SIZE_WIDTH 100

// Describes window height size.
#define WINDOW_SIZE_HEIGHT 200

// Describes all available mutexes used for philosophers.
pthread_mutex_t locks[5]; 

// Describes lock indecies for all available mutexed used for philosophers.
int lock_indeces[5];

// Describes global window entity.
WINDOW* window;

// Describes mutex used to handle window refresh.
pthread_mutex_t refresh_lock;

class WindowUpdater {
private:
    std::thread thread;

public:
    WindowUpdater() : thread(&WindowUpdater::startRaw, this) {}

    // Locks window refresh mutex.
    void lock() {
        pthread_mutex_lock(&refresh_lock);
    }

    // Unlocks window refresh mutex.
    void unlock() {
        pthread_mutex_unlock(&refresh_lock);
    }

    // Updates window with the updated set of text.
    void updateWindow() {
        wclear(window);

        mvwprintw(window, 0, 35, "(%d) |(0)| (%d)", lock_indeces[1], lock_indeces[0]);
        mvwprintw(window, 10, 10, "(%d) |(1)| (%d)", lock_indeces[1], lock_indeces[2]);
        mvwprintw(window, 10, 60, "(%d) |(4)| (%d)", lock_indeces[4], lock_indeces[0]);
        mvwprintw(window, 20, 20, "(%d) |(2)| (%d)", lock_indeces[2], lock_indeces[3]);
        mvwprintw(window, 20, 50, "(%d) |(3)| (%d)", lock_indeces[3], lock_indeces[4]);
    
        wrefresh(window);
    }

    // Starts philosopher flow.
    void startRaw() {
        while (true) {
            std::chrono::milliseconds duration(LOAD_DURATION_SHIFT);
            std::this_thread::sleep_for(duration);

            lock();
            updateWindow();
            unlock();
        }
    }

    // Starts philosopher flow in a seperated thread.
    void start() {
        thread.join();
    }   
};

// Describes philosopher basic logic.
class Philosopher {
private:
    int id;

    std::thread thread;

    pthread_mutex_t left, right;

    int left_index, right_index;
public:
    Philosopher(int id, pthread_mutex_t left, pthread_mutex_t right, int left_index, int right_index) : id(id), left(left), right(right), left_index(left_index), right_index(right_index), thread(&Philosopher::startRaw, this) {}

    // Stops current philosophers.
    void await() {
        std::chrono::milliseconds duration(rand() % (AWAIT_DURATION_MAX - AWAIT_DURATION_MIN + 1) + AWAIT_DURATION_MIN);
        std::this_thread::sleep_for(duration);
    }

    // Locks both left and right mutexes.
    void lock() {
        pthread_mutex_lock(&left);
        lock_indeces[left_index] = id;
    
        pthread_mutex_lock(&right);
        lock_indeces[right_index] = id;
    }

    // Unlocks both left and right mutexes.
    void unlock() {
        pthread_mutex_unlock(&left);
        lock_indeces[left_index] = id;

        pthread_mutex_unlock(&right);
        lock_indeces[right_index] = id;
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
        thread.detach();
    }
};

void init() {
    initscr();
    keypad(stdscr, TRUE);
    cbreak();
    refresh();

    window = newwin(WINDOW_SIZE_WIDTH, WINDOW_SIZE_HEIGHT, 0, 0);
}

int main() {
    srand(time(0)); 

    init();

    std::vector<Philosopher*> philosophers;

    for (int i = 0; i < 5; i++) {
        philosophers.push_back(new Philosopher(i, locks[i], locks[(i+1)%5], i, (i + 1) % 5));
    }

    std::for_each(philosophers.begin(), philosophers.end(),
        [](Philosopher* philosopher)
        {
            philosopher->start();
        });

    WindowUpdater windowUpdater;
    windowUpdater.start();

    return -1;
}