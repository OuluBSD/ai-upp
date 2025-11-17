# U++ to STL Mapping: Core Package I/O System

This document provides comprehensive mapping between U++ Core package I/O system and their STL equivalents.

## 1. Stream ↔ std::iostream hierarchy

### U++ Declaration
```cpp
enum {
    STRM_ERROR   =  0x20,  // Error state flag
    STRM_READ    =  0x10,  // Read capability flag
    STRM_WRITE   =  0x08,  // Write capability flag
    STRM_SEEK    =  0x04,  // Seek capability flag
    STRM_LOADING =  0x02,  // Loading mode flag
    STRM_THROW   =  0x01,  // Throw on error flag
};

struct StreamError {};        // Base stream error type
struct LoadingError : StreamError {}; // Loading-specific error

enum EOLenum { EOL };         // End-of-line enumeration

class Stream {
protected:
    int64  pos;              // Current position
    byte  *buffer;            // Buffer pointer
    byte  *ptr;              // Current position in buffer
    byte  *rdlim;            // Read limit
    byte  *wrlim;            // Write limit

    unsigned style:6;         // Stream style flags
    unsigned errorcode:16;    // Error code

    int      version = 0;     // Serialization version

    virtual void  _Put(int w);                 // Internal put operation
    virtual int   _Term();                    // Internal terminal operation
    virtual int   _Get();                     // Internal get operation
    virtual void  _Put(const void *data, dword size); // Internal put block
    virtual dword _Get(void *data, dword size); // Internal get block

private:
    int       _Get8();        // Get 8-bit value
    int       _Get16();       // Get 16-bit value
    int       _Get32();       // Get 32-bit value
    int64     _Get64();       // Get 64-bit value

public:
    virtual void  Seek(int64 pos);            // Seek to position
    virtual int64 GetSize() const;            // Get stream size
    virtual void  SetSize(int64 size);        // Set stream size
    virtual void  Flush();                    // Flush buffer
    virtual void  Close();                    // Close stream
    virtual bool  IsOpen() const = 0;        // Check if open (pure virtual)

    Stream();                                 // Default constructor
    virtual ~Stream();                        // Virtual destructor

    word      GetStyle() const;               // Get style flags
    void      SetVersion(int ver);            // Set version
    int       GetVersion() const;             // Get version

    bool      IsError() const;                // Check error state
    bool      IsOK() const;                   // Check if OK (no error)
    void      SetError(int c = 0);            // Set error state
    int       GetError() const;               // Get error code
    String    GetErrorText() const;           // Get error text
    void      ClearError();                   // Clear error state

    int64     GetPos() const;                 // Get current position
    int64     GetLeft() const;                // Get remaining bytes
    void      SeekEnd(int64 rel = 0);         // Seek to end
    void      SeekCur(int64 rel);             // Seek from current position

    bool      IsEof();                        // Check if at end of file
    void      Put(int c);                     // Put single byte
    int       Term();                         // Get terminal character
    int       Peek();                         // Peek next character
    int       Get();                          // Get single byte

    const byte *PeekPtr(int size = 1);        // Peek pointer
    const byte *GetPtr(int size = 1);         // Get pointer
    byte       *PutPtr(int size = 1);         // Put pointer
    const byte *GetSzPtr(int& size);          // Get sized pointer

    void      Put(const void *data, int size); // Put block of data
    int       Get(void *data, int size);      // Get block of data
    void      Put(const String& s);           // Put string
    String    Get(int size);                  // Get string with size
    String    GetAll(int size);               // Get all with size

    int       Skip(int size);                 // Skip bytes

    void      LoadThrowing();                 // Enable throwing on load error
    void      LoadError();                    // Handle load error

    bool      GetAll(void *data, int size);   // Get all data

    void      Put64(const void *data, int64 size); // Put 64-bit data
    int64     Get64(void *data, int64 size);       // Get 64-bit data
    bool      GetAll64(void *data, int64 size);     // Get all 64-bit data

    size_t    Get(Huge& h, size_t size);      // Get into huge object
    bool      GetAll(Huge& h, size_t size);   // Get all into huge object

    int       Get8();                         // Get 8-bit value (byte)
    int       Get16();                        // Get 16-bit value (word)
    int       Get32();                        // Get 32-bit value (dword)
    int64     Get64();                        // Get 64-bit value (int64)

    int       GetUtf8();                      // Get UTF-8 character
    String    GetLine();                      // Get line of text

    void      Put16(word q);                  // Put 16-bit value
    void      Put32(dword q);                 // Put 32-bit value
    void      Put64(int64 q);                 // Put 64-bit value

    int       Get16le();                      // Get 16-bit little endian
    int       Get32le();                      // Get 32-bit little endian
    int64     Get64le();                      // Get 64-bit little endian
    int       Get16be();                      // Get 16-bit big endian
    int       Get32be();                      // Get 32-bit big endian
    int64     Get64be();                      // Get 64-bit big endian

    void      Put16le(word q);                // Put 16-bit little endian
    void      Put32le(dword q);               // Put 32-bit little endian
    void      Put64le(int64 q);               // Put 64-bit little endian
    void      Put16be(word q);                // Put 16-bit big endian
    void      Put32be(dword q);               // Put 32-bit big endian
    void      Put64be(int64 q);               // Put 64-bit big endian

    void      PutUtf8(int c);                 // Put UTF-8 character
    void      Put(const char *s);             // Put C-string
    void      Put(int c, int count);          // Put repeated character
    void      Put0(int count);                // Put zero bytes
    void      PutCrLf();                      // Put CRLF
    void      PutEol();                       // Put end-of-line
    Stream&   operator<<(EOLenum);            // End-of-line operator
    void      PutLine(const char *s);         // Put line
    void      PutLine(const String& s);       // Put line with string
    void      Put(Stream& s, int64 size = INT64_MAX, dword click = 4096); // Put from another stream

    // Serialization interface
    void      SetLoading();                   // Set loading mode
    void      SetStoring();                   // Set storing mode
    bool      IsLoading() const;              // Check if loading
    bool      IsStoring() const;              // Check if storing
    void      SerializeRaw(byte *data, int64 count); // Raw serialization
    // ... more serialization methods
};
```

