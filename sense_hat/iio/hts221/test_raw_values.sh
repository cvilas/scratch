#!/bin/bash

DEVICE="/sys/bus/iio/devices/iio:device0"

echo "=== Raw Sysfs Values ==="
for i in {1..5}; do
    echo "Sample $i:"
    echo -n "  Humidity raw: "
    cat $DEVICE/in_humidityrelative_raw 2>/dev/null || echo "N/A"
    echo -n "  Humidity scale: "
    cat $DEVICE/in_humidityrelative_scale 2>/dev/null || echo "N/A"
    echo -n "  Temp raw: "
    cat $DEVICE/in_temp_raw 2>/dev/null || echo "N/A"
    echo -n "  Temp scale: "
    cat $DEVICE/in_temp_scale 2>/dev/null || echo "N/A"
    echo
    sleep 1
done

echo "=== Manual Calculation ==="
HUM_RAW=$(cat $DEVICE/in_humidityrelative_raw)
HUM_SCALE=$(cat $DEVICE/in_humidityrelative_scale 2>/dev/null || echo "1")
TEMP_RAW=$(cat $DEVICE/in_temp_raw)
TEMP_SCALE=$(cat $DEVICE/in_temp_scale 2>/dev/null || echo "1")

echo "Humidity = $HUM_RAW × $HUM_SCALE = $(echo "$HUM_RAW * $HUM_SCALE" | bc -l)"
echo "Temperature = $TEMP_RAW × $TEMP_SCALE = $(echo "$TEMP_RAW * $TEMP_SCALE" | bc -l)"
