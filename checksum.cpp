#include <array>
#include <iostream>

// Compute checksum using sum-complement method and append it to message
void setChecksum(std::array<unsigned char, 4>& message)
{
    int check_sum = 0;
    for(size_t i = 0; i < 3; ++i) {
        check_sum += static_cast<int>(message[i]); 
    }
    message[3] = ~static_cast<int8_t>(check_sum & 0xFF) + 1;
}

// confirm data integrity
bool isMessageValid(const std::array<unsigned char, 4>& message)
{
    int sum = 0;
	for(size_t i = 0; i < 4; ++i) { 
		sum += static_cast<int>(message[i]); 
    }
	return ((sum & 0xFF) == 0);
}

// test method
int main()
{
    std::array<unsigned char, 4> val = {0xAF, 0xBF, 0xCF, 0x0};
    setChecksum(val);

    std::cout << "checksum: " << static_cast<int>(val[3]) << std::endl;
    std::cout << "is valid: " << isMessageValid(val) << std::endl;
    return 0;
}
