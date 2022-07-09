#include "xtl.h"

int main()
{
    std::cout << xtl::timestamp::now().to_localtime_string() << std::endl;
}
