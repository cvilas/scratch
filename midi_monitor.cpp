// Demo example to interface with a MIDI device and report position of all controls
// compile with: g++ -o midi_monitor midi_monitor.cpp
// usage: ./midi_monitor

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define MIDI_DEVICE "/dev/midi1"  // Adjust this to your MIDI device

enum MidiMessageType {
    NOTE_OFF = 0x80,
    NOTE_ON = 0x90,
    POLY_KEY_PRESSURE = 0xA0,
    CONTROL_CHANGE = 0xB0,
    PROGRAM_CHANGE = 0xC0,
    CHANNEL_PRESSURE = 0xD0,
    PITCH_BEND = 0xE0,
    SYSTEM_MESSAGE = 0xF0
};

void parse_system_message(unsigned char* buffer, int* i, int bytes_read) {
    switch (buffer[*i]) {
        case 0xF0: // System Exclusive (prints product info, serial number, etc)
            printf("System Exclusive message: ");
            (*i)++; // Move past the 0xF0
            while (*i < bytes_read && buffer[*i] != 0xF7) {
                printf("%02X ", buffer[*i]);
                (*i)++;
            }
            if (*i < bytes_read && buffer[*i] == 0xF7) {
                printf("F7\n"); // End of SysEx
                (*i)++;
            } else {
                printf("(incomplete)\n");
            }
            break;
        case 0xF1:
            printf("MIDI Time Code Quarter Frame\n");
            (*i)++;
            break;
        case 0xF2:
            printf("Song Position Pointer\n");
            *i += 2;
            break;
        case 0xF3:
            printf("Song Select\n");
            (*i)++;
            break;
        case 0xF6:
            printf("Tune Request\n");
            break;
        case 0xF8:
            printf("Timing Clock\n");
            break;
        case 0xFA:
            printf("Start\n");
            break;
        case 0xFB:
            printf("Continue\n");
            break;
        case 0xFC:
            printf("Stop\n");
            break;
        case 0xFE:
            printf("Active Sensing\n");
            break;
        case 0xFF:
            printf("System Reset\n");
            break;
    }
}

int main() {
    int fd;
    unsigned char buffer[128];
    ssize_t bytes_read;

    fd = open(MIDI_DEVICE, O_RDWR);
    if (fd < 0) {
        perror("Error opening MIDI device");
        exit(1);
    }

    // Send SysEx message to request all control values
    unsigned char sysex[] = {0xF0, 0x7E, 0x7F, 0x06, 0x01, 0xF7};
    if (write(fd, sysex, sizeof(sysex)) < 0) {
        perror("Error sending SysEx message");
        close(fd);
        exit(1);
    }

    while (1) {
        bytes_read = read(fd, buffer, sizeof(buffer));
        if (bytes_read < 0) {
            perror("Error reading MIDI data");
            break;
        }

        for (int i = 0; i < bytes_read; i++) {
            enum MidiMessageType message_type = static_cast<MidiMessageType>(buffer[i] & 0xF0);
            unsigned char channel = buffer[i] & 0x0F;

            switch (message_type) {
                case NOTE_OFF:
                    printf("NOTE_OFF - Channel: %d, Note: %d, Velocity: %d\n",
                           channel, buffer[i+1], buffer[i+2]);
                    i += 2;
                    break;
                case NOTE_ON:
                    printf("NOTE_ON - Channel: %d, Note: %d, Velocity: %d\n",
                           channel, buffer[i+1], buffer[i+2]);
                    i += 2;
                    break;
                case POLY_KEY_PRESSURE:
                    printf("POLY_KEY_PRESSURE - Channel: %d, Note: %d, Pressure: %d\n",
                           channel, buffer[i+1], buffer[i+2]);
                    i += 2;
                    break;
                case CONTROL_CHANGE:
                    printf("CONTROL_CHANGE - Channel: %d, Controller: %d, Value: %d\n",
                           channel, buffer[i+1], buffer[i+2]);
                    i += 2;
                    break;
                case PROGRAM_CHANGE:
                    printf("PROGRAM_CHANGE - Channel: %d, Program: %d\n",
                           channel, buffer[i+1]);
                    i += 1;
                    break;
                case CHANNEL_PRESSURE:
                    printf("CHANNEL_PRESSURE - Channel: %d, Pressure: %d\n",
                           channel, buffer[i+1]);
                    i += 1;
                    break;
                case PITCH_BEND:
                    printf("PITCH_BEND - Channel: %d, Value: %d\n",
                           channel, (buffer[i+2] << 7) | buffer[i+1]);
                    i += 2;
                    break;
                case SYSTEM_MESSAGE:
                    parse_system_message(buffer, &i, bytes_read);
                    break;
                default:
                    if (buffer[i] < 0x80) {
                        // This is likely a data byte, not a status byte
                        printf("Data byte: 0x%02X\n", buffer[i]);
                    } else {
                        printf("Unknown message type: 0x%02X\n", buffer[i]);
                    }
                    break;
            }
        }
    }

    close(fd);
    return 0;
}