### STL Equivalent
```cpp
#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>

// Base stream classes
class std::ios_base {
    // Configuration and status flags
    // Virtual base for iostream classes
};

class std::ios : public std::ios_base {
    std::streambuf* rdbuf() const;              // Get stream buffer
    // Base for stream classes
};

class std::istream : virtual public std::ios {
    // Input operations
    int get();                                  // Get single character
    int peek();                                 // Peek at next character
    std::istream& get(char& c);                 // Get character into variable
    std::istream& get(char* s, std::streamsize n); // Get into C-string
    std::istream& read(char* s, std::streamsize n); // Read binary data
    std::streampos tellg();                     // Get get position
    std::istream& seekg(std::streampos pos);    // Seek to position
    std::istream& seekg(std::streamoff off, std::ios_base::seekdir dir); // Seek with direction
    std::istream& ignore(std::streamsize count, int delim); // Ignore up to delimiter
    bool eof() const;                           // Check if at end
    bool good() const;                          // Check if good state
    bool fail() const;                          // Check if fail state
    bool bad() const;                           // Check if bad state
};

class std::ostream : virtual public std::ios {
    // Output operations
    std::ostream& put(char c);                  // Put single character
    std::ostream& write(const char* s, std::streamsize n); // Write binary data
    std::streampos tellp();                     // Get put position
    std::ostream& seekp(std::streampos pos);    // Seek put position
    std::ostream& seekp(std::streamoff off, std::ios_base::seekdir dir); // Seek with direction
};

class std::iostream : public std::istream, public std::ostream {
    // Combined input and output operations
};
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| Stream | std::iostream (base) | ✓ Complete | U++ Stream is similar to std::iostream |
| Stream::Put(c) | std::ostream::put(c) | ✓ Complete | |
| Stream::Get() | std::istream::get() | ✓ Complete | |
| Stream::Peek() | std::istream::peek() | ✓ Complete | |
| Stream::Seek(pos) | std::iostream::seekg(pos) and seekp(pos) | ✓ Complete | |
| Stream::GetSize() | stream.rdbuf()->pubseekoff(0, std::ios_base::end) | ⚠️ Complex | Implementation needed |
| Stream::GetPos() | std::iostream::tellg() or tellp() | ✓ Complete | |
| Stream::IsEof() | std::iostream::eof() | ✓ Complete | |
| Stream::IsError() | std::iostream::fail() or bad() | ✓ Complete | |
| Stream::Put(const void *data, int size) | std::ostream::write(const char*, size) | ✓ Complete | |
| Stream::Get(void *data, int size) | std::istream::read(char*, size) | ✓ Complete | |
| Stream::Flush() | std::ostream::flush() | ✓ Complete | |

### Conversion Notes
- U++ Stream provides both input and output operations in a single class, similar to std::iostream
- U++ has built-in buffering and a more complex internal structure compared to STL streams
- U++ provides convenience methods for data types (Get16, Put32, etc.) that STL doesn't have built-in
- U++ serialization methods map to manual serialization in STL using the streaming operators

## 2. StringStream ↔ std::stringstream

### U++ Declaration
```cpp
class StringStream : public Stream {
protected:
    virtual void  _Put(int w);                    // Internal put
    virtual int   _Term();                        // Internal terminal
    virtual int   _Get();                         // Internal get
    virtual void  _Put(const void *data, dword size); // Internal put block
    virtual dword _Get(void *data, dword size);   // Internal get block

public:
    virtual void  Seek(int64 pos);                // Seek to position
    virtual int64 GetSize() const;                // Get size
    virtual void  SetSize(int64 size);            // Set size
    virtual bool  IsOpen() const;                 // Check if open
    virtual void  Close();                        // Close stream

protected:
    bool           writemode;                     // Write mode flag
    String         data;                          // Data in read mode
    StringBuffer   wdata;                         // Buffer in write mode
    dword          size;                          // Current size

public:
    void        Open(const String& data);        // Open for reading
    void        Create();                        // Create for writing
    void        Reserve(int n);                  // Reserve space

