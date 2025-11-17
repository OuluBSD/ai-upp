# U++ to STL Mapping: Core Package Time/Date

This document provides comprehensive mapping between U++ Core package time/date types and their STL/C++11 equivalents.

## 1. Date ↔ std::chrono::year_month_day (C++20) or custom implementation

### U++ Declaration
```cpp
class Nuller;  // Forward declaration

struct Date : RelOps< Date, Moveable<Date> > {
    byte   day;                                     // Day of month (1-31)
    byte   month;                                   // Month (1-12)
    int16  year;                                    // Year

    void     Serialize(Stream& s);                 // Serialization

    bool     IsValid() const;                       // Check if date is valid
    void     Set(int scalar);                       // Set from scalar day count
    int      Get() const;                           // Get scalar day count

    static Date Low();                              // Get lowest representable date
    static Date High();                             // Get highest representable date

    int      Compare(Date b) const;                 // Compare with another date

    Date& operator++();                             // Pre-increment (next day)
    Date& operator--();                             // Pre-decrement (previous day)
    Date  operator++(int);                          // Post-increment
    Date  operator--(int);                          // Post-decrement

    Date();                                         // Default constructor
    Date(const Nuller&);                            // Constructor from null
    Date(int y, int m, int d);                      // Constructor from year/month/day
};

inline hash_t GetHashValue(Date t);                 // Hash function
inline bool operator==(Date a, Date b);             // Equality operator
template<> inline bool  IsNull(const Date& d);      // Null check

bool operator<(Date a, Date b);                     // Less-than operator

int   operator-(Date a, Date b);                    // Date difference in days
Date  operator+(Date a, int b);                     // Add days to date
Date  operator+(int a, Date b);                     // Add days to date (reversed)
Date  operator-(Date a, int b);                     // Subtract days from date
Date& operator+=(Date& a, int b);                   // Add days in-place
Date& operator-=(Date& a, int b);                   // Subtract days in-place

bool IsLeapYear(int year);                          // Check if leap year
int  GetDaysOfMonth(int month, int year);           // Get days in month
int  DayOfWeek(Date date);                          // Get day of week (0=Sunday)
Date LastDayOfMonth(Date d);                        // Get last day of month
Date FirstDayOfMonth(Date d);                       // Get first day of month
Date LastDayOfYear(Date d);                         // Get last day of year
Date FirstDayOfYear(Date d);                        // Get first day of year
int  DayOfYear(Date d);                             // Get day of year (1-365/366)

Date AddMonths(Date date, int months);              // Add months to date
int  GetMonths(Date since, Date till);              // Get months between dates
int  GetMonthsP(Date since, Date till);             // Get months between dates (partial)
Date AddYears(Date date, int years);                // Add years to date

Date GetWeekDate(int year, int week);               // Get date from year/week
int  GetWeek(Date d, int& year);                    // Get week number

Date EasterDay(int year);                           // Get Easter date

Date GetSysDate();                                  // Get current system date

String DayName(int i, int lang = 0);                // Get day name
String DyName(int i, int lang = 0);                 // Get day short name
String MonthName(int i, int lang = 0);              // Get month name
String MonName(int i, int lang = 0);                // Get month short name

void   SetDateFormat(const char *fmt);              // Set format string
void   SetDateScan(const char *scan);               // Set scan format
void   SetDateFilter(const char *seps);             // Set separators
String GetDateFormat();                              // Get format string

const char *StrToDate(const char *fmt, Date& d, const char *s, Date def = Null); // Parse with format
const char *StrToDate(Date& d, const char *s, Date def); // Parse with default
const char *StrToDate(Date& d, const char *s);      // Parse
Date        ScanDate(const char *fmt, const char *s, Date def = Null); // Scan with format
Date        ScanDate(const char *s, Date def = Null); // Scan
String      Format(Date date, const char *fmt);      // Format with custom format
String      Format(Date date);                      // Format with default format
int         CharFilterDate(int c);                  // Date character filter

template<> inline String AsString(const Date& date); // String conversion
```

