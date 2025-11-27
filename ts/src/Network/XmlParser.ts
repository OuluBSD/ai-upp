/**
 * XmlParser - XML parsing and generation similar to U++ XML functionality
 * Uses the xml2js library for XML operations
 */

// Note: This implementation will use the xml2js library which needs to be added as a dependency
// For now, we'll implement it using the built-in Node.js APIs where possible
// but a full implementation would typically use a library like xml2js or fast-xml-parser

/**
 * XMLNode - Represents an XML node with tag name, attributes, and children
 */
export class XmlNode {
    private _tagName: string;
    private _attributes: { [key: string]: string };
    private _children: XmlNode[];
    private _textContent: string;

    constructor(tagName: string) {
        this._tagName = tagName;
        this._attributes = {};
        this._children = [];
        this._textContent = '';
    }

    /**
     * Get the tag name of the node
     */
    GetTagName(): string {
        return this._tagName;
    }

    /**
     * Set the tag name of the node
     * @param tagName The new tag name
     */
    SetTagName(tagName: string): XmlNode {
        this._tagName = tagName;
        return this;
    }

    /**
     * Get the attributes of the node
     */
    GetAttributes(): { [key: string]: string } {
        return { ...this._attributes };
    }

    /**
     * Get an attribute value by name
     * @param name The attribute name
     */
    GetAttribute(name: string): string | undefined {
        return this._attributes[name];
    }

    /**
     * Set an attribute
     * @param name The attribute name
     * @param value The attribute value
     */
    SetAttribute(name: string, value: string): XmlNode {
        this._attributes[name] = value;
        return this;
    }

    /**
     * Remove an attribute
     * @param name The attribute name to remove
     */
    RemoveAttribute(name: string): XmlNode {
        delete this._attributes[name];
        return this;
    }

    /**
     * Add a child node
     * @param child The child node to add
     */
    AddChild(child: XmlNode): XmlNode {
        this._children.push(child);
        return this;
    }

    /**
     * Insert a child at a specific index
     * @param index The index at which to insert the child
     * @param child The child node to insert
     */
    InsertChild(index: number, child: XmlNode): XmlNode {
        this._children.splice(index, 0, child);
        return this;
    }

    /**
     * Remove a child at a specific index
     * @param index The index of the child to remove
     */
    RemoveChild(index: number): XmlNode {
        this._children.splice(index, 1);
        return this;
    }

    /**
     * Remove a specific child node
     * @param child The child node to remove
     */
    RemoveChildNode(child: XmlNode): XmlNode {
        const index = this._children.indexOf(child);
        if (index !== -1) {
            this._children.splice(index, 1);
        }
        return this;
    }

    /**
     * Get all children of this node
     */
    GetChildren(): XmlNode[] {
        return [...this._children];
    }

    /**
     * Get the number of children
     */
    GetChildCount(): number {
        return this._children.length;
    }

    /**
     * Get a child at a specific index
     * @param index The index of the child to get
     */
    GetChild(index: number): XmlNode | null {
        return index >= 0 && index < this._children.length ? this._children[index] : null;
    }

    /**
     * Find a child by tag name
     * @param tagName The tag name to find
     */
    FindChild(tagName: string): XmlNode | null {
        for (const child of this._children) {
            if (child.GetTagName() === tagName) {
                return child;
            }
        }
        return null;
    }

    /**
     * Find all children with a specific tag name
     * @param tagName The tag name to find
     */
    FindChildren(tagName: string): XmlNode[] {
        return this._children.filter(child => child.GetTagName() === tagName);
    }

    /**
     * Get the text content of the node
     */
    GetTextContent(): string {
        return this._textContent;
    }

    /**
     * Set the text content of the node
     * @param content The new text content
     */
    SetTextContent(content: string): XmlNode {
        this._textContent = content;
        return this;
    }

    /**
     * Convert the node to an XML string
     */
    ToXml(): string {
        let xml = `<${this._tagName}`;
        
        // Add attributes
        for (const [name, value] of Object.entries(this._attributes)) {
            xml += ` ${name}="${this.escapeXml(value)}"`;
        }
        
        // If node has no children and no text content, make it self-closing
        if (this._children.length === 0 && !this._textContent) {
            xml += `/>`;
        } else {
            xml += `>`;
            
            // Add text content first
            if (this._textContent) {
                xml += this.escapeXml(this._textContent);
            }
            
            // Add children
            for (const child of this._children) {
                xml += child.ToXml();
            }
            
            xml += `</${this._tagName}>`;
        }
        
        return xml;
    }

