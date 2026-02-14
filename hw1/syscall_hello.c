#include <unistd.h>
#include <string.h>

int main() {
    
    // write(FileDescriptor, Buffer, Length);
    // FileDescriptor 1 = Standard Output (the screen)
    write(1, "Hello, OS!\n", strlen("Hello, OS!\n"));
    return 0;
}