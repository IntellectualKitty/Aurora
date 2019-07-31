//===============================================================================
// MIT License
//
// Copyright (c) 2019 IntellectualKitty
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//===============================================================================

// Pragma guard.
#pragma once

// Include standard C++ header(s).
#include <stdexcept>
#include <system_error>

// Namespace.
namespace std
{

// Exception with error code.
#define DECLARE_EXCEPTION_WITH_ERROR_CODE(classname,baseclassname) \
class classname : public baseclassname \
{ \
public: \
    classname ( \
        const int __ev, \
        const string& __what_arg) \
        noexcept(true) \
        : \
        baseclassname(__ev,__what_arg) \
    {} \
}

// Exception without error code.
#define DECLARE_EXCEPTION_WITHOUT_ERROR_CODE(classname,baseclassname) \
class classname : public baseclassname \
{ \
public: \
    classname ( \
        const string& __what_arg) \
        noexcept(true) \
        : \
        baseclassname(__what_arg) \
    {} \
}

// Exception with and without error code.
#define DECLARE_EXCEPTION_WITH_AND_WITHOUT_ERROR_CODE(classname,baseclassname) \
class classname : public baseclassname \
{ \
public: \
    classname ( \
        const int __ev, \
        const string& __what_arg) \
        noexcept(true) \
        : \
        baseclassname( \
            error_code( \
                __ev, \
                system_category()), \
            __what_arg) \
    {} \
    classname ( \
        const string& __what_arg) \
        noexcept(true) \
        : \
        baseclassname( \
            error_code( \
                0, \
                system_category()), \
            __what_arg) \
    {} \
}

// Runtime error exceptions.
DECLARE_EXCEPTION_WITHOUT_ERROR_CODE(empty_file_error,runtime_error);
DECLARE_EXCEPTION_WITHOUT_ERROR_CODE(unexpected_end_of_file,runtime_error);

// System error exceptions.
DECLARE_EXCEPTION_WITH_AND_WITHOUT_ERROR_CODE(file_error,system_error);
DECLARE_EXCEPTION_WITH_ERROR_CODE(file_open_error,file_error);
DECLARE_EXCEPTION_WITH_ERROR_CODE(file_close_error,file_error);
DECLARE_EXCEPTION_WITH_ERROR_CODE(file_status_error,file_error);
DECLARE_EXCEPTION_WITH_ERROR_CODE(file_flush_error,file_error);
DECLARE_EXCEPTION_WITH_ERROR_CODE(file_read_error,file_error);
DECLARE_EXCEPTION_WITH_ERROR_CODE(file_write_error,file_error);
DECLARE_EXCEPTION_WITH_ERROR_CODE(file_seek_error,file_error);
DECLARE_EXCEPTION_WITH_ERROR_CODE(file_tell_error,file_error);
DECLARE_EXCEPTION_WITH_ERROR_CODE(file_truncation_error,file_error);
DECLARE_EXCEPTION_WITH_ERROR_CODE(file_memory_mapping_error,file_error);

}
