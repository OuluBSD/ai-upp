/**
 * JSONSerializer - JSON serialization helpers similar to U++ JSON functionality
 * Uses Node.js built-in JSON for serialization/deserialization
 */

/**
 * JsonSerializer - Class for serializing objects to JSON and deserializing from JSON
 */
export class JsonSerializer {
    /**
     * Serialize an object to JSON string
     * @param obj The object to serialize
     * @param space Number of spaces for indentation (optional)
     * @returns JSON string representation of the object
     */
    static Serialize(obj: any, space?: number): string {
        try {
            return JSON.stringify(obj, null, space);
        } catch (error) {
            throw new Error(`JSON serialization error: ${error instanceof Error ? error.message : String(error)}`);
        }
    }

    /**
     * Deserialize a JSON string to an object
     * @param json The JSON string to deserialize
     * @returns Deserialized object
     */
    static Deserialize<T = any>(json: string): T {
        try {
            return JSON.parse(json) as T;
        } catch (error) {
            throw new Error(`JSON deserialization error: ${error instanceof Error ? error.message : String(error)}`);
        }
    }

    /**
     * Pretty print a JSON object with indentation
     * @param obj The object to pretty print
     * @returns Formatted JSON string
     */
    static PrettyPrint(obj: any): string {
        return JSON.stringify(obj, null, 2);
    }

    /**
     * Check if a string is valid JSON
     * @param str The string to check
     * @returns True if the string is valid JSON, false otherwise
     */
    static IsValidJson(str: string): boolean {
        try {
            JSON.parse(str);
            return true;
        } catch {
            return false;
        }
    }

    /**
     * Convert JSON to a formatted string with custom indentation
     * @param obj The object to format
     * @param space Number of spaces for indentation
     * @returns Formatted JSON string
     */
    static Format(obj: any, space: number = 2): string {
        return JSON.stringify(obj, null, space);
    }

    /**
     * Merge two JSON objects, with values from the second object taking precedence
     * @param base The base object to merge into
     * @param override The object with values that override the base
     * @returns Merged object
     */
    static Merge(base: any, override: any): any {
        return { ...base, ...override };
    }

    /**
     * Deep clone an object using JSON serialization/deserialization
     * Note: This only works for JSON-serializable objects (no functions, symbols, etc.)
     * @param obj The object to clone
     * @returns Deep clone of the object
     */
    static DeepClone<T>(obj: T): T {
        return JSON.parse(JSON.stringify(obj));
    }

    /**
     * Extract a value from a JSON object using a dot-notation path
     * @param obj The JSON object to extract from
     * @param path The dot-notation path (e.g., "user.profile.name")
     * @returns The extracted value or undefined if path doesn't exist
     */
    static ExtractValue(obj: any, path: string): any {
        const parts = path.split('.');
        let current: any = obj;

        for (const part of parts) {
            if (current === null || current === undefined || typeof current !== 'object') {
                return undefined;
            }
            current = current[part];
        }

        return current;
    }

    /**
     * Set a value in a JSON object using a dot-notation path
     * @param obj The JSON object to modify
     * @param path The dot-notation path (e.g., "user.profile.name")
     * @param value The value to set
     * @returns The modified object
     */
    static SetValue(obj: any, path: string, value: any): any {
        const parts = path.split('.');
        let current: any = obj;

        for (let i = 0; i < parts.length - 1; i++) {
            const part = parts[i];
            if (current[part] === undefined || current[part] === null) {
                current[part] = {};
            }
            current = current[part];
        }

        current[parts[parts.length - 1]] = value;
        return obj;
    }
}

/**
 * JSON validation helpers
 */
export class JsonValidator {
    /**
     * Validate that a JSON object has required properties
     * @param obj The object to validate
     * @param requiredProps Array of required property names
     * @returns True if all required properties exist, false otherwise
     */
    static HasRequiredProperties(obj: any, requiredProps: string[]): boolean {
        if (typeof obj !== 'object' || obj === null) {
            return false;
        }

        for (const prop of requiredProps) {
            if (!(prop in obj)) {
                return false;
            }
        }

        return true;
    }

