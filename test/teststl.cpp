/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam,
 * Leandro Penz, Kimmo Varis, Vesa Pikki
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */



#include "../src/tokenize.h"
#include "../src/checkstl.h"
#include "testsuite.h"
#include <sstream>

extern std::ostringstream errout;

class TestStl : public TestFixture
{
public:
    TestStl() : TestFixture("TestStl")
    { }

private:
    void run()
    {
        TEST_CASE(iterator1);
        TEST_CASE(iterator2);
        TEST_CASE(iterator3);

        TEST_CASE(STLSize);
        TEST_CASE(STLSizeNoErr);
        TEST_CASE(erase);
        TEST_CASE(eraseBreak);
        TEST_CASE(eraseReturn);
        TEST_CASE(eraseGoto);
        TEST_CASE(eraseAssign);
        TEST_CASE(eraseDereference);

        TEST_CASE(pushback1);
        TEST_CASE(pushback2);
        TEST_CASE(pushback3);
        TEST_CASE(pushback4);

        TEST_CASE(invalidcode);

        TEST_CASE(stlBoundries1);
    }

    void check(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.setVarId();

        // Clear the error buffer..
        errout.str("");

        // Check..
        CheckStl checkStl;
        checkStl.runSimplifiedChecks(&tokenizer, (const Settings *)0, this);
    }


    void iterator1()
    {
        check("void f()\n"
              "{\n"
              "    list<int> l1;\n"
              "    list<int> l2;\n"
              "    for (list<int>::iterator it = l1.begin(); it != l2.end(); ++it)\n"
              "    { }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Same iterator is used with both l1 and l2\n", errout.str());
    }

    void iterator2()
    {
        check("void foo()\n"
              "{\n"
              "    list<int> l1;\n"
              "    list<int> l2;\n"
              "    list<int>::iterator it = l1.begin();\n"
              "    while (it != l2.end())\n"
              "    {\n"
              "        ++it;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6]: (error) Same iterator is used with both l1 and l2\n", errout.str());
    }

    void iterator3()
    {
        check("void foo()\n"
              "{\n"
              "    list<int> l1;\n"
              "    list<int> l2;\n"
              "    list<int>::iterator it = l1.begin();\n"
              "    l2.insert(it, 0);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:6]: (error) Same iterator is used with both l1 and l2\n", errout.str());
    }


