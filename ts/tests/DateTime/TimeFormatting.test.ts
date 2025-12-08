/**
 * Comprehensive tests for TimeFormatting module to improve branch coverage
 */
import { 
    FormatTime,
    FormatDuration,
    ParseDateTime,
    FormatTimeInterval,
    CurrentTimeFormatted,
    FormatDate,
    FormatRelativeTime,
    FormatDurationAsHours,
    FormatDurationAsMinutes
} from '../../src/DateTime/TimeFormatting';
import { Time } from '../../src/DateTime/Time';
import { Duration, DateClass } from '../../src/DateTime/Date';

describe('TimeFormatting Comprehensive Tests', () => {
    test('FormatTime function', () => {
        const time = new Time(2023, 5, 15, 14, 30, 45, 500);
        const formatted = FormatTime(time, '%Y-%m-%d %H:%M:%S');
        expect(formatted).toBe('2023-05-15 14:30:45');
    });

    test('FormatDuration function with various time units', () => {
        // Test 0 duration
        let duration = new Duration(0);
        expect(FormatDuration(duration)).toBe('0s');

        // Test milliseconds only
        duration = new Duration(500);
        expect(FormatDuration(duration)).toBe('500ms');

        // Test seconds
        duration = new Duration(2000); // 2 seconds
        expect(FormatDuration(duration)).toBe('2s');

        // Test minutes and seconds
        duration = new Duration(125000); // 2 min 5 sec
        expect(FormatDuration(duration)).toBe('2m 5s');

        // Test hours, minutes, seconds
        duration = new Duration(3661000); // 1h 1m 1s
        expect(FormatDuration(duration)).toBe('1h 1m 1s');

        // Test days, hours, minutes, seconds
        duration = new Duration(90061000); // 90061 sec = 25h 1m 1s = 1 day, 1h, 1m, 1s
        expect(FormatDuration(duration)).toBe('1d 1h 1m 1s');

        // Test with milliseconds
        duration = new Duration(3661500); // 1h 1m 1s 500ms
        expect(FormatDuration(duration)).toBe('1h 1m 1s 500ms');
    });

    test('ParseDateTime function with ISO format', () => {
        const time = ParseDateTime('2023-05-15 14:30:45', '%Y-%m-%d %H:%M:%S');
        expect(time.GetYear()).toBe(2023);
        expect(time.GetMonth()).toBe(5);
        expect(time.GetDay()).toBe(15);
        expect(time.GetHour()).toBe(14);
        expect(time.GetMinute()).toBe(30);
        expect(time.GetSecond()).toBe(45);
    });

    test('ParseDateTime with ISO format alternative', () => {
        const time = ParseDateTime('2023-05-15T14:30:45.000Z', 'ISO');
        // This should fallback to Time.FromISOString
        expect(time).toBeInstanceOf(Time);
    });

    test('ParseDateTime with invalid format', () => {
        // This should also fallback to Time.FromISOString in the current implementation
        const time = ParseDateTime('2023/05/15 14:30:45', 'invalid format');
        expect(time).toBeInstanceOf(Time);
    });

    test('FormatTimeInterval function', () => {
        const startTime = new Time(2023, 5, 15, 10, 0, 0, 0);
        const endTime = new Time(2023, 5, 15, 11, 30, 0, 0);
        const intervalStr = FormatTimeInterval(startTime, endTime);
        
        // Should be 1h 30m
        expect(intervalStr).toContain('1h');
        expect(intervalStr).toContain('30m');
    });

    test('CurrentTimeFormatted function', () => {
        const formatted = CurrentTimeFormatted();
        // Should be in the format YYYY-MM-DD HH:MM:SS
        expect(formatted).toMatch(/^\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}$/);
    });

    test('FormatDate function with JS Date', () => {
        const jsDate = new Date(2023, 4, 15); // Month is 0-indexed in JS
        const formatted = FormatDate(jsDate, '%Y-%m-%d');
        // The exact date might vary based on timezone, so we check for the year
        expect(formatted).toContain('2023');
    });

    test('FormatDate function with DateClass', () => {
        const dateClass = new DateClass(2023, 5, 15);
        // We'll mock the Format method to simulate DateClass as this is a bit tricky to test
        // In actual implementation, DateClass should have Format method
        const formatted = FormatDate(dateClass as any, '%Y-%m-%d');
        expect(formatted).toBe('2023-05-15');
    });

    test('FormatRelativeTime function - past time', () => {
        // Create a time that is 2 hours ago
        const pastMs = Date.now() - (2 * 60 * 60 * 1000); // 2 hours in ms
        const pastTime = Time.FromTimestamp(pastMs);
        const relativeStr = FormatRelativeTime(pastTime);
        
        expect(relativeStr).toContain('2 hours');
        expect(relativeStr).toContain('ago');
    });

    test('FormatRelativeTime function - future time', () => {
        // Create a time that is 30 minutes in the future
        const futureMs = Date.now() + (30 * 60 * 1000); // 30 minutes in ms
        const futureTime = Time.FromTimestamp(futureMs);
        const relativeStr = FormatRelativeTime(futureTime);
        
        expect(relativeStr).toContain('30 minutes');
        expect(relativeStr).toContain('from now');
    });

    test('FormatRelativeTime function - different time units', () => {
        // Test with less than a minute
        const pastMs = Date.now() - 30000; // 30 seconds ago
        const pastTime = Time.FromTimestamp(pastMs);
        const relativeStr = FormatRelativeTime(pastTime);
        expect(relativeStr).toContain('seconds ago');
        
        // Test with about a minute
        const pastMin = Date.now() - (65 * 1000); // 65 seconds ago
        const pastTimeMin = Time.FromTimestamp(pastMin);
        const relativeStrMin = FormatRelativeTime(pastTimeMin);
        expect(relativeStrMin).toContain('minute ago');
        
        // Test with multiple days
        const pastDay = Date.now() - (3 * 24 * 60 * 60 * 1000); // 3 days ago
        const pastTimeDay = Time.FromTimestamp(pastDay);
        const relativeStrDay = FormatRelativeTime(pastTimeDay);
        expect(relativeStrDay).toContain('3 days ago');
    });

    test('FormatDurationAsHours function', () => {
        const duration = new Duration(7200000); // 2 hours in ms
        const hours = FormatDurationAsHours(duration);
        expect(hours).toBe(2);
    });

    test('FormatDurationAsMinutes function', () => {
        const duration = new Duration(120000); // 2 minutes in ms
        const minutes = FormatDurationAsMinutes(duration);
        expect(minutes).toBe(2);
    });
});