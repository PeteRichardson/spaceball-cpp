#include "serial.h"

#include <iostream>
#include <iomanip>
#include <sys/ioctl.h>
#include <IOKit/IOKitLib.h>

Serial::Serial(std::string portPath, unsigned int baud=B9600) :
    portPath{portPath}, baud{baud} {

    int             handshake;
    struct termios  options;
    
    // Open the serial port read/write, with no controlling terminal, and don't wait for a connection.
    // The O_NONBLOCK flag also causes subsequent I/O on the device to be non-blocking.
    this->fileDescriptor = open(this->portPath.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (this->fileDescriptor == -1) {
        std::cerr << "Error opening serial port " << portPath << " - " << strerror(errno) << "(" << errno << ")." << std::endl;
        return;
    }
    
    // Prevent additional opens except by root-owned processes.    
    if (ioctl(fileDescriptor, TIOCEXCL) == -1) {
        std::cerr << "Error setting TIOCEXCL on " << portPath << " - " << strerror(errno) << "(" << errno << ")." << std::endl;
        if (this->fileDescriptor != -1)
            close(this->fileDescriptor);
        this->fileDescriptor = -1;
        return;
    }
    
    // Clear the O_NONBLOCK flag so subsequent I/O will block.
    if (fcntl(fileDescriptor, F_SETFL, 0) == -1) {
        std::cerr << "Error clearing O_NONBLOCK " << portPath << " - " << strerror(errno) << "(" << errno << ")." << std::endl;
        if (this->fileDescriptor != -1)
            close(this->fileDescriptor);
        this->fileDescriptor = -1;
        return;
    }
    
    // Get the current options and save them so we can restore the default settings later.
    if (tcgetattr(fileDescriptor, &gOriginalTTYAttrs) == -1) {
        std::cerr << "Error getting tty attributes " << portPath << " - " << strerror(errno) << "(" << errno << ")." << std::endl;
        if (this->fileDescriptor != -1)
            close(this->fileDescriptor);
        this->fileDescriptor = -1;
        return;
    }
    
    // Set Serial port attributes  
    // Set raw input (non-canonical) mode, with reads blocking until either a single character
    // has been received or a one second timeout expires.
    options = gOriginalTTYAttrs;
    cfmakeraw(&options);
    cfsetspeed(&options, this->baud);
    options.c_iflag &= ~(INLCR | ICRNL);
    options.c_iflag |= IGNPAR | IGNBRK;
    options.c_oflag &= ~(OPOST | ONLCR | OCRNL);
    options.c_cflag &= ~(PARENB | PARODD | CSTOPB | CSIZE | CRTSCTS);
    options.c_cflag |= CLOCAL | CREAD | CS8;
    options.c_lflag &= ~(ICANON | ISIG | ECHO);
    options.c_cc[VTIME] = 1;
    options.c_cc[VMIN]  = 0;
     
    // Cause the new options to take effect immediately.
    if (tcsetattr(fileDescriptor, TCSANOW, &options) == -1) {
        std::cerr << "Error setting tty attributes " << portPath << " - " << strerror(errno) << "(" << errno << ")." << std::endl;
        if (this->fileDescriptor != -1)
            close(this->fileDescriptor);
        this->fileDescriptor = -1;
        return;
    }

    if (-1 == fileDescriptor)
        std::cerr << "# ERROR: Could not open device on specified Serial Port: " << portPath  << std::endl;
}

Serial::~Serial() {
    // Block until all written output has been sent from the device.
    if (tcdrain(this->fileDescriptor) == -1) {
        std::cerr << "# Error waiting for drain - " << strerror(errno) << "(" << errno << ")." << std::endl;
    }
    
    // reset serial port back to original state
    if (tcsetattr(fileDescriptor, TCSANOW, &gOriginalTTYAttrs) == -1) {
        std::cerr << "# Error resetting tty attributes - " << strerror(errno) << "(" << errno << ")." << std::endl;
    }

    close(this->fileDescriptor);
}
