#include "bitmap.h"

#include <string>
#include <assert.h>
#include <iostream>

#include "utils.h"

int failed = 0;
int tests = 0;

void test(std::string msg, bool correct)
{
    tests++;
    std::string dots (30-msg.size(), '.');
    std::cout << msg << dots;
    if (!correct) {
        std::cout << RED << "FAILED\n" << RESET_CLR;
        failed++;
    }
    else {
        std::cout << GREEN << "OK\n" << RESET_CLR;
    }
}

void test_constructor_empty()
{
    BitMap empty;
    bool size = (empty.size() == 0);
    bool noEl = (empty.get(0) == -1);
    test("Empty constructor", size && empty.isEmpty() && noEl);
    test("  Size 0", size);
    test("  Is empty", empty.isEmpty());
    test("  No elements", noEl);
    std::cout << "-----------------------------------\n";
}

void test_constructor_zeros()
{
    BitMap zeros(5);
    assert(!zeros.isEmpty());
    bool size = (zeros.size() == 5);
    bool all0 = true;
    for (size_t i = 0; i < zeros.size(); i++) {
        if (zeros.get(i) != 0) {
            all0 = false;
            break;
        }
    }
    test("Zero constructor", size && all0);
    std::cout << "  " << zeros.toString() << " <- bitmap\n";
    test("  Size()", size);
    test("  Initialized correctly", all0);
    std::cout << "-----------------------------------\n";
}

void test_constructor_str()
{
    std::string bits = "01001101001101011000";
    BitMap vBits(bits);
    assert(!vBits.isEmpty());
    bool size = (bits.size() == vBits.size());
    bool sameBits = true;
    for (size_t i = 0; i < vBits.size() && i < bits.size(); i++) {
        if (bits[i] - '0' != vBits.get(i)) {
            sameBits = false;
            break;
        }
    }

    test("String constructor", size && sameBits);
    std::cout << "  " << bits << " <- string" << "\n";
    std::cout << "  " << vBits.toString() << " <- bitmap\n";
    test("  Size()", size);
    test("  Initialized correctly", sameBits);
    std::cout << "-----------------------------------\n";
}

void test_constructor_copy()
{
    BitMap original("01001110101");
    BitMap copy(original);
    bool size = (original.size() == copy.size());
    bool sameBits = true;
    for (size_t i = 0; i < original.size() && i < copy.size(); i++) {
        if (copy.get(i) != original.get(i)) {
            sameBits = false;
            break;
        }
    }

    test("Copy constructor", size && sameBits);
    std::cout << "  " << original.toString() << " <- original\n";
    std::cout << "  " << copy.toString() << " <- copy\n";
    test("  Size()", size);
    test("  Initialized correctly", sameBits);
    std::cout << "------------------------" << "\n";
}

void test_get()
{
    std::string bits = "01001101001101011000";
    BitMap bitmap(bits);
    std::cout << bitmap.toString() << " <- bitmap\n";

    bool passed = true;
    for(size_t i = 0; i < bitmap.size(); i++) {
        int8_t bit = bitmap.get(i);
        if (bit != bits[i] - '0') {
            passed = false;
        }
        printf("%d", bit);
    }
    printf(" <- get()\n");

    if (!failed)
        test("Get()", passed);
    test("Get() out of bounds", bitmap.get(100) == -1);
    std::cout << "------------------------" << "\n";
}

void test_set()
{
    BitMap bitmap("0101");
    assert(bitmap.set(0) == 1);
    test("Set()", bitmap.get(0) == 1);
    assert(bitmap.set(1) == 1);
    test("Double set()", bitmap.get(1) == 1);
    test("Set() out of bounds", bitmap.set(100) == 0);
    std::cout << "------------------------" << "\n";
}

void test_clear()
{
    BitMap bitmap("0101");
    assert(bitmap.clear(1) == 1);
    test("Clear()", bitmap.get(1) == 0);
    assert(bitmap.clear(0) == 1);
    test("Double clear()", bitmap.get(0) == 0);
    test("Clear() out of bounds", bitmap.clear(100) == 0);
    std::cout << "------------------------" << "\n";
}

void test_toggle()
{
    BitMap bitmap("0101");
    test("Toggle() output", bitmap.toggle(1) == 0);
    test("Toggle()", bitmap.get(1) == 0);
    test("Toggle() output", bitmap.toggle(0) == 1);
    test("Toggle()", bitmap.get(0) == 1);
    test("Toggle() out of bounds", bitmap.toggle(100) == -1);
    std::cout << "------------------------" << "\n";
}

void test_push_back()
{
    BitMap bitmap;
    bitmap.push_back(0);
    test("push_back(0)", bitmap.size() == 1 && bitmap.get(0) == 0);
    bitmap.push_back(1);
    test("push_back(1)", bitmap.size() == 2 && bitmap.get(1) == 1);
    std::cout << bitmap.toString() << " <- result\n";
    BitMap bitmap2(8);
    bitmap2.push_back(1);
    test("push_back() new word", bitmap2.size() == 9 && bitmap2.get(8) == 1);
    std::cout << bitmap2.toString() << " <- result\n";
    std::cout << "------------------------" << "\n";
}

void test_pop_back()
{
    BitMap bitmap(10);
    size_t oSize = bitmap.size();
    bitmap.pop_back();
    test("pop_back()", bitmap.size() == oSize-1);
    oSize = bitmap.size();
    bitmap.pop_back();
    test("pop_back() remove word", bitmap.size() == oSize-1);
    std::cout << "-----------------------------------\n";
}

void test_toString()
{
    std::string original = "01001101001101011000";
    BitMap bitmap(original);
    std::string result = bitmap.toString();
    test("toString()", original == result);
    std::cout << "-----------------------------------\n";
}

void test_rank()
{
    std::string bits = "011010111010101011010100";
    BitMap bmap(bits);
    bool works = true;
    if (bmap.rank(2) != 2) works = false;
    if (bmap.rank(7) != 5) works = false;
    if (bmap.rank(9) != 6) works = false;
    if (bmap.rank(14) != 9) works = false;
    if (bmap.rank(19) != 12) works = false;
    if (bmap.rank(22) != 13) works = false;
    test("rank()", works);
    std::cout << "-----------------------------------\n";
}

int main (void)
{
    failed = 0;
    tests = 0;

    test_constructor_empty();
    test_constructor_zeros();
    test_constructor_str();
    test_constructor_copy();

    test_get();
    test_set();
    test_clear();
    test_toggle();
    test_push_back();
    test_pop_back();

    test_rank();

    test_toString();

    if (!failed) {
        std::cout << GREEN << "PASSED ALL TESTS" << RESET_CLR << "\n";
    }
    else {
        std::cout << RED << "Failed " << failed << "/" << tests << " tests" << "\n";
    }

    return 0;
}
