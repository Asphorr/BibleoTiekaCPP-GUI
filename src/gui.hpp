#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <unordered_map>

namespace gui {

// Forward declarations
class Widget;
class Window;

// Event types
enum class EventType {
    Click,
    TextChanged,
    KeyPress,
    MouseMove,
    WindowClose
};

// Event structure
struct Event {
    EventType type;
    Widget* source;
    std::unordered_map<std::string, std::string> data;
};

// Base widget class
class Widget {
protected:
    std::string id;
    int x, y, width, height;
    bool visible;
    bool enabled;
    Widget* parent;
    std::vector<std::unique_ptr<Widget>> children;
    
public:
    Widget(const std::string& id = "");
    virtual ~Widget() = default;
    
    // Property setters with method chaining
    Widget& setPosition(int x, int y);
    Widget& setSize(int width, int height);
    Widget& setVisible(bool visible);
    Widget& setEnabled(bool enabled);
    
    // Property getters
    std::string getId() const { return id; }
    int getX() const { return x; }
    int getY() const { return y; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    bool isVisible() const { return visible; }
    bool isEnabled() const { return enabled; }
    
    // Child management
    Widget& add(std::unique_ptr<Widget> child);
    Widget* find(const std::string& id);
    
    // Event handling
    using EventHandler = std::function<void(const Event&)>;
    Widget& on(EventType type, EventHandler handler);
    
    virtual void render() = 0;
    
protected:
    std::unordered_map<EventType, std::vector<EventHandler>> eventHandlers;
    void emit(const Event& event);
};

// Button widget
class Button : public Widget {
private:
    std::string text;
    
public:
    Button(const std::string& text = "", const std::string& id = "");
    
    Button& setText(const std::string& text);
    std::string getText() const { return text; }
    
    void render() override;
    void click();
};

// Label widget
class Label : public Widget {
private:
    std::string text;
    
public:
    Label(const std::string& text = "", const std::string& id = "");
    
    Label& setText(const std::string& text);
    std::string getText() const { return text; }
    
    void render() override;
};

// TextInput widget
class TextInput : public Widget {
private:
    std::string text;
    std::string placeholder;
    
public:
    TextInput(const std::string& placeholder = "", const std::string& id = "");
    
    TextInput& setText(const std::string& text);
    TextInput& setPlaceholder(const std::string& placeholder);
    std::string getText() const { return text; }
    std::string getPlaceholder() const { return placeholder; }
    
    void render() override;
};

// Layout managers
class Layout {
public:
    virtual ~Layout() = default;
    virtual void apply(Widget* container) = 0;
};

class VerticalLayout : public Layout {
private:
    int spacing;
    int padding;
    
public:
    VerticalLayout(int spacing = 10, int padding = 10);
    void apply(Widget* container) override;
};

class HorizontalLayout : public Layout {
private:
    int spacing;
    int padding;
    
public:
    HorizontalLayout(int spacing = 10, int padding = 10);
    void apply(Widget* container) override;
};

// Container widget
class Container : public Widget {
private:
    std::unique_ptr<Layout> layout;
    
public:
    Container(const std::string& id = "");
    
    Container& setLayout(std::unique_ptr<Layout> layout);
    void render() override;
};

// Window class
class Window : public Widget {
private:
    std::string title;
    bool running;
    static std::vector<Window*> windows;
    
public:
    Window(const std::string& title = "Window", int width = 800, int height = 600);
    ~Window();
    
    Window& setTitle(const std::string& title);
    std::string getTitle() const { return title; }
    
    void show();
    void close();
    void render() override;
    
    static void runEventLoop();
    static void stopEventLoop();
};

// Utility functions
namespace utils {
    // Create widgets with fluent interface
    template<typename T, typename... Args>
    std::unique_ptr<T> create(Args&&... args) {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }
}

} // namespace gui
