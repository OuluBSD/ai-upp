#include <Core/Core.h>
#include <GameLib/GameLib.h>
#include <Test/Test.h>

using namespace UPP;

CONSOLE_APP_MAIN {
    // Initialize test framework
    InitTestMode();

    // Test ObjectSet
    TEST("ObjectSet basic functionality") {
        ObjectSet<String> set;
        
        // Add elements
        set.Add("first");
        set.Add("second");
        set.Add("third");
        
        // Test size
        TESTEQ(set.GetCount(), 3);
        
        // Test contains
        TEST(set.Contains("first"));
        TEST(set.Contains("second"));
        TEST(set.Contains("third"));
        TEST(!set.Contains("fourth"));
        
        // Test iteration
        int count = 0;
        for (const auto& item : set) {
            count++;
        }
        TESTEQ(count, 3);
        
        // Test removal
        TEST(set.Remove("second"));
        TESTEQ(set.GetCount(), 2);
        TEST(!set.Contains("second"));
        
        // Test clear
        set.Clear();
        TESTEQ(set.GetCount(), 0);
        TEST(set.IsEmpty());
    }

    // Test IntMap
    TEST("IntMap basic functionality") {
        IntMap<String> intMap;
        
        // Add elements
        intMap.Put(1, "one");
        intMap.Put(2, "two");
        intMap.Put(3, "three");
        
        // Test size
        TESTEQ(intMap.GetCount(), 3);
        
        // Test retrieval
        TESTEQ(intMap.Get(1), "one");
        TESTEQ(intMap.Get(2), "two");
        TESTEQ(intMap.Get(3), "three");
        TESTEQ(intMap.Get(4, "default"), "default");  // Non-existent key with default
        
        // Test contains
        TEST(intMap.Contains(1));
        TEST(intMap.Contains(2));
        TEST(!intMap.Contains(5));
        
        // Test removal
        TEST(intMap.Remove(2));
        TESTEQ(intMap.GetCount(), 2);
        TEST(!intMap.Contains(2));
        
        // Test clear
        intMap.Clear();
        TESTEQ(intMap.GetCount(), 0);
        TEST(intMap.IsEmpty());
    }

    // Test IntFloatMap
    TEST("IntFloatMap basic functionality") {
        IntFloatMap floatMap;
        
        // Add elements
        floatMap.Put(1, 1.5);
        floatMap.Put(2, 2.7);
        floatMap.Put(3, 3.14);
        
        // Test size
        TESTEQ(floatMap.GetCount(), 3);
        
        // Test retrieval
        TEST(ApproxEqual(floatMap.Get(1), 1.5, 0.001));
        TEST(ApproxEqual(floatMap.Get(2), 2.7, 0.001));
        TEST(ApproxEqual(floatMap.Get(3), 3.14, 0.001));
        TEST(ApproxEqual(floatMap.Get(4, -1.0), -1.0, 0.001));  // Non-existent key with default
        
        // Test contains
        TEST(floatMap.Contains(1));
        TEST(floatMap.Contains(2));
        TEST(!floatMap.Contains(5));
        
        // Test removal
        TEST(floatMap.Remove(2));
        TESTEQ(floatMap.GetCount(), 2);
        TEST(!floatMap.Contains(2));
        
        // Test clear
        floatMap.Clear();
        TESTEQ(floatMap.GetCount(), 0);
        TEST(floatMap.IsEmpty());
    }

    // Test IntIntMap
    TEST("IntIntMap basic functionality") {
        IntIntMap intIntMap;
        
        // Add elements
        intIntMap.Put(10, 100);
        intIntMap.Put(20, 200);
        intIntMap.Put(30, 300);
        
        // Test size
        TESTEQ(intIntMap.GetCount(), 3);
        
        // Test retrieval
        TESTEQ(intIntMap.Get(10), 100);
        TESTEQ(intIntMap.Get(20), 200);
        TESTEQ(intIntMap.Get(30), 300);
        TESTEQ(intIntMap.Get(40, -1), -1);  // Non-existent key with default
        
        // Test contains
        TEST(intIntMap.Contains(10));
        TEST(intIntMap.Contains(20));
        TEST(!intIntMap.Contains(50));
        
        // Test removal
        TEST(intIntMap.Remove(20));
        TESTEQ(intIntMap.GetCount(), 2);
        TEST(!intIntMap.Contains(20));
        
        // Test clear
        intIntMap.Clear();
        TESTEQ(intIntMap.GetCount(), 0);
        TEST(intIntMap.IsEmpty());
    }

    // Test ObjectMap
    TEST("ObjectMap basic functionality") {
        ObjectMap<String, int> objMap;
        
        // Add elements
        objMap.Put("first", 1);
        objMap.Put("second", 2);
        objMap.Put("third", 3);
        
        // Test size
        TESTEQ(objMap.GetCount(), 3);
        
        // Test retrieval
        TESTEQ(objMap.Get("first"), 1);
        TESTEQ(objMap.Get("second"), 2);
        TESTEQ(objMap.Get("third"), 3);
        TESTEQ(objMap.Get("fourth", -1), -1);  // Non-existent key with default
        
        // Test contains
        TEST(objMap.Contains("first"));
        TEST(objMap.Contains("second"));
        TEST(!objMap.Contains("fifth"));
        
        // Test removal
        TEST(objMap.Remove("second"));
        TESTEQ(objMap.GetCount(), 2);
        TEST(!objMap.Contains("second"));
        
        // Test clear
        objMap.Clear();
        TESTEQ(objMap.GetCount(), 0);
        TEST(objMap.IsEmpty());
    }

    // Test complex usage scenarios
    TEST("Complex usage with custom objects") {
        struct TestObj {
            int id;
            String name;
            
            TestObj() : id(0) {}
            TestObj(int i, const String& n) : id(i), name(n) {}
            
            bool operator==(const TestObj& other) const {
                return id == other.id && name == other.name;
            }
        };
        
        // Specialize hasher for TestObj to use in HashMap
        UPP::Hasher<TestObj> TestObjHasher = [](const TestObj& obj) -> unsigned {
            return UPP::GetHash(obj.id) ^ UPP::GetHash(obj.name);
        };
        
        // Create ObjectSet with custom object
        ObjectSet<TestObj> objSet;
        
        TestObj obj1(1, "test1");
        TestObj obj2(2, "test2");
        TestObj obj3(3, "test3");
        
        objSet.Add(obj1);
        objSet.Add(obj2);
        objSet.Add(obj3);
        
        TESTEQ(objSet.GetCount(), 3);
        TEST(objSet.Contains(obj1));
        TEST(objSet.Contains(obj2));
        TEST(objSet.Contains(obj3));
    }

    REPORT("Collection classes tests completed");
}