// WindowManager tests. Opens a real window, so it needs a desktop session (not SSH/headless).
#include <cmath>
#include <iostream>

#include "WindowManager.h"

using ECSEngine::Point2D;
using ECSEngine::Rect;

static int gFailures = 0;

// Like assert, but prints the result and keeps going instead of stopping.
#define CHECK(cond)                                                          \
    do {                                                                     \
        if (cond) {                                                          \
            std::cout << "  pass: " #cond "\n";                              \
        } else {                                                             \
            std::cout << "  FAIL: " #cond " (line " << __LINE__ << ")\n";    \
            ++gFailures;                                                     \
        }                                                                    \
    } while (false)

// Floats rarely come out exactly equal, so compare with a small tolerance.
bool Near(float a, float b) { return std::fabs(a - b) < 0.001f; }
bool Near(const Point2D& a, const Point2D& b) { return Near(a.x, b.x) && Near(a.y, b.y); }

// Every test uses an 800x600 window, world center (100, 100), and 2 world units per pixel.
// So the window covers world x from -700 to 900 and world y from -500 to 700.
void Setup(ECSEngine::WindowManager& wm) {
    wm.SetWorldScale(2.0f);
    wm.SetWorldCenter(Point2D(100.0f, 100.0f));
}

void TestGettersReturnWhatWasSet(ECSEngine::WindowManager& wm) {
    std::cout << "GettersReturnWhatWasSet\n";
    Setup(wm);
    CHECK(Near(wm.GetWorldScale(), 2.0f));
    CHECK(Near(wm.GetWorldCenter(), Point2D(100.0f, 100.0f)));
}

void TestWorldCenterIsWindowCenter(ECSEngine::WindowManager& wm) {
    std::cout << "WorldCenterIsWindowCenter\n";
    Setup(wm);
    CHECK(Near(wm.WorldToWindow(Point2D(100.0f, 100.0f)), Point2D(400.0f, 300.0f)));
}

void TestWindowTopLeftInWorld(ECSEngine::WindowManager& wm) {
    std::cout << "WindowTopLeftInWorld\n";
    Setup(wm);
    CHECK(Near(wm.WindowToWorld(Point2D(0.0f, 0.0f)), Point2D(-700.0f, -500.0f)));
}

void TestRoundTrip(ECSEngine::WindowManager& wm) {
    std::cout << "RoundTrip\n";
    Setup(wm);
    Point2D p(37.0f, -12.0f);
    CHECK(Near(wm.WindowToWorld(wm.WorldToWindow(p)), p));
}

void TestDistancesOnlyScale(ECSEngine::WindowManager& wm) {
    std::cout << "DistancesOnlyScale\n";
    Setup(wm);
    CHECK(Near(wm.WorldToWindow(10.0f), 5.0f));   // 10 world units = 5 pixels
    CHECK(Near(wm.WindowToWorld(5.0f), 10.0f));
}

void TestRectConversion(ECSEngine::WindowManager& wm) {
    std::cout << "RectConversion\n";
    Setup(wm);
    Rect r = wm.WorldToWindow(Rect{Point2D(100.0f, 100.0f), 20.0f, 10.0f});
    CHECK(Near(r.topLeft, Point2D(400.0f, 300.0f)));
    CHECK(Near(r.width, 10.0f));
    CHECK(Near(r.height, 5.0f));
}

void TestSetCameraPinsPoint(ECSEngine::WindowManager& wm) {
    std::cout << "SetCameraPinsPoint\n";
    Setup(wm);
    wm.SetCamera(Point2D(0.0f, 0.0f), Point2D(0.0f, 0.0f));  // world origin at window top-left
    CHECK(Near(wm.WorldToWindow(Point2D(0.0f, 0.0f)), Point2D(0.0f, 0.0f)));
    CHECK(Near(wm.WorldToWindow(Point2D(20.0f, 0.0f)), Point2D(10.0f, 0.0f)));
}

void TestFractionalScale(ECSEngine::WindowManager& wm) {
    std::cout << "FractionalScale\n";
    wm.SetWorldScale(0.5f);  // zoomed in: 1 world unit = 2 pixels
    wm.SetWorldCenter(Point2D(100.0f, 100.0f));
    CHECK(Near(wm.WorldToWindow(10.0f), 20.0f));
    CHECK(Near(wm.WorldToWindow(Point2D(110.0f, 100.0f)), Point2D(420.0f, 300.0f)));
}

