/**
 * Comprehensive tests for Time class to improve coverage
 */
import { Time, GetTickCount } from '../../src/DateTime/Time';

describe('Time Comprehensive Tests', () => {
    test('constructor with all parameters', () => {
        const time = new Time(2023, 10, 15, 14, 30, 45, 123);
        expect(time.GetYear()).toBe(2023);
        expect(time.GetMonth()).toBe(10);
        expect(time.GetDay()).toBe(15);
        expect(time.GetHour()).toBe(14);
        expect(time.GetMinute()).toBe(30);
        expect(time.GetSecond()).toBe(45);
        expect(time.GetMillisecond()).toBe(123);
    });

    test('constructor without parameters (now)', () => {
        const time = new Time();
        expect(time).toBeInstanceOf(Time);
    });

    test('static Now method creates time instance', () => {
        const time = Time.Now();
        expect(time).toBeInstanceOf(Time);
    });

    test('static FromTimestamp method', () => {
        const timestamp = Date.now();
        const time = Time.FromTimestamp(timestamp);
        expect(time).toBeInstanceOf(Time);
        expect(time.GetTimestamp()).toBe(timestamp);
    });

    test('static FromISOString method', () => {
        const isoString = new Date().toISOString();
        const time = Time.FromISOString(isoString);
        expect(time).toBeInstanceOf(Time);
    });

    test('GetHour, GetMinute, GetSecond methods', () => {
        // Testing current time - just ensure methods exist and return numbers
        const time = Time.Now();
        expect(typeof time.GetHour()).toBe('number');
        expect(typeof time.GetMinute()).toBe('number');
        expect(typeof time.GetSecond()).toBe('number');
    });

    test('GetDayOfWeek and GetDayOfYear methods', () => {
        const time = Time.Now();
        expect(typeof time.GetDayOfWeek()).toBe('number');
        expect(time.GetDayOfWeek()).toBeGreaterThanOrEqual(0);
        expect(time.GetDayOfWeek()).toBeLessThanOrEqual(6);

        expect(typeof time.GetDayOfYear()).toBe('number');
        expect(time.GetDayOfYear()).toBeGreaterThanOrEqual(1);
        expect(time.GetDayOfYear()).toBeLessThanOrEqual(366);
    });

    test('GetMillisecond method', () => {
        const time = Time.Now();
        expect(typeof time.GetMillisecond()).toBe('number');
    });

    test('GetTimestamp method', () => {
        const time = Time.Now();
        expect(typeof time.GetTimestamp()).toBe('number');
        expect(time.GetTimestamp()).toBeGreaterThan(0);
    });

    test('comparison methods', () => {
        const time1 = new Time(2023, 10, 15, 10, 0, 0, 0);
        const time2 = new Time(2023, 10, 15, 11, 0, 0, 0);
        expect(time1.IsEqual(time2)).toBe(false);
        expect(time1.IsBefore(time2)).toBe(true);
        expect(time1.IsAfter(time2)).toBe(false);

        expect(time2.IsEqual(time1)).toBe(false);
        expect(time2.IsBefore(time1)).toBe(false);
        expect(time2.IsAfter(time1)).toBe(true);

        // Equal times
        const time3 = new Time(2023, 10, 15, 10, 0, 0, 0);
        expect(time1.IsEqual(time3)).toBe(true);
    });

    test('Clone method', () => {
        const time = new Time(2023, 10, 15, 14, 30, 45, 123);
        const cloned = time.Clone();
        expect(cloned).toBeInstanceOf(Time);
        expect(cloned).not.toBe(time);
        expect(cloned.IsEqual(time)).toBe(true);
    });

    test('Format method', () => {
        const time = new Time(2023, 5, 7, 14, 30, 45, 500);
        expect(time.Format('%Y-%m-%d %H:%M:%S')).toBe('2023-05-07 14:30:45');
        expect(time.Format('%f')).toBe('500');
    });

    test('ToISOString method', () => {
        const time = new Time(2023, 10, 15, 14, 30, 45, 0);
        const isoString = time.ToISOString();
        expect(typeof isoString).toBe('string');
        expect(isoString).toContain('2023-10');
    });

    test('ToDate method', () => {
        const time = new Time();
        expect(time.ToDate()).toBeInstanceOf(Date);
    });
});

describe('GetTickCount function', () => {
    test('GetTickCount returns number', () => {
        const tickCount = GetTickCount();
        expect(typeof tickCount).toBe('number');
        expect(tickCount).toBeGreaterThan(0);
    });
});