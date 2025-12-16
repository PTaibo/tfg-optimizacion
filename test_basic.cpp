#include "bitmap.h"

#include <string>
#include <assert.h>
#include <iostream>

#include "utils.h"

void test(std::string msg, bool correct, BitMap& vBits)
{
    std::string dots (30-msg.size(), '.');
    std::cout << msg << dots;
    if (!correct) {
        std::cout << RED << "FAILED\n" << RESET_CLR;
    }
    else {
        std::cout << GREEN << "OK\n" << RESET_CLR;
    }
    vBits.print();
}

int main (void)
{
    int8_t testBit;
    std::string bits = "01001101001101011000";

    // Constructor tests
    std::cout << "CONSTRUCTOR TEST" << "\n";
    BitMap zeros(5);
    std::cout << "Size constructor (5): ";
    zeros.print();
    BitMap vBits(bits);
    std::cout << "String: " << bits << "\n";
    std::cout << "Copy from string: ";
    vBits.print();
    BitMap copy(vBits);
    std::cout << "Copy constructor from previous: ";
    copy.print();
    std::cout << "------------------------" << "\n";

    // Size test
    std::cout << "SIZE TEST" << "\n";
    std::cout << "Size of bitmap: " << vBits.size() << " | "
              << "Number of bits: " << bits.size() << "\n";
    test("Size()", vBits.size() == bits.size(), vBits);
    std::cout << "------------------------" << "\n";

    // Get test
    std::cout << "GET TEST" << "\n";
    testBit = 15;
    printf("Bit %d of bitmap: %d | Original bit %d: %c\n", testBit, vBits.get(testBit), testBit, bits[testBit]);
    vBits.print();
    std::cout << bits << " <- string\n";
    test("Get()", vBits.get(testBit) == bits[testBit] - '0', vBits);
    test("Direct get()", vBits.get(testBit) == 1, vBits);
    test("Get() out of bounds", vBits.get(100) == -1, vBits);
    std::cout << "------------------------" << "\n";

    // Set test
    std::cout << "SET TEST" << "\n";
    testBit = 2;
    assert(vBits.get(testBit) == 0);
    assert(vBits.set(testBit) == 1);
    vBits.print();
    test("Set()", vBits.get(testBit) == 1, vBits);
    assert(vBits.set(testBit) == 1);
    test("Double set()", vBits.get(testBit) == 1, vBits);
    test("Set() out of bounds", vBits.set(100) == 0, vBits);
    std::cout << "------------------------" << "\n";

    // Clear test
    std::cout << "CLEAR TEST" << "\n";
    testBit = 2;
    vBits.set(testBit);
    assert(vBits.get(testBit) == 1);
    assert(vBits.clear(testBit) == 1);
    test("Clear()", vBits.get(testBit) == 0, vBits);
    assert(vBits.clear(testBit) == 1);
    test("Double clear()", vBits.get(testBit) == 0, vBits);
    test("Clear() out of bounds", vBits.clear(100) == 0, vBits);
    std::cout << "------------------------" << "\n";

    // Toggle test
    std::cout << "TOGGLE TEST" << "\n";
    testBit = 2;
    assert(vBits.set(testBit) == 1);
    assert(vBits.get(testBit) == 1);
    test("Toggle() output", vBits.toggle(testBit) == 0, vBits);
    test("Toggle()", vBits.get(testBit) == 0, vBits);
    test("Toggle() output", vBits.toggle(testBit) == 1, vBits);
    test("Toggle()", vBits.get(testBit) == 1, vBits);
    test("Toggle() out of bounds", vBits.toggle(100) == -1, vBits);
    std::cout << "------------------------" << "\n";

    // Push-back test
    std::cout << "PUSH-BACK TEST\n";
    vBits.print();
    size_t oSize = vBits.size();
    vBits.push_back(0);
    test("push_back(0)", oSize + 1 == vBits.size(), vBits);
    oSize = vBits.size();
    vBits.push_back(1);
    test("push_back(1)", vBits.size() == oSize+1, vBits);
    std::cout << "------------------------" << "\n";

    // Pop-back test
    std::cout << "POP-BACK TEST" << "\n";
    vBits.print();
    oSize = vBits.size();
    vBits.pop_back();
    test("pop_back()", vBits.size() == oSize-1, vBits);
    std::cout << "-----------------------------------\n";

    return 0;
}
