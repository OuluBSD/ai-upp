// Test for stdsrc Core functionality

#include "Core/Core.h"

using namespace Upp;

CONSOLE_APP_MAIN {
    // Test String functionality
    {
        String str = "Hello, World!";
        ASSERT(str.GetLength() == 13);
        ASSERT(str[0] == 'H');
        ASSERT(str[5] == ',');
        ASSERT(str.Find("World") == 7);
        ASSERT(str.Left(5) == "Hello");
        ASSERT(str.Right(6) == "World!");
        
        String sub = str.Mid(7, 5);
        ASSERT(sub == "World");
        
        String appended = str + " Welcome";
        ASSERT(appended == "Hello, World! Welcome");
        
        RLOG("String tests passed!");
    }
    
    // Test WString functionality
    {
        WString wstr = "Hello, Wide World!";
        ASSERT(wstr.GetLength() == 18);
        ASSERT(!wstr.IsEmpty());
        ASSERT(wstr.Is());
        
        RLOG("WString tests passed!");
    }
    
    // Test Vector functionality (Vcont.h)
    {
        Vector<int> vec;
        vec.Add(1);
        vec.Add(2);
        vec.Add(3);
        
        ASSERT(vec.GetCount() == 3);
        ASSERT(vec[0] == 1);
        ASSERT(vec[1] == 2);
        ASSERT(vec[2] == 3);
        
        vec.Insert(0, 0);
        ASSERT(vec[0] == 0);
        ASSERT(vec.GetCount() == 4);
        
        vec.Remove(0);
        ASSERT(vec[0] == 1);
        ASSERT(vec.GetCount() == 3);
        
        RLOG("Vector tests passed!");
    }
    
    // Test Index functionality
    {
        Index<String> index;
        index.Add("First");
        index.Add("Second");
        index.Add("Third");
        
        ASSERT(index.GetCount() == 3);
        ASSERT(index.Find("Second") == 1);
        ASSERT(index.Find("Missing") == -1);
        ASSERT(index[1] == "Second");
        
        ASSERT(index.FindAdd("Fourth") == 3);
        ASSERT(index.GetCount() == 4);
        ASSERT(index[3] == "Fourth");
        
        RLOG("Index tests passed!");
    }
    
    // Test Map functionality
    {
        Map<String, int> map;
        map.Add("One", 1);
        map.Add("Two", 2);
        map.Add("Three", 3);
        
        ASSERT(map.GetCount() == 3);
        ASSERT(map.Get("Two") == 2);
        ASSERT(map.GetKey(1) == "Two");
        
        map.Set("Four", 4);
        ASSERT(map.GetCount() == 4);
        ASSERT(map.Get("Four") == 4);
        
        RLOG("Map tests passed!");
    }
    
    // Test Value functionality
    {
        Value val1 = 42;
        Value val2 = "Hello";
        Value val3 = 3.14;
        Value val4 = true;
        
        ASSERT(val1 == 42);
        ASSERT(val2 == "Hello");
        ASSERT(val3 == 3.14);
        ASSERT(val4 == true);
        
        ASSERT(val1.IsInt());
        ASSERT(val2.IsString());
        ASSERT(val3.IsDouble());
        ASSERT(val4.IsBool());
        
        RLOG("Value tests passed!");
    }
    
    // Test TimeDate functionality
    {
        Time now = GetSysTime();
        Time specific(2023, 10, 28, 12, 30, 45);
        
        ASSERT(specific.year == 2023);
        ASSERT(specific.month == 10);
        ASSERT(specific.day == 28);
        ASSERT(specific.hour == 12);
        ASSERT(specific.minute == 30);
        ASSERT(specific.second == 45);
        
        Time next = specific.Add(1, 0, 0); // Add 1 day
        ASSERT(next.day == 29 || (next.day == 1 && next.month == 11));
        
        RLOG("TimeDate tests passed!");
    }
    
    // Test Point functionality via Draw (if available)
    // Note: This would require linking with Draw module
    
    RLOG("All Core tests passed successfully!");
}