    String      GetResult();                     // Get resulting string
    operator    String();                        // Convert to String

    struct LimitExc : public StreamError {};     // Size limit exception

    StringStream();                               // Default constructor
    StringStream(const String& data);            // Constructor with initial data
};
```

### STL Equivalent
```cpp
#include <sstream>
class std::stringstream : public std::iostream {
    std::stringbuf* rdbuf() const;                // Get string buffer

public:
    std::stringstream();                          // Default constructor
    explicit std::stringstream(std::ios_base::openmode which); // Constructor with mode
    std::stringstream(const std::string& str, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out); // Constructor with string

    std::string str() const;                      // Get underlying string
    void str(const std::string& s);               // Set underlying string
};
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| StringStream | std::stringstream | ✓ Complete | Direct mapping |
| StringStream() | std::stringstream() | ✓ Complete | |
| StringStream(const String& data) | std::stringstream(const std::string& str) | ✓ Complete | |
| StringStream::Open(data) | std::stringstream::str(data) | ✓ Complete | |
| StringStream::Create() | std::stringstream construction | ✓ Complete | |
| StringStream::GetResult() | std::stringstream::str() | ✓ Complete | |
| StringStream::operator String() | std::stringstream::str() | ✓ Complete | |
| StringStream::GetSize() | std::stringstream::str().size() | ✓ Complete | |

### Conversion Notes
- StringStream maps directly to std::stringstream
- StringStream::Open() corresponds to std::stringstream::str() to set the initial content for reading
- StringStream::GetResult() corresponds to std::stringstream::str() to get the content after writing

## 3. FileStream ↔ std::fstream

### U++ Declaration
```cpp
class FileStream : public BlockStream {
protected:
    virtual void  SetStreamSize(int64 size);      // Set stream size
    virtual dword Read(int64 at, void *ptr, dword size); // Read from position
    virtual void  Write(int64 at, const void *data, dword size); // Write to position

public:
    virtual void  Close();                        // Close file
    virtual bool  IsOpen() const;                 // Check if file is open

protected:
#ifdef PLATFORM_WIN32
    HANDLE    handle;                             // File handle (Windows)
#else
    int       handle;                             // File descriptor (POSIX)
#endif

public:
    operator bool() const;                        // Boolean conversion
    FileTime  GetTime() const;                    // Get file modification time

#ifdef PLATFORM_WIN32
    void      SetTime(const FileTime& tm);        // Set file modification time
    bool      Open(const char *filename, dword mode); // Open file
    FileStream(const char *filename, dword mode); // Constructor with filename and mode
#endif

#ifdef PLATFORM_POSIX
    bool      Open(const char *filename, dword mode, mode_t acm = 0644); // Open with permissions
    FileStream(const char *filename, dword mode, mode_t acm = 0644); // Constructor with permissions
    FileStream(int std_handle);                   // Constructor from file descriptor
#endif

    FileStream();                                 // Default constructor
    ~FileStream();                                // Destructor

#ifdef PLATFORM_WIN32
    HANDLE    GetHandle() const;                  // Get Windows handle
#endif
#ifdef PLATFORM_POSIX
    int       GetHandle() const;                  // Get POSIX file descriptor
#endif
};
```

