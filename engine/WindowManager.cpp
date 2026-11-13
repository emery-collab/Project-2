#include "WindowManager.h"

namespace ECSEngine {

WindowManager::WindowManager(unsigned int width, unsigned int height, const std::string& title)
    : mWindow(std::make_shared<sf::RenderWindow>(sf::VideoMode({width, height}), title)) {
        mWorldUnitsPerPixel = 1;
    }

std::shared_ptr<sf::RenderWindow> WindowManager::GetWindow() const { return mWindow; }

Rect WindowManager::GetWindowBounds() const { 
    sf::Vector2u wSize = mWindow->getSize();
    Point2D topL(0.f, 0.f);
    return Rect(topL,(float)wSize.x,(float)wSize.y); 
}

void WindowManager::DebugDrawRect(const Rect& r, int red, int green, int blue, int alpha) {
    sf::ConvexShape rectangle;
    rectangle.setPointCount(4);
    Point2D topLeft = r.topLeft;
    float width = r.width;
    float height = r.height;
    rectangle.setPoint(0, sf::Vector2f(topLeft.x, topLeft.y));
    rectangle.setPoint(1, sf::Vector2f(topLeft.x + width, topLeft.y));
    rectangle.setPoint(2, sf::Vector2f(topLeft.x + width, topLeft.y +height));
    rectangle.setPoint(3, sf::Vector2f(topLeft.x, topLeft.y + height));
   
    rectangle.setFillColor(sf::Color(red, green, blue, alpha));
    mWindow->draw(rectangle);

}

void WindowManager::DebugFrameRect(const Rect& r, int red, int green, int blue, int alpha) {
    sf::ConvexShape rectangle;
    rectangle.setPointCount(4);
    Point2D topLeft = r.topLeft;
    float width = r.width;
    float height = r.height;
    rectangle.setPoint(0, sf::Vector2f(topLeft.x, topLeft.y));
    rectangle.setPoint(1, sf::Vector2f(topLeft.x + width, topLeft.y));
    rectangle.setPoint(2, sf::Vector2f(topLeft.x + width, topLeft.y +height));
    rectangle.setPoint(3, sf::Vector2f(topLeft.x, topLeft.y + height));
   
    rectangle.setOutlineColor(sf::Color(red, green, blue, alpha));
    rectangle.setFillColor(sf::Color(0, 0, 0, 0));
    rectangle.setOutlineThickness(5);
    mWindow->draw(rectangle);
}

Rect WindowManager::WindowToWorld(const Rect& rect) const { 
    
    Point2D nTopLeft = WindowToWorld(rect.topLeft);
    float width = WindowToWorld(rect.width);
    float height = WindowToWorld(rect.height);
    return Rect(nTopLeft,width,height); 
}

Point2D WindowManager::WindowToWorld(const Point2D& pt) const { 
    sf::Vector2u wSize = mWindow->getSize();
    float width = (float)wSize.x;
    //std::cout << "\nwidth: " << width << "\n\n";
    float height = (float)wSize.y;
    //std::cout << "\nheight: " << height << "\n\n";
    Point2D windowP = Point2D(width,height);
    //std::cout << "\nWindowP: " << windowP << "\n\n";
    return mWorldCenter - ((windowP * 0.5f) * mWorldUnitsPerPixel) + (pt * mWorldUnitsPerPixel);
}

Rect WindowManager::WorldToWindow(const Rect& rect) const { 
    
    Point2D nTopLeft = WorldToWindow(rect.topLeft);
    float width = WorldToWindow(rect.width);
    float height = WorldToWindow(rect.height);
    return Rect(nTopLeft,width,height); 
}

Point2D WindowManager::WorldToWindow(const Point2D& pt) const {
    sf::Vector2u wSize = mWindow->getSize();
    float width = (float)wSize.x;
    //std::cout << "\nwidth: " << width << "\n\n";
    float height = (float)wSize.y;
    //std::cout << "\nheight: " << height << "\n\n";
    Point2D windowP = Point2D(width,height);
    //std::cout << "\nWindowP: " << windowP << "\n";
    return (pt - (mWorldCenter - (windowP * 0.5f) * mWorldUnitsPerPixel)) / mWorldUnitsPerPixel;
}

void WindowManager::SetCamera(const Point2D& worldPt, const Point2D& screenPt) {
    sf::Vector2u wSize = mWindow->getSize();
    float width = (float)wSize.x;
    //std::cout << "\nwidth: " << width << "\n\n";
    float height = (float)wSize.y;
    //std::cout << "\nheight: " << height << "\n\n";
    Point2D windowP = Point2D(width,height);
    //std::cout << "\nWindowP: " << windowP << "\n\n";
    mWorldCenter = worldPt - screenPt * mWorldUnitsPerPixel + windowP * 0.5f * mWorldUnitsPerPixel;
}

Point2D WindowManager::GetWorldCenter() const { return mWorldCenter; }

float WindowManager::GetWorldScale() const { return mWorldUnitsPerPixel; }

void WindowManager::SetWorldCenter(const Point2D& worldCenter) {
    mWorldCenter = worldCenter;
}

void WindowManager::SetWorldScale(float worldUnitsPerPixel) {
    mWorldUnitsPerPixel = worldUnitsPerPixel;
}

void WindowManager::SetWorldRect(const Rect& worldCameraRect) {
    
    Point2D nTopLeft = worldCameraRect.topLeft;
    float rWidth = worldCameraRect.width;
    float rHeight = worldCameraRect.height;

    sf::Vector2u wSize = mWindow->getSize();
    float wWidth = (float)wSize.x;
    float wHeight = (float)wSize.y;

    mWorldCenter = Point2D(nTopLeft.x + rWidth/2, nTopLeft.y + rHeight/2);
    mWorldUnitsPerPixel = std::max(rWidth/wWidth, rHeight/wHeight);
}

float WindowManager::WindowToWorld(float x) const { 
    return x * mWorldUnitsPerPixel;
}

float WindowManager::WorldToWindow(float x) const {
    return x / mWorldUnitsPerPixel;
}

}  // namespace ECSEngine
