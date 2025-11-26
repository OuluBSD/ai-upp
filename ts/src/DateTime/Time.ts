/**
 * Time class for time handling with U++-like API
 * Provides time operations similar to U++ Time functionality
 * 
 * @example
 * ```typescript
 * const time = new Time(2023, 10, 15, 14, 30, 45, 123); // Year, Month, Day, Hour, Min, Sec, Ms
 * console.log(time.GetYear()); // 2023
 * console.log(time.GetMonth()); // 10
 * 
 * const now = Time.Now();
 * console.log(now.Format('%Y-%m-%d %H:%M:%S'));
 * ```
 */
export class Time {
    private date: Date;

    /**
     * Creates a new Time instance
     * @param year The year (e.g., 2023)
     * @param month The month (1-12)
     * @param day The day of the month (1-31)
     * @param hour The hour (0-23)
     * @param minute The minute (0-59)
     * @param second The second (0-59)
     * @param millisecond The millisecond (0-999)
     */
    constructor(
        year?: number,
        month?: number,
        day?: number,
        hour?: number,
        minute?: number,
        second?: number,
        millisecond?: number
    ) {
        if (year !== undefined) {
            // Month in JavaScript Date is 0-11, so we subtract 1 to maintain consistency with U++
            this.date = new Date(year, (month || 1) - 1, day || 1, hour || 0, minute || 0, second || 0, millisecond || 0);
        } else {
            this.date = new Date();
        }
    }

    /**
     * Creates a Time instance representing the current time
     * @returns A Time instance for the current time
     */
    static Now(): Time {
        return new Time();
    }

    /**
     * Creates a Time instance from a timestamp
     * @param timestamp The timestamp in milliseconds since Unix epoch
     * @returns A Time instance representing the given timestamp
     */
    static FromTimestamp(timestamp: number): Time {
        const time = new Time();
        time.date = new Date(timestamp);
        return time;
    }

    /**
     * Creates a Time instance from an ISO string
     * @param isoString The ISO format date string
     * @returns A Time instance representing the given ISO string
     */
    static FromISOString(isoString: string): Time {
        const time = new Time();
        time.date = new Date(isoString);
        return time;
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
     * Gets the hour (0-23)
     * @returns The hour value
     */
    GetHour(): number {
        return this.date.getHours();
    }

    /**
     * Gets the minute (0-59)
     * @returns The minute value
     */
    GetMinute(): number {
        return this.date.getMinutes();
    }

    /**
     * Gets the second (0-59)
     * @returns The second value
     */
    GetSecond(): number {
        return this.date.getSeconds();
    }

    /**
     * Gets the millisecond (0-999)
     * @returns The millisecond value
     */
    GetMillisecond(): number {
        return this.date.getMilliseconds();
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
     * Gets the timestamp (milliseconds since Unix epoch)
     * @returns The timestamp value
     */
    GetTimestamp(): number {
        return this.date.getTime();
    }

    /**
     * Checks if this time is equal to another time
     * @param other The other Time instance to compare with
     * @returns true if the times are equal, false otherwise
     */
    IsEqual(other: Time): boolean {
        return this.date.getTime() === other.date.getTime();
    }

    /**
     * Checks if this time is before another time
     * @param other The other Time instance to compare with
     * @returns true if this time is before the other time, false otherwise
     */
    IsBefore(other: Time): boolean {
        return this.date.getTime() < other.date.getTime();
    }

    /**
     * Checks if this time is after another time
     * @param other The other Time instance to compare with
     * @returns true if this time is after the other time, false otherwise
     */
    IsAfter(other: Time): boolean {
        return this.date.getTime() > other.date.getTime();
    }

    /**
     * Creates a copy of this Time instance
     * @returns A new Time instance with the same values
     */
    Clone(): Time {
        return Time.FromTimestamp(this.date.getTime());
    }

    /**
     * Formats the time according to the specified format string
     * Supports: %Y (year), %m (month), %d (day), %H (hour), %M (minute), %S (second), %f (millisecond)
     * @param format The format string
     * @returns The formatted time string
     */
    Format(format: string): string {
        let result = format;
        result = result.replace('%Y', this.GetYear().toString());
        result = result.replace('%m', this.GetMonth().toString().padStart(2, '0'));
        result = result.replace('%d', this.GetDay().toString().padStart(2, '0'));
        result = result.replace('%H', this.GetHour().toString().padStart(2, '0'));
        result = result.replace('%M', this.GetMinute().toString().padStart(2, '0'));
        result = result.replace('%S', this.GetSecond().toString().padStart(2, '0'));
        result = result.replace('%f', this.GetMillisecond().toString().padStart(3, '0'));
        
        return result;
    }

    /**
     * Gets the ISO string representation of the time
     * @returns The ISO string
     */
    ToISOString(): string {
        return this.date.toISOString();
    }

    /**
     * Gets the time as a JavaScript Date object
     * @returns The JavaScript Date object
     */
    ToDate(): Date {
        return new Date(this.date.getTime());
    }
}

/**
 * GetTickCount: Gets the number of milliseconds that have elapsed since the system was started
 * This is a simplified implementation for the Node.js environment
 *
 * @returns The number of milliseconds since epoch (not system start as in Windows)
 */
export function GetTickCount(): number {
    return Date.now();
}