### STL Equivalent
```cpp
#include <fstream>
class std::fstream : public std::iostream {
    std::filebuf* rdbuf() const;                   // Get file buffer

public:
    std::fstream();                               // Default constructor
    explicit std::fstream(const std::string& filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out); // Constructor with filename
    explicit std::fstream(const char* filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out); // Constructor with C-string

    void open(const std::string& filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out); // Open file
    void open(const char* filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out); // Open with C-string
    bool is_open() const;                         // Check if file is open
    void close();                                 // Close file
};
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| FileStream | std::fstream | ✓ Complete | |
| FileStream::Open(filename, mode) | std::fstream::open(filename, mode) | ✓ Complete | |
| FileStream::IsOpen() | std::fstream::is_open() | ✓ Complete | |
| FileStream::Close() | std::fstream::close() | ✓ Complete | |
| FileStream(filename, mode) | std::fstream(filename, mode) | ✓ Complete | |
| FileStream() | std::fstream() | ✓ Complete | |
| operator bool() | std::fstream::is_open() | ✓ Complete | |

### Conversion Notes
- U++ FileStream maps directly to std::fstream, though with different internal implementations
- U++ uses platform-specific file handles while STL uses std::filebuf
- Both support similar file operations and modes
- U++ provides additional platform-specific functionality like GetTime/SetTime

## 4. FileIn/FileOut ↔ std::ifstream/std::ofstream

### U++ Declaration
```cpp
class FileOut : public FileStream {
public:
#ifdef PLATFORM_POSIX
    bool Open(const char *fn, mode_t acm = 0644); // Open for output with permissions
#endif
#ifdef PLATFORM_WIN32
    bool Open(const char *fn);                    // Open for output
#endif

    FileOut(const char *fn) { Open(fn); }        // Constructor opening file
    FileOut() {}                                 // Default constructor
};

class FileIn : public FileStream {
public:
    bool Open(const char *fn) { return FileStream::Open(fn, FileStream::READ); } // Open for input

    FileIn(const char *fn) { Open(fn); }         // Constructor opening file
    FileIn() {}                                  // Default constructor
};
```

### STL Equivalent
```cpp
#include <fstream>
class std::ifstream : public std::istream {
public:
    std::ifstream();                              // Default constructor
    explicit std::ifstream(const std::string& filename, std::ios_base::openmode mode = std::ios_base::in); // Constructor with filename
    explicit std::ifstream(const char* filename, std::ios_base::openmode mode = std::ios_base::in); // Constructor with C-string

    void open(const std::string& filename, std::ios_base::openmode mode = std::ios_base::in); // Open file
    void open(const char* filename, std::ios_base::openmode mode = std::ios_base::in); // Open with C-string
    bool is_open() const;                         // Check if open
    void close();                                 // Close file
};

class std::ofstream : public std::ostream {
public:
    std::ofstream();                              // Default constructor
    explicit std::ofstream(const std::string& filename, std::ios_base::openmode mode = std::ios_base::out); // Constructor with filename
    explicit std::ofstream(const char* filename, std::ios_base::openmode mode = std::ios_base::out); // Constructor with C-string

    void open(const std::string& filename, std::ios_base::openmode mode = std::ios_base::out); // Open file
    void open(const char* filename, std::ios_base::openmode mode = std::ios_base::out); // Open with C-string
    bool is_open() const;                         // Check if open
    void close();                                 // Close file
};
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| FileIn | std::ifstream | ✓ Complete | |
| FileOut | std::ofstream | ✓ Complete | |
| FileIn(filename) | std::ifstream(filename) | ✓ Complete | |
| FileOut(filename) | std::ofstream(filename) | ✓ Complete | |
| FileIn::Open(filename) | std::ifstream::open(filename) | ✓ Complete | |
| FileOut::Open(filename) | std::ofstream::open(filename) | ✓ Complete | |
| FileIn::IsOpen() | std::ifstream::is_open() | ✓ Complete | |
| FileOut::IsOpen() | std::ofstream::is_open() | ✓ Complete | |

### Conversion Notes
- FileIn maps directly to std::ifstream for input operations
- FileOut maps directly to std::ofstream for output operations
- Both have similar constructors and open methods
- FileOut::Open on POSIX systems supports permission settings, which can be accomplished in STL using custom file operations before opening

## 5. Serialization ↔ Manual stream operations

