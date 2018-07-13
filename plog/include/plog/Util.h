#pragma once
#include <cassert>
#include <cstring>
#include <cstdio>
#include <sstream>
#include <fcntl.h>
#include <sys/stat.h>

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>

namespace plog
{
    namespace util
    {
        inline unsigned int gettid()
        {
            return static_cast<unsigned int>(::syscall(__NR_gettid));
        }

        inline std::string processFuncName(const char* func)
        {
            const char* funcBegin = func;
            const char* funcEnd = ::strchr(funcBegin, '(');

            if (!funcEnd)
            {
                return std::string(func);
            }

            for (const char* i = funcEnd - 1; i >= funcBegin; --i) // search backwards for the first space char
            {
                if (*i == ' ')
                {
                    funcBegin = i + 1;
                    break;
                }
            }

            return std::string(funcBegin, funcEnd);
        }

        inline const char* findExtensionDot(const char* fileName)
        {
            return std::strrchr(fileName, '.');
        }

        inline void splitFileName(const char* fileName, std::string& fileNameNoExt, std::string& fileExt)
        {
            const char* dot = findExtensionDot(fileName);

            if (dot)
            {
                fileNameNoExt.assign(fileName, dot);
                fileExt.assign(dot + 1);
            }
            else
            {
                fileNameNoExt.assign(fileName);
                fileExt.clear();
            }
        }

        class NonCopyable
        {
        protected:
            NonCopyable()
            {
            }

        private:
            NonCopyable(const NonCopyable&);
            NonCopyable& operator=(const NonCopyable&);
        };

        class File : NonCopyable
        {
        public:
            File() : m_file(-1)
            {
            }

            File(const char* fileName) : m_file(-1)
            {
                open(fileName);
            }

            ~File()
            {
                close();
            }

            off_t open(const char* fileName)
            {
                m_file = ::open(fileName, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                return seek(0, SEEK_END);
            }

            int write(const void* buf, size_t count)
            {
                return m_file != -1 ? static_cast<int>(::write(m_file, buf, count)) : -1;
            }

            template<class CharType>
            int write(const std::basic_string<CharType>& str)
            {
                return write(str.data(), str.size() * sizeof(CharType));
            }

            off_t seek(off_t offset, int whence)
            {
                return m_file != -1 ? ::lseek(m_file, offset, whence) : -1;
            }

            void close()
            {
                if (m_file != -1)
                {
                    ::close(m_file);
                    m_file = -1;
                }
            }

            static int unlink(const char* fileName)
            {
                return ::unlink(fileName);
            }

            static int rename(const char* oldFilename, const char* newFilename)
            {
                return ::rename(oldFilename, newFilename);
            }

        private:
            int m_file;
        };

        class Mutex : NonCopyable
        {
        public:
            Mutex()
            {
                ::pthread_mutex_init(&m_sync, 0);
            }

            ~Mutex()
            {
                ::pthread_mutex_destroy(&m_sync);
            }

            friend class MutexLock;

        private:
            void lock()
            {
                ::pthread_mutex_lock(&m_sync);
            }

            void unlock()
            {
                ::pthread_mutex_unlock(&m_sync);
            }

        private:
            pthread_mutex_t m_sync;
        };

        class MutexLock : NonCopyable
        {
        public:
            MutexLock(Mutex& mutex) : m_mutex(mutex)
            {
                m_mutex.lock();
            }

            ~MutexLock()
            {
                m_mutex.unlock();
            }

        private:
            Mutex& m_mutex;
        };

        template<class T>
        class Singleton : NonCopyable
        {
        public:
            Singleton()
            {
                assert(!m_instance);
                m_instance = static_cast<T*>(this);
            }

            ~Singleton()
            {
                assert(m_instance);
                m_instance = 0;
            }

            static T* getInstance()
            {
                return m_instance;
            }

        private:
            static T* m_instance;
        };

        template<class T>
        T* Singleton<T>::m_instance = NULL;
    }
}