void TestFreshManagerScaleIsOne() {
    std::cout << "FreshManagerScaleIsOne\n";
    ECSEngine::WindowManager fresh(800, 600, "fresh");  // no Setup(): checks the constructor
    CHECK(Near(fresh.GetWorldScale(), 1.0f));
}

void TestWindowToWorldRect(ECSEngine::WindowManager& wm) {
    std::cout << "WindowToWorldRect\n";
    Setup(wm);
    Rect r = wm.WindowToWorld(Rect{Point2D(400.0f, 300.0f), 10.0f, 5.0f});
    CHECK(Near(r.topLeft, Point2D(100.0f, 100.0f)));
    CHECK(Near(r.width, 20.0f));
    CHECK(Near(r.height, 10.0f));
}

void TestSetCameraNonZeroY(ECSEngine::WindowManager& wm) {
    std::cout << "SetCameraNonZeroY\n";
    Setup(wm);
    wm.SetCamera(Point2D(50.0f, 80.0f), Point2D(200.0f, 150.0f));
    CHECK(Near(wm.WorldToWindow(Point2D(50.0f, 80.0f)), Point2D(200.0f, 150.0f)));
    CHECK(Near(wm.WorldToWindow(Point2D(70.0f, 100.0f)), Point2D(210.0f, 160.0f)));
}

// SetWorldRect: one scale, "fit the whole rect inside" (the larger of the two scales).
void TestSetWorldRectSquare(ECSEngine::WindowManager& wm) {
    std::cout << "SetWorldRectSquare\n";
    wm.SetWorldRect(Rect{Point2D(0.0f, 0.0f), 100.0f, 100.0f});
    CHECK(Near(wm.GetWorldScale(), 100.0f / 600.0f));  // height is the tighter fit
    CHECK(Near(wm.GetWorldCenter(), Point2D(50.0f, 50.0f)));
    CHECK(Near(wm.WorldToWindow(Point2D(0.0f, 0.0f)), Point2D(100.0f, 0.0f)));      // rect's top-left
    CHECK(Near(wm.WorldToWindow(Point2D(100.0f, 100.0f)), Point2D(700.0f, 600.0f))); // rect's bottom-right
}

void TestSetWorldRectWide(ECSEngine::WindowManager& wm) {
    std::cout << "SetWorldRectWide\n";
    wm.SetWorldRect(Rect{Point2D(-200.0f, 0.0f), 1600.0f, 300.0f});
    CHECK(Near(wm.GetWorldScale(), 2.0f));  // width is the tighter fit
    CHECK(Near(wm.GetWorldCenter(), Point2D(600.0f, 150.0f)));
}

void TestSetWorldRectIgnoresOldCamera(ECSEngine::WindowManager& wm) {
    std::cout << "SetWorldRectIgnoresOldCamera\n";
    wm.SetWorldScale(5.0f);
    wm.SetWorldCenter(Point2D(999.0f, -999.0f));  // a camera that has nothing to do with the rect
    wm.SetWorldRect(Rect{Point2D(0.0f, 0.0f), 100.0f, 100.0f});
    CHECK(Near(wm.GetWorldScale(), 100.0f / 600.0f));
    CHECK(Near(wm.GetWorldCenter(), Point2D(50.0f, 50.0f)));
}

// Assumes your choice: GetWindowBounds returns the window in window (pixel) coordinates.
void TestWindowBoundsInPixels(ECSEngine::WindowManager& wm) {
    std::cout << "WindowBoundsInPixels\n";
    Rect b = wm.GetWindowBounds();
    CHECK(Near(b.topLeft, Point2D(0.0f, 0.0f)));
    CHECK(Near(b.width, 800.0f));
    CHECK(Near(b.height, 600.0f));
}


int main() {
    ECSEngine::WindowManager wm(800, 600, "WindowManager test");

    TestGettersReturnWhatWasSet(wm);
    TestWorldCenterIsWindowCenter(wm);
    TestWindowTopLeftInWorld(wm);
    TestRoundTrip(wm);
    TestDistancesOnlyScale(wm);
    TestRectConversion(wm);
    TestSetCameraPinsPoint(wm);
    TestFractionalScale(wm);
    TestFreshManagerScaleIsOne();
    TestWindowToWorldRect(wm);
    TestSetCameraNonZeroY(wm);
    TestSetWorldRectSquare(wm);
    TestSetWorldRectWide(wm);
    TestSetWorldRectIgnoresOldCamera(wm);
    TestWindowBoundsInPixels(wm);

    std::cout << (gFailures == 0 ? "All tests passed\n" : "Some tests FAILED\n");
    return gFailures == 0 ? 0 : 1;
}