### U++ Declaration
```cpp
// Serialization methods in Stream class:
void Stream::SerializeRaw(byte *data, int64 count); // Raw serialization
void Stream::SerializeRaw(word *data, int64 count);
void Stream::SerializeRaw(int16 *data, int64 count);
void Stream::SerializeRaw(dword *data, int64 count);
void Stream::SerializeRaw(int *data, int64 count);
void Stream::SerializeRaw(uint64 *data, int64 count);
void Stream::SerializeRaw(float *data, int64 count);
void Stream::SerializeRaw(double *data, int64 count);

Stream& Stream::operator%(bool& d);               // Serialize boolean
Stream& Stream::operator%(char& d);               // Serialize char
Stream& Stream::operator%(signed char& d);        // Serialize signed char
Stream& Stream::operator%(unsigned char& d);      // Serialize unsigned char
Stream& Stream::operator%(short& d);              // Serialize short
Stream& Stream::operator%(unsigned short& d);     // Serialize unsigned short
Stream& Stream::operator%(int& d);                // Serialize int
Stream& Stream::operator%(unsigned int& d);       // Serialize unsigned int
Stream& Stream::operator%(long& d);               // Serialize long
Stream& Stream::operator%(unsigned long& d);      // Serialize unsigned long
Stream& Stream::operator%(float& d);              // Serialize float
Stream& Stream::operator%(double& d);             // Serialize double
Stream& Stream::operator%(int64& d);              // Serialize int64
Stream& Stream::operator%(uint64& d);             // Serialize uint64
Stream& Stream::operator%(String& s);             // Serialize string
Stream& Stream::operator%(WString& s);            // Serialize wide string

// Stream loading/storing mode
void Stream::SetLoading();                        // Enter loading mode
void Stream::SetStoring();                        // Enter storing mode
bool Stream::IsLoading() const;                   // Check loading mode
bool Stream::IsStoring() const;                   // Check storing mode
```

### STL Equivalent
```cpp
#include <iostream>
#include <fstream>
#include <cstring>

// Manual serialization using stream operators:
template<typename T>
void serialize_read(std::istream& stream, T& value) {
    stream.read(reinterpret_cast<char*>(&value), sizeof(T));
}

template<typename T>
void serialize_write(std::ostream& stream, const T& value) {
    stream.write(reinterpret_cast<const char*>(&value), sizeof(T));
}

// For strings, special handling is needed:
void serialize_read(std::istream& stream, std::string& str) {
    size_t len;
    stream.read(reinterpret_cast<char*>(&len), sizeof(len));
    str.resize(len);
    if (len > 0) {
        stream.read(&str[0], len);
    }
}

void serialize_write(std::ostream& stream, const std::string& str) {
    size_t len = str.length();
    stream.write(reinterpret_cast<const char*>(&len), sizeof(len));
    if (len > 0) {
        stream.write(str.data(), len);
    }
}

// Stream operators for convenience:
template<typename T>
std::istream& operator>>(std::istream& s, T& value) {
    serialize_read(s, value);
    return s;
}

template<typename T>
std::ostream& operator<<(std::ostream& s, const T& value) {
    serialize_write(s, value);
    return s;
}
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| operator%(value) | operator<<(stream, value) or operator>>(stream, value) | ⚠️ Complex | Need custom implementation |
| SerializeRaw(data, count) | stream.read/write with reinterpret_cast | ⚠️ Complex | Need careful implementation |
| Stream::SetLoading() | Manual mode tracking | ⚠️ Complex | Need custom implementation |
| Stream::SetStoring() | Manual mode tracking | ⚠️ Complex | Need custom implementation |
| String serialization | Custom string handling | ⚠️ Complex | Need length-prefixed approach |

### Conversion Notes
- U++ provides built-in serialization with the % operator, while STL requires manual implementation
- U++ serialization automatically handles endianness for multi-byte values, while STL implementations would need to handle this manually
- U++ has built-in support for complex types like String, while STL needs custom serializers
- The U++ loading/storing mode is a higher-level concept that can be implemented on top of basic STL streams

## Summary of I/O Mappings

| U++ I/O Type | STL Equivalent | Notes |
|--------------|----------------|-------|
| Stream | std::iostream | U++ Stream is similar but with built-in serialization |
| StringStream | std::stringstream | Direct mapping |
| FileStream | std::fstream | Direct mapping |
| FileIn | std::ifstream | Direct mapping |
| FileOut | std::ofstream | Direct mapping |
| Serialization | Manual implementation | U++ provides built-in operators, STL requires custom code |