### STL Equivalent
```cpp
#include <chrono>
#include <format>  // C++20

// For C++20 and later:
namespace std::chrono {
    class year_month_day {  // Represents a date
        chrono::year y;      // Year
        chrono::month m;     // Month
        chrono::day d;       // Day
        
    public:
        constexpr year_month_day(const chrono::year& y, const chrono::month& m, const chrono::day& d) noexcept;
        constexpr chrono::year year() const noexcept;
        constexpr chrono::month month() const noexcept;
        constexpr chrono::day day() const noexcept;
        // ... other methods
        
        constexpr bool ok() const noexcept;          // Check if valid
    };
    
    // Date arithmetic
    constexpr year_month_day operator+(const year_month_day& ymd, const months& dm) noexcept;
    constexpr year_month_day operator+(const months& dm, const year_month_day& ymd) noexcept;
    constexpr year_month_day operator-(const year_month_day& ymd, const months& dm) noexcept;
    constexpr year_month_day operator+(const year_month_day& ymd, const years& dy) noexcept;
    constexpr year_month_day operator-(const year_month_day& ymd, const years& dy) noexcept;
    
    // Difference operations
    days operator-(const year_month_day& x, const year_month_day& y) noexcept;
}

// For pre-C++20, a custom implementation might be needed:
#include <chrono>

struct Date {
    int year;   // Year
    int month;  // Month (1-12)
    int day;    // Day (1-31)
    
    Date() = default;
    Date(int y, int m, int d);
    
    bool operator==(const Date& other) const;
    bool operator<(const Date& other) const;
    
    // Arithmetic operations
    Date& operator+=(int days);
    Date& operator-=(int days);
    
    // Convert to system_clock time_point for calculations
    std::chrono::system_clock::time_point to_time_point() const;
    static Date from_time_point(const std::chrono::system_clock::time_point& tp);
};

// Utilities for date operations
bool is_leap(int year);
int days_in_month(int month, int year);
int day_of_week(const Date& d);  // 0=Sunday, 1=Monday, etc.
int day_of_year(const Date& d);
Date operator+(const Date& d, int days);
Date operator-(const Date& d, int days);
int operator-(const Date& a, const Date& b);
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| Date | std::chrono::year_month_day (C++20) | ✓ Complete | |
| Date(year, month, day) | year_month_day(year, month, day) | ✓ Complete | |
| Date::IsValid() | year_month_day::ok() | ✓ Complete | |
| Date::Get() | Convert to time_point and use duration | ⚠️ Complex | |
| Date::Set() | From time_point and convert back | ⚠️ Complex | |
| operator+(Date, int) | operator+(year_month_day, days) | ✓ Complete | |
| operator-(Date, int) | operator-(year_month_day, days) | ✓ Complete | |
| operator-(Date, Date) | operator-(year_month_day, year_month_day) | ✓ Complete | Returns days |
| Date::operator++() | Add 1 day using std::chrono operations | ✓ Complete | |
| Date::operator--() | Subtract 1 day using std::chrono operations | ✓ Complete | |
| GetSysDate() | std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now()) | ✓ Complete | |
| AddMonths(Date, months) | std::chrono::operator+(year_month_day, months) | ✓ Complete | |
| AddYears(Date, years) | std::chrono::operator+(year_month_day, years) | ✓ Complete | |
| IsLeapYear(year) | std::chrono::year::is_leap() (C++23) or custom implementation | ⚠️ Complex | |
| GetDaysOfMonth(month, year) | Custom implementation or std::chrono::year_month::days() | ⚠️ Complex | |
| DayOfWeek(Date) | Use std::chrono::weekday | ✓ Complete | |
| Format(Date) | std::format (C++20) or strftime | ⚠️ Complex | |

### Conversion Notes
- U++ Date maps closely to std::chrono::year_month_day in C++20
- Pre-C++20 requires custom implementation or use of std::tm with C functions
- U++ Date provides many convenience functions that need to be reimplemented using std::chrono
- Date arithmetic is more natural with std::chrono::duration types
- Formatting and parsing require different approaches between U++ and STL

## 2. Time ↔ std::chrono::time_point or std::tm

### U++ Declaration
```cpp
struct Time : Date, RelOps< Time, Moveable<Time> > {
    byte   hour;                                    // Hour (0-23)
    byte   minute;                                  // Minute (0-59)
    byte   second;                                  // Second (0-59)