    /**
     * Escape XML special characters in a string
     * @param str The string to escape
     */
    private escapeXml(str: string): string {
        return str
            .replace(/&/g, '&amp;')
            .replace(/</g, '&lt;')
            .replace(/>/g, '&gt;')
            .replace(/"/g, '&quot;')
            .replace(/'/g, '&apos;');
    }

    /**
     * Escape XML special characters in a string
     * @param str The string to unescape
     */
    private unescapeXml(str: string): string {
        return str
            .replace(/&amp;/g, '&')
            .replace(/&lt;/g, '<')
            .replace(/&gt;/g, '>')
            .replace(/&quot;/g, '"')
            .replace(/&apos;/g, "'");
    }
}

/**
 * XmlDocument - Represents an XML document
 */
export class XmlDocument {
    private _root: XmlNode | null;

    constructor() {
        this._root = null;
    }

    /**
     * Get the root node of the document
     */
    GetRoot(): XmlNode | null {
        return this._root;
    }

    /**
     * Set the root node of the document
     * @param root The root node to set
     */
    SetRoot(root: XmlNode): XmlDocument {
        this._root = root;
        return this;
    }

    /**
     * Create and return an element with the given tag name
     * @param tagName The tag name for the new element
     */
    CreateElement(tagName: string): XmlNode {
        return new XmlNode(tagName);
    }

    /**
     * Load XML from a string
     * @param xml The XML string to parse
     * @returns A promise resolving to the parsed document
     */
    static async LoadFromString(xml: string): Promise<XmlDocument> {
        // In a complete implementation, this would use an XML parser like xml2js
        // For now, we'll use a simple implementation that can't handle all cases
        const doc = new XmlDocument();
        
        // This is a basic implementation that won't handle all XML features
        // A full implementation would require a proper XML parser
        doc._root = XmlDocument.parseSimpleXml(xml);
        
        return doc;
    }

    /**
     * Save the document to a string
     */
    SaveToString(): string {
        if (!this._root) {
            return '';
        }
        return this._root.ToXml();
    }

    /**
     * Simple XML parsing implementation
     * Note: This is a very basic implementation that only handles simple XML.
     * A complete implementation would use an XML library like xml2js or fast-xml-parser.
     * @param xml The XML string to parse
     */
    private static parseSimpleXml(xml: string): XmlNode | null {
        // Remove XML declaration if present
        xml = xml.replace(/^<\?xml[^>]*\?>\s*/, '');
        
        // Find the root tag
        const rootTagMatch = xml.match(/^<([a-zA-Z][^>\s]*)/);
        if (!rootTagMatch) {
            return null; // Invalid XML
        }
        
        const rootTagName = rootTagMatch[1];
        
        // Extract attributes
        const attrMatches = [...xml.matchAll(/(\w+)=["']([^"']*)["']/g)];
        const attributes: { [key: string]: string } = {};
        for (const [, name, value] of attrMatches) {
            attributes[name] = value;
        }
        
        // Create root node
        const root = new XmlNode(rootTagName);
        Object.entries(attributes).forEach(([name, value]) => {
            root.SetAttribute(name, value);
        });
        
        // Find the content between the opening and closing tags
        const endTag = `</${rootTagName}>`;
        const startClosingIndex = xml.lastIndexOf(endTag);
        if (startClosingIndex === -1) {
            throw new Error(`Unclosed tag: ${rootTagName}`);
        }
        
        const contentStart = xml.indexOf('>', rootTagMatch.index) + 1;
        const content = xml.substring(contentStart, startClosingIndex);
        
        // Determine if this is a self-closing tag
        const isSelfClosing = xml.substring(0, contentStart).endsWith('/>');
        if (!isSelfClosing) {
            // Process content: text content and child elements
            if (content.trim()) {
                // Check if content contains child elements
                const firstChildTagMatch = content.match(/<([a-zA-Z][^>\s]*)/);
                if (firstChildTagMatch) {
                    // This element contains child elements
                    root.SetTextContent(content.substring(0, firstChildTagMatch.index).trim());
                    // For a complete implementation, we would recursively parse child elements
                    // For now, we'll just add them as text content
                    root.SetTextContent(content.trim());
                } else {
                    // This element only contains text content
                    root.SetTextContent(content.trim());
                }
            }
        }
        
        return root;
    }
}


/**
 * XmlParser - Utility class for parsing XML
 */
export class XmlParser {
    /**
     * Parse an XML string to an XmlDocument
     * @param xml The XML string to parse
     * @returns Promise resolving to the parsed document
     */
    static async Parse(xml: string): Promise<XmlDocument> {
        return XmlDocument.LoadFromString(xml);
    }

    /**
     * Convert XML to a simplified object representation
     * @param xml The XML string to convert
     * @returns Promise resolving to a JavaScript object
     */
    static async ToObject(xml: string): Promise<any> {
        const doc = await XmlParser.Parse(xml);
        const root = doc.GetRoot();
        
        if (!root) {
            return null;
        }
        
        return XmlParser.xmlNodeToObject(root);
    }

    private static xmlNodeToObject(node: XmlNode): any {
        const result: any = {
            tagName: node.GetTagName(),
            attributes: node.GetAttributes(),
            textContent: node.GetTextContent()
        };
        
        const children = node.GetChildren();
        if (children.length > 0) {
            result.children = children.map(child => XmlParser.xmlNodeToObject(child));
        }
        
        return result;
    }

    /**
     * Convert an object to XML
     * @param obj The object to convert to XML
     * @param rootTagName The tag name for the root element
     * @returns XML string
     */
    static FromObject(obj: any, rootTagName: string = 'root'): string {
        const root = new XmlNode(rootTagName);
        XmlParser.objectToXmlNode(obj, root);
        return root.ToXml();
    }

    private static objectToXmlNode(obj: any, node: XmlNode): void {
        if (typeof obj === 'string' || typeof obj === 'number' || typeof obj === 'boolean') {
            node.SetTextContent(obj.toString());
            return;
        }
        
        if (obj.tagName) {
            node.SetTagName(obj.tagName);
        }
        
        if (obj.attributes && typeof obj.attributes === 'object') {
            Object.entries(obj.attributes as { [key: string]: string }).forEach(([key, value]) => {
                node.SetAttribute(key, value);
            });
        }
        
        if (obj.textContent) {
            node.SetTextContent(obj.textContent);
        }
        
        if (obj.children && Array.isArray(obj.children)) {
            for (const child of obj.children) {
                const childNode = new XmlNode('temp');
                XmlParser.objectToXmlNode(child, childNode);
                node.AddChild(childNode);
            }
        }
    }
}