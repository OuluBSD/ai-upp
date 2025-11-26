/**
 * Date class for calendar date handling with U++-like API
 * Provides calendar date operations (without time component) similar to U++ Date functionality
 * 
 * @example
 * ```typescript
 * const date = new DateClass(2023, 10, 15); // Year, Month, Day
 * console.log(date.GetYear()); // 2023
 * console.log(date.GetMonth()); // 10
 * 
 * const today = DateClass.Today();
 * console.log(today.Format('%Y-%m-%d'));
 * ```
 */
export class DateClass {
    private date: Date;

    /**
     * Creates a new Date instance
     * @param year The year (e.g., 2023)
     * @param month The month (1-12)
     * @param day The day of the month (1-31)
     */
    constructor(year?: number, month?: number, day?: number) {
        if (year !== undefined) {
            // Month in JavaScript Date is 0-11, so we subtract 1 to maintain consistency with U++
            this.date = new Date(year, (month || 1) - 1, day || 1);
            // Reset time component to 00:00:00
            this.date.setHours(0, 0, 0, 0);
        } else {
            this.date = new Date();
            // Reset time component to 00:00:00
            this.date.setHours(0, 0, 0, 0);
        }
    }

    /**
     * Creates a Date instance representing today's date
     * @returns A Date instance for today
     */
    static Today(): DateClass {
        return new DateClass();
    }

    /**
     * Creates a Date instance from a JavaScript Date object
     * @param date The JavaScript Date object
     * @returns A Date instance representing the given date
     */
    static FromDate(date: Date): DateClass {
        const dateInstance = new DateClass();
        dateInstance.date = new Date(date);
        // Reset time component to 00:00:00
        dateInstance.date.setHours(0, 0, 0, 0);
        return dateInstance;
    }

    /**
     * Creates a Date instance from an ISO string
     * @param isoString The ISO format date string
     * @returns A Date instance representing the given ISO string
     */
    static FromISOString(isoString: string): DateClass {
        const dateInstance = new DateClass();
        dateInstance.date = new Date(isoString);
        // Reset time component to 00:00:00
        dateInstance.date.setHours(0, 0, 0, 0);
        return dateInstance;
    }

    /**
     * Gets the year
     * @returns The year value
     */
    GetYear(): number {
        return this.date.getFullYear();
    }

    /**
     * Gets the month (1-12)
     * @returns The month value
     */
    GetMonth(): number {
        return this.date.getMonth() + 1; // JavaScript months are 0-11
    }

    /**
     * Gets the day of the month (1-31)
     * @returns The day value
     */
    GetDay(): number {
        return this.date.getDate();
    }

    /**
     * Gets the day of the week (0-6, 0 = Sunday)
     * @returns The day of the week value
     */
    GetDayOfWeek(): number {
        return this.date.getDay();
    }

    /**
     * Gets the day of the year (1-366)
     * @returns The day of the year value
     */
    GetDayOfYear(): number {
        const start = new Date(this.date.getFullYear(), 0, 0);
        const diff = this.date.getTime() - start.getTime();
        const oneDay = 1000 * 60 * 60 * 24;
        return Math.floor(diff / oneDay);
    }

    /**
     * Gets the ISO string representation of the date
     * @returns The ISO string
     */
    ToISOString(): string {
        return this.date.toISOString().split('T')[0];
    }

    /**
     * Checks if this date is equal to another date
     * @param other The other Date instance to compare with
     * @returns true if the dates are equal, false otherwise
     */
    IsEqual(other: DateClass): boolean {
        return this.date.getTime() === other.date.getTime();
    }

    /**
     * Checks if this date is before another date
     * @param other The other Date instance to compare with
     * @returns true if this date is before the other date, false otherwise
     */
    IsBefore(other: DateClass): boolean {
        return this.date.getTime() < other.date.getTime();
    }

    /**
     * Checks if this date is after another date
     * @param other The other Date instance to compare with
     * @returns true if this date is after the other date, false otherwise
     */
    IsAfter(other: DateClass): boolean {
        return this.date.getTime() > other.date.getTime();
    }

    /**
     * Creates a copy of this Date instance
     * @returns A new Date instance with the same values
     */
    Clone(): DateClass {
        const newDate = new DateClass();
        newDate.date = new Date(this.date.getTime());
        return newDate;
    }

    /**
     * Formats the date according to the specified format string
     * Supports: %Y (year), %m (month), %d (day)
     * @param format The format string
     * @returns The formatted date string
     */
    Format(format: string): string {
        let result = format;
        result = result.replace('%Y', this.GetYear().toString());
        result = result.replace('%m', this.GetMonth().toString().padStart(2, '0'));
        result = result.replace('%d', this.GetDay().toString().padStart(2, '0'));
        
        return result;
    }

    /**
     * Gets the time as a JavaScript Date object
     * @returns The JavaScript Date object
     */
    ToDate(): Date {
        return new Date(this.date.getTime());
    }

    /**
     * Compares this date to another date
     * @param other The other Date instance to compare with
     * @returns -1 if this date is before other, 0 if equal, 1 if after
     */
    Compare(other: DateClass): number {
        const thisTime = this.date.getTime();
        const otherTime = other.date.getTime();
        
        if (thisTime < otherTime) {
            return -1;
        } else if (thisTime > otherTime) {
            return 1;
        } else {
            return 0;
        }
    }
}

/**
 * TimePoint class that represents a point in time with nanosecond precision
 */
export class TimePoint {
    private timestamp: number; // milliseconds since epoch
    private nanoseconds: number; // additional nanoseconds (0-999999)

