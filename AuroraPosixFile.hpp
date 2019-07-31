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
class PosixFile
{
public:

    // Open flags.
    static constexpr sint32_t READ_ONLY                 = O_RDONLY;
    static constexpr sint32_t WRITE_ONLY                = O_WRONLY;
    static constexpr sint32_t READ_WRITE                = O_RDWR;
    static constexpr sint32_t NON_BLOCKING              = O_NONBLOCK;
    static constexpr sint32_t APPEND                    = O_APPEND;
    static constexpr sint32_t CREATE                    = O_CREAT;
    static constexpr sint32_t TRUNCATE                  = O_TRUNC;
    static constexpr sint32_t EXCLUSIVE                 = O_EXCL;
    static constexpr sint32_t SHARED_LOCK               = O_SHLOCK;
    static constexpr sint32_t EXCLUSIVE_LOCK            = O_EXLOCK;
    static constexpr sint32_t NO_SYMLINK                = O_NOFOLLOW;
    static constexpr sint32_t SYMLINK                   = O_SYMLINK;
    static constexpr sint32_t EVENT_NOTIFICATIONS_ONLY  = O_EVTONLY;
    static constexpr sint32_t CLOSE_ON_EXECUTE          = O_CLOEXEC;

    // Mode flags.
    static constexpr uint16_t USER_READ                 = S_IRUSR;
    static constexpr uint16_t USER_WRITE                = S_IWUSR;
    static constexpr uint16_t USER_EXECUTE              = S_IXUSR;
    static constexpr uint16_t GROUP_READ                = S_IRGRP;
    static constexpr uint16_t GROUP_WRITE               = S_IWGRP;
    static constexpr uint16_t GROUP_EXECUTE             = S_IXGRP;
    static constexpr uint16_t OTHER_READ                = S_IROTH;
    static constexpr uint16_t OTHER_WRITE               = S_IWOTH;
    static constexpr uint16_t OTHER_EXECUTE             = S_IXOTH;
    static constexpr uint16_t SET_USER_ID               = S_ISUID;
    static constexpr uint16_t SET_GROUP_ID              = S_ISGID;
    static constexpr uint16_t SAVE_SWAPPED_TEXT         = S_ISVTX;

    // Read and write composite mode flags.
    static constexpr uint16_t USER_READ_AND_WRITE       = S_IRUSR | S_IWUSR;
    static constexpr uint16_t GROUP_READ_AND_WRITE      = S_IRGRP | S_IWGRP;
    static constexpr uint16_t OTHER_READ_AND_WRITE      = S_IROTH | S_IWOTH;

    // Read, write, and execute composite mode flags.
    static constexpr uint16_t USER_ALL                  = S_IRUSR | S_IWUSR | S_IXUSR;
    static constexpr uint16_t GROUP_ALL                 = S_IRGRP | S_IWGRP | S_IXGRP;
    static constexpr uint16_t OTHER_ALL                 = S_IROTH | S_IWOTH | S_IXOTH;

