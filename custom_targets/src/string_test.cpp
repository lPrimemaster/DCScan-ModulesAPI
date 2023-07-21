#include "../../DCS_Utils/include/DCS_ModuleUtils.h"
#include <iostream>

using namespace DCS::Utils;

int main()
{
    String s = "this,is,a,comma,sepparated,string.";
    std::cout << s.c_str() << std::endl;
    auto split = s.split(',');
    std::cout << split.size() << std::endl;

    for(auto& string : split)
    {
        std::cout << string.c_str() << std::endl;
    }
}