#pragma once

// Include standard C header(s).
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <unistd.h>

// Include Aurora headers.
#include "AuroraStandardInteger.hpp"
#include "AuroraFileExceptions.hpp"

// Include standard Boost header(s), if requested.
#if defined(ENABLE_BOOST) || defined(ENABLE_BOOST_FILESYSTEM)
    #if defined(__clang__)
        #pragma clang diagnostic push
        #pragma clang diagnostic ignored "-Wconversion"
        #pragma clang diagnostic ignored "-Wexit-time-destructors"
    #endif
    #include <boost/filesystem.hpp>
    #if defined(__clang__)
        #pragma clang diagnostic pop
    #endif
#endif

// Namespace.
namespace std
{

// Class definition.
class CFile
{
public:

    // File types.
    enum class FileType : uint8_t
    {
        TEXT,
        BINARY
    };

    // Total file types.
    static constexpr uint8_t TOTAL_FILE_TYPES = static_cast<uint8_t>(FileType::BINARY)+1;

    // File type names.
    static const char* FILE_TYPE_NAMES[TOTAL_FILE_TYPES];

    // File access modes.
    enum class FileAccessMode : uint8_t
    {
        READ,                   // Reading
        WRITE,                  // Writing
        APPEND,                 // Writing
        READ_EXTENDED,          // Reading and writing
        WRITE_EXTENDED,         // Reading and writing
        APPEND_EXTENDED         // Reading and writing
    };

    // Total file access modes.
    static constexpr uint8_t TOTAL_FILE_ACCESS_MODES = static_cast<uint8_t>(FileAccessMode::APPEND_EXTENDED)+1;

    // File access mode descriptions.
    static const char* FILE_ACCESS_MODE_DESCRIPTIONS[TOTAL_FILE_ACCESS_MODES];

    // File access mode strings.
    static const char* FILE_ACCESS_MODE_STRINGS[TOTAL_FILE_ACCESS_MODES][TOTAL_FILE_TYPES];

    // Buffer modes.
    enum class BufferMode : uint8_t
    {
        NONE,
        LINE,
        FULL
    };

    // Total buffer modes.
    static constexpr uint8_t TOTAL_BUFFER_MODES = static_cast<uint8_t>(BufferMode::FULL)+1;

    // Buffer mode constants.
    static constexpr sint32_t BUFFER_MODE_CONSTANTS[TOTAL_BUFFER_MODES] =
    {
        _IONBF,                 // NONE
        _IOLBF,                 // LINE
        _IOFBF                  // FULL
    };

    // Character modes.
    enum class CharacterMode : sint32_t
    {
        BYTE_ORIENTATION        = -1,
        NO_ORIENTATION          =  0,
        WIDE_ORIENTATION        =  1
    };

    // Exception message length.
    static constexpr uint64_t EXCEPTION_MESSAGE_LENGTH = 2*1024;

    // String buffer lengths.
    static constexpr uint64_t BYTE_STRING_BUFFER_LENGTH = 4*1024;
    static constexpr uint64_t WIDE_STRING_BUFFER_LENGTH = 1024;

    // End of file markers.
    static constexpr sint32_t BYTE_END_OF_FILE = EOF;
    static constexpr wint_t WIDE_END_OF_FILE = WEOF;

    // Recommended file block size.
#if defined (_WIN32) || defined (_WIN64)
    static constexpr uint64_t RECOMMENDED_FILE_BLOCK_SIZE = 64*1024;
#else
    static constexpr uint64_t RECOMMENDED_FILE_BLOCK_SIZE = 2*64*1024;
#endif

    // Construction and destruction.
    CFile(
#if defined(ENABLE_BOOST) || defined(ENABLE_BOOST_FILESYSTEM)
        const boost::filesystem::path& rkFilePath,
#else
        const std::string& rkFilePath,
#endif
        const FileType eFileType,
        const FileAccessMode eFileAccessMode) noexcept(false)
        :
        m_kFilePath(rkFilePath),
        m_eFileType(eFileType),
        m_eFileAccessMode(eFileAccessMode)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Open the file.
        assert(!m_kFilePath.empty());
#if defined(ENABLE_BOOST) || defined(ENABLE_BOOST_FILESYSTEM)
        assert(m_kFilePath.is_absolute());
        assert(m_kFilePath.is_complete());
#endif
        assert(static_cast<uint8_t>(m_eFileAccessMode) < TOTAL_FILE_ACCESS_MODES);
        assert(static_cast<uint8_t>(m_eFileType) < TOTAL_FILE_TYPES);
        m_pkFilePointer = fopen(
            m_kFilePath.c_str(),
            FILE_ACCESS_MODE_STRINGS[
                static_cast<uint8_t>(m_eFileAccessMode)][
                static_cast<uint8_t>(m_eFileType)]);
        if (!m_pkFilePointer)
        {
            char acExceptionMessage[EXCEPTION_MESSAGE_LENGTH];
            snprintf(
                acExceptionMessage,
                sizeof(acExceptionMessage),
                "Error opening %s file %s for %s: %s (%d).",
                FILE_TYPE_NAMES[static_cast<uint8_t>(m_eFileType)],
                m_kFilePath.c_str(),
                FILE_ACCESS_MODE_DESCRIPTIONS[static_cast<uint8_t>(m_eFileAccessMode)],
                strerror(errno),
                errno);
            throw std::file_open_error(
                errno,
                std::string(acExceptionMessage));
        }
        assert(0 == errno);
        assert(m_pkFilePointer);
    }
    ~CFile() noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Close the file.
        assert(m_pkFilePointer);
        if (fclose(m_pkFilePointer) < 0)
        {
            char acExceptionMessage[EXCEPTION_MESSAGE_LENGTH];
            snprintf(
                acExceptionMessage,
                sizeof(acExceptionMessage),
                "Error closing file %s: %s (%d).",
                m_kFilePath.c_str(),
                strerror(errno),
                errno);
            throw std::file_close_error(
                errno,
                std::string(acExceptionMessage));
        }
        assert(0 == errno);

