#include <Core/Core.h>
#include <GameLib/GameLib.h>
#include <Test/Test.h>

using namespace UPP;

CONSOLE_APP_MAIN {
    // Initialize test framework
    InitTestMode();

    // Test Linear interpolation
    TEST("Linear interpolation values") {
        double result1 = Interpolations::linear.Apply(0.0);
        double result2 = Interpolations::linear.Apply(0.5);
        double result3 = Interpolations::linear.Apply(1.0);
        
        TESTEQ(result1, 0.0);
        TESTEQ(result2, 0.5);
        TESTEQ(result3, 1.0);
    }

    // Test Sine interpolation
    TEST("Sine interpolation start/end values") {
        double result1 = Interpolations::sine.Apply(0.0);
        double result2 = Interpolations::sine.Apply(1.0);
        
        TESTEQ(result1, 0.0);
        TESTEQ(result2, 1.0);
    }

    // Test SineInOut smoothness
    TEST("SineInOut interpolation smoothness") {
        double result1 = Interpolations::sineInOut.Apply(0.0);
        double result2 = Interpolations::sineInOut.Apply(0.5);
        double result3 = Interpolations::sineInOut.Apply(1.0);
        
        TESTEQ(result1, 0.0);
        TESTEQ(result2, 0.5);
        TESTEQ(result3, 1.0);
    }

    // Test exponential interpolations
    TEST("Exponential in/out boundary values") {
        double expIn0 = Interpolations::expIn.Apply(0.0);
        double expIn1 = Interpolations::expIn.Apply(1.0);
        double expOut0 = Interpolations::expOut.Apply(0.0);
        double expOut1 = Interpolations::expOut.Apply(1.0);
        
        TESTEQ(expIn0, 0.0);
        TESTEQ(expIn1, 1.0);
        TESTEQ(expOut0, 0.0);
        TESTEQ(expOut1, 1.0);
    }

    // Test range interpolation
    TEST("Range interpolation") {
        double result1 = Interpolations::linear.Apply(10.0, 20.0, 0.0);  // Should be 10
        double result2 = Interpolations::linear.Apply(10.0, 20.0, 0.5);  // Should be 15
        double result3 = Interpolations::linear.Apply(10.0, 20.0, 1.0);  // Should be 20
        
        TESTEQ(result1, 10.0);
        TESTEQ(result2, 15.0);
        TESTEQ(result3, 20.0);
    }

    // Test custom power interpolation
    TEST("Power interpolation") {
        PowIn pow2(2.0);
        double result1 = pow2.Apply(0.0);  // Should be 0
        double result2 = pow2.Apply(0.5);  // Should be 0.25
        double result3 = pow2.Apply(1.0);  // Should be 1
        
        TESTEQ(result1, 0.0);
        TESTEQ(result2, 0.25);
        TESTEQ(result3, 1.0);
    }

    // Test inverse interpolation
    TEST("Inverse interpolation") {
        double original = 0.5;
        double interpolated = Interpolations::linear.Apply(original);
        double inverse = Interpolations::linear.Inverse(interpolated);
        
        TEST(ApproxEqual(original, inverse, 1e-10));
    }

    // Test bounce interpolation boundaries
    TEST("Bounce interpolation boundaries") {
        double bounceIn0 = Interpolations::bounceIn.Apply(0.0);
        double bounceIn1 = Interpolations::bounceIn.Apply(1.0);
        double bounceOut0 = Interpolations::bounceOut.Apply(0.0);
        double bounceOut1 = Interpolations::bounceOut.Apply(1.0);
        
        TESTEQ(bounceIn0, 0.0);
        TESTEQ(bounceIn1, 1.0);
        TESTEQ(bounceOut0, 0.0);
        TESTEQ(bounceOut1, 1.0);
    }
    
    // Final test summary
    REPORT("Interpolation utilities tests completed");
}