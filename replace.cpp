#include <string>
#include <iostream>

std::string toTomlNamespace(std::string orig)
{   
    size_t pos = std::string::npos;
    while((pos=orig.find("::")) != std::string::npos)
    {
        orig = orig.replace(pos, 2, ".", 0, 1);
    }
    return orig;
}

int main()
{
    std::string name = "hive::types::io::Value";
    auto out = toTomlNamespace(name);
    std::cout << name << " = " << out << "\n";
}
