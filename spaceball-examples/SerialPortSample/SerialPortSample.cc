
 
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <paths.h>
#include <termios.h>
#include <sysexits.h>
#include <sys/param.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/serial/ioss.h>
#include <IOKit/IOBSD.h>

 
// Hold the original termios attributes so we can reset them
static struct termios gOriginalTTYAttrs;
 
// Given the path to a serial device, open the device and configure it.
// Return the file descriptor associated with the device.
static int openSerialPort(const char *bsdPath)
{
    int             fileDescriptor = -1;
    int             handshake;
    struct termios  options;
    
    // Open the serial port read/write, with no controlling terminal, and don't wait for a connection.
    // The O_NONBLOCK flag also causes subsequent I/O on the device to be non-blocking.
    fileDescriptor = open(bsdPath, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fileDescriptor == -1) {
        std::cerr << "Error opening serial port " << bsdPath << " - " << strerror(errno) << "(" << errno << ")." << std::endl;
        return -1;
    }
    
    // Prevent additional opens except by root-owned processes.    
    if (ioctl(fileDescriptor, TIOCEXCL) == -1) {
        std::cerr << "Error setting TIOCEXCL on " << bsdPath << " - " << strerror(errno) << "(" << errno << ")." << std::endl;
        if (fileDescriptor != -1)
            close(fileDescriptor);
        return -1;
    }
    
    // Now that the device is open, clear the O_NONBLOCK flag so subsequent I/O will block.
    if (fcntl(fileDescriptor, F_SETFL, 0) == -1) {
        std::cerr << "Error clearing O_NONBLOCK " << bsdPath << " - " << strerror(errno) << "(" << errno << ")." << std::endl;
        if (fileDescriptor != -1)
            close(fileDescriptor);
        return -1;
    }
    
    // Get the current options and save them so we can restore the default settings later.
    if (tcgetattr(fileDescriptor, &gOriginalTTYAttrs) == -1) {
        std::cerr << "Error getting tty attributes " << bsdPath << " - " << strerror(errno) << "(" << errno << ")." << std::endl;
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
        std::cerr << "Error setting tty attributes " << bsdPath << " - " << strerror(errno) << "(" << errno << ")." << std::endl;
        if (fileDescriptor != -1)
            close(fileDescriptor);
        return -1;
    }
  
    // Success!
    return fileDescriptor;
}

 
// Given the file descriptor for a modem device, attempt to initialize the modem by sending it
// a standard AT command and reading the response. If successful, the modem's response will be "OK".
// Return true if successful, otherwise false.
static Boolean initializeSpaceball(int fileDescriptor) {

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
 
// Given the file descriptor for a serial device, close that device.
void closeSerialPort(int fileDescriptor)
{
    // Block until all written output has been sent from the device.
    // Note that this call is simply passed on to the serial device driver.
    // See tcsendbreak(3) <x-man-page://3/tcsendbreak> for details.
    if (tcdrain(fileDescriptor) == -1) {
        std::cerr << "# Error waiting for drain - " << strerror(errno) << "(" << errno << ")." << std::endl;
    }
    
    // Traditionally it is good practice to reset a serial port back to
    // the state in which you found it. This is why the original termios struct
    // was saved.
    if (tcsetattr(fileDescriptor, TCSANOW, &gOriginalTTYAttrs) == -1) {
        std::cerr << "# Error resetting tty attributes - " << strerror(errno) << "(" << errno << ")." << std::endl;
    }
    
    close(fileDescriptor);
}
 
// void dumpLine(int fileDescriptor) {
//     ssize_t     numBytes = 0;       // Number of bytes read or written
//     UInt8       buffer[256];    // Input buffer
//     UInt8       *bufPtr;        // Current char in buffer
    
//     // Read characters into our buffer until we get a CR or LF
//     bufPtr = buffer;
//     do {
//         numBytes = read(fileDescriptor, bufPtr, 1);
        
//         if (numBytes == -1) {
//             printf("Error reading from Spaceball - %s(%d).\n", strerror(errno), errno);
//         } else if (numBytes > 0) {
//             bufPtr += numBytes;
//             if (*(bufPtr - 1) == '\r') {
//                 break;
//             }
//         }
//     } while (*bufPtr != '\r' );
      
//     bufPtr = buffer;
//     if (numBytes > 0) {
//         printf("%c", (int) *bufPtr);
//         for (bufPtr = buffer+1; *bufPtr != '\0'; bufPtr++) {
//         printf("%02X", (unsigned char) *bufPtr);
//         }
//         printf("\n");
//     }
// }

#define MAX_EVENT_SIZE 32

ssize_t getEvent(uint8_t *event, int fileDescriptor) {
    ssize_t numBytes{};       // Number of bytes read or written
    uint8_t byte{};
    uint8_t *next = event;

    // Find a D or K indicating start of an event
    while (byte != 'D' && byte != 'K')
        numBytes = read(fileDescriptor, &byte, 1);
    *next++ = byte;
    
    // Read until the \r indicating end of event
    while (byte != '\r') {
        numBytes = read(fileDescriptor, &byte, 1);
        *next++ = byte;
    }
    return (next - event - 1);
}

void dumpEvent(const uint8_t *event, const ssize_t length) {
    std::cout << event[0];
    for (int i=1; i<length; i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << int(event[i]);
    }
    std::cout << std::endl;
}

int main(int argc, const char * argv[]) {
    int fileDescriptor;
    char portPath[] = "/dev/tty.usbserial-AJ03ACPV";
      
    fileDescriptor = openSerialPort(portPath);
    if (-1 == fileDescriptor) {
        std::cerr << "# ERROR: Could not open Spaceball on specified Serial Port: " << portPath  << std::endl;
        return EX_IOERR;
    }
    
    if (!initializeSpaceball(fileDescriptor)) {
        std::cerr << "# ERROR: Could not initialize Spaceball." << std::endl;
        return EX_IOERR;
    }

    uint8_t event[MAX_EVENT_SIZE];
    uint8_t *eventPtr = event;
    while(true) {
        auto length = getEvent(eventPtr, fileDescriptor);
        //std::cout << "length = " << length << std::endl;
        dumpEvent(event, length);
    }

    closeSerialPort(fileDescriptor);

    return EX_OK;
}