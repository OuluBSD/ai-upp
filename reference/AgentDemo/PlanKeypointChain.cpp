#include "AgentDemo.h"


#if 0


You have an progamming goal: e.g. "c++: 3-class hello world in main"


#include <iostream>
#include <string>

// Class 1: provides "Hello"
class Hello {
public:
    std::string get() const {
        return "Hello";
    }
};

// Class 2: provides "World"
class World {
public:
    std::string get() const {
        return "World";
    }
};

// Class 3: combines Hello and World and prints the greeting
class Greeter {
    Hello hello_;
    World world_;
public:
    void greet() const {
        std::cout << hello_.get() << " " << world_.get() << "!" << std::endl;
    }
};

int main() {
    Greeter greeter;
    greeter.greet();  // prints: Hello World!
    return 0;
}




Here’s a simple 4-step decomposition (“action planner”) for
the task “print Hello World!” using our three classes.
For each intermediate goal we list the key predicates you’d need to track.

Goal 1: Construct Hello
    Predicates:
    • HelloCreated(hello)
    – true once Hello hello; has run

Goal 2: Construct World
    Predicates:
    • WorldCreated(world)
    – true once World world; has run

Goal 3: Construct Greeter
    Predicates:
    • GreeterCreated(greeter)
    – true once Greeter greeter; has run
    • GreeterHasHello(greeter, hello)
    – true if greeter holds a reference/member to hello
    • GreeterHasWorld(greeter, world)
    – true if greeter holds a reference/member to world

Goal 4: Invoke greet() → print
    Predicates:
    • GreetInvoked(greeter)
    – true once greeter.greet(); is called
    • OutputEmitted("Hello World!")
    – true once standard‐output contains “Hello World!”

You can collapse steps (e.g. 1+2 into “make dependencies”) if you only need 2–3 goals, or expand further for constructor return‐value checks, error states, etc.





---->
You have a GOAL:::: PRINT HELLO WRODL
AND THEN....
YOU FIND GOALS BEFOOOOR THAT
4...3..2.1
YOUUU NEEEDS A BRIDGE BETWEEN 4-->3, 3-->2, 2--->1
	DAT IS THE FORMAT OF LLM OUTPUT... DAT GIVES DAT FORMAT
		BRIDGES ARE TEMPLATES.... TO BRIDGE GOAL/KEYPOINTS
			PREDICATE OF GOAL 4 ---> (SPECIALIZED) TEMPLATE GOAL BEFORE GOAL 4


<----
- LAYERS
	1. high-level user interaction and expectations
		- time/parallel problem (phases + section of data in focus)
	2. data
	3. program procedural architecture (main + phases of program (e.g. init + usage + deinit))
		3.1	sub procedures (includes User interactions)
	4. data structure architecture
	5. data interation architecture
	6. data interaction functions

- start from single multi-layer-keypoint
	- 1. output "hello world"
	- 2. text="hello", text="world"
	- 3. init: 3 objects 3 different classes
	- 4. class1 class2 class3
	- 5. class1 -> [class2, class3]
	- 6. class1::*(...) -> [class2::*(...), class3::*(...)]

- templates
	- 1. "usage phase: output \"hello world\""
		--> 2. "output ordered to_string() of data vector [hello, world]"
				(separable texts, separable data)
		--> 3. "1 default template for program" == "init: 3 objects 3 different classes"
	- 3. "class-relations: usage" --> "MainProgram::Main"
		--> 4. "create a class1 which inherits MainProgram"
	- 4. "create separate classes for separable data, which owns data"
	- 5. "make virtual MainProgram (==class1) to own data (classes)
	- 6. "implement to_string() of class1 fields"




---->

- 4.goals is a reverse action-planner problem (starting from goal 4.)
	- "solve in time/parallel domain"


<----

- 3D-network
	- X-axis: TIME-DOMAIN
	- Y-axis: KEYPOINT-LAYER
	- Z-axis: PARALLEL

- Y-axis action planner: KEYPOINT-LAYER(1.), PARALLEL(text="hello world")
		- point A: TIME-DOMAIN(program-main-phase)
		- point B: TIME-DOMAIN(human-viewer-phase)
- X-axis action planner: TIME-DOMAIN(program-main-phase), KEYPOINT-LAYER(5.)
		- point A: PARALLEL(class1)
		- point B: PARALLEL()
- Z-axis action planner: TIME-DOMAIN(program-main-phase), PARALLEL(MainProgram)
		- point A: KEYPOINT-LAYER(1.)
		- point B: KEYPOINT-LAYER(2.)
	


---->

- KINDA:		multiple domains of atoms
- KINDA:		multiple paths -> add details (earlier will be kept valid)
- ACTUALLY:		try every axis & move freely in Y-axis KEYPOINT-LAYER (with own problems though)
- ACTUALLY:		find route forward in TIME-DOMAIN using TIME/VALUE-SUBSET
- ACTUALLY:		find route backward in TIME-DOMAIN using (VALUE/CONSTRAINT)*(CONSTRAINT/TIME)-SUBSET
					(convert a need to assertions and create assertions to earlier phases)
- REQUIRE:		vertical visit in KEYPOINT-LAYER in initial point and in goal point
- WTF:			moving in vertical domain... doesnt make much sense
				e.g. init ctor: X(0)Y(4)Z(0) -> x(0)Y(5)z(1)
					 add field: X(main:6)Y(6)Z(mainclass:1) -> X(init:0)Y(4)Z(mainclass:2)
- KINDA:		move freely sometimes just to prepare an action
- KINDA:		only some points are valid: add fields only in some "times"
- IN THE END:	make the program from all the visited paths
- MAIN POINT:	you have multiple paths, which are visited in the end to make the AST of program
- KINDA:		- you move in 3D-world as a "processor with registers" or a "last known cache + priority focus"
				- when you visit a node: your register gets a value set
				- pre & post conditions requires a value in the processor
				- the route network what processor visits forms the program AST
				- you kinda enable different domains of that processor in different Y-axis points
					- you need easy context translation thought
#endif