    // Construction and destruction.
    PosixFile(
#if defined(ENABLE_BOOST) || defined(ENABLE_BOOST_FILESYSTEM)
        const boost::filesystem::path& rkFilePath,
#else
        const std::string& rkFilePath,
#endif
        const sint32_t siOpenFlags,
        const uint16_t usModeFlags = 0) noexcept(false)
        :
        m_kFilePath(rkFilePath)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Open the file.
        m_siFileDescriptor = open(m_kFilePath.c_str(),siOpenFlags,usModeFlags);
        if (m_siFileDescriptor < 0)
        {
            char acExceptionMessage[512];
            snprintf(
                acExceptionMessage,
                sizeof(acExceptionMessage),
                "Error opening file %s: %s (%d).",
                m_kFilePath.c_str(),
                strerror(errno),
                errno);
            throw std::file_open_error(
                errno,
                std::string(acExceptionMessage));
       }
        assert(0 == errno);
        assert(m_siFileDescriptor >= 0);
    }
    ~PosixFile() noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Close the file.
        assert(m_siFileDescriptor >= 0);
        assert(fileno(stdin) != m_siFileDescriptor);
        assert(fileno(stdout) != m_siFileDescriptor);
        assert(fileno(stderr) != m_siFileDescriptor);
        if (close(m_siFileDescriptor) < 0)
        {
            char acExceptionMessage[512];
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
        
        // Clear the file descriptor.
        m_siFileDescriptor = -1;
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

    // File descriptor.
    sint32_t GetFileDescriptor() const noexcept(true)
    {
        return m_siFileDescriptor;
    }

    // File block size.
    uint64_t GetFileBlockSize() const noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Get the file status.
        struct stat kFileStatus;
        assert(m_siFileDescriptor >= 0);
        if (fstat(m_siFileDescriptor,&kFileStatus) < 0)
        {
            char acExceptionMessage[512];
            snprintf(
                acExceptionMessage,
                sizeof(acExceptionMessage),
                "Error getting the status of file %s: %s (%d).",
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

    // Explicit file positioning.
    uint64_t GetFilePosition() const noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Get the current file position.
        assert(m_siFileDescriptor >= 0);
        static_assert(
            sizeof(sint64_t) >= sizeof(off_t),
            "sint64_t is smaller than off_t");
        static_assert(
            std::numeric_limits<sint64_t>::is_signed == std::numeric_limits<off_t>::is_signed,
            "sint64_t signedness does not match off_t");
        static_assert(
            std::numeric_limits<sint64_t>::max() >= std::numeric_limits<off_t>::max(),
            "sint64_t max insufficient for off_t");
        static_assert(
            std::numeric_limits<sint64_t>::min() <= std::numeric_limits<off_t>::min(),
            "sint64_t min insufficient for off_t");
        const sint64_t slPosition = lseek(m_siFileDescriptor,0,SEEK_CUR);
        if (slPosition < 0)
        {
            char acExceptionMessage[512];
            snprintf(
                acExceptionMessage,
                sizeof(acExceptionMessage),
                "Error getting position for file %s: %s (%d).",
                m_kFilePath.c_str(),
                strerror(errno),
                errno);
            throw std::file_seek_error(
                errno,
                std::string(acExceptionMessage));
        }
        assert(0 == errno);
        assert(slPosition >= 0);
        return static_cast<uint64_t>(slPosition);
    }
    void SetFilePosition(const uint64_t ulPosition) noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Seek to the requested file position.
        assert(m_siFileDescriptor >= 0);
        assert(ulPosition <= std::numeric_limits<off_t>::max());
        if (lseek(m_siFileDescriptor,static_cast<off_t>(ulPosition),SEEK_SET) < 0)
        {
            char acExceptionMessage[512];
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

        // Seek from the beginning of the file.
        assert(m_siFileDescriptor >= 0);
        if (lseek(m_siFileDescriptor,slOffset,SEEK_SET) < 0)
        {
            char acExceptionMessage[512];
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
        assert(0 == errno);
    }
    void SeekCurrent(const sint64_t slOffset) const noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Seek from the current file position.
        assert(m_siFileDescriptor >= 0);
        if (lseek(m_siFileDescriptor,slOffset,SEEK_CUR) < 0)
        {
            char acExceptionMessage[512];
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
        assert(0 == errno);
    }
    void SeekEnd(const sint64_t slOffset) const noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;
        
        // Seek from the end of the file.
        assert(m_siFileDescriptor >= 0);
        if (lseek(m_siFileDescriptor,slOffset,SEEK_END) < 0)
        {
            char acExceptionMessage[512];
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
        assert(0 == errno);
    }

    // File length.
    uint64_t GetFileLength() const noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Get the file status.
        struct stat kFileStatus;
        assert(m_siFileDescriptor >= 0);
        if (fstat(m_siFileDescriptor,&kFileStatus) < 0)
        {
            char acExceptionMessage[512];
            snprintf(
                acExceptionMessage,
                sizeof(acExceptionMessage),
                "Error getting the status of file %s: %s (%d).",
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
    void SetFileLength(const uint64_t ulLength) noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Set the file length.
        assert(m_siFileDescriptor >= 0);
        assert(ulLength <= std::numeric_limits<off_t>::max());
        if (ftruncate(m_siFileDescriptor,static_cast<off_t>(ulLength)) < 0)
        {
            char acExceptionMessage[512];
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
        assert(m_siFileDescriptor >= 0);
        return GetFilePosition() >= GetFileLength();
    }
    uint64_t GetBytesRemaining() const noexcept(false)
    {
        assert(m_siFileDescriptor >= 0);
        const uint64_t ulPosition = GetFilePosition();
        const uint64_t ulLength = GetFileLength();
        return ulPosition < ulLength ? ulLength-ulPosition : 0;
    }

    // Read and write bytes.
    uint64_t ReadBytes(
        void* avBuffer,
        const uint64_t ulTotalBytes) noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Attempt to read the requested number of bytes.
        assert(m_siFileDescriptor >= 0);
        assert(0 == ulTotalBytes || nullptr != avBuffer);
        assert(ulTotalBytes <= std::numeric_limits<ssize_t>::max());
        static_assert(
            sizeof(sint64_t) >= sizeof(ssize_t),
            "sint64_t is smaller than ssize_t");
        static_assert(
            std::numeric_limits<sint64_t>::is_signed == std::numeric_limits<ssize_t>::is_signed,
            "sint64_t signedness does not match ssize_t");
        static_assert(
            std::numeric_limits<sint64_t>::max() >= std::numeric_limits<ssize_t>::max(),
            "sint64_t max insufficient for ssize_t");
        static_assert(
            std::numeric_limits<sint64_t>::min() <= std::numeric_limits<ssize_t>::min(),
            "sint64_t min insufficient for ssize_t");
        const sint64_t slActualByteCount = read(m_siFileDescriptor,avBuffer,ulTotalBytes);
        if (slActualByteCount < 0)
        {
            char acExceptionMessage[512];
            snprintf(
                acExceptionMessage,
                sizeof(acExceptionMessage),
                "Error reading from file %s: %s (%d).",
                m_kFilePath.c_str(),
                strerror(errno),
                errno);
            throw std::file_read_error(
                errno,
                std::string(acExceptionMessage));
        }
        assert(0 == errno);
        assert(slActualByteCount >= 0);
        return static_cast<uint64_t>(slActualByteCount);
    }
    uint64_t WriteBytes(
        const void* avBuffer,
        const uint64_t ulTotalBytes) noexcept(false)
    {
        // Clear the error number in preparation for library call(s).
        errno = 0;

        // Attempt to write the requested number of bytes.
        assert(m_siFileDescriptor >= 0);
        assert(0 == ulTotalBytes || nullptr != avBuffer);
        assert(ulTotalBytes <= std::numeric_limits<ssize_t>::max());
        static_assert(
            sizeof(sint64_t) >= sizeof(ssize_t),
            "sint64_t is smaller than ssize_t");
        static_assert(
            std::numeric_limits<sint64_t>::is_signed == std::numeric_limits<ssize_t>::is_signed,
            "sint64_t signedness does not match ssize_t");
        static_assert(
            std::numeric_limits<sint64_t>::max() >= std::numeric_limits<ssize_t>::max(),
            "sint64_t max insufficient for ssize_t");
        static_assert(
            std::numeric_limits<sint64_t>::min() <= std::numeric_limits<ssize_t>::min(),
            "sint64_t min insufficient for ssize_t");
        const sint64_t slActualByteCount = write(m_siFileDescriptor,avBuffer,ulTotalBytes);
        if (slActualByteCount < 0)
        {
            char acExceptionMessage[512];
            snprintf(
                acExceptionMessage,
                sizeof(acExceptionMessage),
                "Error writing to file %s: %s (%d).",
                m_kFilePath.c_str(),
                strerror(errno),
                errno);
            throw std::file_write_error(
                errno,
                std::string(acExceptionMessage));
        }
        assert(0 == errno);
        assert(slActualByteCount >= 0);
        return static_cast<uint64_t>(slActualByteCount);
    }

protected:

    // File path.
#if defined(ENABLE_BOOST) || defined(ENABLE_BOOST_FILESYSTEM)
    const boost::filesystem::path m_kFilePath;
#else
    const std::string m_kFilePath;
#endif

    // File descriptor.
    sint32_t m_siFileDescriptor = -1;
};

}
