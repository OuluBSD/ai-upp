/**
 * TimeZone class for time zone handling with U++-like API
 * Provides time zone operations similar to U++ TimeZone functionality
 * 
 * @example
 * ```typescript
 * const timeZone = new TimeZone('UTC');
 * console.log(timeZone.GetName()); // UTC
 * 
 * const localTimeZone = TimeZone.GetLocal();
 * console.log(localTimeZone.GetName());
 * ```
 */

export class TimeZone {
    private name: string;
    
    constructor(name: string = 'UTC') {
        this.name = name;
    }

    /**
     * Gets the name of the time zone
     * @returns The time zone name
     */
    GetName(): string {
        return this.name;
    }

    /**
     * Gets the UTC offset in minutes for this time zone at the given date
     * @param date The date to check the offset for
     * @returns The UTC offset in minutes
     */
    GetUtcOffset(date: Date): number {
        // For simplicity in this implementation, we'll use the system offset
        // In a real implementation, this would handle more complex time zone rules
        const utcDate = new Date(date.getTime() + (date.getTimezoneOffset() * 60000));
        return (utcDate.getTime() - date.getTime()) / 60000;
    }

    /**
     * Converts a date from UTC to this time zone
     * @param utcDate The UTC date to convert
     * @returns The date in this time zone
     */
    UtcToLocal(utcDate: Date): Date {
        // In a real implementation, this would apply proper time zone conversions
        // For now, we'll use a basic conversion based on the system's offset
        const offset = this.GetUtcOffset(utcDate);
        const localTime = new Date(utcDate.getTime() + offset * 60000);
        return localTime;
    }

    /**
     * Converts a date from this time zone to UTC
     * @param localDate The local date to convert
     * @returns The date in UTC
     */
    LocalToUtc(localDate: Date): Date {
        // In a real implementation, this would apply proper time zone conversions
        // For now, we'll use a basic conversion based on the system's offset
        const offset = this.GetUtcOffset(localDate);
        const utcTime = new Date(localDate.getTime() - offset * 60000);
        return utcTime;
    }

    /**
     * Creates a time zone instance for the system's local time zone
     * @returns A TimeZone instance for the local time zone
     */
    static GetLocal(): TimeZone {
        // Create a timezone based on the system's local timezone
        const now = new Date();
        const offset = -now.getTimezoneOffset(); // JavaScript returns offset with opposite sign
        const offsetHours = Math.floor(Math.abs(offset) / 60);
        const offsetMinutes = Math.abs(offset) % 60;
        const sign = offset >= 0 ? '+' : '-';
        
        // Create a simple name based on the offset
        const name = `UTC${sign}${offsetHours.toString().padStart(2, '0')}:${offsetMinutes.toString().padStart(2, '0')}`;
        
        return new TimeZone(name);
    }

    /**
     * Creates a time zone instance for UTC
     * @returns A TimeZone instance for UTC
     */
    static GetUtc(): TimeZone {
        return new TimeZone('UTC');
    }

    /**
     * Creates a time zone instance by name
     * @param name The IANA time zone name (e.g., 'America/New_York', 'Europe/London')
     * @returns A TimeZone instance for the specified time zone
     */
    static FromName(name: string): TimeZone {
        // In a full implementation, we would validate the name against a list of known time zones
        // For this implementation, we'll just return a TimeZone with the given name
        return new TimeZone(name);
    }
}

/**
 * TimeZonedTime class that combines a time with a time zone
 */
export class TimeZonedTime {
    private time: Date;
    private timeZone: TimeZone;

    /**
     * Creates a new TimeZonedTime instance
     * @param time The time in the specified time zone
     * @param timeZone The time zone for this time
     */
    constructor(time: Date, timeZone: TimeZone) {
        this.time = new Date(time.getTime());
        this.timeZone = timeZone;
    }

    /**
     * Gets the time component
     * @returns The time as a Date object
     */
    GetTime(): Date {
        return new Date(this.time.getTime());
    }

    /**
     * Gets the time zone component
     * @returns The time zone
     */
    GetTimeZone(): TimeZone {
        return this.timeZone;
    }

    /**
     * Converts this time to UTC
     * @returns A Date object representing the equivalent UTC time
     */
    ToUtc(): Date {
        return this.timeZone.LocalToUtc(this.time);
    }

    /**
     * Converts this time to the local system time zone
     * @returns A Date object representing the equivalent local time
     */
    ToLocal(): Date {
        // Get the local timezone and convert
        const localTz = TimeZone.GetLocal();
        // For this implementation, we'll just return the time since
        // the conversion logic would be complex without more robust time zone data
        return new Date(this.time.getTime());
    }

    /**
     * Gets the ISO string representation in the current time zone
     * @returns The ISO string
     */
    ToISOString(): string {
        return this.time.toISOString();
    }

    /**
     * Creates a copy of this TimeZonedTime instance
     * @returns A new TimeZonedTime instance with the same values
     */
    Clone(): TimeZonedTime {
        return new TimeZonedTime(this.time, this.timeZone);
    }
}

/**
 * Gets the current time in the system's local time zone
 * @returns A Date object representing the current local time
 */
export function GetLocalTime(): Date {
    return new Date();
}

/**
 * Gets the current time in UTC
 * @returns A Date object representing the current UTC time
 */
export function GetUtcTime(): Date {
    const now = new Date();
    return new Date(now.getTime() + now.getTimezoneOffset() * 60000);
}