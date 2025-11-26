import { Path } from '../../src/IO/Path';

describe('Path', () => {
    test('constructor initializes with empty string', () => {
        const p = new Path();
        expect(p.ToString()).toBe('');
    });

    test('constructor initializes with provided path', () => {
        const p = new Path('/some/path/file.txt');
        expect(p.ToString()).toBe('/some/path/file.txt');
    });

    test('Set sets the path value', () => {
        const p = new Path();
        const result = p.Set('/new/path');
        expect(p.ToString()).toBe('/new/path');
        expect(result).toBe(p); // Should return this for chaining
    });

    test('GetDirectory returns directory part', () => {
        const p = new Path('/some/path/file.txt');
        expect(p.GetDirectory()).toBe('/some/path');
    });

    test('GetFileName returns file name with extension', () => {
        const p = new Path('/some/path/file.txt');
        expect(p.GetFileName()).toBe('file.txt');
    });

    test('GetFileNameWithoutExtension returns file name without extension', () => {
        const p = new Path('/some/path/file.txt');
        expect(p.GetFileNameWithoutExtension()).toBe('file');
    });

    test('GetExtension returns file extension', () => {
        const p = new Path('/some/path/file.txt');
        expect(p.GetExtension()).toBe('.txt');
    });

    test('ChangeExtension changes the file extension', () => {
        const p = new Path('/some/path/file.txt');
        const newP = p.ChangeExtension('.log');
        expect(newP.ToString()).toBe('/some/path/file.log');
    });

    test('Append appends a subpath', () => {
        const p = new Path('/some/path');
        const newP = p.Append('file.txt');
        expect(newP.ToString()).toMatch(/\/some\/path\/file\.txt$|\\some\\path\\file\.txt$/);
    });

    test('Normalize normalizes the path', () => {
        const p = new Path('/some/../other/path/./file.txt');
        const newP = p.Normalize();
        expect(newP.ToString()).toBe('/other/path/file.txt');
    });

    test('IsAbsolute checks if path is absolute', () => {
        const p1 = new Path('/absolute/path');
        expect(p1.IsAbsolute()).toBe(true);
        
        const p2 = new Path('relative/path');
        expect(p2.IsAbsolute()).toBe(false);
    });

    test('MakeAbsolute makes path absolute', () => {
        const p = new Path('relative/path');
        const newP = p.MakeAbsolute('/base/path');
        expect(newP.IsAbsolute()).toBe(true);
        expect(newP.ToString()).toMatch(/\/base\/path\/relative\/path$|\\base\\path\\relative\\path$/);
    });

    test('MakeRelative makes path relative', () => {
        const p = new Path('/base/path/relative/file.txt');
        const newP = p.MakeRelative('/base/path');
        expect(newP.ToString()).toBe('relative/file.txt');
    });

    test('IsSameAs compares paths', () => {
        const p1 = new Path('/path/to/file.txt');
        const p2 = new Path('/path/to/file.txt');
        expect(p1.IsSameAs(p2)).toBe(true);
    });

    test('Combine combines two paths', () => {
        const p = Path.Combine('/path', 'to/file.txt');
        expect(p.ToString()).toMatch(/\/path\/to\/file\.txt$|\\path\\to\\file\.txt$/);
    });

    test('GetSeparator returns path separator', () => {
        const separator = Path.GetSeparator();
        expect(typeof separator).toBe('string');
        expect(separator.length).toBe(1);
    });

    test('GetDelimiter returns path delimiter', () => {
        const delimiter = Path.GetDelimiter();
        expect(typeof delimiter).toBe('string');
        expect(delimiter.length).toBe(1);
    });

    test('IsValid checks if path is valid', () => {
        expect(Path.IsValid('valid/path/file.txt')).toBe(true);
        expect(Path.IsValid('../valid/path')).toBe(true);
        // Note: Invalid path checking might be limited and platform dependent
    });
});