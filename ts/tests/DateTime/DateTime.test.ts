/**
 * Tests for the DateTime module
 * Tests all aspects of the date/time functionality
 */

import { Time } from '../../src/DateTime/Time';
import { DateClass, TimePoint, Duration } from '../../src/DateTime/Date';
import { TimeZone, TimeZonedTime } from '../../src/DateTime/TimeZone';
import { FormatTime, FormatDuration, ParseDateTime, FormatTimeInterval, FormatRelativeTime } from '../../src/DateTime/TimeFormatting';
import { AddDays, DaysBetween, AddDuration, TimeDifference, IsLeapYear, DaysInMonth } from '../../src/DateTime/DateArithmetic';
import { Timer, PeriodicTimer, ScheduleTask, Delay } from '../../src/DateTime/Timers';
import { PerformanceTimer, GetPerformanceCounter, MeasureTime, Stopwatch } from '../../src/DateTime/Performance';

describe('Time', () => {
    test('should create a Time instance with specified values', () => {
        const time = new Time(2023, 10, 15, 14, 30, 45, 123);
        
        expect(time.GetYear()).toBe(2023);
        expect(time.GetMonth()).toBe(10);
        expect(time.GetDay()).toBe(15);
        expect(time.GetHour()).toBe(14);
        expect(time.GetMinute()).toBe(30);
        expect(time.GetSecond()).toBe(45);
        expect(time.GetMillisecond()).toBe(123);
    });

    test('should create a Time instance for now', () => {
        const now = Time.Now();
        expect(now).toBeInstanceOf(Time);
    });

    test('should format time correctly', () => {
        const time = new Time(2023, 5, 15, 14, 30, 45, 123);
        const formatted = time.Format('%Y-%m-%d %H:%M:%S.%f');
        
        expect(formatted).toBe('2023-05-15 14:30:45.123');
    });

    test('should compare times correctly', () => {
        const time1 = new Time(2023, 5, 15, 10, 0, 0);
        const time2 = new Time(2023, 5, 15, 12, 0, 0);
        
        expect(time1.IsBefore(time2)).toBe(true);
        expect(time1.IsAfter(time2)).toBe(false);
        expect(time1.IsEqual(time1)).toBe(true);
    });
});

describe('DateClass', () => {
    test('should create a Date instance with specified values', () => {
        const date = new DateClass(2023, 10, 15);
        
        expect(date.GetYear()).toBe(2023);
        expect(date.GetMonth()).toBe(10);
        expect(date.GetDay()).toBe(15);
    });

    test('should create a Date instance for today', () => {
        const today = DateClass.Today();
        expect(today).toBeInstanceOf(DateClass);
    });

    test('should format date correctly', () => {
        const date = new DateClass(2023, 5, 15);
        const formatted = date.Format('%Y-%m-%d');
        
        expect(formatted).toBe('2023-05-15');
    });

    test('should compare dates correctly', () => {
        const date1 = new DateClass(2023, 5, 15);
        const date2 = new DateClass(2023, 5, 20);
        
        expect(date1.IsBefore(date2)).toBe(true);
        expect(date1.IsAfter(date2)).toBe(false);
        expect(date1.IsEqual(date1)).toBe(true);
    });
});

describe('Duration', () => {
    test('should create a Duration and convert units correctly', () => {
        const duration = Duration.FromHours(2);
        
        expect(duration.GetHours()).toBe(2);
        expect(duration.GetMinutes()).toBe(120);
        expect(duration.GetSeconds()).toBe(7200);
        expect(duration.GetMilliseconds()).toBe(7200000);
    });

    test('should perform arithmetic operations on durations', () => {
        const duration1 = Duration.FromMinutes(30);
        const duration2 = Duration.FromMinutes(45);
        
        const sum = duration1.Add(duration2);
        expect(sum.GetMinutes()).toBe(75);
        
        const diff = duration2.Subtract(duration1);
        expect(diff.GetMinutes()).toBe(15);
    });
});

