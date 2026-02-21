# Humidity sensor HTS221

- Check driver is loaded
  ```
  lsmod | grep hts221
  ```

- load module
  ```
  sudo modprobe hts221_i2c
  ```

- Configure device tree (if not already)
  ```
  /dts-v1/;
/plugin/;

/ {
    compatible = "brcm,bcm2835";

    fragment@0 {
        target = <&i2c1>;
        __overlay__ {
            #address-cells = <1>;
            #size-cells = <0>;
            
            hts221@5f {
                compatible = "st,hts221";
                reg = <0x5f>;
            };
        };
    };
};

```

- Compile and load device tree
```
dtc -@ -I dts -O dtb -o hts221-sense-hat.dtbo hts221-sense-hat.dts
sudo cp hts221-sense-hat.dtbo /boot/overlays/

# Add to /boot/firmware/config.txt:
dtoverlay=hts221-sense-hat
```

- Reboot

- Verify device appears (You should see an iio::deviceX entry
```
ls -l /sys/bus/iio/devices/
```

- You can query all values with `cat`
```
# Find your HTS221 device
ls /sys/bus/iio/devices/

# Check what attributes exist
ls -l /sys/bus/iio/devices/iio:device0/

# Read the actual values
cat /sys/bus/iio/devices/iio:device0/in_humidityrelative_raw
cat /sys/bus/iio/devices/iio:device0/in_humidityrelative_scale
cat /sys/bus/iio/devices/iio:device0/in_temp_raw  
cat /sys/bus/iio/devices/iio:device0/in_temp_scale
cat /sys/bus/iio/devices/iio:device0/in_temp_offset
```

- `sudo apt install libiio-dev libiio-utils`
