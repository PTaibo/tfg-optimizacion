#include "bitmap.h"

#include <string>
#include <assert.h>
#include <iostream>

#include "utils.h"

void test(std::string msg, bool correct)
{
    std::string dots (30-msg.size(), '.');
    std::cout << msg << dots;
    if (!correct) {
        std::cout << RED << "FAILED\n" << RESET_CLR;
    }
    else {
        std::cout << GREEN << "OK\n" << RESET_CLR;
    }
}

int main (void)
{
    int8_t testBit;
    std::string bits = "01001101001101011000";

    // Constructor tests
    BitMap zeros(5);
    zeros.print();
    BitMap copy(zeros);
    copy.print();
    BitMap vBits(bits);
    vBits.print();

    // Size test
    std::cout << "Size of bitmap: " << vBits.size() << " | "
              << "Number of bits: " << bits.size() << "\n";
    test("Size()",
         vBits.size() == bits.size());

    // Get test
    testBit = 15;
    std::cout << "Bit " << testBit << " of bitmap: " << vBits.getBit(testBit) << " | "
              << "Original bit " << testBit << ": " << bits[testBit]-'0' << "\n";
    vBits.print();
    std::cout << bits << " <- string\n";
    test("Get()", vBits.getBit(testBit) == bits[testBit] - '0');
    test("Direct get()", vBits.getBit(testBit) == 1);
    test("Get() out of bounds", vBits.getBit(100) == -1);

    // Set test
    testBit = 2;
    assert(vBits.getBit(testBit) == 0);
    assert(vBits.setBit(testBit) == 1);
    test("Set()", vBits.getBit(testBit) == 1);
    assert(vBits.setBit(testBit) == 1);
    test("Double set()", vBits.getBit(testBit) == 1);
    test("Set() out of bounds", vBits.setBit(100) == 0);

    // Clear test
    testBit = 2;
    vBits.setBit(testBit);
    assert(vBits.getBit(testBit) == 1);
    assert(vBits.clearBit(testBit) == 1);
    test("Clear()", vBits.getBit(testBit) == 0);
    assert(vBits.clearBit(testBit) == 1);
    test("Double clear()", vBits.getBit(testBit) == 0);
    test("Clear() out of bounds", vBits.clearBit(100) == 0);

    // Toggle test
    testBit = 2;
    assert(vBits.setBit(testBit) == 1);
    assert(vBits.getBit(testBit) == 1);
    test("Toggle() output", vBits.toggleBit(testBit) == 0);
    test("Toggle()", vBits.getBit(testBit) == 0);
    test("Toggle() output", vBits.toggleBit(testBit) == 1);
    test("Toggle()", vBits.getBit(testBit) == 1);
    test("Toggle() out of bounds", vBits.toggleBit(100) == -1);

    return 0;
}
