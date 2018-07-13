#include <iostream>
#include "EmbeddedSystem.h"

int main()
{
    constexpr int MESSENGER_DOMAIN_ID = 0;

    EmbeddedSystem m(MESSENGER_DOMAIN_ID);

    std::cout << "Press q and enter to quit" << std::endl;
    while(std::cin.get() != 'q') { }

    return 0;
}
