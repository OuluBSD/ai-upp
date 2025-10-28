#pragma once
#ifndef _CtrlLib_ChatCtrl_h_
#define _CtrlLib_ChatCtrl_h_

#include "CtrlLib.h"
#include "Ctrl.h"
#include "Label.h"
#include "EditField.h"
#include "Button.h"
#include <functional>
#include <vector>
#include <memory>

// Chat message structure
struct ChatMessage {
    String sender;
    String text;
    Time timestamp;
    String avatar; // Path to avatar image
    bool isOwnMessage; // Whether it's the user's own message
    
    ChatMessage() : timestamp(GetSysTime()), isOwnMessage(false) {}
    ChatMessage(const String& s, const String& t, bool own = false) 
        : sender(s), text(t), timestamp(GetSysTime()), isOwnMessage(own) {}
};

// Chat control for displaying conversation
class ChatCtrl : public Ctrl {
private:
    Vector<ChatMessage> messages;
    std::shared_ptr<Ctrl> message_list; // Could be a custom list control
    std::shared_ptr<EditField> input_field;
    std::shared_ptr<Button> send_button;
    std::function<void(const String&)> when_send;
    std::function<void(const ChatMessage&)> when_message;
    int max_messages;
    bool auto_scroll;
    
public:
    ChatCtrl() : max_messages(1000), auto_scroll(true) {
        // Initialize UI components
        input_field = std::make_shared<EditField>();
        send_button = std::make_shared<Button>();
        send_button->SetLabel("Send");
        
        // Layout the components
        CtrlChild(*this)
            .Add(input_field, 0, 0, 200, 24)
            .Add(send_button, 210, 0, 60, 24);
        
        // Connect events
        send_button->WhenAction = [this]() { 
            if (input_field && !input_field->GetText().IsEmpty()) {
                SendMessage(input_field->GetText());
                input_field->SetText("");
            }
        };
        
        input_field->WhenEnter = [this]() { 
            if (!input_field->GetText().IsEmpty()) {
                SendMessage(input_field->GetText());
                input_field->SetText("");
            }
        };
    }
    
    virtual void Paint(Draw& draw) const override {
        Rect r = GetRect();
        
        // Draw background
        draw.DrawRect(r, GetBackgroundColor());
        
        // Draw messages
        DrawMessages(draw, r);
        
        // Draw child controls
        Ctrl::Paint(draw);
    }
    
    // Add a message to the chat
    ChatCtrl& AddMessage(const String& sender, const String& text, bool isOwn = false) {
        ChatMessage msg(sender, text, isOwn);
        messages.Add(msg);
        
        // Enforce message limit
        if (max_messages > 0 && messages.GetCount() > max_messages) {
            messages.Remove(0);
        }
        
        if (when_message) when_message(msg);
        
        if (auto_scroll) {
            ScrollToBottom();
        }
        
        Refresh();
        return *this;
    }
    
    // Add own message (from user)
    ChatCtrl& AddOwnMessage(const String& text) {
        return AddMessage("Me", text, true);
    }
    
    // Send a message (triggers when_send event)
    ChatCtrl& SendMessage(const String& text) {
        AddOwnMessage(text);
        if (when_send) when_send(text);
        return *this;
    }
    
    // Get all messages
    const Vector<ChatMessage>& GetMessages() const { return messages; }
    
    // Clear all messages
    ChatCtrl& ClearMessages() {
        messages.Clear();
        Refresh();
        return *this;
    }
    
    // Event handlers
    ChatCtrl& WhenSend(std::function<void(const String&)> h) { 
        when_send = h; 
        return *this; 
    }
    
    ChatCtrl& WhenMessage(std::function<void(const ChatMessage&)> h) { 
        when_message = h; 
        return *this; 
    }
    
    // Configuration
    ChatCtrl& MaxMessages(int n) { 
        max_messages = n; 
        return *this; 
    }
    
    ChatCtrl& AutoScroll(bool b = true) { 
        auto_scroll = b; 
        return *this; 
    }
    
    // Input field access
    EditField& GetInputField() { 
        return *input_field; 
    }
    
