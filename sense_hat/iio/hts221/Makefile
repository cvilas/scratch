CXX := g++
CXXFLAGS := -std=c++23 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -pthread -O2
LDFLAGS := -liio

TARGET := hts221
SOURCES := example.cpp hts221.cpp
HEADERS := hts221.h

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES) $(LDFLAGS)

clean:
	rm -f $(TARGET)

install-deps:
	sudo apt-get install -y libiio-dev libiio-utils

info:
	@echo "=== IIO Devices ==="
	@iio_info || echo "libiio-utils not installed. Run 'make install-deps'"

attrs:
	@echo "=== HTS221 Attributes ==="
	@iio_attr -d hts221 || echo "Device not found"

monitor:
	@iio_attr hts221 -c humidityrelative raw
	@iio_attr hts221 -c temp raw

run-single: $(TARGET)
	./$(TARGET) single

run-rate: $(TARGET)
	./$(TARGET) rate

run-all: $(TARGET)
	./$(TARGET) all

.PHONY: all clean install-deps info attrs monitor run-single run-rate run-all