    void     Serialize(Stream& s);                 // Serialization

    static Time High();                             // Get highest representable time
    static Time Low();                              // Get lowest representable time

    void   Set(int64 scalar);                      // Set from scalar seconds
    int64  Get() const;                            // Get scalar seconds
    bool   IsValid() const;                        // Check validity

    int    Compare(Time b) const;                  // Compare with another time

    Time();                                         // Default constructor
    Time(const Nuller&);                           // Constructor from null
    Time(int y, int m, int d, int h = 0, int n = 0, int s = 0); // Full constructor

    Time(FileTime filetime);                       // Constructor from Windows FILETIME
    FileTime AsFileTime() const;                   // Convert to Windows FILETIME
};

inline Time ToTime(const Date& d);                 // Convert Date to Time

inline hash_t GetHashValue(Time t);                // Hash function
template<> inline bool  IsNull(const Time& t);     // Null check

bool operator==(Time a, Time b);                   // Equality
bool operator<(Time a, Time b);                    // Less than

int64  operator-(Time a, Time b);                  // Time difference in seconds
Time   operator+(Time a, int64 seconds);           // Add seconds to time
Time   operator+(int64 seconds, Time a);           // Add seconds to time (reversed)
Time   operator-(Time a, int64 secs);              // Subtract seconds from time
Time&  operator+=(Time& a, int64 secs);            // Add seconds in-place
Time&  operator-=(Time& a, int64 secs);            // Subtract seconds in-place

inline Time   operator+(Time a, int i);            // Add seconds (int overload)
inline Time   operator-(Time a, int i);            // Subtract seconds (int overload)
inline Time&  operator+=(Time& a, int i);          // Add seconds in-place (int overload)
inline Time&  operator-=(Time& a, int i);          // Subtract seconds in-place (int overload)

inline Time   operator+(Time a, double i);         // Add seconds (double overload)
inline Time   operator-(Time a, double i);         // Subtract seconds (double overload)
inline Time&  operator+=(Time& a, double i);       // Add seconds in-place (double overload)
inline Time&  operator-=(Time& a, double i);       // Subtract seconds in-place (double overload)

Time  GetSysTime();                                // Get current system time
Time  GetUtcTime();                                // Get current UTC time

String Format(Time time, bool seconds = true);     // Format time
const char *StrToTime(const char *datefmt, Time& d, const char *s); // Parse with format
const char *StrToTime(Time& d, const char *s);     // Parse
Time        ScanTime(const char *datefmt, const char *s, Time def = Null); // Scan with format
Time        ScanTime(const char *s, Time def = Null); // Scan

template<> inline String AsString(const Time& time); // String conversion

bool SetSysTime(Time time);                        // Set system time (admin required)

int    GetTimeZone();                              // Get timezone offset
String GetTimeZoneText();                          // Get timezone text
int    ScanTimeZoneText(const char *s);            // Parse timezone text
int    ScanTimeZone(const char *s);                // Parse timezone
int    GetLeapSeconds(Date dt);                    // Get leap seconds
int64 GetUTCSeconds(Time tm);                      // Get UTC seconds
Time  TimeFromUTC(int64 seconds);                  // Convert UTC seconds to local time
```

### STL Equivalent
```cpp
#include <chrono>
#include <ctime>

