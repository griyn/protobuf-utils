#include <iostream>
#include "example.pb.h"
    
int main() {
    example::Person person;
    person.set_name("Tom");
    person.set_age(10);
    std::cout << person.ShortDebugString() << std::endl;
    return 0;
}
