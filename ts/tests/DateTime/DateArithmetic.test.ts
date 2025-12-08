/**
 * Comprehensive tests for DateArithmetic module to improve branch coverage
 */
import { 
    AddDays,
    AddMonths,
    AddYears,
    DaysBetween,
    MonthsBetween,
    YearsBetween,
    AddDuration,
    SubtractDuration,
    TimeDifference,
    AddHours,
    AddMinutes,
    AddSeconds,
    AddMilliseconds,
    CompareDates,
    CompareTimes,
    StartOfMonth,
    EndOfMonth,
    StartOfYear,
    EndOfYear,
    DaysInMonth,
    IsLeapYear
} from '../../src/DateTime/DateArithmetic';
import { DateClass, Duration } from '../../src/DateTime/Date';
import { Time } from '../../src/DateTime/Time';

describe('DateArithmetic Comprehensive Tests', () => {
    test('AddDays function with positive and negative days', () => {
        const date = new DateClass(2023, 5, 15);
        const futureDate = AddDays(date, 10);
        expect(futureDate.GetDay()).toBe(25);
        expect(futureDate.GetMonth()).toBe(5);
        expect(futureDate.GetYear()).toBe(2023);

        const pastDate = AddDays(date, -5);
        expect(pastDate.GetDay()).toBe(10);
        expect(pastDate.GetMonth()).toBe(5);
        expect(pastDate.GetYear()).toBe(2023);
    });

    test('AddMonths function with positive and negative months', () => {
        const date = new DateClass(2023, 5, 15);
        const futureDate = AddMonths(date, 3);
        expect(futureDate.GetMonth()).toBe(8);
        expect(futureDate.GetYear()).toBe(2023);

        const pastDate = AddMonths(date, -2);
        expect(pastDate.GetMonth()).toBe(3);
        expect(pastDate.GetYear()).toBe(2023);
    });

    test('AddYears function with positive and negative years', () => {
        const date = new DateClass(2023, 5, 15);
        const futureDate = AddYears(date, 2);
        expect(futureDate.GetYear()).toBe(2025);

        const pastDate = AddYears(date, -1);
        expect(pastDate.GetYear()).toBe(2022);
    });

    test('DaysBetween function', () => {
        const startDate = new DateClass(2023, 5, 15);
        const endDate = new DateClass(2023, 5, 25);
        const days = DaysBetween(startDate, endDate);
        expect(days).toBe(10);

        // Negative difference
        const negativeDays = DaysBetween(endDate, startDate);
        expect(negativeDays).toBe(-10);
    });

    test('MonthsBetween function', () => {
        const startDate = new DateClass(2023, 3, 15);
        const endDate = new DateClass(2023, 7, 20);
        const months = MonthsBetween(startDate, endDate);
        expect(months).toBe(4);

        // Across years
        const startDate2 = new DateClass(2022, 10, 15);
        const endDate2 = new DateClass(2023, 2, 20);
        const months2 = MonthsBetween(startDate2, endDate2);
        expect(months2).toBe(4);
    });

    test('YearsBetween function', () => {
        const startDate = new DateClass(2020, 5, 15);
        const endDate = new DateClass(2023, 7, 20);
        const years = YearsBetween(startDate, endDate);
        expect(years).toBe(3);
    });

    test('AddDuration function', () => {
        const time = new Time(2023, 5, 15, 10, 0, 0, 0);
        const duration = Duration.FromHours(2);
        const newTime = AddDuration(time, duration);
        expect(newTime.GetHour()).toBe(12);
        expect(newTime.GetMinute()).toBe(0);
    });

    test('SubtractDuration function', () => {
        const time = new Time(2023, 5, 15, 10, 0, 0, 0);
        const duration = Duration.FromHours(2);
        const newTime = SubtractDuration(time, duration);
        expect(newTime.GetHour()).toBe(8);
        expect(newTime.GetMinute()).toBe(0);
    });

    test('TimeDifference function', () => {
        const startTime = new Time(2023, 5, 15, 10, 0, 0, 0);
        const endTime = new Time(2023, 5, 15, 12, 0, 0, 0);
        const duration = TimeDifference(startTime, endTime);
        expect(duration.GetHours()).toBe(2);
    });

    test('AddHours, AddMinutes, AddSeconds, AddMilliseconds functions', () => {
        const time = new Time(2023, 5, 15, 10, 30, 45, 500);

        // Add 2 hours
        let newTime = AddHours(time, 2);
        expect(newTime.GetHour()).toBe(12);

        // Add 15 minutes
        newTime = AddMinutes(time, 15);
        expect(newTime.GetMinute()).toBe(45);

        // Add 10 seconds
        newTime = AddSeconds(time, 10);
        expect(newTime.GetSecond()).toBe(55);

        // Add 250 milliseconds
        newTime = AddMilliseconds(time, 250);
        expect(newTime.GetMillisecond()).toBe(750);
    });

    test('CompareDates function', () => {
        const date1 = new DateClass(2023, 5, 15);
        const date2 = new DateClass(2023, 5, 20);
        const date3 = new DateClass(2023, 5, 15);

        expect(CompareDates(date1, date2)).toBe(-1); // date1 is before date2
        expect(CompareDates(date2, date1)).toBe(1);  // date2 is after date1
        expect(CompareDates(date1, date3)).toBe(0);  // dates are equal
    });

    test('CompareTimes function', () => {
        const time1 = new Time(2023, 5, 15, 10, 0, 0, 0);
        const time2 = new Time(2023, 5, 15, 12, 0, 0, 0);
        const time3 = new Time(2023, 5, 15, 10, 0, 0, 0);

        expect(CompareTimes(time1, time2)).toBe(-1); // time1 is before time2
        expect(CompareTimes(time2, time1)).toBe(1);  // time2 is after time1
        expect(CompareTimes(time1, time3)).toBe(0);  // times are equal
    });

    test('StartOfMonth function', () => {
        const date = new DateClass(2023, 5, 15);
        const startOfMonth = StartOfMonth(date);
        expect(startOfMonth.GetDay()).toBe(1);
        expect(startOfMonth.GetMonth()).toBe(5);
        expect(startOfMonth.GetYear()).toBe(2023);
    });

    test('EndOfMonth function', () => {
        // May has 31 days
        const date = new DateClass(2023, 5, 15);
        const endOfMonth = EndOfMonth(date);
        expect(endOfMonth.GetDay()).toBe(31);
        expect(endOfMonth.GetMonth()).toBe(5);
        expect(endOfMonth.GetYear()).toBe(2023);

        // February in a non-leap year has 28 days
        const febDate = new DateClass(2023, 2, 15);
        const endOfFeb = EndOfMonth(febDate);
        expect(endOfFeb.GetDay()).toBe(28);
    });

    test('StartOfYear function', () => {
        const date = new DateClass(2023, 5, 15);
        const startOfYear = StartOfYear(date);
        expect(startOfYear.GetDay()).toBe(1);
        expect(startOfYear.GetMonth()).toBe(1);
        expect(startOfYear.GetYear()).toBe(2023);
    });

    test('EndOfYear function', () => {
        const date = new DateClass(2023, 5, 15);
        const endOfYear = EndOfYear(date);
        expect(endOfYear.GetDay()).toBe(31);
        expect(endOfYear.GetMonth()).toBe(12);
        expect(endOfYear.GetYear()).toBe(2023);
    });

    test('DaysInMonth function', () => {
        // January has 31 days
        expect(DaysInMonth(2023, 1)).toBe(31);

        // February in a non-leap year has 28 days
        expect(DaysInMonth(2023, 2)).toBe(28);

        // February in a leap year has 29 days
        expect(DaysInMonth(2024, 2)).toBe(29);

        // April has 30 days
        expect(DaysInMonth(2023, 4)).toBe(30);

        // May has 31 days
        expect(DaysInMonth(2023, 5)).toBe(31);

        // June has 30 days
        expect(DaysInMonth(2023, 6)).toBe(30);

        // July has 31 days
        expect(DaysInMonth(2023, 7)).toBe(31);

        // December has 31 days
        expect(DaysInMonth(2023, 12)).toBe(31);
    });

    test('IsLeapYear function', () => {
        // Regular year
        expect(IsLeapYear(2023)).toBe(false);

        // Leap year
        expect(IsLeapYear(2024)).toBe(true);

        // Century year not divisible by 400 (not leap year)
        expect(IsLeapYear(1900)).toBe(false);

        // Century year divisible by 400 (leap year)
        expect(IsLeapYear(2000)).toBe(true);
    });
});