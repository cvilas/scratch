#include "writer.h"
#include "reader.h"
#include <fastrtps/rtps/RTPSDomain.h>

#include <iostream>
#include <unistd.h>

//=====================================================================================================================
void reader_callback(uint8_t* const data, uint32_t dataLength)
//=====================================================================================================================
{
    std::cout << *(uint32_t*)data << std::endl;
}


//=====================================================================================================================
int main(int argc, char** argv)
//=====================================================================================================================
{
    std::cout << "Starting RTPS example" << std::endl;
    int type;
    if(argc > 1)
    {
        if(strcmp(argv[1],"writer")==0)
            type = 1;
        else if(strcmp(argv[1],"reader")==0)
            type = 2;
        else
        {
            std::cout << "NEEDS writer OR reader as first argument"<< std::endl;
            return 0;
            
        }
        
    }
    else
    {
        std::cout << "NEEDS writer OR reader ARGUMENT"<< std::endl;
        std::cout << "RTPSTest writer"<< std::endl;
        std::cout << "RTPSTest reader" << std::endl;
        return 0;
    }
	switch (type)
    {
        case 1:
        {
            std::cout << "WRITER - PID " << getpid() << "\n";
            test::Writer writer;
            uint32_t count = 0;
            while(1)
            {
                count++;
                writer.write((uint8_t*)&count, sizeof(count));
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
            break;
        }
        case 2:
        {
            std::cout << "READER - PID " << getpid() << "\n";
            test::Reader reader(&reader_callback);
            while(1)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
            break;
        }
    }
    eprosima::fastrtps::rtps::RTPSDomain::stopAll();
    std::cout << "EVERYTHING STOPPED FINE"<< std::endl;
    return 0;
}


