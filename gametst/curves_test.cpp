#include <Core/Core.h>
#include <GameLib/GameLib.h>
#include <Test/Test.h>

using namespace UPP;

CONSOLE_APP_MAIN {
    // Initialize test framework
    InitTestMode();

    // Test PointCurve
    TEST("PointCurve basic functionality") {
        Vector<Point3> points;
        points.Add(Point3(0, 0, 0));
        points.Add(Point3(10, 0, 0));
        points.Add(Point3(10, 10, 0));
        
        PointCurve curve(points);
        
        // Test start point
        Point3 p0 = curve.Apply(0.0);
        TESTEQ(p0.x, 0.0);
        TESTEQ(p0.y, 0.0);
        TESTEQ(p0.z, 0.0);
        
        // Test mid point (should be at first segment midpoint)
        Point3 p05 = curve.Apply(0.5);
        TESTEQ(p05.x, 5.0);
        TEST(ApproxEqual(p05.y, 0.0, 0.1));
        
        // Test end point
        Point3 p1 = curve.Apply(1.0);
        TESTEQ(p1.x, 10.0);
        TESTEQ(p1.y, 10.0);
        TESTEQ(p1.z, 0.0);
    }

    // Test BezierCurve
    TEST("BezierCurve quadratic functionality") {
        BezierCurve bezier(Point3(0, 0, 0), Point3(5, 10, 0), Point3(10, 0, 0));
        
        // Test start point
        Point3 p0 = bezier.Apply(0.0);
        TESTEQ(p0.x, 0.0);
        TESTEQ(p0.y, 0.0);
        TESTEQ(p0.z, 0.0);
        
        // Test end point
        Point3 p1 = bezier.Apply(1.0);
        TESTEQ(p1.x, 10.0);
        TESTEQ(p1.y, 0.0);
        TESTEQ(p1.z, 0.0);
        
        // Test midpoint (should be influenced by control point)
        Point3 p05 = bezier.Apply(0.5);
        TEST(p05.y > 0.0); // Should be above the line due to control point
    }

    // Test CatmullRomCurve
    TEST("CatmullRomCurve basic functionality") {
        Vector<Point3> points;
        points.Add(Point3(0, 0, 0));
        points.Add(Point3(10, 0, 0));
        points.Add(Point3(10, 10, 0));
        points.Add(Point3(0, 10, 0));
        
        CatmullRomCurve curve(points);
        
        // Test start and end points
        Point3 p0 = curve.Apply(0.0);
        Point3 p1 = curve.Apply(1.0);
        
        TEST(ApproxEqual(p0.x, 0.0, 0.1));
        TEST(ApproxEqual(p1.x, 0.0, 0.1));
    }

    // Test CompositePath
    TEST("CompositePath basic functionality") {
        auto path = std::make_shared<CompositePath>();
        
        // Add a line
        path->AddLine(Point3(0, 0, 0), Point3(10, 0, 0));
        
        // Test the point on the path
        Point3 p0 = path->GetPoint(0.0);
        Point3 p05 = path->GetPoint(0.5);
        Point3 p1 = path->GetPoint(1.0);
        
        TESTEQ(p0.x, 0.0);
        TESTEQ(p05.x, 5.0);
        TESTEQ(p1.x, 10.0);
        
        // Test path length (approximately 10 units for this line)
        TEST(p0.0 < path->GetLength());
    }

    // Test PathFactory
    TEST("PathFactory CreateLinearPath") {
        Vector<Point3> points;
        points.Add(Point3(0, 0, 0));
        points.Add(Point3(1, 1, 0));
        points.Add(Point3(2, 2, 0));
        
        auto path = PathFactory::CreateLinearPath(points);
        
        Point3 p0 = path->GetPoint(0.0);
        Point3 p1 = path->GetPoint(1.0);
        
        TEST(ApproxEqual(p0.x, 0.0, 0.001));
        TEST(ApproxEqual(p0.y, 0.0, 0.001));
        TEST(ApproxEqual(p1.x, 2.0, 0.001));
        TEST(ApproxEqual(p1.y, 2.0, 0.001));
    }

    // Test PathFactory rectangle
    TEST("PathFactory CreateRectanglePath") {
        double width = 10.0, height = 5.0;
        auto path = PathFactory::CreateRectanglePath(width, height);
        
        Point3 p0 = path->GetPoint(0.0);  // Should be bottom-left
        Point3 p025 = path->GetPoint(0.25);  // Should be bottom-right
        Point3 p05 = path->GetPoint(0.5);  // Should be top-right
        
        TEST(ApproxEqual(p0.x, 0.0, 0.1));
        TEST(ApproxEqual(p0.y, 0.0, 0.1));
        TEST(ApproxEqual(p025.x, width, 0.1));
        TEST(ApproxEqual(p05.y, height, 0.1));
    }

    // Test curve with interpolation
    TEST("PointCurve with interpolation") {
        Vector<Point3> points;
        points.Add(Point3(0, 0, 0));
        points.Add(Point3(10, 0, 0));
        
        PointCurve curve(points);
        
        // Test with sine interpolation
        Point3 p05 = curve.Apply(0.5, Interpolations::sineInOut);
        
        // The result should still be in a reasonable range
        TEST(p05.x >= 0.0 && p05.x <= 10.0);
    }

    REPORT("Curve and Path tests completed");
}