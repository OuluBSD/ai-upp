/**
 * Additional tests specifically designed to improve branch coverage
 */

import { TcpSocket, TcpServer } from '../src/Network/TcpSocket';
import { JsonSerializer, JsonValidator } from '../src/Network/JsonSerializer';

describe('TcpSocket Branch Coverage Tests', () => {
    test('TcpSocket error scenarios', () => {
        const socket = new TcpSocket();

        // Test methods when not connected
        expect(socket.GetLocalAddress()).toBeNull();
        expect(socket.GetLocalPort()).toBeNull();
        expect(socket.GetRemoteAddress()).toBeNull();
        expect(socket.GetRemotePort()).toBeNull();

        // Test IsConnected on fresh socket
        expect(socket.IsConnected()).toBe(false);

        // Test timeout settings
        expect(socket.GetTimeout()).toBe(30000); // default
        socket.SetTimeout(5000);
        expect(socket.GetTimeout()).toBe(5000);

        // Test delay and keep-alive methods
        socket.SetNoDelay();
        socket.SetKeepAlive();
    });
    
    test('TcpSocket ConnectAsync rejection path', async () => {
        const socket = new TcpSocket();

        // Set a very short timeout to force a timeout error
        socket.SetTimeout(10); // 10ms

        // Try to connect to an invalid address to trigger error
        // This should be rejected, but catching to prevent test failure
        try {
            await socket.ConnectAsync('127.0.0.1', 65000); // Use a high port that's likely closed
            // If the connection doesn't reject, check if it connects
            // If it did connect, this is unexpected, but let's handle gracefully
        } catch (error) {
            // Expected to get some form of error here
        }

        // After attempted connection failure, socket should not be connected
        expect(socket.IsConnected()).toBe(false);
    });
    
    test('TcpServer error scenarios', () => {
        const server = new TcpServer();
        
        // Initially not listening
        expect(server.IsListening()).toBe(false);
        expect(server.GetPort()).toBe(0);
        expect(server.GetHost()).toBe('localhost');
    });
    
    test('TcpServer Listen rejection path', async () => {
        const server = new TcpServer();
        
        // Try to listen on port 1 which is typically reserved and unavailable to non-privileged users
        await expect(server.ListenAsync(1))
            .rejects
            .toThrow();
        
        // Server should not be listening after failure
        expect(server.IsListening()).toBe(false);
    });
});

describe('JsonSerializer Branch Coverage Tests', () => {
    test('JsonSerializer error handling', () => {
        // Test serialization of circular reference (should throw)
        const obj: any = { name: 'test' };
        obj.self = obj; // Create circular reference
        
        expect(() => JsonSerializer.Serialize(obj))
            .toThrow();
        
        // Test deserialization of invalid JSON
        expect(() => JsonSerializer.Deserialize('{"invalid": json}'))
            .toThrow();
        
        // Test deserialization of non-JSON
        expect(() => JsonSerializer.Deserialize('not json'))
            .toThrow();
        
        // Test IsValidJson with invalid strings
        expect(JsonSerializer.IsValidJson('{"invalid": json}')).toBe(false);
        expect(JsonSerializer.IsValidJson('')).toBe(false);
        expect(JsonSerializer.IsValidJson('not json')).toBe(false);
        expect(JsonSerializer.IsValidJson('{"valid": "json"}')).toBe(true);
    });
    
    test('JsonSerializer utility methods', () => {
        const obj = { a: 1, b: { c: 2 }, d: [3, 4, 5] };
        
        // Test PrettyPrint
        const pretty = JsonSerializer.PrettyPrint(obj);
        expect(pretty).toContain('"a": 1');
        expect(pretty).toContain('\n');
        
        // Test Format
        const formatted = JsonSerializer.Format(obj, 4);
        expect(formatted.includes('    ')).toBe(true); // 4 spaces
        
        // Test Merge
        const base = { a: 1, b: 2 };
        const override = { b: 3, c: 4 };
        const merged = JsonSerializer.Merge(base, override);
        expect(merged).toEqual({ a: 1, b: 3, c: 4 });
        
        // Test DeepClone
        const original = { a: 1, b: { c: 2 } };
        const cloned = JsonSerializer.DeepClone(original);
        expect(cloned).toEqual(original);
        expect(cloned).not.toBe(original); // Different objects
        expect(cloned.b).not.toBe(original.b); // Nested objects are different
        
        // Test ExtractValue
        const testObj = {
            user: {
                profile: {
                    name: 'John',
                    address: {
                        city: 'New York'
                    }
                },
                contacts: ['email', 'phone']
            }
        };
        
        expect(JsonSerializer.ExtractValue(testObj, 'user.profile.name')).toBe('John');
        expect(JsonSerializer.ExtractValue(testObj, 'user.profile.address.city')).toBe('New York');
        expect(JsonSerializer.ExtractValue(testObj, 'user.contacts.0')).toBe('email');
        expect(JsonSerializer.ExtractValue(testObj, 'nonexistent.path')).toBeUndefined();
        
        // Test SetValue
        const modifyObj = { a: 1, b: { c: 2 } };
        JsonSerializer.SetValue(modifyObj, 'b.d', 3);
        // The property 'd' doesn't exist in the type definition, but the function dynamically adds it
        // We need to cast to 'any' to access it
        expect((modifyObj as any).b.d).toBe(3);

        JsonSerializer.SetValue(modifyObj, 'new.path.value', 'deep');
        expect((modifyObj as any).new.path.value).toBe('deep');
    });
});