        // Clear the file pointer.
        m_pkFilePointer = nullptr;
    }

    // File path.
#if defined(ENABLE_BOOST) || defined(ENABLE_BOOST_FILESYSTEM)
    const boost::filesystem::path& GetFilePath() const noexcept(true)
#else
    const std::string& GetFilePath() const noexcept(true)
#endif
    {
        return m_kFilePath;
    }

    // File type.
    const FileType GetFileType() const noexcept(true)
    {
        return m_eFileType;
    }
    bool IsTextFile() const noexcept(true)
    {
        return FileType::TEXT == m_eFileType;
    }
    bool IsBinaryFile() const noexcept(true)
    {
        return FileType::BINARY == m_eFileType;
    }

    // File access mode.
    FileAccessMode GetFileAccessMode() const noexcept(true)
    {
        return m_eFileAccessMode;
    }
    bool IsReadOnly() const noexcept(true)
    {
        return FileAccessMode::READ == m_eFileAccessMode;
    }
    bool IsWriteOnly() const noexcept(true)
    {
        return
            FileAccessMode::WRITE == m_eFileAccessMode ||
            FileAccessMode::APPEND == m_eFileAccessMode;
    }
    bool IsReadWrite() const noexcept(true)
    {
        return
            FileAccessMode::READ_EXTENDED == m_eFileAccessMode ||
            FileAccessMode::WRITE_EXTENDED == m_eFileAccessMode ||
            FileAccessMode::APPEND_EXTENDED == m_eFileAccessMode;
    }

    // File pointer.
    FILE* GetFilePointer() const noexcept(true)
    {
        assert(m_pkFilePointer);
        return m_pkFilePointer;
    }

    // File descriptor.
    sint32_t GetFileDescriptor() const noexcept(true)
    {
        assert(m_pkFilePointer);
        return fileno(m_pkFilePointer);
    }

    // File locking.
    bool TryLockFile() const noexcept(true)
    {
        assert(m_pkFilePointer);
        return 0 == ftrylockfile(m_pkFilePointer);
    }
    void LockFile() const noexcept(true)
    {
        assert(m_pkFilePointer);
        flockfile(m_pkFilePointer);
    }
    void UnlockFile() const noexcept(true)
    {
        assert(m_pkFilePointer);
        funlockfile(m_pkFilePointer);
    }

    // File block size.
    uint64_t GetFileBlockSize() const noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Get the file status.
        assert(m_pkFilePointer);
        struct stat kFileStatus;
        if (fstat(fileno(m_pkFilePointer),&kFileStatus) < 0)
        {
            char acExceptionMessage[EXCEPTION_MESSAGE_LENGTH];
            snprintf(
                acExceptionMessage,
                sizeof(acExceptionMessage),
                "Error getting status of file %s: %s (%d).",
                m_kFilePath.c_str(),
                strerror(errno),
                errno);
            throw std::file_status_error(
                errno,
                std::string(acExceptionMessage));
        }
        assert(0 == errno);

        // Return the file block size.
        assert(kFileStatus.st_blksize >= 0);
        return static_cast<uint64_t>(kFileStatus.st_blksize);
    }

    // Buffer support.
    bool SetBuffer(
        const BufferMode eBufferMode,
        const uint64_t ulBufferSize,
        void* avUserBuffer = nullptr) noexcept(true)
    {
        // Set the buffer mode and size.
        assert(m_pkFilePointer);
        assert(static_cast<uint8_t>(eBufferMode) < TOTAL_BUFFER_MODES);
        return 0 == setvbuf(
            m_pkFilePointer,
            reinterpret_cast<char *>(avUserBuffer),
            BUFFER_MODE_CONSTANTS[static_cast<uint8_t>(eBufferMode)],
            ulBufferSize);
    }
    bool SetOptimalBuffer() noexcept(false)
    {
        return SetBuffer(
            BufferMode::FULL,
            std::max(GetFileBlockSize(),RECOMMENDED_FILE_BLOCK_SIZE));
    }

    // Flush the file.
    void Flush() noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Flush the file.
        assert(m_pkFilePointer);
        if (fflush(m_pkFilePointer) < 0)
        {
            char acExceptionMessage[EXCEPTION_MESSAGE_LENGTH];
            snprintf(
                acExceptionMessage,
                sizeof(acExceptionMessage),
                "Error flushing file %s: %s (%d).",
                m_kFilePath.c_str(),
                strerror(errno),
                errno);
            throw std::file_flush_error(
                errno,
                std::string(acExceptionMessage));
        }
        assert(0 == errno);
    }

    // Explicit file positioning.
    uint64_t GetFilePosition() const noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Get the current file position.
        assert(m_pkFilePointer);
        const sint64_t slPosition = ftello(m_pkFilePointer);
        if (slPosition < 0)
        {
            char acExceptionMessage[EXCEPTION_MESSAGE_LENGTH];
            snprintf(
                acExceptionMessage,
                sizeof(acExceptionMessage),
                "Error getting position of file %s: %s (%d).",
                m_kFilePath.c_str(),
                strerror(errno),
                errno);
            throw std::file_tell_error(
                errno,
                std::string(acExceptionMessage));
        }
        assert(0 == errno);
        assert(slPosition >= 0);
        return static_cast<uint64_t>(slPosition);
    }
    void SetFilePosition(const uint64_t ulPosition) const noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Seek to the requested file position.
        assert(m_pkFilePointer);
        assert(ulPosition <= std::numeric_limits<sint64_t>::max());
        if (fseeko(m_pkFilePointer,static_cast<sint64_t>(ulPosition),SEEK_SET) < 0)
        {
            char acExceptionMessage[EXCEPTION_MESSAGE_LENGTH];
            snprintf(
                acExceptionMessage,
                sizeof(acExceptionMessage),
                "Error setting position of file %s to %llu: %s (%d).",
                m_kFilePath.c_str(),
                ulPosition,
                strerror(errno),
                errno);
            throw std::file_seek_error(
                errno,
                std::string(acExceptionMessage));
        }
        assert(0 == errno);
    }

    // Traditional file positioning.
    void SeekSet(const sint64_t slOffset) const noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Seek from the start of the file.
        assert(m_pkFilePointer);
        if (fseeko(m_pkFilePointer,slOffset,SEEK_SET) < 0)
        {
            char acExceptionMessage[EXCEPTION_MESSAGE_LENGTH];
            snprintf(
                acExceptionMessage,
                sizeof(acExceptionMessage),
                "Error seeking to position %lld of file %s: %s (%d).",
                slOffset,
                m_kFilePath.c_str(),
                strerror(errno),
                errno);
            throw std::file_seek_error(
                errno,
                std::string(acExceptionMessage));
        }
    }
    void SeekCurrent(const sint64_t slOffset) const noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Seek from the current file position.
        assert(m_pkFilePointer);
        if (fseeko(m_pkFilePointer,slOffset,SEEK_CUR) < 0)
        {
            char acExceptionMessage[EXCEPTION_MESSAGE_LENGTH];
            snprintf(
                acExceptionMessage,
                sizeof(acExceptionMessage),
                "Error seeking from current position of file %s by %lld: %s (%d).",
                m_kFilePath.c_str(),
                slOffset,
                strerror(errno),
                errno);
            throw std::file_seek_error(
                errno,
                std::string(acExceptionMessage));
        }
    }
    void SeekEnd(const sint64_t slOffset) const noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Seek from the end of the file.
        assert(m_pkFilePointer);
        if (fseeko(m_pkFilePointer,slOffset,SEEK_END) < 0)
        {
            char acExceptionMessage[EXCEPTION_MESSAGE_LENGTH];
            snprintf(
                acExceptionMessage,
                sizeof(acExceptionMessage),
                "Error seeking from the end of file %s by %lld: %s (%d).",
                m_kFilePath.c_str(),
                slOffset,
                strerror(errno),
                errno);
            throw std::file_seek_error(
                errno,
                std::string(acExceptionMessage));
        }
    }
    void Rewind() noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Seek to the beginning of the file.
        assert(m_pkFilePointer);
        if (fseeko(m_pkFilePointer,0,SEEK_SET) < 0)
        {
            char acExceptionMessage[EXCEPTION_MESSAGE_LENGTH];
            snprintf(
                acExceptionMessage,
                sizeof(acExceptionMessage),
                "Error rewinding file %s: %s (%d).",
                m_kFilePath.c_str(),
                strerror(errno),
                errno);
            throw std::file_seek_error(
                errno,
                std::string(acExceptionMessage));
        }
        assert(0 == errno);

        // Clear the error and end-of-file flags.
        clearerr(m_pkFilePointer);
    }

    // File length.
    uint64_t GetFileLength() const noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Get the file status.
        assert(m_pkFilePointer);
        struct stat kFileStatus;
        if (fstat(fileno(m_pkFilePointer),&kFileStatus) < 0)
        {
            char acExceptionMessage[EXCEPTION_MESSAGE_LENGTH];
            snprintf(
                acExceptionMessage,
                sizeof(acExceptionMessage),
                "Error getting status of file %s: %s (%d).",
                m_kFilePath.c_str(),
                strerror(errno),
                errno);
            throw std::file_status_error(
                errno,
                std::string(acExceptionMessage));
        }
        assert(0 == errno);

        // Return the file length.
        assert(kFileStatus.st_size >= 0);
        return static_cast<uint64_t>(kFileStatus.st_size);
    }
    void SetFileLength(const uint64_t ulLength) const noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Set the file length.
        assert(m_pkFilePointer);
        assert(ulLength <= std::numeric_limits<sint64_t>::max());
        if (ftruncate(fileno(m_pkFilePointer),static_cast<sint64_t>(ulLength)) < 0)
        {
            char acExceptionMessage[EXCEPTION_MESSAGE_LENGTH];
            snprintf(
                acExceptionMessage,
                sizeof(acExceptionMessage),
                "Error setting length of file %s to %llu: %s (%d).",
                m_kFilePath.c_str(),
                ulLength,
                strerror(errno),
                errno);
            throw std::file_truncation_error(
                errno,
                std::string(acExceptionMessage));
        }
        assert(0 == errno);
    }

    // End of file support.
    bool EndOfFile() const noexcept(false)
    {
        assert(m_pkFilePointer);
        return GetFilePosition() >= GetFileLength();
    }
    uint64_t GetBytesRemaining() const noexcept(false)
    {
        assert(m_pkFilePointer);
        const uint64_t ulPosition = GetFilePosition();
        const uint64_t ulLength = GetFileLength();
        return ulPosition < ulLength ? ulLength-ulPosition : 0;
    }
    // Character mode.  Initially the file stream will have no orientation, but once
    // it's set -- either explicitly or by a write operation -- it cannot be changed.
    CharacterMode SetCharacterMode(const CharacterMode eDesiredCharacterMode) const noexcept(true)
    {
        // Attempt to set the file stream's character mode.
        assert(m_pkFilePointer);
        assert(IsTextFile());
        assert(
            CharacterMode::BYTE_ORIENTATION == eDesiredCharacterMode ||
            CharacterMode::NO_ORIENTATION   == eDesiredCharacterMode ||
            CharacterMode::WIDE_ORIENTATION == eDesiredCharacterMode);
        const sint32_t siActualCharacterMode = fwide(m_pkFilePointer,
            static_cast<sint32_t>(eDesiredCharacterMode));

        // Return the file stream's actual character orientation.
        if (siActualCharacterMode < 0)
        {
            return CharacterMode::BYTE_ORIENTATION;
        }
        else if (siActualCharacterMode > 0)
        {
            return CharacterMode::WIDE_ORIENTATION;
        }
        else
        {
            assert(0 == siActualCharacterMode);
            return CharacterMode::NO_ORIENTATION;
        }
    }
    CharacterMode GetCharacterMode() const noexcept(true)
    {
        // Get the file stream's actual character mode without changing it by setting it
        // to no orientation.
        assert(m_pkFilePointer);
        assert(IsTextFile());
        return SetCharacterMode(CharacterMode::NO_ORIENTATION);
    }

    // Unformatted character input and output (byte).
    sint32_t GetByteCharacter() noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Read the character.
        assert(m_pkFilePointer);
        assert(IsTextFile());
        assert(IsReadOnly() || IsReadWrite());
        const sint32_t siCharacter = fgetc(m_pkFilePointer);
        if (siCharacter < 0 && 0 != errno)
        {
            char acExceptionMessage[EXCEPTION_MESSAGE_LENGTH];
            snprintf(
                acExceptionMessage,
                sizeof(acExceptionMessage),
                "Error getting character from file %s: %s (%d).",
                m_kFilePath.c_str(),
                strerror(errno),
                errno);
            throw std::file_read_error(
                errno,
                std::string(acExceptionMessage));
        }
        assert(0 == errno);
        return siCharacter;
    }
    void UngetByteCharacter(const sint32_t siCharacter) noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Un-read the character.
        assert(m_pkFilePointer);
        assert(IsTextFile());
        assert(IsReadOnly() || IsReadWrite());
        if (ungetc(siCharacter,m_pkFilePointer) < 0)
        {
            char acExceptionMessage[EXCEPTION_MESSAGE_LENGTH];
            snprintf(
                acExceptionMessage,
                sizeof(acExceptionMessage),
                EOF == siCharacter ?
                    "Error ungetting EOF character from file %s: %s (%d)."
                    :
                    "Error ungetting regular character from file %s: %s (%d).",
                m_kFilePath.c_str(),
                strerror(errno),
                errno);
            throw std::file_read_error(
                errno,
                std::string(acExceptionMessage));
        }
        assert(0 == errno);
    }
    void PutByteCharacter(const sint32_t siCharacter) noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Write the character.
        assert(m_pkFilePointer);
        assert(IsTextFile());
        assert(IsWriteOnly() || IsReadWrite());
        if (fputc(siCharacter,m_pkFilePointer) < 0)
        {
            char acExceptionMessage[EXCEPTION_MESSAGE_LENGTH];
            snprintf(
                acExceptionMessage,
                sizeof(acExceptionMessage),
                "Error putting character to file %s: %s (%d).",
                m_kFilePath.c_str(),
                strerror(errno),
                errno);
            throw std::file_write_error(
                errno,
                std::string(acExceptionMessage));
        }
        assert(0 == errno);
    }

    // Unformatted character input and output (wide).
    wint_t GetWideCharacter() noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Read the wide character.
        assert(m_pkFilePointer);
        assert(IsTextFile());
        assert(IsReadOnly() || IsReadWrite());
        const wint_t wcCharacter = fgetwc(m_pkFilePointer);
        if (wcCharacter < 0 && 0 != errno)
        {
            char acExceptionMessage[EXCEPTION_MESSAGE_LENGTH];
            snprintf(
                acExceptionMessage,
                sizeof(acExceptionMessage),
                "Error getting wide character from file %s: %s (%d).",
                m_kFilePath.c_str(),
                strerror(errno),
                errno);
            throw std::file_read_error(
                errno,
                std::string(acExceptionMessage));
        }
        assert(0 == errno);
        return wcCharacter;
    }
    void UngetWideCharacter(const wint_t wcCharacter) noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Un-read the wide character.
        assert(m_pkFilePointer);
        assert(IsTextFile());
        assert(IsReadOnly() || IsReadWrite());
        if (ungetwc(wcCharacter,m_pkFilePointer) < 0)
        {
            char acExceptionMessage[EXCEPTION_MESSAGE_LENGTH];
            snprintf(
                acExceptionMessage,
                sizeof(acExceptionMessage),
                EOF == wcCharacter ?
                    "Error ungetting EOF wide character from file %s: %s (%d)."
                    :
                    "Error ungetting regular wide character from file %s: %s (%d).",
                m_kFilePath.c_str(),
                strerror(errno),
                errno);
            throw std::file_read_error(
                errno,
                std::string(acExceptionMessage));
        }
        assert(0 == errno);
    }
    void PutWideCharacter(const wint_t wcCharacter) noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Write the wide character.
        assert(m_pkFilePointer);
        assert(IsTextFile());
        assert(IsWriteOnly() || IsReadWrite());
        if (fputwc(wcCharacter,m_pkFilePointer) < 0)
        {
            char acExceptionMessage[EXCEPTION_MESSAGE_LENGTH];
            snprintf(
                acExceptionMessage,
                sizeof(acExceptionMessage),
                "Error putting wide character to file %s: %s (%d).",
                m_kFilePath.c_str(),
                strerror(errno),
                errno);
            throw std::file_write_error(
                errno,
                std::string(acExceptionMessage));
        }
        assert(0 == errno);
    }

    // Unformatted string input and output (byte).
    std::string GetByteString() noexcept(false)
    {
        assert(m_pkFilePointer);
        assert(IsTextFile());
        assert(IsReadOnly() || IsReadWrite());
        std::string rkString;
        char acStringBuffer[BYTE_STRING_BUFFER_LENGTH];
        uint64_t ulStringBufferSize = 0;
        while (true)
        {
            sint32_t siCharacter = GetByteCharacter();
            if (BYTE_END_OF_FILE == siCharacter || '\n' == siCharacter)
            {
                if (ulStringBufferSize > 0)
                {
                    rkString.append(acStringBuffer,ulStringBufferSize);
                    ulStringBufferSize = 0;
                }
                break;
            }
            assert(ulStringBufferSize < BYTE_STRING_BUFFER_LENGTH);
            acStringBuffer[ulStringBufferSize++] = static_cast<char>(siCharacter);
            assert(ulStringBufferSize <= BYTE_STRING_BUFFER_LENGTH);
            if (ulStringBufferSize >= BYTE_STRING_BUFFER_LENGTH)
            {
                rkString.append(acStringBuffer,ulStringBufferSize);
                ulStringBufferSize = 0;
            }
        }
        assert(0 == ulStringBufferSize);
        return rkString;
    }
    sint32_t PutByteString(const char* acString) noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Write the string.
        assert(m_pkFilePointer);
        assert(IsTextFile());
        assert(IsWriteOnly() || IsReadWrite());
        assert(acString);
        const sint32_t siCharactersWritten = fputs(acString,m_pkFilePointer);
        if (siCharactersWritten < 0)
        {
            char acExceptionMessage[EXCEPTION_MESSAGE_LENGTH];
            snprintf(
                acExceptionMessage,
                sizeof(acExceptionMessage),
                "Error putting string to file %s: %s (%d).",
                m_kFilePath.c_str(),
                strerror(errno),
                errno);
            throw std::file_write_error(
                errno,
                std::string(acExceptionMessage));
        }
        assert(0 == errno);
        assert(siCharactersWritten >= 0);
        return siCharactersWritten;
    }

    // Unformatted string input and output (wide).
    std::wstring GetWideString() noexcept(false)
    {
        assert(m_pkFilePointer);
        assert(IsTextFile());
        assert(IsReadOnly() || IsReadWrite());
        std::wstring rkString;
        wchar_t awcStringBuffer[WIDE_STRING_BUFFER_LENGTH];
        uint64_t ulStringBufferSize = 0;
        while (true)
        {
            wint_t wcCharacter = GetWideCharacter();
            if (WIDE_END_OF_FILE == wcCharacter || L'\n' == wcCharacter)
            {
                if (ulStringBufferSize > 0)
                {
                    rkString.append(awcStringBuffer,ulStringBufferSize);
                    ulStringBufferSize = 0;
                }
                break;
            }
            assert(ulStringBufferSize < WIDE_STRING_BUFFER_LENGTH);
            awcStringBuffer[ulStringBufferSize++] = wcCharacter;
            assert(ulStringBufferSize <= WIDE_STRING_BUFFER_LENGTH);
            if (ulStringBufferSize >= WIDE_STRING_BUFFER_LENGTH)
            {
                rkString.append(awcStringBuffer,ulStringBufferSize);
                ulStringBufferSize = 0;
            }
        }
        assert(0 == ulStringBufferSize);
        return rkString;
    }
    sint32_t PutWideString(const wchar_t* awcString) noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Write the wide string.
        assert(m_pkFilePointer);
        assert(IsTextFile());
        assert(IsWriteOnly() || IsReadWrite());
        assert(awcString);
        const sint32_t siCharactersWritten = fputws(awcString,m_pkFilePointer);
        if (siCharactersWritten < 0)
        {
            char acExceptionMessage[EXCEPTION_MESSAGE_LENGTH];
            snprintf(
                acExceptionMessage,
                sizeof(acExceptionMessage),
                "Error putting wide string to file %s: %s (%d).",
                m_kFilePath.c_str(),
                strerror(errno),
                errno);
            throw std::file_write_error(
                errno,
                std::string(acExceptionMessage));
        }
        assert(0 == errno);
        assert(siCharactersWritten >= 0);
        return siCharactersWritten;
    }

    // Unformatted line input and output (byte).
    std::string GetByteLine() noexcept(false)
    {
        assert(m_pkFilePointer);
        assert(IsTextFile());
        assert(IsReadOnly() || IsReadWrite());
        std::string kString;
        char acStringBuffer[BYTE_STRING_BUFFER_LENGTH];
        uint64_t ulStringBufferSize = 0;
        while (true)
        {
            sint32_t siCharacter = GetByteCharacter();
            if (BYTE_END_OF_FILE == siCharacter || '\n' == siCharacter)
            {
                if ('\n' == siCharacter)
                {
                    assert(ulStringBufferSize < BYTE_STRING_BUFFER_LENGTH);
                    acStringBuffer[ulStringBufferSize++] = '\n';
                    assert(ulStringBufferSize <= BYTE_STRING_BUFFER_LENGTH);
                }
                kString.append(acStringBuffer,ulStringBufferSize);
                ulStringBufferSize = 0;
                break;
            }
            assert(ulStringBufferSize < BYTE_STRING_BUFFER_LENGTH);
            acStringBuffer[ulStringBufferSize++] = static_cast<char>(siCharacter);
            assert(ulStringBufferSize <= BYTE_STRING_BUFFER_LENGTH);
            if (ulStringBufferSize >= BYTE_STRING_BUFFER_LENGTH)
            {
                kString.append(acStringBuffer,ulStringBufferSize);
                ulStringBufferSize = 0;
            }
        }
        assert(0 == ulStringBufferSize);
        return kString;
    }
    sint32_t PutByteLine(const char* acString) noexcept(false)
    {
        assert(m_pkFilePointer);
        assert(IsTextFile());
        assert(IsWriteOnly() || IsReadWrite());
        assert(acString);
        const sint32_t siCharactersWritten = PutByteString(acString);
        PutByteCharacter('\n');
        return siCharactersWritten+1;
    }

    // Unformatted line input and output (wide).
    std::wstring GetWideLine() noexcept(false)
    {
        assert(m_pkFilePointer);
        assert(IsTextFile());
        assert(IsReadOnly() || IsReadWrite());
        std::wstring kString;
        wchar_t awcStringBuffer[WIDE_STRING_BUFFER_LENGTH];
        uint64_t ulStringBufferSize = 0;
        while (true)
        {
            wint_t wcCharacter = GetWideCharacter();
            if (WIDE_END_OF_FILE == wcCharacter || L'\n' == wcCharacter)
            {
                if (L'\n' == wcCharacter)
                {
                    assert(ulStringBufferSize < WIDE_STRING_BUFFER_LENGTH);
                    awcStringBuffer[ulStringBufferSize++] = L'\n';
                    assert(ulStringBufferSize <= WIDE_STRING_BUFFER_LENGTH);
                }
                kString.append(awcStringBuffer,ulStringBufferSize);
                ulStringBufferSize = 0;
                break;
            }
            assert(ulStringBufferSize < WIDE_STRING_BUFFER_LENGTH);
            awcStringBuffer[ulStringBufferSize++] = wcCharacter;
            assert(ulStringBufferSize <= WIDE_STRING_BUFFER_LENGTH);
            if (ulStringBufferSize >= WIDE_STRING_BUFFER_LENGTH)
            {
                kString.append(awcStringBuffer,ulStringBufferSize);
                ulStringBufferSize = 0;
            }
        }
        assert(0 == ulStringBufferSize);
        return kString;
    }
    sint32_t PutWideLine(const wchar_t* awcString) noexcept(false)
    {
        assert(m_pkFilePointer);
        assert(IsTextFile());
        assert(IsWriteOnly() || IsReadWrite());
        assert(awcString);
        const sint32_t siCharactersWritten = PutWideString(awcString);
        PutWideCharacter(L'\n');
        return siCharactersWritten+1;
    }

    // Formatted input and output (byte).
    sint32_t PrintByte(const char* acFormat, ...) noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Print the data.
        assert(m_pkFilePointer);
        assert(IsTextFile());
        assert(IsWriteOnly() || IsReadWrite());
        assert(acFormat);
        va_list kArguments;
        va_start(kArguments,acFormat);
        const sint32_t siResult = vfprintf(m_pkFilePointer,acFormat,kArguments);
        va_end(kArguments);

        // Check for errors.
        if (siResult < 0)
        {
            char acExceptionMessage[EXCEPTION_MESSAGE_LENGTH];
            snprintf(
                acExceptionMessage,
                sizeof(acExceptionMessage),
                "Error printing to file %s: %s (%d).",
                m_kFilePath.c_str(),
                strerror(errno),
                errno);
            throw std::file_write_error(
                errno,
                std::string(acExceptionMessage));
        }
        assert(0 == errno);
        return siResult;
    }
    sint32_t ScanByte(const char* acFormat, ...) noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Scan the data.
        assert(m_pkFilePointer);
        assert(IsTextFile());
        assert(IsReadOnly() || IsReadWrite());
        assert(acFormat);
        va_list kArguments;
        va_start(kArguments,acFormat);
        const sint32_t siResult = vfscanf(m_pkFilePointer,acFormat,kArguments);
        va_end(kArguments);

        // Check for errors.
        if (siResult < 0)
        {
            char acExceptionMessage[EXCEPTION_MESSAGE_LENGTH];
            snprintf(
                acExceptionMessage,
                sizeof(acExceptionMessage),
                "Error scanning from file %s: %s (%d).",
                m_kFilePath.c_str(),
                strerror(errno),
                errno);
            throw std::file_read_error(
                errno,
                std::string(acExceptionMessage));
        }
        assert(0 == errno);
        return siResult;
    }

    // Formatted input and output (wide).
    sint32_t PrintWide(const wchar_t* awcFormat, ...) noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Print the data.
        assert(m_pkFilePointer);
        assert(IsTextFile());
        assert(IsWriteOnly() || IsReadWrite());
        assert(awcFormat);
        va_list kArguments;
        va_start(kArguments,awcFormat);
        const sint32_t siResult = vfwprintf(m_pkFilePointer,awcFormat,kArguments);
        va_end(kArguments);

        // Check for errors.
        if (siResult < 0)
        {
            char acExceptionMessage[EXCEPTION_MESSAGE_LENGTH];
            snprintf(
                acExceptionMessage,
                sizeof(acExceptionMessage),
                "Error printing to file %s: %s (%d).",
                m_kFilePath.c_str(),
                strerror(errno),
                errno);
            throw std::file_write_error(
                errno,
                std::string(acExceptionMessage));
        }
        assert(0 == errno);
        return siResult;
    }
    sint32_t ScanWide(const wchar_t* awcFormat, ...) noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Scan the data.
        assert(m_pkFilePointer);
        assert(IsTextFile());
        assert(IsReadOnly() || IsReadWrite());
        assert(awcFormat);
        va_list kArguments;
        va_start(kArguments,awcFormat);
        const sint32_t siResult = vfwscanf(m_pkFilePointer,awcFormat,kArguments);
        va_end(kArguments);

        // Check for errors.
        if (siResult < 0)
        {
            char acExceptionMessage[EXCEPTION_MESSAGE_LENGTH];
            snprintf(
                acExceptionMessage,
                sizeof(acExceptionMessage),
                "Error scanning from file %s: %s (%d).",
                m_kFilePath.c_str(),
                strerror(errno),
                errno);
            throw std::file_read_error(
                errno,
                std::string(acExceptionMessage));
        }
        assert(0 == errno);
        return siResult;
    }

    // Read and write binary bytes.
    uint64_t ReadBytes(
        void* avBuffer,
        const uint64_t ulTotalBytesToRead) noexcept(false)
    {
        static_assert(sizeof(unsigned char) == 1, "sizeof(unsigned char) is not equal to 1 byte");
        assert(m_pkFilePointer);
        assert(IsBinaryFile());
        assert(IsReadOnly() || IsReadWrite());
        assert(avBuffer);
        assert(ulTotalBytesToRead > 0);
        return ReadElements(avBuffer,sizeof(unsigned char),ulTotalBytesToRead);
    }
    uint64_t WriteBytes(
        const void* avBuffer,
        const uint64_t ulTotalBytesToWrite) noexcept(false)
    {
        static_assert(sizeof(unsigned char) == 1, "sizeof(unsigned char) is not equal to 1 byte");
        assert(m_pkFilePointer);
        assert(IsBinaryFile());
        assert(IsWriteOnly() || IsReadWrite());
        assert(avBuffer);
        assert(ulTotalBytesToWrite > 0);
        return WriteElements(avBuffer,sizeof(unsigned char),ulTotalBytesToWrite);
    }

    // Read and write binary elements.
    uint64_t ReadElements(
        void* avBuffer,
        const uint64_t ulElementSize,
        const uint64_t ulTotalElementsToRead) noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Attempt to read the requested number of elements.
        assert(m_pkFilePointer);
        assert(IsBinaryFile());
        assert(IsReadOnly() || IsReadWrite());
        assert(avBuffer);
        assert(ulElementSize > 0);
        assert(ulTotalElementsToRead > 0);
        uint8_t* aubBuffer = reinterpret_cast<uint8_t*>(avBuffer);
        uint64_t ulTotalElementsRead = 0;
        while (ulTotalElementsRead < ulTotalElementsToRead)
        {
            const uint64_t ulElementsToActuallyRead = ulTotalElementsToRead-ulTotalElementsRead;
            const uint64_t ulElementsActuallyRead = fread(aubBuffer,ulElementSize,ulElementsToActuallyRead,m_pkFilePointer);
            ulTotalElementsRead += ulElementsActuallyRead;
            aubBuffer += ulElementsActuallyRead*ulElementSize;
            if (ulElementsActuallyRead < ulElementsToActuallyRead)
            {
                if (0 != errno)
                {
                    char acExceptionMessage[EXCEPTION_MESSAGE_LENGTH];
                    snprintf(
                        acExceptionMessage,
                        sizeof(acExceptionMessage),
                        "Error reading from file %s: read %llu elements but expected to read %llu elements (of size %llu): %s (%d).",
                        m_kFilePath.c_str(),
                        ulTotalElementsRead,
                        ulTotalElementsToRead,
                        ulElementSize,
                        strerror(errno),
                        errno);
                    throw std::file_read_error(
                        errno,
                        std::string(acExceptionMessage));
                }
                else if (EndOfFile())
                {
                    break;
                }
            }
        }
        assert(0 == errno);
        assert(ulTotalElementsToRead == ulTotalElementsRead || EndOfFile());
        return ulTotalElementsRead;
    }
    uint64_t WriteElements(
        const void* avBuffer,
        const uint64_t ulElementSize,
        const uint64_t ulTotalElementsToWrite) noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Attempt to write the requested number of elements.
        assert(m_pkFilePointer);
        assert(IsBinaryFile());
        assert(IsWriteOnly() || IsReadWrite());
        assert(avBuffer);
        assert(ulElementSize > 0);
        assert(ulTotalElementsToWrite > 0);
        const uint8_t* aubBuffer = reinterpret_cast<const uint8_t*>(avBuffer);
        uint64_t ulTotalElementsWritten = 0;
        while (ulTotalElementsWritten < ulTotalElementsToWrite)
        {
            const uint64_t ulElementsToActuallyWrite = ulTotalElementsToWrite-ulTotalElementsWritten;
            const uint64_t ulElementsActuallyWritten = fwrite(aubBuffer,ulElementSize,ulElementsToActuallyWrite,m_pkFilePointer);
            ulTotalElementsWritten += ulElementsActuallyWritten;
            aubBuffer += ulElementsActuallyWritten*ulElementSize;
            if (ulElementsActuallyWritten < ulElementsToActuallyWrite)
            {
                if (0 != errno)
                {
                    char acExceptionMessage[EXCEPTION_MESSAGE_LENGTH];
                    snprintf(
                        acExceptionMessage,
                        sizeof(acExceptionMessage),
                        "Error writing to file %s: wrote %llu elements but expected to write %llu elements (of size %llu): %s (%d).",
                        m_kFilePath.c_str(),
                        ulTotalElementsWritten,
                        ulTotalElementsToWrite,
                        ulElementSize,
                        strerror(errno),
                        errno);
                    throw std::file_write_error(
                        errno,
                        std::string(acExceptionMessage));
                }
            }
        }
        assert(0 == errno);
        assert(ulTotalElementsToWrite == ulTotalElementsWritten);
        return ulTotalElementsWritten;
    }

