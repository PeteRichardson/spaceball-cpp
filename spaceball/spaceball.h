#include <string>

class Spaceball {
    static const unsigned int version = 1u;

public:
    const char *device_path;

    const unsigned int getVersion();
    Spaceball(const char*);
    ~Spaceball(void);
    void Stream(void);
};