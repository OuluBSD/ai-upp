/**
 * DateTime Patterns Documentation
 * 
 * This document describes the DateTime functionality implemented in the uppts library,
 * which provides U++-like functionality for date and time operations in TypeScript/JavaScript.
 * 
 * The DateTime module includes classes and utilities for:
 * 1. Time and Date representations
 * 2. Time zones and time zone conversions
 * 3. Formatting and parsing date/time values
 * 4. Date/time arithmetic operations
 * 5. Timing utilities and performance measurement
 * 6. Timer and scheduling capabilities
 * 
 * Each class is fully documented with TypeDoc comments in the source code.
 */

// ================
// 1. TIME AND DATE CLASSES
// ================

/**
 * Time class:
 * Represents a specific moment in time with precision down to milliseconds.
 * Provides methods for getting and setting time components, comparisons,
 * and formatting.
 * 
 * Usage:
 * ```typescript
 * const time = new Time(2023, 10, 15, 14, 30, 45, 123); // Year, Month, Day, Hour, Minute, Second, Millisecond
 * console.log(time.GetYear()); // 2023
 * console.log(time.Format('%Y-%m-%d %H:%M:%S')); // '2023-10-15 14:30:45'
 * 
 * const now = Time.Now();
 * ```
 * 
 * Key methods:
 * - GetYear(), GetMonth(), GetDay(), GetHour(), GetMinute(), GetSecond(), GetMillisecond()
 * - Format(formatString): Formats the time according to the format string
 * - IsBefore(other), IsAfter(other), IsEqual(other): Comparison methods
 * - GetTimestamp(): Gets milliseconds since Unix epoch
 * 
 * Static methods:
 * - Now(): Gets current time
 * - FromTimestamp(timestamp): Creates from timestamp
 * - FromISOString(isoString): Creates from ISO string
 */

/**
 * DateClass (aliased as Date):
 * Represents a calendar date (year, month, day) without time component.
 * Provides methods for date manipulations and formatting.
 * 
 * Usage:
 * ```typescript
 * const date = new DateClass(2023, 10, 15); // Year, Month, Day
 * console.log(date.GetYear()); // 2023
 * 
 * const today = DateClass.Today();
 * ```
 * 
 * Key methods:
 * - GetYear(), GetMonth(), GetDay()
 * - Format(formatString): Formats the date according to the format string
 * - IsBefore(other), IsAfter(other), IsEqual(other): Comparison methods
 * 
 * Static methods:
 * - Today(): Gets current date
 * - FromDate(date): Creates from JavaScript Date
 * - FromISOString(isoString): Creates from ISO string
 */

/**
 * TimePoint and Duration:
 * TimePoint represents a point in time with nanosecond precision.
 * Duration represents a time interval.
 * 
 * Usage:
 * ```typescript
 * const timePoint = TimePoint.Now();
 * 
 * const duration = Duration.FromSeconds(3661); // 1h 1m 1s
 * console.log(FormatDuration(duration)); // Human-readable format
 * ```
 */

// ================
// 2. TIME ZONE HANDLING
// ================

/**
 * TimeZone class:
 * Provides time zone operations and conversions between time zones.
 * 
 * Usage:
 * ```typescript
 * const utcTz = TimeZone.GetUtc();
 * const localTz = TimeZone.GetLocal();
 * const nyTz = TimeZone.FromName('America/New_York');
 * ```
 * 
 * Key methods:
 * - GetName(): Gets the time zone name
 * - GetUtcOffset(date): Gets UTC offset in minutes
 * - UtcToLocal(utcDate): Converts UTC to local time
 * - LocalToUtc(localDate): Converts local time to UTC
 * 
 * Static methods:
 * - GetLocal(): Gets system's local time zone
 * - GetUtc(): Gets UTC time zone
 * - FromName(name): Gets time zone by IANA name
 */

/**
 * TimeZonedTime class:
 * Combines a time with a specific time zone.
 * 
 * Usage:
 * ```typescript
 * const tzTime = new TimeZonedTime(someDate, someTimeZone);
 * const utcTime = tzTime.ToUtc();
 * ```
 */

// ================
// 3. FORMATTING AND PARSING
// ================

/**
 * Formatting functions:
 * - FormatTime(time, format): Formats a Time object
 * - FormatDate(date, format): Formats a Date object  
 * - FormatDuration(duration): Formats a Duration in human-readable format
 * - FormatTimeInterval(startTime, endTime): Formats the interval between times
 * - FormatRelativeTime(time): Formats time as relative (e.g., "2 hours ago")
 * - ParseDateTime(dateString, format): Parses a date/time string
 * 
 * Format specifiers supported:
 * - %Y: Year (4 digits)
 * - %m: Month (01-12)
 * - %d: Day (01-31)
 * - %H: Hour (00-23)
 * - %M: Minute (00-59)
 * - %S: Second (00-59)
 * - %f: Millisecond (000-999)
 */