    void STLSize()
    {
        check("void foo()\n"
              "{\n"
              "    std::vector<int> foo;\n"
              "    for (unsigned int ii = 0; ii <= foo.size(); ++ii)\n"
              "    {\n"
              "       foo[ii] = 0;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS(std::string("[test.cpp:6]: (error) When ii==foo.size(), foo[ii] is out of bounds\n"), errout.str());
    }

    void STLSizeNoErr()
    {
        {
            check("void foo()\n"
                  "{\n"
                  "    std::vector<int> foo;\n"
                  "    for (unsigned int ii = 0; ii < foo.size(); ++ii)\n"
                  "    {\n"
                  "       foo[ii] = 0;\n"
                  "    }\n"
                  "}\n");
            ASSERT_EQUALS(std::string(""), errout.str());
        }

        {
            check("void foo()\n"
                  "{\n"
                  "    std::vector<int> foo;\n"
                  "    for (unsigned int ii = 0; ii <= foo.size(); ++ii)\n"
                  "    {\n"
                  "    }\n"
                  "}\n");
            ASSERT_EQUALS(std::string(""), errout.str());
        }

        {
            check("void foo()\n"
                  "{\n"
                  "    std::vector<int> foo;\n"
                  "    for (unsigned int ii = 0; ii <= foo.size(); ++ii)\n"
                  "    {\n"
                  "        if (ii == foo.size())\n"
                  "        {\n"
                  "        }\n"
                  "        else\n"
                  "        {\n"
                  "            foo[ii] = 0;\n"
                  "        }\n"
                  "    }\n"
                  "}\n");
            TODO_ASSERT_EQUALS(std::string(""), errout.str());
        }
    }





    void erase()
    {
        {
            check("void f()\n"
                  "{\n"
                  "    for (it = foo.begin(); it != foo.end(); ++it)\n"
                  "    {\n"
                  "        foo.erase(it);\n"
                  "    }\n"
                  "}\n");
            ASSERT_EQUALS("[test.cpp:5]: (error) Dangerous usage of erase\n", errout.str());
        }

        {
            check("for (it = foo.begin(); it != foo.end(); ++it)\n"
                  "{\n"
                  "    foo.erase(it);\n"
                  "}\n"
                  "for (it = foo.begin(); it != foo.end(); ++it)\n"
                  "{\n"
                  "    foo.erase(it);\n"
                  "}\n");
            ASSERT_EQUALS("[test.cpp:3]: (error) Dangerous usage of erase\n[test.cpp:7]: (error) Dangerous usage of erase\n", errout.str());
        }
    }

    void eraseBreak()
    {
        check("void f()\n"
              "{\n"
              "    for (it = foo.begin(); it != foo.end(); ++it)\n"
              "    {\n"
              "        foo.erase(it);\n"
              "        break;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void eraseReturn()
    {
        check("void f()\n"
              "{\n"
              "    for (it = foo.begin(); it != foo.end(); ++it)\n"
              "    {\n"
              "        foo.erase(it);\n"
              "        return;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void eraseGoto()
    {
        check("void f()\n"
              "{\n"
              "    for (it = foo.begin(); it != foo.end(); ++it)\n"
              "    {\n"
              "        foo.erase(it);\n"
              "        goto abc;\n"
              "    }\n"
              "bar:\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void eraseAssign()
    {
        check("void f()\n"
              "{\n"
              "    for (it = foo.begin(); it != foo.end(); ++it)\n"
              "    {\n"
              "        foo.erase(it);\n"
              "        it = foo.begin();\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void eraseDereference()
    {
        check("void f(std::vector<ints> &ints)\n"
              "{\n"
              "    std::vector<int>::iterator iter;\n"
              "    iter = ints.begin() + 2;\n"
              "    ints.erase(iter);\n"
              "    std::cout << (*iter) << std::endl;\n"
              "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:6]: (error) Dereferencing invalid iterator\n", errout.str());
    }



    void pushback1()
    {
        check("void f()\n"
              "{\n"
              "    std::vector<int>::const_iterator it = foo.begin();\n"
              "    foo.push_back(123);\n"
              "    *it;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) After push_back or push_front, the iterator 'it' may be invalid\n", errout.str());
    }

    void pushback2()
    {
        check("void f()\n"
              "{\n"
              "    std::vector<int>::const_iterator it = foo.begin();\n"
              "    foo.push_back(123);\n"
              "    {\n"
              "        int *it = &foo[0];\n"
              "        *it = 456;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void pushback3()
    {
        check("void f()\n"
              "{\n"
              "    std::vector<int>::const_iterator it = foo.begin();\n"
              "    for (it = foo.begin(); it != foo.end(); ++it)\n"
              "    {\n"
              "        foo.push_back(123);\n"
              "    }\n"
              "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:6]: (error) After push_back or push_front, the iterator 'it' may be invalid\n", errout.str());
    }

    void pushback4()
    {
        check("void f()\n"
              "{\n"
              "    std::vector<int> ints;\n"
              "    ints.push_back(1);\n"
              "    int *first = &ints[0];\n"
              "    ints.push_back(2);\n"
              "    *first;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:7]: (error) Invalid pointer 'first' after push_back / push_front\n", errout.str());
    }



    void invalidcode()
    {
        check("void f()\n"
              "{\n"
              "    for ( \n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }



    void stlBoundries1()
    {
        check("void f()\n"
              "{\n"
              "    for (it = ab.begin(); it < ab.end(); ++it)\n"
              "        ;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) STL range check should be using != and not < since the order of the pointers isn't guaranteed\n", errout.str());
    }

};

REGISTER_TEST(TestStl)
