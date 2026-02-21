#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <cstdint>
#include <iostream>
#include <chrono>

class SenseHatIMU {
private:
    int i2c_fd;
    const int LSM9DS1_AG_ADDR = 0x6A;  // Accel/Gyro
    const int LSM9DS1_M_ADDR = 0x1C;   // Magnetometer
    
    // Register addresses
    const uint8_t CTRL_REG1_G = 0x10;  // Gyro control
    const uint8_t CTRL_REG6_XL = 0x20; // Accel control
    const uint8_t OUT_X_L_G = 0x18;    // Gyro data start
    const uint8_t OUT_X_L_XL = 0x28;   // Accel data start
    
public:
    struct IMUData {
        int16_t accel_x, accel_y, accel_z;
        int16_t gyro_x, gyro_y, gyro_z;
        int16_t mag_x, mag_y, mag_z;
    };
    
    SenseHatIMU() {
        i2c_fd = open("/dev/i2c-1", O_RDWR);
        if (i2c_fd < 0) {
            throw std::runtime_error("Failed to open I2C bus");
        }
        
        // Initialize accelerometer and gyroscope for maximum rate
        initAccelGyro();
    }
    
    ~SenseHatIMU() {
        close(i2c_fd);
    }
    
    void initAccelGyro() {
        // Set to accel/gyro device
        if (ioctl(i2c_fd, I2C_SLAVE, LSM9DS1_AG_ADDR) < 0) {
            throw std::runtime_error("Failed to set I2C slave address");
        }
        
        // Gyroscope:  952 Hz ODR, 2000 dps scale
        writeRegister(CTRL_REG1_G, 0xC0);  // 952 Hz, 2000 dps
        
        // Accelerometer: 952 Hz ODR, ±16g scale
        writeRegister(CTRL_REG6_XL, 0xC0); // 952 Hz, ±16g
    }
    
    void writeRegister(uint8_t reg, uint8_t value) {
        uint8_t buf[2] = {reg, value};
        if (write(i2c_fd, buf, 2) != 2) {
            throw std:: runtime_error("Failed to write to I2C device");
        }
    }
    
    int16_t readInt16(uint8_t reg) {
        uint8_t data[2];
        if (write(i2c_fd, &reg, 1) != 1) {
            return 0;
        }
        if (read(i2c_fd, data, 2) != 2) {
            return 0;
        }
        return (int16_t)(data[1] << 8 | data[0]);
    }
    
    IMUData readData() {
        IMUData data;
        
        // Set to accel/gyro device
        ioctl(i2c_fd, I2C_SLAVE, LSM9DS1_AG_ADDR);
        
        // Read accelerometer (auto-increment registers)
        data.accel_x = readInt16(0x28 | 0x80);
        data.accel_y = readInt16(0x2A | 0x80);
        data.accel_z = readInt16(0x2C | 0x80);
        
        // Read gyroscope
        data.gyro_x = readInt16(0x18 | 0x80);
        data.gyro_y = readInt16(0x1A | 0x80);
        data.gyro_z = readInt16(0x1C | 0x80);
        
        return data;
    }
};

int main() {
  SenseHatIMU imu;
// Read IMU at maximum rate
        auto last_time = std::chrono:: high_resolution_clock::now();
        int count = 0;
        
        while (true) {
            auto data = imu.readData();
            
            count++;
            auto now = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count();
            
            if (elapsed >= 1000) {
                std::cout << "Update rate: " << count << " Hz" << std::endl;
                std::cout << "Accel: " << data.accel_x << ", " << data.accel_y << ", " << data.accel_z << std::endl;
                std::cout << "Gyro: " << data.gyro_x << ", " << data. gyro_y << ", " << data.gyro_z << std::endl;
                count = 0;
                last_time = now;
            }
        }
}