// ================
// 4. DATE ARITHMETIC
// ================

/**
 * Date arithmetic functions:
 * - AddDays(date, days): Adds days to a date
 * - AddMonths(date, months): Adds months to a date
 * - AddYears(date, years): Adds years to a date
 * - DaysBetween(startDate, endDate): Calculates days between dates
 * - MonthsBetween(startDate, endDate): Calculates months between dates
 * - AddDuration(time, duration): Adds duration to a time
 * - TimeDifference(startTime, endTime): Calculates duration between times
 * - IsLeapYear(year): Checks if year is leap year
 * - DaysInMonth(year, month): Gets number of days in month
 * 
 * Usage:
 * ```typescript
 * const startDate = DateClass.Today();
 * const futureDate = AddDays(startDate, 7); // 7 days from now
 * const diff = DaysBetween(startDate, futureDate); // Should be 7
 * 
 * const now = Time.Now();
 * const later = AddDuration(now, Duration.FromHours(2)); // 2 hours from now
 * ```
 */

// ================
// 5. TIMERS AND SCHEDULING
// ================

/**
 * Timer classes:
 * - Timer: Executes callback once after specified delay
 * - PeriodicTimer: Executes callback repeatedly at specified intervals
 * - ScheduledTask: Represents a scheduled task that can be cancelled
 * 
 * Usage:
 * ```typescript
 * const timer = new Timer(() => console.log('Executed!'), 1000); // 1 second
 * timer.Start();
 * 
 * const periodicTimer = new PeriodicTimer(() => console.log('Periodic'), 2000); // 2 seconds
 * periodicTimer.Start();
 * 
 * const task = ScheduleTask(() => console.log('Scheduled'), 5000); // 5 seconds
 * task.Cancel(); // Cancel if needed
 * ```
 */

/**
 * Additional scheduling functions:
 * - ExecuteAfter(task, duration): Executes task after duration
 * - ExecuteAtInterval(task, interval): Executes task at interval
 * - Timeout: Executes callback if not cancelled within timeout
 */

// ================
// 6. PERFORMANCE TIMING
// ================

/**
 * Performance timing utilities:
 * - GetTickCount(): Gets milliseconds since system start
 * - GetPerformanceCounter(): Gets high-resolution timestamp
 * - PerformanceTimer: Measures elapsed time
 * - Stopwatch: Stopwatch functionality
 * - MeasureTime(fn): Measures function execution time
 * - MeasureTimeAsync(fn): Measures async function execution time
 * 
 * Usage:
 * ```typescript
 * const start = GetPerformanceCounter();
 * // ... do some work ...
 * const end = GetPerformanceCounter();
 * const elapsedMs = CounterToMilliseconds(end.GetTimestamp() - start.GetTimestamp());
 * 
 * const timer = new PerformanceTimer();
 * timer.Start();
 * // ... do some work ...
 * const timeMs = timer.ElapsedMilliseconds();
 * 
 * const perfResult = MeasureTime(() => {
 *   // Function to measure
 *   return someOperation();
 * });
 * console.log(`Operation took ${perfResult.timeMs}ms`);
 * ```
 */

// ================
// BEST PRACTICES
// ================

/**
 * 1. Use the appropriate class for your needs:
 * - Use Time for moments in time with time components
 * - Use DateClass for calendar dates without time
 * - Use Duration for time intervals
 * 
 * 2. Time zones:
 * - Be aware of time zone differences when working with global applications
 * - Use TimeZonedTime when working with specific time zones
 * 
 * 3. Formatting:
 * - Use appropriate format strings for your locale and requirements
 * - Consider using ISO formats for data interchange
 * 
 * 4. Performance:
 * - Use PerformanceTimer or Stopwatch for accurate timing measurements
 * - Use timers judiciously to avoid blocking the event loop
 * 
 * 5. Arithmetic:
 * - Be careful with month arithmetic as months have varying lengths
 * - Consider leap years in calculations
 */

// ================
// MIGRATION FROM U++
// ================

/**
 * Developers migrating from U++ to uppts should note:
 * 
 * 1. Date/Time:
 * - U++ Time → uppts Time (similar API)
 * - U++ Date → uppts DateClass (aliased as Date)
 * 
 * 2. Time zones:
 * - U++ Tz* classes → uppts TimeZone
 * 
 * 3. Formatting:
 * - U++ format functions → uppts Format* functions
 * 
 * 4. Timing:
 * - U++ TimeStop, TimeDiff → uppts PerformanceTimer, Stopwatch
 * - U++ GetTickCount → uppts GetTickCount (same functionality)
 * 
 * 5. Timers:
 * - U++ Timer classes → uppts Timer and PeriodicTimer
 */