// The std::chrono way (recommended):
namespace std::chrono {
    using time_point = time_point<system_clock, duration<int64_t, ratio<1>>>;
    
    // Create from components:
    time_point<system_clock> make_time_point(int year, int month, int day, int hour, int minute, int second);
    
    // Extract components:
    year_month_day get_ymd(const time_point<system_clock>& tp);
    hours get_hour(const time_point<system_clock>& tp);
    minutes get_minute(const time_point<system_clock>& tp);
    seconds get_second(const time_point<system_clock>& tp);
}

// Alternative using C-style struct:
struct std::tm {
    int tm_sec;   // seconds after the minute [0-61]
    int tm_min;   // minutes after the hour [0-59]
    int tm_hour;  // hours since midnight [0-23]
    int tm_mday;  // day of the month [1-31]
    int tm_mon;   // months since January [0-11]
    int tm_year;  // years since 1900
    int tm_wday;  // days since Sunday [0-6]
    int tm_yday;  // days since January 1 [0-365]
    int tm_isdst; // Daylight Saving Time flag
};

// Or custom structure similar to U++ Time:
struct Time {
    int year, month, day, hour, minute, second;
    
    Time() = default;
    Time(int y, int m, int d, int h = 0, int n = 0, int s = 0);
    
    // Convert to std::chrono::time_point for operations
    std::chrono::system_clock::time_point to_time_point() const;
    static Time from_time_point(const std::chrono::system_clock::time_point& tp);
};
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| Time | std::chrono::time_point<system_clock> or custom struct | ✓ Complete | |
| Time(year, month, day, hour, minute, second) | time_point from components or tm struct | ✓ Complete | |
| Time::Get() | time_point::time_since_epoch().count() | ✓ Complete | |
| Time::Set() | From time_point with time_since_epoch | ✓ Complete | |
| operator+(Time, seconds) | operator+(time_point, seconds) | ✓ Complete | |
| operator-(Time, seconds) | operator-(time_point, seconds) | ✓ Complete | |
| operator-(Time, Time) | operator-(time_point, time_point) | ✓ Complete | Returns duration |
| GetSysTime() | std::chrono::system_clock::now() | ✓ Complete | |
| GetUtcTime() | Implementation with timezones | ⚠️ Complex | |
| Format(Time) | std::format() (C++20) or strftime() | ⚠️ Complex | |
| Time zones | std::chrono::zoned_time (C++20) | ⚠️ Complex | |

### Conversion Notes
- U++ Time combines date and time components in a single struct
- STL typically separates date and time handling or uses std::chrono::time_point
- std::chrono::time_point with system_clock is the closest equivalent
- Time zone handling is more sophisticated in std::chrono (C++20) with zoned_time
- Parsing and formatting require different approaches between U++ and STL

## 3. Time/Date utility functions ↔ std::chrono utilities

### U++ Declaration
```cpp
// Time/Date utility functions (many already listed above)
// Additional date-specific utilities:
Date LastDayOfMonth(Date d);                       // Get last day of month
Date FirstDayOfMonth(Date d);                      // Get first day of month
Date LastDayOfYear(Date d);                        // Get last day of year
Date FirstDayOfYear(Date d);                       // Get first day of year
int  DayOfYear(Date d);                            // Get day of year (1-365/366)
Date AddMonths(Date date, int months);             // Add months to date
int  GetMonths(Date since, Date till);             // Get months between dates
int  GetMonthsP(Date since, Date till);            // Get months between dates (partial)
Date AddYears(Date date, int years);               // Add years to date
Date GetWeekDate(int year, int week);              // Get date from year/week
int  GetWeek(Date d, int& year);                   // Get week number
Date EasterDay(int year);                          // Get Easter date

// Time zone and UTC functions:
int    GetTimeZone();                              // Get timezone offset
String GetTimeZoneText();                          // Get timezone text
int    ScanTimeZoneText(const char *s);            // Parse timezone text
int    ScanTimeZone(const char *s);                // Parse timezone
int    GetLeapSeconds(Date dt);                    // Get leap seconds
int64 GetUTCSeconds(Time tm);                      // Get UTC seconds
Time  TimeFromUTC(int64 seconds);                  // Convert UTC seconds to local time
```

