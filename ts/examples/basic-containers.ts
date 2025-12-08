/**
 * Example 1: Basic container operations
 * Demonstrates core containers: Vector, Map, String
 */

import { Vector } from '../src/Core/Vector';
import { Map } from '../src/Core/Map';
import { String } from '../src/Core/String';

// Example: Managing a simple inventory system
function inventoryExample() {
    console.log('=== Inventory Management Example ===');
    
    // Create inventory items using Vector
    const items = new Vector<{ id: number, name: string, quantity: number }>();
    
    items.Add({ id: 1, name: 'Laptop', quantity: 5 });
    items.Add({ id: 2, name: 'Mouse', quantity: 15 });
    items.Add({ id: 3, name: 'Keyboard', quantity: 10 });
    
    console.log(`Total items in inventory: ${items.GetCount()}`);
    
    // Find item by ID
    const laptopIndex = items.Find(item => item.id === 1);
    if (laptopIndex !== -1) {
        const laptop = items.At(laptopIndex);
        console.log(`Found item: ${laptop.name}, Quantity: ${laptop.quantity}`);
    }
    
    // Use Map for quick lookup by ID
    const itemMap = new Map<number, { id: number, name: string, quantity: number }>();
    
    for (let i = 0; i < items.GetCount(); i++) {
        const item = items.At(i);
        itemMap.Set(item.id, item);
    }
    
    // Quick lookup
    const targetItem = itemMap.Get(2, { id: 0, name: 'Unknown', quantity: 0 });
    console.log(`Quick lookup result: ${targetItem.name}`);
    
    // String operations
    const category = new String('Electronics');
    const description = new String('Computer accessories');
    
    const fullDescription = category + ' - ' + description;
    console.log(`Category: ${fullDescription}`);
    
    console.log('Inventory example completed.\n');
}

// Example: Processing text data
function textProcessingExample() {
    console.log('=== Text Processing Example ===');
    
    const lines = new Vector<String>();
    lines.Add(new String('The quick brown fox'));
    lines.Add(new String('jumps over the lazy dog'));
    lines.Add(new String('This is a sample text'));
    
    // Count words in each line
    for (let i = 0; i < lines.GetCount(); i++) {
        const line = lines.At(i);
        const words = line.ToString().split(' ');
        console.log(`Line ${i+1} has ${words.length} words: "${line.ToString()}"`);
    }
    
    // Combine all lines
    let fullText = new String('');
    for (let i = 0; i < lines.GetCount(); i++) {
        fullText = fullText + lines.At(i) + new String(' ');
    }
    
    console.log(`Combined text length: ${fullText.GetLength()}`);
    console.log('Text processing example completed.\n');
}

// Run examples
inventoryExample();
textProcessingExample();

console.log('All basic examples completed!');