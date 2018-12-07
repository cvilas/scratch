/*
 * https://stackoverflow.com/questions/27063833/linux-input-device-events-how-to-retrieve-initial-state
 * 
 * The evdev devices queue events until you read() them, so in most cases opening the device, doing the ioctl() and immediately starting to read events from it should work. If the driver dropped some events from the queue, it sends you a SYN_DROPPED event, so you can detect situations where that happened. The libevdev documentation has some ideas on how one should handle that situation; the way I read it you should simply retry, i.e. drop all pending events, and redo the ioctl() until there are no more SYN_DROPPED events.

I used this code to verify that this approach works:
 */ 
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <string.h>

int main(int argc, char **argv) {
    unsigned char key_states[KEY_MAX/8 + 1];
    struct input_event evt;
    int fd;

    memset(key_states, 0, sizeof(key_states));
    fd = open(argv[1], O_RDWR);
    ioctl(fd, EVIOCGKEY(sizeof(key_states)), key_states);

    // Create some inconsistency
    printf("Type (lots) now to make evdev drop events from the queue\n");
    sleep(5);
    printf("\n");

    while(read(fd, &evt, sizeof(struct input_event)) > 0) {
        if(evt.type == EV_SYN && evt.code == SYN_DROPPED) {
            printf("Received SYN_DROPPED. Restart.\n");
            fsync(fd);
            ioctl(fd, EVIOCGKEY(sizeof(key_states)), key_states);
        }
        else if(evt.type == EV_KEY) {
            // Ignore repetitions
            if(evt.value > 1) continue;

            key_states[evt.code / 8] ^= 1 << (evt.code % 8);
            if((key_states[evt.code / 8] >> (evt.code % 8)) & 1 != evt.value) {
                printf("Inconsistency detected: Keycode %d is reported as %d, but %d is stored\n", evt.code, evt.value,
                        (key_states[evt.code / 8] >> (evt.code % 8)) & 1);
            }
        }
    }
}
