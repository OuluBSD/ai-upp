/**
 * Date arithmetic operations for U++-like date/time arithmetic
 * Provides arithmetic functions similar to U++ date arithmetic functionality
 * 
 * @example
 * ```typescript
 * const date = DateClass.Today();
 * const futureDate = AddDays(date, 7); // Add 7 days
 * const diff = DaysBetween(date, futureDate); // Get days between dates
 * 
 * const time = Time.Now();
 * const futureTime = AddDuration(time, Duration.FromHours(2)); // Add 2 hours
 * ```
 */

import { DateClass } from './Date';
import { Time } from './Time';
import { Duration } from './Date';

/**
 * Adds days to a date
 * @param date The original date
 * @param days Number of days to add (can be negative)
 * @returns A new Date instance with the added days
 */
export function AddDays(date: DateClass, days: number): DateClass {
    const jsDate = date.ToDate();
    jsDate.setDate(jsDate.getDate() + days);
    return DateClass.FromDate(jsDate);
}

/**
 * Adds months to a date
 * @param date The original date
 * @param months Number of months to add (can be negative)
 * @returns A new Date instance with the added months
 */
export function AddMonths(date: DateClass, months: number): DateClass {
    const jsDate = date.ToDate();
    jsDate.setMonth(jsDate.getMonth() + months);
    return DateClass.FromDate(jsDate);
}

/**
 * Adds years to a date
 * @param date The original date
 * @param years Number of years to add (can be negative)
 * @returns A new Date instance with the added years
 */
export function AddYears(date: DateClass, years: number): DateClass {
    const jsDate = date.ToDate();
    jsDate.setFullYear(jsDate.getFullYear() + years);
    return DateClass.FromDate(jsDate);
}

/**
 * Calculates the difference in days between two dates
 * @param startDate The start date
 * @param endDate The end date
 * @returns The number of days between the dates
 */
export function DaysBetween(startDate: DateClass, endDate: DateClass): number {
    const start = startDate.ToDate().getTime();
    const end = endDate.ToDate().getTime();
    const diff = end - start;
    return Math.floor(diff / (1000 * 60 * 60 * 24));
}

/**
 * Calculates the difference in months between two dates
 * @param startDate The start date
 * @param endDate The end date
 * @returns The number of months between the dates
 */
export function MonthsBetween(startDate: DateClass, endDate: DateClass): number {
    const startYear = startDate.GetYear();
    const startMonth = startDate.GetMonth();
    const endYear = endDate.GetYear();
    const endMonth = endDate.GetMonth();
    
    return (endYear - startYear) * 12 + (endMonth - startMonth);
}

/**
 * Calculates the difference in years between two dates
 * @param startDate The start date
 * @param endDate The end date
 * @returns The number of years between the dates
 */
export function YearsBetween(startDate: DateClass, endDate: DateClass): number {
    return endDate.GetYear() - startDate.GetYear();
}

/**
 * Adds a duration to a time
 * @param time The original time
 * @param duration The duration to add
 * @returns A new Time instance with the added duration
 */
export function AddDuration(time: Time, duration: Duration): Time {
    const newTimestamp = time.GetTimestamp() + duration.GetMilliseconds();
    return Time.FromTimestamp(newTimestamp);
}

/**
 * Subtracts a duration from a time
 * @param time The original time
 * @param duration The duration to subtract
 * @returns A new Time instance with the subtracted duration
 */
export function SubtractDuration(time: Time, duration: Duration): Time {
    const newTimestamp = time.GetTimestamp() - duration.GetMilliseconds();
    return Time.FromTimestamp(newTimestamp);
}

/**
 * Calculates the difference between two times
 * @param startTime The start time
 * @param endTime The end time
 * @returns A Duration representing the time difference
 */
export function TimeDifference(startTime: Time, endTime: Time): Duration {
    const diff = endTime.GetTimestamp() - startTime.GetTimestamp();
    return new Duration(diff);
}

