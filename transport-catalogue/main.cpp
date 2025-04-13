#include <iostream>
#include "json_reader.h"

using namespace std;

int main() {

    jsonreader::JsonReader jr;
    jr.ProcessJson(std::cin, std::cout);

}


