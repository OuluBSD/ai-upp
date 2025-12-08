#!/usr/bin/env python3
"""
Script to convert legacy Eon loop syntax to new router-based net syntax.

This script transforms .eon files from the old loop-based syntax to the new
net-based syntax with explicit port-to-port connections. It handles:

1. Converting 'loop' blocks to 'net' blocks
2. Adding explicit port connections for atoms in the loop
3. Preserving atom definitions and their parameters
4. Handling multi-loop chains by converting them appropriately

Usage:
    python convert_loop_to_net.py input.eon output.eon
"""

import re
import sys
import argparse
from typing import List


class EonLoopToNetConverter:
    """
    Converts legacy Eon loop syntax to new router net syntax.
    """
    
    def __init__(self):
        pass
        
    def parse_loop_content(self, content: str) -> List[str]:
        """
        Parse the content of a loop and extract atom definitions with their parameters.
        """
        atoms = []
        lines = content.strip().split('\n')
        
        current_atom = ""
        
        for line in lines:
            stripped = line.strip()
            
            # Skip empty lines and comments
            if not stripped or stripped.startswith('#') or stripped.startswith('//'):
                continue
            
            # Check if this line is an atom definition (contains a dot but not assignment)
            # An atom definition typically has format like 'center.customer' or 'audio.sink.test'
            is_atom = ('.' in stripped and 
                      '=' not in stripped.split()[0] if stripped.split() else False
                      and not any(stripped.startswith(kw) for kw in ['loop', 'chain', 'net', 'connections:', 'state']))
            
            if is_atom:
                # If we have a previous atom, save it
                if current_atom:
                    atoms.append(current_atom.strip())
                
                # Start new atom
                current_atom = line
            elif (stripped.startswith(' ') or stripped.startswith('\t')) and current_atom:
                # This is a parameter for the current atom (indented)
                current_atom += '\n' + line
            elif stripped and not is_atom:
                # This might be an assignment that belongs to the previous atom
                if current_atom and '=' in stripped:
                    current_atom += '\n' + line
                elif current_atom:
                    # This is a new top-level item, save current atom and start new
                    atoms.append(current_atom.strip())
                    current_atom = ""
        
        # Add the last atom if exists
        if current_atom:
            atoms.append(current_atom.strip())
        
        return atoms
    
    def generate_port_connections(self, atoms: List[str]) -> List[str]:
        """
        Generate explicit port connections for atoms in a loop (linear sequence).
        """
        connections = []

        # Only create connections if there are at least 2 atoms
        if len(atoms) < 2:
            return connections

        # Extract base atom names (without parameters) for connections
        base_atoms = []
        for atom in atoms:
            first_line = atom.split('\n')[0]  # Get the first line which contains the atom definition
            # Extract just the atom part (before any colon for parameters)
            atom_part = first_line.split(':')[0].strip()
            base_atoms.append(atom_part)

        # Create linear connections from first to last atom
        for i in range(len(base_atoms) - 1):
            connections.append(f"{base_atoms[i]}.0 -> {base_atoms[i+1]}.0")

        return connections
    
    def format_atom_block(self, atom_block: str) -> str:
        """
        Format an atom block properly with correct indentation for parameters.
        """
        lines = atom_block.split('\n')
        formatted_lines = [lines[0]]  # First line is the atom definition
        
        # Add parameters with proper indentation
        for line in lines[1:]:
            stripped = line.strip()
            if stripped and not stripped.startswith('#') and not stripped.startswith('//'):
                formatted_lines.append(f"    {stripped}")
        
        return '\n'.join(formatted_lines)

    def convert_file(self, input_file: str) -> str:
        """
        Convert an entire .eon file from loop syntax to net syntax.
        Uses a line-by-line parsing to handle nested constructs properly.
        """
        with open(input_file, 'r') as f:
            lines = f.readlines()
        
        result_lines = []
        i = 0
        
        while i < len(lines):
            line = lines[i]
            stripped = line.lstrip()
            indent = line[:len(line)-len(stripped)]  # Preserve original indentation
            
            # Check if this is a loop definition (only convert loops, not chains)
            loop_match = re.match(r'^(\s*)loop\s+([^:\n]+):', line)

            if loop_match:
                construct_indent = loop_match.group(1)
                construct_name = loop_match.group(2)

                # Find the content of this loop by looking for lines with greater indentation
                content_lines = []
                i += 1  # Move to the next line

                # Collect content lines (indented more than the construct)
                while i < len(lines):
                    current_line = lines[i]
                    current_stripped = current_line.lstrip()
                    current_indent = len(current_line) - len(current_line.lstrip())
                    construct_indent_len = len(construct_indent)

                    # If the line is not indented more than the construct, it's not part of this construct
                    if current_indent <= construct_indent_len:
                        break

                    content_lines.append(current_line)
                    i += 1

                # Process the content
                content_str = ''.join(content_lines)

                # Create the new net block
                result_lines.append(f"{construct_indent}net {construct_name}:\n")

                # Parse atoms in the content
                atoms = self.parse_loop_content(content_str)

                # Add atom definitions with proper formatting
                for atom in atoms:
                    formatted_atom = self.format_atom_block(atom)
                    # Add each line of the formatted_atom with proper indentation
                    for atom_line in formatted_atom.split('\n'):
                        if atom_line.strip():
                            result_lines.append(f"{construct_indent}    {atom_line}\n")

                # Add connections if there are multiple atoms
                if len(atoms) > 1:
                    connections = self.generate_port_connections(atoms)
                    if connections:
                        result_lines.append(f"{construct_indent}    # Explicit port-to-port connections\n")
                        for conn in connections:
                            result_lines.append(f"{construct_indent}    {conn}\n")
            else:
                # Check for chain definitions - don't convert them, but process their content for nested loops
                chain_match = re.match(r'^(\s*)chain\s+([^:\n]+):', line)
                if chain_match:
                    # Add the chain line as is
                    result_lines.append(line)
                    i += 1  # Move to the next line

                    # Process the content within the chain for any nested loops
                    chain_indent = chain_match.group(1)
                    chain_indent_len = len(chain_indent)

                    # Process nested content until we find a line that's not more indented
                    while i < len(lines):
                        current_line = lines[i]
                        current_stripped = current_line.lstrip()
                        current_indent = len(current_line) - len(current_line.lstrip())

                        if current_indent <= chain_indent_len:
                            # This line is not part of the chain's content, break and process normally
                            break

                        # Process this line recursively (it might contain loop definitions)
                        temp_line = current_line
                        temp_stripped = temp_line.lstrip()
                        temp_indent = len(temp_line) - len(temp_line.lstrip())

                        # Check if this line is a nested loop definition
                        nested_loop_match = re.match(r'^(\s*)loop\s+([^:\n]+):', temp_line)

                        if nested_loop_match and temp_indent > chain_indent_len:
                            # This is a nested loop inside the chain, handle it recursively
                            nested_construct_indent = nested_loop_match.group(1)
                            nested_construct_name = nested_loop_match.group(2)

                            # Find content of this nested loop
                            nested_content_lines = []
                            i += 1  # Move to next line

                            while i < len(lines):
                                next_line = lines[i]
                                next_stripped = next_line.lstrip()
                                next_indent = len(next_line) - len(next_line.lstrip())

                                if next_indent <= temp_indent:
                                    break

                                nested_content_lines.append(next_line)
                                i += 1

                            # Process the nested loop
                            nested_content_str = ''.join(nested_content_lines)
                            result_lines.append(f"{nested_construct_indent}net {nested_construct_name}:\n")

                            # Parse atoms in the nested content
                            nested_atoms = self.parse_loop_content(nested_content_str)

                            # Add atom definitions for nested loop
                            for atom in nested_atoms:
                                formatted_atom = self.format_atom_block(atom)
                                # Add each line of the formatted_atom with proper indentation
                                for atom_line in formatted_atom.split('\n'):
                                    if atom_line.strip():
                                        result_lines.append(f"{nested_construct_indent}    {atom_line}\n")

                            # Add connections if there are multiple atoms in nested loop
                            if len(nested_atoms) > 1:
                                connections = self.generate_port_connections(nested_atoms)
                                if connections:
                                    result_lines.append(f"{nested_construct_indent}    # Explicit port-to-port connections\n")
                                    for conn in connections:
                                        result_lines.append(f"{nested_construct_indent}    {conn}\n")
                        else:
                            # Regular line inside chain, just add it
                            result_lines.append(current_line)
                            i += 1
                else:
                    # Regular line, just add it
                    result_lines.append(line)
                    i += 1
        
        return ''.join(result_lines)
    
    def convert_file_from_string(self, content: str) -> str:
        """
        Helper function to convert content from a string (used for nested content).
        """
        # For now, just return the content as is with appropriate processing
        # This function can be expanded to handle nested conversion if needed
        return content

    def add_file_header_comment(self, original_content: str, new_content: str) -> str:
        """
        Add a header comment to indicate the file was converted from loop to net syntax.
        """
        header_comment = (
            "// CONVERTED FROM LOOP TO NET SYNTAX\n"
            "// This file was automatically converted from legacy loop-based syntax\n"
            "// to the new router-based net syntax with explicit port connections.\n"
            "// Review and adjust port connections as needed.\n"
            "\n"
        )
        
        return header_comment + new_content


def main():
    parser = argparse.ArgumentParser(description='Convert Eon loop syntax to net syntax')
    parser.add_argument('input', help='Input .eon file path')
    parser.add_argument('output', help='Output .eon file path')
    parser.add_argument('--dry-run', action='store_true', help='Print conversion without writing')
    
    args = parser.parse_args()
    
    converter = EonLoopToNetConverter()
    
    try:
        with open(args.input, 'r') as f:
            original_content = f.read()
        
        converted_content = converter.convert_file(args.input)
        final_content = converter.add_file_header_comment(original_content, converted_content)
        
        if args.dry_run:
            print("Converted content (not written to file):")
            print(final_content)
        else:
            with open(args.output, 'w') as f:
                f.write(final_content)
            print(f"Successfully converted {args.input} to {args.output}")
    
    except Exception as e:
        print(f"Error during conversion: {str(e)}")
        sys.exit(1)


if __name__ == "__main__":
    main()