    /**
     * Validate that a JSON object has properties with specific types
     * @param obj The object to validate
     * @param typeSpec Object specifying property types (e.g., {name: 'string', age: 'number'})
     * @returns True if all properties have the correct types, false otherwise
     */
    static ValidateTypes(obj: any, typeSpec: { [key: string]: string }): boolean {
        if (typeof obj !== 'object' || obj === null) {
            return false;
        }

        for (const [prop, expectedType] of Object.entries(typeSpec)) {
            if (!(prop in obj)) {
                continue; // Skip if property doesn't exist (not required)
            }

            const actualType = typeof obj[prop];
            
            // Handle special case for arrays
            if (expectedType === 'array' && !Array.isArray(obj[prop])) {
                return false;
            } else if (expectedType !== 'array' && actualType !== expectedType) {
                return false;
            }
        }

        return true;
    }

    /**
     * Validate that a JSON object conforms to a schema
     * @param obj The object to validate
     * @param schema The schema to validate against
     * @returns True if the object conforms to the schema, false otherwise
     */
    static ValidateSchema(obj: any, schema: JsonSchema): boolean {
        return JsonValidator.validateSchemaRecursive(obj, schema);
    }

    private static validateSchemaRecursive(obj: any, schema: JsonSchema, path: string = ''): boolean {
        if (!schema) return true;

        // Check type if specified
        if (schema.type) {
            if (schema.type === 'array') {
                if (!Array.isArray(obj)) return false;
            } else if (typeof obj !== schema.type) {
                return false;
            }
        }

        // Check properties if it's an object
        if (typeof obj === 'object' && obj !== null && !Array.isArray(obj) && schema.properties) {
            for (const [propName, propSchema] of Object.entries(schema.properties)) {
                const propValue = obj[propName];
                if (propValue !== undefined) { // Only validate if the property exists
                    if (!JsonValidator.validateSchemaRecursive(propValue, propSchema, `${path}.${propName}`)) {
                        return false;
                    }
                }
            }
        }

        // Check items if it's an array
        if (Array.isArray(obj) && schema.items) {
            for (let i = 0; i < obj.length; i++) {
                if (!JsonValidator.validateSchemaRecursive(obj[i], schema.items, `${path}[${i}]`)) {
                    return false;
                }
            }
        }

        return true;
    }
}

/**
 * A simple JSON schema interface
 */
interface JsonSchema {
    type?: 'string' | 'number' | 'boolean' | 'object' | 'array' | 'null';
    properties?: { [key: string]: JsonSchema };
    items?: JsonSchema;
    required?: string[];
}

/**
 * JSONPathQuery - Class for querying JSON objects using a path-like syntax
 */
export class JsonPathQuery {
    private obj: any;

    constructor(obj: any) {
        this.obj = obj;
    }

    /**
     * Get a value using a path string (e.g., "users[0].name")
     * @param path The path to the value
     * @returns The value at the path
     */
    Get(path: string): any {
        return JsonPathQuery.getValueAtPath(this.obj, path);
    }

    /**
     * Set a value using a path string
     * @param path The path to the value
     * @param value The value to set
     */
    Set(path: string, value: any): JsonPathQuery {
        JsonPathQuery.setValueAtPath(this.obj, path, value);
        return this;
    }

    /**
     * Check if a path exists in the object
     * @param path The path to check
     * @returns True if the path exists
     */
    Exists(path: string): boolean {
        try {
            return JsonPathQuery.getValueAtPath(this.obj, path) !== undefined;
        } catch {
            return false;
        }
    }

    /**
     * Get the underlying object
     */
    GetObject(): any {
        return this.obj;
    }

    private static getValueAtPath(obj: any, path: string): any {
        const parts = path.replace(/\[(\w+)\]/g, '.$1').split('.');
        let current = obj;

        for (const part of parts) {
            if (part === '') continue; // Skip empty strings
            if (current === null || current === undefined) return undefined;
            current = current[part];
        }

        return current;
    }

    private static setValueAtPath(obj: any, path: string, value: any): void {
        const parts = path.replace(/\[(\w+)\]/g, '.$1').split('.');
        let current = obj;

        for (let i = 0; i < parts.length - 1; i++) {
            const part = parts[i];
            if (part === '') continue; // Skip empty strings

            if (current[part] === undefined || current[part] === null) {
                // Check if next part looks like an array index
                const nextPart = parts[i + 1];
                if (nextPart && /^\d+$/.test(nextPart)) {
                    current[part] = [];
                } else {
                    current[part] = {};
                }
            }
            current = current[part];
        }

        const lastPart = parts[parts.length - 1];
        if (lastPart !== '') {
            current[lastPart] = value;
        }
    }
}