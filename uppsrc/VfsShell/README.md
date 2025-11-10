# VfsShell

VfsShell is a command-line interface for VFS (Virtual File System) operations in the U++ framework. It provides a shell-like environment for navigating and manipulating virtual file systems, similar to traditional Unix shells but operating within the VFS context.

## Architecture

The VfsShell implements a dual-path filesystem architecture:

1. **System filesystem** mounted at root (`/`) - Provides access to the actual host system files
2. **Virtual filesystem** mounted at `/vfs` - Provides access to internal VFS data structures

## Mount System

The VfsShell uses U++'s MountManager to provide unified access to both system and virtual files:

- System files are accessible via standard paths (e.g., `/home/user/documents/`)
- VFS data is accessible via the `/vfs/` prefix (e.g., `/vfs/project/data/`)
- The current working directory can be either system or VFS paths

## Commands

VfsShell provides various common shell commands that work with both system and VFS paths:

- `pwd` - Print working directory
- `cd [path]` - Change directory (works with both system and VFS paths)
- `ls [path]` - List directory contents
- `tree [path]` - Show directory tree structure
- `mkdir <path>` - Create directory
- `touch <path>` - Create or update file timestamp
- `rm <path>` - Remove file or directory
- `mv <src> <dst>` - Move/rename file or directory
- `link <src> <dst>` - Create symlink
- `export <vfs> <host>` - Export VFS content to host filesystem
- `cat [paths...]` - Display file contents
- `grep [-i] <pattern> [path]` - Search for pattern in files
- `head [-n N] [path]` - Show first N lines of file
- `tail [-n N] [path]` - Show last N lines of file
- `uniq [path]` - Show unique lines in file
- `count [path]` - Count lines in file
- `echo <data...>` - Display data

## Usage Examples

### Working with System Files
```bash
# List system directory
ls /home/user/

# Navigate to system directory
cd /tmp

# Show current directory
pwd
```

### Working with VFS
```bash
# List VFS directory
ls /vfs/

# Navigate to VFS directory
cd /vfs/project/src

# Show VFS directory structure
tree /vfs/project
```

### Mixing System and VFS Operations
```bash
# Copy from system to VFS
cp /home/user/file.txt /vfs/data/

# Export VFS data to system
export /vfs/project/config /tmp/config_backup
```

## Implementation Notes

The implementation properly handles both system paths and VFS paths through:

1. Path detection using the `/vfs/` prefix
2. Proper use of MountManager for filesystem operations
3. Consistent path handling and normalization
4. Integration with U++'s VFS system

## Future Improvements

- Full implementation of file operations (create, delete, move) for VFS paths
- Enhanced search and filtering capabilities
- Performance optimization for large directory listings
- Additional command-line utilities

This implementation replaces the original ~/Dev/VfsBoot/src/VfsShell/ functionality with a more integrated approach using U++'s native systems.