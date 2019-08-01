# Aurora
C++ file I/O overlays

Aurora is a C++ header-only library designed to make fast, clean, C++ file I/O easier to incorporate into your project.

Aurora adds a clean, object-oriented interface with exception handling, complete errno support, and carefully-crafted data type handling.

Aurora relies exclusively on standard C mechanisms for I/O and completely ignores C++ iostream since C functions are both significantly faster and consume less memory.

Aurora optionally includes boost support if ENABLE_BOOST or ENABLE_BOOST_FILESYSTEM are defined.  (Please refer to the boost filesystem documentation at https://www.boost.org/doc/libs/1_70_0/libs/filesystem/doc/index.htm).

Aurora is currently pre-release.  It currently includes (1) CFile which is based of the standard C FILE* stream mechanism with complete binary, byte character, and wide support and (2) binary-only PosixFile.  Support is planned for better binary and text handling and high-speed memory mapping with the inclusion of additional classes.

Every attempt has been made to ensure that Aurora is completely C, C++, and Posix compliant.  However, Aurora is being developed and tested only on a late 2008 MacBook that is capable of running only OS X El Capitan (10.11.6).  If you encounter any problems, please open an issue or make a pull request.
