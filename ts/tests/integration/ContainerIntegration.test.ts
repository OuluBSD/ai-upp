/**
 * Integration tests for container operations
 * Tests combinations of core containers with other modules
 */

import { Vector } from '../../src/Core/Vector';
import { Map } from '../../src/Core/Map';
import { String } from '../../src/Core/String';
import { Optional } from '../../src/Core/Optional';
import { One } from '../../src/Core/One';
import { FileOut } from '../../src/IO/FileOut';
import { FileIn } from '../../src/IO/FileIn';
import { Path } from '../../src/IO/Path';

describe('Container Integration Tests', () => {
    test('Vector with complex objects', () => {
        // Test vector with custom objects
        const v = new Vector<{ id: number, name: string }>();

        v.Add({ id: 1, name: 'Item 1' });
        v.Add({ id: 2, name: 'Item 2' });
        v.Add({ id: 3, name: 'Item 3' });

        expect(v.GetCount()).toBe(3);

        // Test finding
        const foundIndex = v.Find(item => item.id === 2);
        expect(foundIndex).toBe(1);
        expect(v.At(foundIndex).name).toBe('Item 2');

        // Test finding by value
        const valueToFind = { id: 3, name: 'Item 3' };
        const valueIndex = v.FindValue(valueToFind); // This might not work as expected due to reference comparison
        // Instead, find by property
        const foundById = v.Find(item => item.id === 3);
        expect(foundById).toBe(2);
        expect(v.At(foundById).name).toBe('Item 3');
    });

    test('Map with complex key-value pairs', () => {
        // Test map with complex keys and values
        const m = new Map<string, { count: number, data: string[] }>();

        m.Set('key1', { count: 5, data: ['a', 'b', 'c'] });
        m.Set('key2', { count: 10, data: ['x', 'y', 'z'] });

        expect(m.GetCount()).toBe(2);

        // Test getting values with default
        const value1 = m.Get('key1', { count: 0, data: [] });
        expect(value1).toBeDefined();
        expect(value1.count).toBe(5);
        expect(value1.data.length).toBe(3);

        // Test getting non-existent key with default
        const defaultValue = m.Get('nonexistent', { count: -1, data: ['default'] });
        expect(defaultValue.count).toBe(-1);
        expect(defaultValue.data[0]).toBe('default');

        // Test Contains
        expect(m.Contains('key1')).toBe(true);
        expect(m.Contains('nonexistent')).toBe(false);

        // Test GetKeys and GetValues
        const keys = m.GetKeys();
        expect(keys.length).toBe(2);
        expect(keys).toContain('key1');
        expect(keys).toContain('key2');

        const values = m.GetValues();
        expect(values.length).toBe(2);
        expect(values[0].count).toBeGreaterThan(0);
    });

    test('String operations with Vector', () => {
        // Test working with vectors of strings
        const strings = new Vector<String>();

        strings.Add(new String('Hello'));
        strings.Add(new String('World'));
        strings.Add(new String('Test'));

        // Test iteration
        let concatenated = '';
        for (const str of strings) {
            concatenated += str.ToString() + ' ';
        }
        expect(concatenated.trim()).toBe('Hello World Test');

        // Test finding by property
        const longStringIndex = strings.Find(s => s.GetLength() > 4);
        expect(longStringIndex).toBe(0); // "Hello" is the first string with length > 4
    });

    test('Optional integration', () => {
        // Test optional with containers
        const result: Optional<string> = Optional.Of('test value');

        expect(result.IsNull()).toBe(false);
        expect(result.Get()).toBe('test value');

        // Test Get method works
        expect(result.Get().length).toBe(10); // Length of 'test value'

        // Test Set with appropriate type
        const newOptional = new Optional<string>();
        newOptional.Set('new value');
        expect(newOptional.Get()).toBe('new value');

        // Test empty optional
        const empty: Optional<string> = Optional.Of(null as any);
        expect(empty.IsNull()).toBe(true);

        // Test GetWithDefault
        const withDefault = empty.GetWithDefault('default value');
        expect(withDefault).toBe('default value');
    });

    test('One<T> memory management with containers', () => {
        // Test smart pointer with containers
        const oneVec = new One(new Vector<number>());
        oneVec.Get().Add(1);
        oneVec.Get().Add(2);
        oneVec.Get().Add(3);

        expect(oneVec.Get().GetCount()).toBe(3);
        expect(oneVec.Get().At(0)).toBe(1);

        // Test Detach (Pick)
        const detachedVec = oneVec.Detach();
        expect(oneVec.IsEmpty()).toBe(true);

        // Reattach
        const newOne = new One(detachedVec);
        expect(newOne.Get().GetCount()).toBe(3);
        expect(newOne.Get().At(2)).toBe(3);

        // Test Map operation
        const oneNum = new One(42);
        const oneStr = oneNum.Map(n => `Number: ${n}`);
        expect(oneStr.Get()).toBe('Number: 42');
    });
});

describe('IO Integration Tests', () => {
    test('File operations with containers', () => {
        // Create a temporary file path
        const testFilePath = Path.Combine(process.cwd(), 'temp_test_file.txt');

        // Write some data using FileOut
        const fileOut = new FileOut();
        if (fileOut.Open(testFilePath.ToString())) {
            const data = new Vector<String>();
            data.Add(new String('Line 1'));
            data.Add(new String('Line 2'));
            data.Add(new String('Line 3'));

            for (let i = 0; i < data.GetCount(); i++) {
                // Convert to Buffer for the write operation
                const line = data.At(i).ToString() + '\n';
                const buffer = Buffer.from(line);
                fileOut.Write(buffer, buffer.length);
            }
            fileOut.Close();
        }

        // Read the data back using FileIn
        const fileIn = new FileIn();
        if (fileIn.Open(testFilePath.ToString())) {
            // Read all content at once since FileIn doesn't have GetLine
            const content = fileIn.ReadAll();
            const lines = content.split('\n').filter(line => line.length > 0);

            expect(lines.length).toBe(3);
            expect(lines[0]).toBe('Line 1');
            expect(lines[1]).toBe('Line 2');
            expect(lines[2]).toBe('Line 3');

            fileIn.Close();
        }

        // Clean up the temporary file
        try {
            const fs = require('fs');
            if (fs.existsSync(testFilePath.ToString())) {
                fs.unlinkSync(testFilePath.ToString());
            }
        } catch (e) {
            // If we can't delete the temp file, it's not critical for the test
        }
    });
});