/**
 * Adds hours to a time
 * @param time The original time
 * @param hours Number of hours to add (can be negative)
 * @returns A new Time instance with the added hours
 */
export function AddHours(time: Time, hours: number): Time {
    return AddDuration(time, Duration.FromHours(hours));
}

/**
 * Adds minutes to a time
 * @param time The original time
 * @param minutes Number of minutes to add (can be negative)
 * @returns A new Time instance with the added minutes
 */
export function AddMinutes(time: Time, minutes: number): Time {
    return AddDuration(time, Duration.FromMinutes(minutes));
}

/**
 * Adds seconds to a time
 * @param time The original time
 * @param seconds Number of seconds to add (can be negative)
 * @returns A new Time instance with the added seconds
 */
export function AddSeconds(time: Time, seconds: number): Time {
    return AddDuration(time, Duration.FromSeconds(seconds));
}

/**
 * Adds milliseconds to a time
 * @param time The original time
 * @param milliseconds Number of milliseconds to add (can be negative)
 * @returns A new Time instance with the added milliseconds
 */
export function AddMilliseconds(time: Time, milliseconds: number): Time {
    return AddDuration(time, Duration.FromMilliseconds(milliseconds));
}

/**
 * Compares two dates
 * @param date1 First date to compare
 * @param date2 Second date to compare
 * @returns -1 if date1 is before date2, 0 if equal, 1 if date1 is after date2
 */
export function CompareDates(date1: DateClass, date2: DateClass): number {
    if (date1.IsBefore(date2)) return -1;
    if (date1.IsAfter(date2)) return 1;
    return 0;
}

/**
 * Compares two times
 * @param time1 First time to compare
 * @param time2 Second time to compare
 * @returns -1 if time1 is before time2, 0 if equal, 1 if time1 is after time2
 */
export function CompareTimes(time1: Time, time2: Time): number {
    if (time1.IsBefore(time2)) return -1;
    if (time1.IsAfter(time2)) return 1;
    return 0;
}

/**
 * Gets the start of the month for a given date
 * @param date The input date
 * @returns A Date instance representing the first day of the month
 */
export function StartOfMonth(date: DateClass): DateClass {
    const year = date.GetYear();
    const month = date.GetMonth();
    return new DateClass(year, month, 1);
}

/**
 * Gets the end of the month for a given date
 * @param date The input date
 * @returns A Date instance representing the last day of the month
 */
export function EndOfMonth(date: DateClass): DateClass {
    const year = date.GetYear();
    const month = date.GetMonth();
    // Setting date to 0 gives the last day of the previous month
    // So setting month to month+1 and date to 0 gives the last day of current month
    const jsDate = new Date(year, month, 0);
    return DateClass.FromDate(jsDate);
}

/**
 * Gets the start of the year for a given date
 * @param date The input date
 * @returns A Date instance representing the first day of the year
 */
export function StartOfYear(date: DateClass): DateClass {
    const year = date.GetYear();
    return new DateClass(year, 1, 1);
}

/**
 * Gets the end of the year for a given date
 * @param date The input date
 * @returns A Date instance representing the last day of the year
 */
export function EndOfYear(date: DateClass): DateClass {
    const year = date.GetYear();
    return new DateClass(year, 12, 31);
}

/**
 * Gets the number of days in a given month
 * @param year The year
 * @param month The month (1-12)
 * @returns The number of days in the month
 */
export function DaysInMonth(year: number, month: number): number {
    // Month in JavaScript Date is 0-11, so pass month-1 to get the next month
    // Setting date to 0 gives the last day of the previous month
    return new Date(year, month, 0).getDate();
}

/**
 * Checks if a year is a leap year
 * @param year The year to check
 * @returns true if the year is a leap year, false otherwise
 */
export function IsLeapYear(year: number): boolean {
    return (year % 4 === 0 && year % 100 !== 0) || (year % 400 === 0);
}