    /**
     * Creates a new TimePoint instance
     * @param timestamp Milliseconds since epoch
     * @param nanoseconds Additional nanoseconds (0-999999)
     */
    constructor(timestamp?: number, nanoseconds?: number) {
        this.timestamp = timestamp || Date.now();
        this.nanoseconds = nanoseconds || 0;
        
        // Normalize nanoseconds
        if (this.nanoseconds >= 1000000) {
            this.timestamp += Math.floor(this.nanoseconds / 1000000);
            this.nanoseconds = this.nanoseconds % 1000000;
        }
    }

    /**
     * Creates a TimePoint representing the current time
     * @returns A TimePoint instance for the current time
     */
    static Now(): TimePoint {
        return new TimePoint();
    }

    /**
     * Gets the milliseconds since epoch
     * @returns The timestamp in milliseconds
     */
    GetTimestamp(): number {
        return this.timestamp;
    }

    /**
     * Gets additional nanoseconds beyond the millisecond timestamp
     * @returns The nanoseconds (0-999999)
     */
    GetNanoseconds(): number {
        return this.nanoseconds;
    }

    /**
     * Gets the total time as milliseconds with nanosecond precision
     * @returns The time as milliseconds (with fractional part for nanoseconds)
     */
    ToMilliseconds(): number {
        return this.timestamp + this.nanoseconds / 1000000;
    }
}

/**
 * Duration class for representing time intervals
 */
export class Duration {
    private milliseconds: number;

    /**
     * Creates a new Duration instance
     * @param milliseconds The duration in milliseconds
     */
    constructor(milliseconds: number = 0) {
        this.milliseconds = milliseconds;
    }

    /**
     * Creates a Duration from days
     * @param days Number of days
     * @returns A Duration instance
     */
    static FromDays(days: number): Duration {
        return new Duration(days * 24 * 60 * 60 * 1000);
    }

    /**
     * Creates a Duration from hours
     * @param hours Number of hours
     * @returns A Duration instance
     */
    static FromHours(hours: number): Duration {
        return new Duration(hours * 60 * 60 * 1000);
    }

    /**
     * Creates a Duration from minutes
     * @param minutes Number of minutes
     * @returns A Duration instance
     */
    static FromMinutes(minutes: number): Duration {
        return new Duration(minutes * 60 * 1000);
    }

    /**
     * Creates a Duration from seconds
     * @param seconds Number of seconds
     * @returns A Duration instance
     */
    static FromSeconds(seconds: number): Duration {
        return new Duration(seconds * 1000);
    }

    /**
     * Creates a Duration from milliseconds
     * @param milliseconds Number of milliseconds
     * @returns A Duration instance
     */
    static FromMilliseconds(milliseconds: number): Duration {
        return new Duration(milliseconds);
    }

    /**
     * Gets the duration in days
     * @returns The number of days
     */
    GetDays(): number {
        return this.milliseconds / (24 * 60 * 60 * 1000);
    }

    /**
     * Gets the duration in hours
     * @returns The number of hours
     */
    GetHours(): number {
        return this.milliseconds / (60 * 60 * 1000);
    }

    /**
     * Gets the duration in minutes
     * @returns The number of minutes
     */
    GetMinutes(): number {
        return this.milliseconds / (60 * 1000);
    }

    /**
     * Gets the duration in seconds
     * @returns The number of seconds
     */
    GetSeconds(): number {
        return this.milliseconds / 1000;
    }

    /**
     * Gets the duration in milliseconds
     * @returns The number of milliseconds
     */
    GetMilliseconds(): number {
        return this.milliseconds;
    }

    /**
     * Adds another duration to this duration
     * @param other The other duration to add
     * @returns A new Duration instance representing the sum
     */
    Add(other: Duration): Duration {
        return new Duration(this.milliseconds + other.milliseconds);
    }

    /**
     * Subtracts another duration from this duration
     * @param other The other duration to subtract
     * @returns A new Duration instance representing the difference
     */
    Subtract(other: Duration): Duration {
        return new Duration(this.milliseconds - other.milliseconds);
    }

    /**
     * Multiplies this duration by a scalar value
     * @param factor The scalar factor to multiply by
     * @returns A new Duration instance representing the product
     */
    Multiply(factor: number): Duration {
        return new Duration(this.milliseconds * factor);
    }

    /**
     * Divides this duration by a scalar value
     * @param divisor The scalar divisor to divide by
     * @returns A new Duration instance representing the quotient
     */
    Divide(divisor: number): Duration {
        if (divisor === 0) {
            throw new Error('Cannot divide duration by zero');
        }
        return new Duration(this.milliseconds / divisor);
    }

    /**
     * Checks if this duration is equal to another duration
     * @param other The other duration to compare with
     * @returns true if the durations are equal, false otherwise
     */
    IsEqual(other: Duration): boolean {
        return this.milliseconds === other.milliseconds;
    }

    /**
     * Checks if this duration is less than another duration
     * @param other The other duration to compare with
     * @returns true if this duration is less than the other, false otherwise
     */
    IsLessThan(other: Duration): boolean {
        return this.milliseconds < other.milliseconds;
    }

    /**
     * Checks if this duration is greater than another duration
     * @param other The other duration to compare with
     * @returns true if this duration is greater than the other, false otherwise
     */
    IsGreaterThan(other: Duration): boolean {
        return this.milliseconds > other.milliseconds;
    }

    /**
     * Gets the absolute value of this duration
     * @returns A new Duration instance with the absolute value
     */
    Abs(): Duration {
        return new Duration(Math.abs(this.milliseconds));
    }
}