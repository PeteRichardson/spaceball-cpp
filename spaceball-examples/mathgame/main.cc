#include <thread>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <random>
#include <chrono>

#include "spaceball.h"

#define KEYP_MASK 0x1000
#define KEY1_MASK 0x0001
#define KEY2_MASK 0x0002
#define KEY3_MASK 0x0004
#define KEY4_MASK 0x0008
#define KEY5_MASK 0x0100
#define KEY6_MASK 0x0200
#define KEY7_MASK 0x0400
#define KEY8_MASK 0x0800

#define KEYUP_MASK 0x1F0F

static int answer = -1;

void *spaceball_monitor(void *arg) {
    // A thread to monitor button input from the Spaceball and update
    // a static variable (answer)
    auto sb = Spaceball("/dev/tty.usbserial-AJ03ACPV");
    bool quitting = false;
    answer = -1;
    while (!quitting) {
        auto event = sb.NextEvent();
        uint16_t keyflags = (int(event[1]) << 8)  + int(event[2]);
        
        // check for quitting
        char type = char(event[0]);
        if (type == 'K') {
            uint16_t keyflags = (int(event[1]) << 8)  + int(event[2]);
            if (keyflags & KEYP_MASK) answer = 0;
            if (!(keyflags & KEYUP_MASK)) continue;
            if (keyflags & KEY1_MASK) answer = 1;
            if (keyflags & KEY2_MASK) answer = 2;
            if (keyflags & KEY3_MASK) answer = 3;
            if (keyflags & KEY4_MASK) answer = 4;
            if (keyflags & KEY5_MASK) answer = 5;
            if (keyflags & KEY6_MASK) answer = 6;
            if (keyflags & KEY7_MASK) answer = 7;
            if (keyflags & KEY8_MASK) answer = 8;
            quitting = true;
        }
    }
    return 0;
}

int WaitForAnswer(pthread_t &t) {
    
    return answer;
}

int main(void) {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 generator(seed);  // mt19937 is a standard mersenne_twister_engine
  
    std::cout << "Welcome to the Math game!\n\n";
    std::cout << "Use the Spaceball buttons to answer the questions.\n";
    std::cout << "Press the Pick button (on the ball) to quit.\n\n";

    int x,y;
    bool generate_new_question = true;
    while (true) {
        pthread_t spaceball_thread;
        int err = 0;

        err = pthread_create(&spaceball_thread, NULL, spaceball_monitor, NULL);
        if (err) {
            std::cerr << "# Error: " << strerror(err) << " (" << stderr << ")\n";
            exit(EXIT_FAILURE);
        }

        if (generate_new_question) {
            x = (generator() % (4 - 1 + 1) ) + 1;
            y = (generator() % (4 - 1 + 1) ) + 1;
        }
        std::cout << "What is " << x << " + " << y << "? " << std::flush;
        
        pthread_join(spaceball_thread, NULL);
        
        if (answer == 0) {
            std::cout << "\nGoodbye" << std::endl;
            break;
        }
        
        if (answer == x + y) {
            std::cout << answer << " is correct!\n";
            generate_new_question = true;
        } else {
            std::cout << answer << " is wrong.  Try again!\n";
            generate_new_question = false;
        }
    }

    exit(EXIT_SUCCESS);
}