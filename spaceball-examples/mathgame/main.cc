#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <random>
#include <chrono>

#include "spaceball.h"

static int answer = -1;

void *spaceball_monitor(void *arg) {
    // A thread to monitor button input from the Spaceball and update
    // a static variable (answer)
    auto sb = Spaceball("/dev/tty.usbserial-AJ03ACPV");
    answer = -1;
    while (true) {
        auto event = sb.NextEvent();
        if (!event)
            break;

        if (auto* ke = std::get_if<KeyEvent>(&*event)) {
            if (ke->pick) {
                answer = 0;
                break;
            }
            bool any = false;
            for (int i = 0; i < 8; i++) {
                if (ke->buttons[i]) { answer = i + 1; any = true; }
            }
            if (any) break;
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