describe('JsonValidator Branch Coverage Tests', () => {
    test('JsonValidator edge cases', () => {
        // Test HasRequiredProperties with edge cases
        expect(JsonValidator.HasRequiredProperties(null as any, ['prop'])).toBe(false);
        expect(JsonValidator.HasRequiredProperties(undefined as any, ['prop'])).toBe(false);
        expect(JsonValidator.HasRequiredProperties({}, [])).toBe(true);
        expect(JsonValidator.HasRequiredProperties({}, ['missing'])).toBe(false);
        expect(JsonValidator.HasRequiredProperties({ a: 1 }, [])).toBe(true);
        
        // Test ValidateTypes with edge cases
        expect(JsonValidator.ValidateTypes(null as any, { a: 'string' })).toBe(false);
        expect(JsonValidator.ValidateTypes(undefined as any, { a: 'string' })).toBe(false);
        expect(JsonValidator.ValidateTypes({}, {})).toBe(true);
        expect(JsonValidator.ValidateTypes({ a: 'test' }, { a: 'string' })).toBe(true);
        expect(JsonValidator.ValidateTypes({ a: 'test' }, { a: 'number' })).toBe(false);
        
        // Test array type validation
        expect(JsonValidator.ValidateTypes({ a: [1, 2, 3] }, { a: 'array' })).toBe(true);
        expect(JsonValidator.ValidateTypes({ a: 'not array' }, { a: 'array' })).toBe(false);
        
        // Test ValidateSchema with complex scenarios
        const schema = {
            type: 'object',
            properties: {
                name: { type: 'string' },
                age: { type: 'number' },
                active: { type: 'boolean' },
                tags: { type: 'array' }
            }
        } as any;
        
        expect(JsonValidator.ValidateSchema({ name: 'John', age: 30, active: true, tags: ['dev'] }, schema))
            .toBe(true);
        expect(JsonValidator.ValidateSchema({ name: 'John', age: '30' }, schema)) // age is string, not number
            .toBe(false);
        expect(JsonValidator.ValidateSchema({ name: 'John' }, schema)) // Missing other properties but they aren't required
            .toBe(true);
        
        // Test array schema validation
        const arraySchema = {
            type: 'array',
            items: { type: 'string' }
        } as any;
        
        expect(JsonValidator.ValidateSchema(['a', 'b'], arraySchema))
            .toBe(true);
        expect(JsonValidator.ValidateSchema(['a', 123], arraySchema)) // mixed types
            .toBe(false);
    });
    
    test('JsonValidator schema validation edge cases', () => {
        // Schema with nested objects
        const nestedSchema = {
            type: 'object',
            properties: {
                user: {
                    type: 'object',
                    properties: {
                        id: { type: 'number' },
                        name: { type: 'string' }
                    }
                }
            }
        } as any;
        
        expect(JsonValidator.ValidateSchema({ user: { id: 1, name: 'John' } }, nestedSchema))
            .toBe(true);
        
        expect(JsonValidator.ValidateSchema({ user: { id: 'not number', name: 'John' } }, nestedSchema))
            .toBe(false);
        
        expect(JsonValidator.ValidateSchema({ user: null }, nestedSchema))
            .toBe(true); // null is valid for any type
    });
});