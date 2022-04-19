#include "serial.h"

#include <iostream>
#include <iomanip>
#include <sys/ioctl.h>
#include <IOKit/IOKitLib.h>

Serial::Serial(std::string portPath, unsigned int baud) :
    portPath{portPath}, baud{baud} {
}

int Serial::openSerialPort()
{
    int             fileDescriptor = -1;
    int             handshake;
    struct termios  options;
    
    // Open the serial port read/write, with no controlling terminal, and don't wait for a connection.
    // The O_NONBLOCK flag also causes subsequent I/O on the device to be non-blocking.
    fileDescriptor = open(this->portPath.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fileDescriptor == -1) {
        std::cerr << "Error opening serial port " << portPath << " - " << strerror(errno) << "(" << errno << ")." << std::endl;
        return -1;
    }
    
    // Prevent additional opens except by root-owned processes.    
    if (ioctl(fileDescriptor, TIOCEXCL) == -1) {
        std::cerr << "Error setting TIOCEXCL on " << portPath << " - " << strerror(errno) << "(" << errno << ")." << std::endl;
        if (fileDescriptor != -1)
            close(fileDescriptor);
        return -1;
    }
    
    // Now that the device is open, clear the O_NONBLOCK flag so subsequent I/O will block.
    if (fcntl(fileDescriptor, F_SETFL, 0) == -1) {
        std::cerr << "Error clearing O_NONBLOCK " << portPath << " - " << strerror(errno) << "(" << errno << ")." << std::endl;
        if (fileDescriptor != -1)
            close(fileDescriptor);
        return -1;
    }
    
    // Get the current options and save them so we can restore the default settings later.
    if (tcgetattr(fileDescriptor, &gOriginalTTYAttrs) == -1) {
        std::cerr << "Error getting tty attributes " << portPath << " - " << strerror(errno) << "(" << errno << ")." << std::endl;
        if (fileDescriptor != -1)
            close(fileDescriptor);
        return -1;
    }
    
    // Set Serial port attributes  
    // Set raw input (non-canonical) mode, with reads blocking until either a single character
    // has been received or a one second timeout expires.
    options = gOriginalTTYAttrs;
    cfmakeraw(&options);
    cfsetspeed(&options, B9600);
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
        if (fileDescriptor != -1)
            close(fileDescriptor);
        return -1;
    }
  
    // Success!
    return fileDescriptor;
}

void Serial::connect() {
    this->fileDescriptor = openSerialPort();
    if (-1 == fileDescriptor) {
        std::cerr << "# ERROR: Could not open Spaceball on specified Serial Port: " << portPath  << std::endl;
        return;
    }
    
    if (!initializeSpaceball()) {
        std::cerr << "# ERROR: Could not initialize Spaceball." << std::endl;
        return;
    }

}

void Serial::disconnect() {
    // Block until all written output has been sent from the device.
    // Note that this call is simply passed on to the serial device driver.
    // See tcsendbreak(3) <x-man-page://3/tcsendbreak> for details.
    if (tcdrain(this->fileDescriptor) == -1) {
        std::cerr << "# Error waiting for drain - " << strerror(errno) << "(" << errno << ")." << std::endl;
    }
    
    // Traditionally it is good practice to reset a serial port back to
    // the state in which you found it. This is why the original termios struct
    // was saved.
    if (tcsetattr(fileDescriptor, TCSANOW, &gOriginalTTYAttrs) == -1) {
        std::cerr << "# Error resetting tty attributes - " << strerror(errno) << "(" << errno << ")." << std::endl;
    }
    
    close(this->fileDescriptor);
}

Serial::~Serial() {
}

bool Serial::initializeSpaceball() {

    char spaceballInitString[] = "\rCB\rNT\rFT?\rFR?\rP@r@r\rMSSV\rZ\rBc\rH";

    // config_data = [
    //         "\x0D",                 # Clear the line
    //         "CB\x0D",               # Communications Mode Set to Binary
    //         "NT\x0DFT?\x0DFR?\x0D", # Set Null Region and Trans and Rot Sensitivity
    //         "P@r@r\x0D",            # Data rate to 20 events per Second
    //         "MSSV\x0D",             # Ball Event Type: Trans and Rot Vectors
    //         "Z\x0D",                # Rezero the ball
    //         "Bc\x0D",               # Beep it
    //         "H"
    //     ]
  
    ssize_t     numBytes;       // Number of bytes read or written
         
    // Send initialization cmds to Spaceball
    numBytes = write(fileDescriptor, spaceballInitString, strlen(spaceballInitString));
    
    if (numBytes != strlen(spaceballInitString))
        printf("Error writing to Spaceball. Wrote %zd bytes - %s(%d).\n", numBytes, strerror(errno), errno);

    return true;
}

ssize_t Serial::getEvent(uint8_t *event) {
    ssize_t numBytes{};       // Number of bytes read or written
    uint8_t byte{};
    uint8_t *next = event;

    // Find a D or K indicating start of an event
    while (byte != 'D' && byte != 'K')
        numBytes = read(this->fileDescriptor, &byte, 1);
    *next++ = byte;
    
    // Read until the \r indicating end of event
    while (byte != '\r') {
        numBytes = read(this->fileDescriptor, &byte, 1);
        *next++ = byte;
    }
    return (next - event - 1);
}

void Serial::dumpEvent(const uint8_t *event, const ssize_t length) {
    std::cout << event[0];
    for (int i=1; i<length; i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << int(event[i]);
    }
    std::cout << std::endl;
}