    const EditField& GetInputField() const { 
        return *input_field; 
    }
    
    // Send button access
    Button& GetSendButton() { 
        return *send_button; 
    }
    
    const Button& GetSendButton() const { 
        return *send_button; 
    }
    
    // Message formatting
    ChatCtrl& FormatMessage(std::function<String(const ChatMessage&)> formatter) {
        // In a real implementation, this would set message formatting
        return *this;
    }
    
    // System messages
    ChatCtrl& AddSystemMessage(const String& text) {
        ChatMessage msg;
        msg.sender = "System";
        msg.text = text;
        msg.timestamp = GetSysTime();
        msg.isOwnMessage = false;
        
        messages.Add(msg);
        
        // Enforce message limit
        if (max_messages > 0 && messages.GetCount() > max_messages) {
            messages.Remove(0);
        }
        
        if (when_message) when_message(msg);
        
        if (auto_scroll) {
            ScrollToBottom();
        }
        
        Refresh();
        return *this;
    }
    
    // Scroll to show the latest message
    ChatCtrl& ScrollToBottom() {
        // In a real implementation, this would scroll to the bottom
        // of the message list using scroll controls if available
        return *this;
    }
    
    // Find messages by sender
    Vector<ChatMessage> GetMessagesBySender(const String& sender) const {
        Vector<ChatMessage> result;
        for (const auto& msg : messages) {
            if (msg.sender == sender) {
                result.Add(msg);
            }
        }
        return result;
    }
    
    // Get message count
    int GetMessageCount() const { return messages.GetCount(); }
    
    // Save chat to string (for logs, etc.)
    String ToString() const {
        String result;
        for (const auto& msg : messages) {
            result += FormatDateTime(msg.timestamp, "%H:%M:%S") + " " + 
                      msg.sender + ": " + msg.text + "\r\n";
        }
        return result;
    }
    
    // Save chat to file
    bool SaveToFile(const String& filename) const {
        return SaveFile(filename, ToString());
    }
    
    // Load chat from file (simple format)
    bool LoadFromFile(const String& filename) {
        String content = LoadFile(filename);
        if (content.IsEmpty()) return false;
        
        // This would parse the file content back into messages
        // Implementation would depend on the exact format
        return true;
    }
    
private:
    void DrawMessages(Draw& draw, const Rect& bounds) const {
        // In a real implementation, this would draw the chat messages
        // with proper formatting, avatars, timestamps, etc.
        
        // For now, we'll just draw a simple representation
        int y_pos = bounds.top + 5;
        int line_height = 20;
        
        for (int i = 0; i < messages.GetCount(); i++) {
            const ChatMessage& msg = messages[i];
            
            // Draw sender
            String sender_text = msg.sender + ":";
            draw.DrawText(bounds.left + 5, y_pos, sender_text, StdFont(), 
                         msg.isOwnMessage ? Color::Blue() : Color::Black());
            
            // Draw message text
            draw.DrawText(bounds.left + 80, y_pos, msg.text, StdFont(), 
                         msg.isOwnMessage ? Color::Blue() : Color::Black());
            
            // Draw timestamp
            String time_text = Format(msg.timestamp, "%H:%M");
            draw.DrawText(bounds.right - 50, y_pos, time_text, StdFont(), Color::Gray());
            
            y_pos += line_height;
            if (y_pos > bounds.bottom - line_height) break; // Don't draw outside bounds
        }
    }
};

// RichChatCtrl with more features for formatted messages
class RichChatCtrl : public ChatCtrl {
private:
    bool enable_formatting;
    bool show_timestamps;
    bool show_avatars;
    
public:
    RichChatCtrl() : enable_formatting(true), show_timestamps(true), show_avatars(true) {}
    
    RichChatCtrl& EnableFormatting(bool b = true) { 
        enable_formatting = b; 
        return *this; 
    }
    
    RichChatCtrl& ShowTimestamps(bool b = true) { 
        show_timestamps = b; 
        return *this; 
    }
    
    RichChatCtrl& ShowAvatars(bool b = true) { 
        show_avatars = b; 
        return *this; 
    }
    
