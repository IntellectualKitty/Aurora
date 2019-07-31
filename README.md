# Aurora
C++ file I/O overlays

Aurora is a C++ header-only library designed to make fast, clean, C++ file I/O easier to incorporate into your project.

Aurora adds a clean, object-oriented interface with exception handling and carefully-crafted data type handling.

Aurora relies exclusively on standard C mechanisms for I/O and completely ignores C++ iostream since C functions are both significantly faster and consume less memory.

Aurora is currently pre-release.  It currently includes (1) CFile which is based of the standard C FILE* stream mechanism with complete binary, byte character, and wide support and (2) binary-only PosixFile.  Support is planned for better binary and text handling and high-speed memory mapping with the inclusion of additional classes.
