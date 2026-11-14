#pragma once

#include <unordered_map>

#include "MathUtil.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

namespace ECSEngine {

// Owns the window and converts between world and window (pixel) coordinates.
class WindowManager {
public:
    WindowManager(unsigned int width, unsigned int height, const std::string& title);
    ~WindowManager() = default;

    // Shared ownership: the window stays alive while anyone holds this pointer.
    std::shared_ptr<sf::RenderWindow> GetWindow() const;

    // Window size in pixels at (0, 0). A copy, so it won't update on resize.
    Rect GetWindowBounds() const;

    // Camera setters. Every later conversion uses the new camera.
    void SetCamera(const Point2D& worldPt, const Point2D& screenPt);  // worldPt is drawn at screenPt
    void SetWorldCenter(const Point2D& worldCenter);
    void SetWorldRect(const Rect& worldCameraRect);  // whole rect fits in the window
    void SetWorldScale(float worldUnitsPerPixel);
    Point2D GetWorldCenter() const;
    float GetWorldScale() const;

    // Conversions return copies. The float versions convert lengths only.
    float WindowToWorld(float x) const;
    float WorldToWindow(float x) const;
    Rect WindowToWorld(const Rect& rect) const;
    Rect WorldToWindow(const Rect& rect) const;
    Point2D WorldToWindow(const Point2D& pt) const;
    Point2D WindowToWorld(const Point2D& pt) const;

    // Debug: filled / outlined rect in window coordinates.
    void DebugDrawRect(const Rect& r, int red, int green, int blue, int alpha = 255);
    void DebugFrameRect(const Rect& r, int red, int green, int blue, int alpha = 255);

private:
    std::shared_ptr<sf::RenderWindow> mWindow;
    Point2D mWorldCenter;
    float mWorldUnitsPerPixel;

};

}  // namespace ECSEngine
