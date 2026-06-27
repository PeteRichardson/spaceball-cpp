#include <iostream>
#include <atomic>
#include <thread>
#include <stdlib.h>
#include <random>
#include <chrono>

#include "spaceball.h"

static std::atomic<int> answer{-1};

void spaceball_monitor(std::atomic<int>& answer) {
    auto sb = Spaceball("/dev/tty.usbserial-AJ03ACPV");
    answer = -1;
    while (true) {
        auto event = sb.NextEvent();
        if (!event)
            break;

        if (auto* ke = std::get_if<KeyEvent>(&*event)) {
            if (ke->pick) { answer = 0; break; }
            bool any = false;
            for (int i = 0; i < 8; i++) {
                if (ke->buttons[i]) { answer = i + 1; any = true; }
            }
            if (any) break;
        }
    }
}

int main(void) {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 generator(seed);

    std::cout << "Welcome to the Math game!\n\n";
    std::cout << "Use the Spaceball buttons to answer the questions.\n";
    std::cout << "Press the Pick button (on the ball) to quit.\n\n";

    int x, y;
    bool generate_new_question = true;
    while (true) {
        std::jthread spaceball_thread(spaceball_monitor, std::ref(answer));

        if (generate_new_question) {
            x = (generator() % 4) + 1;
            y = (generator() % 4) + 1;
        }
        std::cout << "What is " << x << " + " << y << "? " << std::flush;

        spaceball_thread.join();

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

    return EXIT_SUCCESS;
}
