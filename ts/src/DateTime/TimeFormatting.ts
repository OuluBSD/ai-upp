/**
 * Time formatting functions for U++-like date/time formatting
 * Provides formatting utilities similar to U++ time formatting functionality
 * 
 * @example
 * ```typescript
 * const time = new Time.Now();
 * const formatted = FormatTime(time, '%Y-%m-%d %H:%M:%S');
 * console.log(formatted);
 * 
 * const duration = Duration.FromSeconds(3661); // 1 hour, 1 minute, 1 second
 * const durationStr = FormatDuration(duration);
 * console.log(durationStr); // "1h 1m 1s"
 * ```
 */

import { Time } from './Time';
import { Duration } from './Date';

/**
 * Formats a Time object according to the specified format string
 * Supports: %Y (year), %m (month), %d (day), %H (hour), %M (minute), %S (second), %f (millisecond)
 * @param time The Time object to format
 * @param format The format string
 * @returns The formatted time string
 */
export function FormatTime(time: Time, format: string): string {
    return time.Format(format);
}

/**
 * Formats a time duration in a human-readable format
 * @param duration The Duration object to format
 * @returns The formatted duration string
 */
export function FormatDuration(duration: Duration): string {
    const totalMilliseconds = duration.GetMilliseconds();
    
    // Calculate days, hours, minutes, seconds, milliseconds
    const days = Math.floor(totalMilliseconds / (24 * 60 * 60 * 1000));
    const hours = Math.floor((totalMilliseconds % (24 * 60 * 60 * 1000)) / (60 * 60 * 1000));
    const minutes = Math.floor((totalMilliseconds % (60 * 60 * 1000)) / (60 * 1000));
    const seconds = Math.floor((totalMilliseconds % (60 * 1000)) / 1000);
    const milliseconds = Math.floor(totalMilliseconds % 1000);
    
    const parts: string[] = [];
    
    if (days > 0) parts.push(`${days}d`);
    if (hours > 0) parts.push(`${hours}h`);
    if (minutes > 0) parts.push(`${minutes}m`);
    if (seconds > 0) parts.push(`${seconds}s`);
    if (milliseconds > 0) parts.push(`${milliseconds}ms`);
    
    return parts.length > 0 ? parts.join(' ') : '0s';
}

/**
 * Parses a date/time string according to the specified format
 * Supports: %Y (year), %m (month), %d (day), %H (hour), %M (minute), %S (second)
 * @param dateString The date/time string to parse
 * @param format The format string
 * @returns A Time object representing the parsed date/time
 */
export function ParseDateTime(dateString: string, format: string): Time {
    // This is a simplified implementation
    // A full implementation would need to properly parse the format string
    
    // For now, we'll support ISO format parsing
    if (format === '%Y-%m-%d %H:%M:%S' || format === 'ISO') {
        // Attempt to parse in YYYY-MM-DD HH:MM:SS format
        const regex = /^(\d{4})-(\d{2})-(\d{2})\s+(\d{2}):(\d{2}):(\d{2})$/;
        const match = dateString.match(regex);
        
        if (match) {
            const [, year, month, day, hour, minute, second] = match;
            return new Time(
                parseInt(year, 10),
                parseInt(month, 10),
                parseInt(day, 10),
                parseInt(hour, 10),
                parseInt(minute, 10),
                parseInt(second, 10),
                0
            );
        }
    }
    
    // Default to parsing as ISO string
    return Time.FromISOString(dateString);
}

/**
 * Formats a time interval (difference between two times) as a human-readable string
 * @param startTime The start time
 * @param endTime The end time
 * @returns A formatted string representing the time interval
 */
export function FormatTimeInterval(startTime: Time, endTime: Time): string {
    const diffMs = endTime.GetTimestamp() - startTime.GetTimestamp();
    const duration = new Duration(diffMs);
    return FormatDuration(duration);
}

/**
 * Gets a string representation of the current time in a standard format
 * @returns A formatted string for the current time
 */
export function CurrentTimeFormatted(): string {
    const now = Time.Now();
    return now.Format('%Y-%m-%d %H:%M:%S');
}

/**
 * Formats a date according to the specified format string
 * Supports: %Y (year), %m (month), %d (day)
 * @param date The date to format
 * @param format The format string
 * @returns The formatted date string
 */
export function FormatDate(date: Date, format: string): string {
    // JavaScript Date is different from our DateClass, so we'll handle both
    if (date.constructor.name === 'DateClass') {
        // This is our custom DateClass
        return (date as any).Format(format);
    } else {
        // This is a JavaScript Date object
        let result = format;
        result = result.replace('%Y', date.getFullYear().toString());
        result = result.replace('%m', (date.getMonth() + 1).toString().padStart(2, '0'));
        result = result.replace('%d', date.getDate().toString().padStart(2, '0'));
        return result;
    }
}

/**
 * Formats a time object as a relative time string (e.g., "2 hours ago")
 * @param time The time to format
 * @returns A relative time string
 */
export function FormatRelativeTime(time: Time): string {
    const now = Time.Now();
    const diffMs = now.GetTimestamp() - time.GetTimestamp();
    const diffSec = Math.floor(Math.abs(diffMs) / 1000);
    
    let value: number;
    let unit: string;
    
    if (diffSec < 60) {
        value = Math.floor(diffSec);
        unit = value === 1 ? 'second' : 'seconds';
    } else if (diffSec < 3600) {
        value = Math.floor(diffSec / 60);
        unit = value === 1 ? 'minute' : 'minutes';
    } else if (diffSec < 86400) {
        value = Math.floor(diffSec / 3600);
        unit = value === 1 ? 'hour' : 'hours';
    } else {
        value = Math.floor(diffSec / 86400);
        unit = value === 1 ? 'day' : 'days';
    }
    
    const direction = diffMs >= 0 ? 'ago' : 'from now';
    return `${value} ${unit} ${direction}`;
}

/**
 * Formats a time duration as decimal hours
 * @param duration The duration to format
 * @returns The duration in decimal hours
 */
export function FormatDurationAsHours(duration: Duration): number {
    return duration.GetMilliseconds() / (60 * 60 * 1000);
}

/**
 * Formats a time duration as decimal minutes
 * @param duration The duration to format
 * @returns The duration in decimal minutes
 */
export function FormatDurationAsMinutes(duration: Duration): number {
    return duration.GetMilliseconds() / (60 * 1000);
}