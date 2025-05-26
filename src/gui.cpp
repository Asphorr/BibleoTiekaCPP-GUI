#include "gui.hpp"
#include <iostream>
#include <algorithm>

namespace gui {

// Static member initialization
std::vector<Window*> Window::windows;

// Widget implementation
Widget::Widget(const std::string& id) 
    : id(id), x(0), y(0), width(100), height(30), 
      visible(true), enabled(true), parent(nullptr) {}

Widget& Widget::setPosition(int x, int y) {
    this->x = x;
    this->y = y;
    return *this;
}

Widget& Widget::setSize(int width, int height) {
    this->width = width;
    this->height = height;
    return *this;
}

Widget& Widget::setVisible(bool visible) {
    this->visible = visible;
    return *this;
}

Widget& Widget::setEnabled(bool enabled) {
    this->enabled = enabled;
    return *this;
}

Widget& Widget::add(std::unique_ptr<Widget> child) {
    child->parent = this;
    children.push_back(std::move(child));
    return *this;
}

Widget* Widget::find(const std::string& id) {
    if (this->id == id) return this;
    
    for (auto& child : children) {
        if (Widget* found = child->find(id)) {
            return found;
        }
    }
    return nullptr;
}

Widget& Widget::on(EventType type, EventHandler handler) {
    eventHandlers[type].push_back(handler);
    return *this;
}

void Widget::emit(const Event& event) {
    auto it = eventHandlers.find(event.type);
    if (it != eventHandlers.end()) {
        for (auto& handler : it->second) {
            handler(event);
        }
    }
    
    // Bubble up to parent
    if (parent) {
        parent->emit(event);
    }
}

// Button implementation
Button::Button(const std::string& text, const std::string& id) 
    : Widget(id), text(text) {}

Button& Button::setText(const std::string& text) {
    this->text = text;
    return *this;
}

void Button::render() {
    if (!visible) return;
    std::cout << "[Button: " << text << " at (" << x << ", " << y << ")]" << std::endl;
}

void Button::click() {
    if (!enabled) return;
    Event event{EventType::Click, this, {}};
    emit(event);
}

// Label implementation
Label::Label(const std::string& text, const std::string& id) 
    : Widget(id), text(text) {}

Label& Label::setText(const std::string& text) {
    this->text = text;
    return *this;
}

void Label::render() {
    if (!visible) return;
    std::cout << "[Label: " << text << " at (" << x << ", " << y << ")]" << std::endl;
}

// TextInput implementation
TextInput::TextInput(const std::string& placeholder, const std::string& id) 
    : Widget(id), placeholder(placeholder) {}

TextInput& TextInput::setText(const std::string& text) {
    this->text = text;
    Event event{EventType::TextChanged, this, {{"text", text}}};
    emit(event);
    return *this;
}

TextInput& TextInput::setPlaceholder(const std::string& placeholder) {
    this->placeholder = placeholder;
    return *this;
}

void TextInput::render() {
    if (!visible) return;
    std::string display = text.empty() ? placeholder : text;
    std::cout << "[TextInput: \"" << display << "\" at (" << x << ", " << y << ")]" << std::endl;
}

// VerticalLayout implementation
VerticalLayout::VerticalLayout(int spacing, int padding) 
    : spacing(spacing), padding(padding) {}

void VerticalLayout::apply(Widget* container) {
    int currentY = padding;
    
    for (auto& child : container->children) {
        child->setPosition(padding, currentY);
        currentY += child->getHeight() + spacing;
    }
}

// HorizontalLayout implementation
HorizontalLayout::HorizontalLayout(int spacing, int padding) 
    : spacing(spacing), padding(padding) {}

void HorizontalLayout::apply(Widget* container) {
    int currentX = padding;
    
    for (auto& child : container->children) {
        child->setPosition(currentX, padding);
        currentX += child->getWidth() + spacing;
    }
}

// Container implementation
Container::Container(const std::string& id) : Widget(id) {}

Container& Container::setLayout(std::unique_ptr<Layout> layout) {
    this->layout = std::move(layout);
    if (this->layout) {
        this->layout->apply(this);
    }
    return *this;
}

void Container::render() {
    if (!visible) return;
    std::cout << "[Container at (" << x << ", " << y << ")]" << std::endl;
    for (auto& child : children) {
        child->render();
    }
}

// Window implementation
Window::Window(const std::string& title, int width, int height) 
    : Widget("window"), title(title), running(false) {
    setSize(width, height);
    windows.push_back(this);
}

Window::~Window() {
    windows.erase(std::remove(windows.begin(), windows.end(), this), windows.end());
}

Window& Window::setTitle(const std::string& title) {
    this->title = title;
    return *this;
}

void Window::show() {
    running = true;
    std::cout << "=== Window: " << title << " (" << width << "x" << height << ") ===" << std::endl;
    render();
}

void Window::close() {
    running = false;
    Event event{EventType::WindowClose, this, {}};
    emit(event);
}

void Window::render() {
    for (auto& child : children) {
        child->render();
    }
}

void Window::runEventLoop() {
    std::cout << "\n[Event loop started - Press Ctrl+C to exit]\n" << std::endl;
    // In a real implementation, this would handle OS events
    // For now, it's a placeholder
}

void Window::stopEventLoop() {
    for (auto* window : windows) {
        window->close();
    }
}

} // namespace gui