protected:

    // File path.
#if defined(ENABLE_BOOST) || defined(ENABLE_BOOST_FILESYSTEM)
    const boost::filesystem::path m_kFilePath;
#else
    const std::string m_kFilePath;
#endif

    // File type.
    const FileType m_eFileType;

    // File access mode.
    const FileAccessMode m_eFileAccessMode;

    // File pointer.
    FILE* m_pkFilePointer = nullptr;
};

// File type names.
const char* CFile::FILE_TYPE_NAMES[TOTAL_FILE_TYPES] =
{
    "text",                 // TEXT
    "binary"                // BINARY
};

// File access mode descriptions.
const char* CFile::FILE_ACCESS_MODE_DESCRIPTIONS[TOTAL_FILE_ACCESS_MODES] =
{
    "reading",              // READ
    "writing",              // WRITE
    "appending",            // APPEND
    "extended reading",     // READ_EXTENDED
    "extended writing",     // WRITE_EXTENDED
    "extended appending"    // APPEND_EXTENDED
};

// File access mode strings.
const char* CFile::FILE_ACCESS_MODE_STRINGS[TOTAL_FILE_ACCESS_MODES][TOTAL_FILE_TYPES] =
{
    { "r", "rb" },          // READ
    { "w", "wb" },          // WRITE
    { "a", "ab" },          // APPEND
    { "r+", "rb+" },        // READ_EXTENDED
    { "w+", "wb+" },        // WRITE_EXTENDED
    { "a+", "ab+" }         // APPEND_EXTENDED
};

}
