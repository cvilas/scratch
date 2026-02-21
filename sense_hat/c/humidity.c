
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <stdint.h>

// HTS221 Humidity Sensor I2C address
#define HTS221_ADDRESS 0x5F

// HTS221 Register addresses
#define HTS221_WHO_AM_I 0x0F
#define HTS221_CTRL_REG1 0x20
#define HTS221_HUMIDITY_OUT_L 0x28
#define HTS221_HUMIDITY_OUT_H 0x29
#define HTS221_H0_RH_X2 0x30
#define HTS221_H1_RH_X2 0x31
#define HTS221_H0_T0_OUT_L 0x36
#define HTS221_H0_T0_OUT_H 0x37
#define HTS221_H1_T0_OUT_L 0x3A
#define HTS221_H1_T0_OUT_H 0x3B

// Auto-increment flag for multi-byte reads
#define AUTO_INCREMENT 0x80

typedef struct {
    int file;
    int16_t H0_T0_OUT;
    int16_t H1_T0_OUT;
    uint8_t H0_rH;
    uint8_t H1_rH;
} HTS221_Handle;

// Read a single register
int read_register(int file, uint8_t reg, uint8_t *value) {
    if (write(file, &reg, 1) != 1) {
        return -1;
    }
    if (read(file, value, 1) != 1) {
        return -1;
    }
    return 0;
}

// Write a single register
int write_register(int file, uint8_t reg, uint8_t value) {
    uint8_t buf[2] = {reg, value};
    if (write(file, buf, 2) != 2) {
        return -1;
    }
    return 0;
}

// Initialize the HTS221 sensor
int init_hts221(HTS221_Handle *handle, const char *i2c_device) {
    // Open I2C bus
    handle->file = open(i2c_device, O_RDWR);
    if (handle->file < 0) {
        perror("Failed to open I2C bus");
        return -1;
    }

    // Set I2C slave address
    if (ioctl(handle->file, I2C_SLAVE, HTS221_ADDRESS) < 0) {
        perror("Failed to set I2C address");
        close(handle->file);
        return -1;
    }

    // Check WHO_AM_I register (should return 0xBC)
    uint8_t who_am_i;
    if (read_register(handle->file, HTS221_WHO_AM_I, &who_am_i) < 0) {
        fprintf(stderr, "Failed to read WHO_AM_I register\n");
        close(handle->file);
        return -1;
    }
    
    if (who_am_i != 0xBC) {
        fprintf(stderr, "Invalid WHO_AM_I value: 0x%02X (expected 0xBC)\n", who_am_i);
        close(handle->file);
        return -1;
    }

    // Power on and enable Block Data Update (BDU)
    // CTRL_REG1: PD=1 (power on), BDU=1, ODR=1 (1Hz)
    if (write_register(handle->file, HTS221_CTRL_REG1, 0x85) < 0) {
        fprintf(stderr, "Failed to configure sensor\n");
        close(handle->file);
        return -1;
    }

    // Read calibration values
    uint8_t h0_rh_x2, h1_rh_x2;
    uint8_t h0_out_l, h0_out_h;
    uint8_t h1_out_l, h1_out_h;

    read_register(handle->file, HTS221_H0_RH_X2, &h0_rh_x2);
    read_register(handle->file, HTS221_H1_RH_X2, &h1_rh_x2);
    read_register(handle->file, HTS221_H0_T0_OUT_L, &h0_out_l);
    read_register(handle->file, HTS221_H0_T0_OUT_H, &h0_out_h);
    read_register(handle->file, HTS221_H1_T0_OUT_L, &h1_out_l);
    read_register(handle->file, HTS221_H1_T0_OUT_H, &h1_out_h);

    handle->H0_rH = h0_rh_x2 >> 1;
    handle->H1_rH = h1_rh_x2 >> 1;
    handle->H0_T0_OUT = (int16_t)(h0_out_h << 8) | h0_out_l;
    handle->H1_T0_OUT = (int16_t)(h1_out_h << 8) | h1_out_l;

    return 0;
}

// Read humidity from the sensor
float read_humidity(HTS221_Handle *handle) {
    uint8_t h_out_l, h_out_h;
    
    // Read raw humidity data
    if (read_register(handle->file, HTS221_HUMIDITY_OUT_L | AUTO_INCREMENT, &h_out_l) < 0) {
        return -1.0f;
    }
    if (read_register(handle->file, HTS221_HUMIDITY_OUT_H, &h_out_h) < 0) {
        return -1.0f;
    }

    int16_t H_OUT = (int16_t)(h_out_h << 8) | h_out_l;

    // Linear interpolation using calibration values
    float humidity = (float)(handle->H0_rH) + 
                     (float)(handle->H1_rH - handle->H0_rH) * 
                     (float)(H_OUT - handle->H0_T0_OUT) / 
                     (float)(handle->H1_T0_OUT - handle->H0_T0_OUT);

    return humidity;
}

// Close the sensor
void close_hts221(HTS221_Handle *handle) {
    if (handle->file >= 0) {
        close(handle->file);
    }
}

int main(int argc, char *argv[]) {
    HTS221_Handle sensor;
    const char *i2c_device = "/dev/i2c-1"; // Default I2C bus on Raspberry Pi
    int delay_seconds = 1; // Default delay between readings

    // Parse command line arguments
    if (argc > 1) {
        i2c_device = argv[1];
    }
    if (argc > 2) {
        delay_seconds = atoi(argv[2]);
    }

    printf("Initializing HTS221 humidity sensor on %s...\n", i2c_device);

    // Initialize sensor
    if (init_hts221(&sensor, i2c_device) < 0) {
        fprintf(stderr, "Failed to initialize sensor\n");
        return 1;
    }

    printf("Sensor initialized successfully!\n");
    printf("Reading humidity values (Ctrl+C to stop)...\n\n");

    // Main loop - read and print humidity
    while (1) {
        float humidity = read_humidity(&sensor);
        
        if (humidity < 0) {
            fprintf(stderr, "Error reading humidity\n");
        } else {
            printf("Humidity: %.2f %%\n", humidity);
        }

        sleep(delay_seconds);
    }

    // Cleanup (won't be reached due to infinite loop, but good practice)
    close_hts221(&sensor);
    
    return 0;
}