describe('Time formatting', () => {
    test('should format time correctly', () => {
        const time = new Time(2023, 10, 15, 14, 30, 45);
        const formatted = FormatTime(time, '%Y-%m-%d %H:%M:%S');
        
        expect(formatted).toBe('2023-10-15 14:30:45');
    });

    test('should format duration correctly', () => {
        const duration = Duration.FromSeconds(3661); // 1h 1m 1s
        const formatted = FormatDuration(duration);
        
        // This should produce something like "1h 1m 1s" (order may vary)
        expect(formatted).toContain('h');
        expect(formatted).toContain('m');
        expect(formatted).toContain('s');
    });

    test('should format relative time correctly', () => {
        const now = Time.Now();
        // Create a time that's roughly 1 hour ago
        const pastTimestamp = now.GetTimestamp() - 3600000; // 1 hour in milliseconds
        const pastTime = Time.FromTimestamp(pastTimestamp);

        const formatted = FormatRelativeTime(pastTime);

        // Should say something like "1 hour ago" or "60 minutes ago"
        expect(formatted).toContain('ago');
        // The exact unit depends on the implementation but should contain either unit
        expect(formatted).toMatch(/\d+\s+(hour|minute|second)/);
    });
});

describe('Date arithmetic', () => {
    test('should add days correctly', () => {
        const date = new DateClass(2023, 10, 15);
        const newDate = AddDays(date, 10);
        
        expect(newDate.GetDay()).toBe(25);
    });

    test('should calculate days between dates correctly', () => {
        const startDate = new DateClass(2023, 10, 1);
        const endDate = new DateClass(2023, 10, 10);
        
        const days = DaysBetween(startDate, endDate);
        expect(days).toBe(9);
    });

    test('should check leap year correctly', () => {
        expect(IsLeapYear(2020)).toBe(true); // 2020 is a leap year
        expect(IsLeapYear(2021)).toBe(false); // 2021 is not a leap year
    });

    test('should get days in month correctly', () => {
        expect(DaysInMonth(2023, 2)).toBe(28); // February 2023 has 28 days
        expect(DaysInMonth(2020, 2)).toBe(29); // February 2020 (leap year) has 29 days
        expect(DaysInMonth(2023, 1)).toBe(31); // January has 31 days
    });
});

describe('Timers', () => {
    test('should execute timer callback after delay', (done) => {
        const timer = new Timer(() => {
            expect(true).toBe(true);
            done();
        }, 10); // 10ms delay
        
        timer.Start();
    });

    test('should be able to stop timer', () => {
        const timer = new Timer(() => {
            throw new Error('This should not execute');
        }, 100);
        
        timer.Start();
        timer.Stop();
        
        // If we get here without error, the timer was successfully stopped
        expect(timer.IsRunning()).toBe(false);
    });

    test('should delay execution for specified duration', async () => {
        const start = Date.now();
        const duration = Duration.FromMilliseconds(50);
        
        await Delay(duration);
        
        const end = Date.now();
        expect(end - start).toBeGreaterThanOrEqual(45); // Allow some time tolerance
    });
});

describe('Performance timing', () => {
    test('should measure time correctly', () => {
        const result = MeasureTime(() => {
            return 'test result';
        });
        
        expect(result.result).toBe('test result');
        expect(typeof result.timeMs).toBe('number');
        expect(result.timeMs).toBeGreaterThanOrEqual(0);
    });

    test('should use performance timer correctly', () => {
        const timer = new PerformanceTimer();
        timer.Start();
        
        expect(timer.IsRunning()).toBe(true);
        
        timer.Stop();
        expect(timer.IsRunning()).toBe(false);
        
        // Elapsed time should be non-negative
        expect(timer.ElapsedMilliseconds()).toBeGreaterThanOrEqual(0);
    });

    test('should use stopwatch correctly', () => {
        const stopwatch = new Stopwatch();
        
        expect(stopwatch.IsRunning()).toBe(false);
        expect(stopwatch.ElapsedMilliseconds()).toBe(0);
        
        stopwatch.Start();
        expect(stopwatch.IsRunning()).toBe(true);
        
        stopwatch.Stop();
        expect(stopwatch.IsRunning()).toBe(false);
        
        stopwatch.Reset();
        expect(stopwatch.ElapsedMilliseconds()).toBe(0);
    });
});