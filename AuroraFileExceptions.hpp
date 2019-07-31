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