    // Add a message with formatting
    RichChatCtrl& AddFormattedMessage(const String& sender, const String& html_text, bool isOwn = false) {
        ChatMessage msg(sender, html_text, isOwn);
        msg.text = html_text; // Store as HTML
        // In a real implementation, this would handle HTML formatting
        
        const_cast<RichChatCtrl*>(this)->messages.Add(msg);
        
        // Enforce message limit
        if (this->max_messages > 0 && this->messages.GetCount() > this->max_messages) {
            const_cast<RichChatCtrl*>(this)->messages.Remove(0);
        }
        
        if (this->when_message) this->when_message(msg);
        
        if (this->auto_scroll) {
            ScrollToBottom();
        }
        
        Refresh();
        return *this;
    }
    
    // Add a message with an image
    RichChatCtrl& AddImageMessage(const String& sender, const Image& img, bool isOwn = false) {
        ChatMessage msg(sender, "[Image]", isOwn);
        // In a real implementation, this would store the image with the message
        (void)img; // Use parameter to avoid warning
        
        const_cast<RichChatCtrl*>(this)->messages.Add(msg);
        
        if (this->max_messages > 0 && this->messages.GetCount() > this->max_messages) {
            const_cast<RichChatCtrl*>(this)->messages.Remove(0);
        }
        
        if (this->when_message) this->when_message(msg);
        
        if (this->auto_scroll) {
            ScrollToBottom();
        }
        
        Refresh();
        return *this;
    }
    
    // File transfer message
    RichChatCtrl& AddFileMessage(const String& sender, const String& filename, int64_t size, bool isOwn = false) {
        String size_str = FormatSize(size);
        ChatMessage msg(sender, "[File] " + filename + " (" + size_str + ")", isOwn);
        
        const_cast<RichChatCtrl*>(this)->messages.Add(msg);
        
        if (this->max_messages > 0 && this->messages.GetCount() > this->max_messages) {
            const_cast<RichChatCtrl*>(this)->messages.Remove(0);
        }
        
        if (this->when_message) this->when_message(msg);
        
        if (this->auto_scroll) {
            ScrollToBottom();
        }
        
        Refresh();
        return *this;
    }
};

// ChannelChatCtrl for multiple channels/chats
class ChannelChatCtrl : public Ctrl {
private:
    Vector<String> channels;
    String current_channel;
    Vector<std::shared_ptr<ChatCtrl>> channel_chats;
    
public:
    ChannelChatCtrl() = default;
    
    // Add a channel
    ChannelChatCtrl& AddChannel(const String& name) {
        if (channels.Find(name) < 0) {
            channels.Add(name);
            auto chat = std::make_shared<ChatCtrl>();
            channel_chats.Add(chat);
        }
        return *this;
    }
    
    // Switch to a channel
    ChannelChatCtrl& SetChannel(const String& name) {
        int idx = channels.Find(name);
        if (idx >= 0) {
            current_channel = name;
            // In a real implementation, this would show the selected channel's chat
        }
        return *this;
    }
    
    // Get current channel's chat control
    ChatCtrl* GetCurrentChat() {
        int idx = channels.Find(current_channel);
        return idx >= 0 ? channel_chats[idx].get() : nullptr;
    }
    
    // Send message to current channel
    ChannelChatCtrl& SendMessage(const String& text) {
        ChatCtrl* chat = GetCurrentChat();
        if (chat) {
            chat->SendMessage(text);
        }
        return *this;
    }
    
    // Get list of channels
    const Vector<String>& GetChannels() const { return channels; }
    const String& GetCurrentChannel() const { return current_channel; }
};

// Helper functions
inline std::shared_ptr<ChatCtrl> CreateChatCtrl() {
    return std::make_shared<ChatCtrl>();
}

inline std::shared_ptr<RichChatCtrl> CreateRichChatCtrl() {
    return std::make_shared<RichChatCtrl>();
}

inline std::shared_ptr<ChannelChatCtrl> CreateChannelChatCtrl() {
    return std::make_shared<ChannelChatCtrl>();
}

#endif