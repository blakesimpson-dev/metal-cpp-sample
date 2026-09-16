#include <Foundation/Foundation.hpp>
#include <iostream>

int main() {
  std::cout << "Hello, World!" << std::endl;

  NS::String *str =
      NS::String::string("Hello from metal-cpp", NS::UTF8StringEncoding);
  std::cout << str->utf8String() << std::endl;

  return 0;
}