### STL Equivalent
```cpp
#include <chrono>

// Date calculations with std::chrono (C++20):
namespace std::chrono {
    // Month/Year operations
    year_month_day operator+(const year_month_day& ymd, const months& dm);
    year_month_day operator-(const year_month_day& ymd, const months& dm);
    year_month_day operator+(const year_month_day& ymd, const years& dy);
    year_month_day operator-(const year_month_day& ymd, const years& dy);
    
    // Get last day of month
    constexpr year_month_day_last last_day_of_month(const year_month& ym) noexcept;
    
    // Get first day of month
    constexpr year_month_day first_day_of_month(const year_month_day& ymd) noexcept;
    
    // Day of year calculation
    constexpr int day_of_year(const year_month_day& ymd) noexcept;
    
    // Week calculations (C++23)
    constexpr year_weeknum_weekday week_num(const local_days& ld) noexcept;
}

// For time zones (C++20):
namespace std::chrono::tz {
    zoned_time current_zone();
    zoned_time utc_to_local(const sys_time<Duration>& st);
    sys_time<Duration> local_to_utc(const local_time<Duration>& lt, const time_zone* zone);
}

// Leap seconds (C++23):
namespace std::chrono {
    struct leap_second_info {
        sys_seconds leap_second;
        seconds adjustment;
    };
    
    const leap_second_info* get_leap_second_info(sys_seconds tp);
}
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| LastDayOfMonth(Date) | year_month_day_last (C++20) | ✓ Complete | |
| FirstDayOfMonth(Date) | First day would be {year, month, day{1}} | ✓ Complete | |
| LastDayOfYear(Date) | {year, December, year_month_day_last{December}} | ⚠️ Complex | |
| FirstDayOfYear(Date) | {year, January, day{1}} | ✓ Complete | |
| DayOfYear(Date) | chrono::day_of_year (C++20) | ✓ Complete | |
| AddMonths(Date, months) | operator+(year_month_day, months) | ✓ Complete | |
| AddYears(Date, years) | operator+(year_month_day, years) | ✓ Complete | |
| GetWeek(Date) | Not directly available, custom implementation needed | ⚠️ Complex | |
| GetSysTime() | std::chrono::system_clock::now() | ✓ Complete | |
| GetUtcTime() | std::chrono::utc_clock::now() (C++20) | ✓ Complete | |
| Time zone functions | std::chrono::time_zone (C++20) | ⚠️ Complex | |
| Leap seconds | std::chrono::get_leap_second_info (C++23) | ⚠️ Complex | |

### Conversion Notes
- C++20 provides comprehensive calendar functions that map directly to many U++ utilities
- Time zone handling is more sophisticated in modern std::chrono
- Week calculations require custom implementation in pre-C++23
- U++ provides specialized functions like EasterDay that need custom implementation in STL
- Date arithmetic is more intuitive with std::chrono duration types

## Summary of Time/Date Mappings

| U++ Time/Date Type | STL Equivalent | Notes |
|-------------------|----------------|-------|
| Date | std::chrono::year_month_day (C++20) | Direct mapping with C++20 |
| Time | std::chrono::time_point<system_clock> | More natural with std::chrono |
| Time/Date arithmetic | std::chrono::operator+/- with durations | More type-safe with std::chrono |
| Formatting/Parsing | std::format() (C++20) or strftime | Different approach from U++ |
| Time zones | std::chrono::zoned_time (C++20) | More comprehensive in STL |
| Calendar utilities | std::chrono::operators (C++20) | Many utilities available in C++20 |