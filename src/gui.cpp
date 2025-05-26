#include "gui.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <algorithm>
#include <queue>
#include <chrono>

namespace gui {

// Internal rendering context
struct RenderContext {
    SDL_Renderer* renderer;
    TTF_Font* font;
    SDL_Color textColor;
    SDL_Color backgroundColor;
    SDL_Color borderColor;
    SDL_Color buttonColor;
    SDL_Color buttonHoverColor;
    SDL_Color buttonPressedColor;
    
    RenderContext() : renderer(nullptr), font(nullptr),
        textColor{0, 0, 0, 255},
        backgroundColor{240, 240, 240, 255},
        borderColor{180, 180, 180, 255},
        buttonColor{225, 225, 225, 255},
        buttonHoverColor{210, 210, 210, 255},
        buttonPressedColor{195, 195, 195, 255} {}
};

static RenderContext g_context;
static bool g_sdlInitialized = false;
static bool g_eventLoopRunning = false;

// Helper functions
static void initSDL() {
    if (!g_sdlInitialized) {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            throw std::runtime_error("SDL initialization failed: " + std::string(SDL_GetError()));
        }
        
        if (TTF_Init() < 0) {
            throw std::runtime_error("SDL_ttf initialization failed: " + std::string(TTF_GetError()));
        }
        
        g_context.font = TTF_OpenFont("Arial.ttf", 14);
        if (!g_context.font) {
            // Fallback to a default font path
            g_context.font = TTF_OpenFont("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf", 14);
            if (!g_context.font) {
                g_context.font = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 14);
            }
        }
        
        g_sdlInitialized = true;
    }
}

static void drawRect(int x, int y, int w, int h, const SDL_Color& color, bool filled = true) {
    SDL_SetRenderDrawColor(g_context.renderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {x, y, w, h};
    if (filled) {
        SDL_RenderFillRect(g_context.renderer, &rect);
    } else {
        SDL_RenderDrawRect(g_context.renderer, &rect);
    }
}

static void drawText(const std::string& text, int x, int y, const SDL_Color& color) {
    if (!g_context.font || text.empty()) return;
    
    SDL_Surface* surface = TTF_RenderText_Blended(g_context.font, text.c_str(), color);
    if (!surface) return;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(g_context.renderer, surface);
    if (texture) {
        SDL_Rect destRect = {x, y, surface->w, surface->h};
        SDL_RenderCopy(g_context.renderer, texture, nullptr, &destRect);
        SDL_DestroyTexture(texture);
    }
    
    SDL_FreeSurface(surface);
}

static void getTextSize(const std::string& text, int& w, int& h) {
    if (!g_context.font || text.empty()) {
        w = h = 0;
        return;
    }
    TTF_SizeText(g_context.font, text.c_str(), &w, &h);
}

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
    if (parent && event.type != EventType::WindowClose) {
        parent->emit(event);
    }
}

// Button implementation
Button::Button(const std::string& text, const std::string& id) 
    : Widget(id), text(text) {
    // Auto-size based on text
    if (!text.empty() && g_context.font) {
        int textW, textH;
        getTextSize(text, textW, textH);
        width = textW + 20;  // Add padding
        height = textH + 10;
    }
}

Button& Button::setText(const std::string& text) {
    this->text = text;
    // Auto-resize
    if (!text.empty() && g_context.font) {
        int textW, textH;
        getTextSize(text, textW, textH);
        width = textW + 20;
        height = textH + 10;
    }
    return *this;
}

void Button::render() {
    if (!visible) return;
    
    // Get absolute position
    int absX = x;
    int absY = y;
    Widget* p = parent;
    while (p) {
        absX += p->getX();
        absY += p->getY();
        p = p->parent;
    }
    
    // Check mouse state for hover/press effects
    int mouseX, mouseY;
    Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);
    bool hover = mouseX >= absX && mouseX < absX + width && 
                 mouseY >= absY && mouseY < absY + height;
    bool pressed = hover && (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT));
    
    // Draw button background
    SDL_Color btnColor = pressed ? g_context.buttonPressedColor : 
                        (hover ? g_context.buttonHoverColor : g_context.buttonColor);
    
    if (enabled) {
        drawRect(absX, absY, width, height, btnColor);
    } else {
        SDL_Color disabledColor = {200, 200, 200, 255};
        drawRect(absX, absY, width, height, disabledColor);
    }
    
    // Draw border
    drawRect(absX, absY, width, height, g_context.borderColor, false);
    
    // Draw text centered
    if (!text.empty()) {
        int textW, textH;
        getTextSize(text, textW, textH);
        int textX = absX + (width - textW) / 2;
        int textY = absY + (height - textH) / 2;
        
        SDL_Color textColor = enabled ? g_context.textColor : SDL_Color{150, 150, 150, 255};
        drawText(text, textX, textY, textColor);
    }
}

void Button::click() {
    if (!enabled) return;
    Event event{EventType::Click, this, {}};
    emit(event);
}

// Label implementation
Label::Label(const std::string& text, const std::string& id) 
    : Widget(id), text(text) {
    // Auto-size based on text
    if (!text.empty() && g_context.font) {
        int textW, textH;
        getTextSize(text, textW, textH);
        width = textW;
        height = textH;
    }
}

Label& Label::setText(const std::string& text) {
    this->text = text;
    // Auto-resize
    if (!text.empty() && g_context.font) {
        int textW, textH;
        getTextSize(text, textW, textH);
        width = textW;
        height = textH;
    }
    return *this;
}

void Label::render() {
    if (!visible) return;
    
    // Get absolute position
    int absX = x;
    int absY = y;
    Widget* p = parent;
    while (p) {
        absX += p->getX();
        absY += p->getY();
        p = p->parent;
    }
    
    // Draw text
    if (!text.empty()) {
        drawText(text, absX, absY, g_context.textColor);
    }
}

// TextInput implementation
TextInput::TextInput(const std::string& placeholder, const std::string& id) 
    : Widget(id), placeholder(placeholder) {
    width = 200;
    height = 30;
}

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
    
    // Get absolute position
    int absX = x;
    int absY = y;
    Widget* p = parent;
    while (p) {
        absX += p->getX();
        absY += p->getY();
        p = p->parent;
    }
    
    // Draw background
    SDL_Color bgColor = enabled ? SDL_Color{255, 255, 255, 255} : SDL_Color{240, 240, 240, 255};
    drawRect(absX, absY, width, height, bgColor);
    
    // Draw border
    drawRect(absX, absY, width, height, g_context.borderColor, false);
    
    // Draw text or placeholder
    std::string displayText = text.empty() ? placeholder : text;
    SDL_Color textColor = text.empty() ? SDL_Color{150, 150, 150, 255} : g_context.textColor;
    
    if (!displayText.empty()) {
        int textW, textH;
        getTextSize(displayText, textW, textH);
        int textY = absY + (height - textH) / 2;
        drawText(displayText, absX + 5, textY, textColor);
    }
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
    
    // Get absolute position
    int absX = x;
    int absY = y;
    Widget* p = parent;
    while (p) {
        absX += p->getX();
        absY += p->getY();
        p = p->parent;
    }
    
    // Optionally draw container background
    // drawRect(absX, absY, width, height, {250, 250, 250, 255});
    
    // Render children
    for (auto& child : children) {
        child->render();
    }
}

// Window implementation
Window::Window(const std::string& title, int width, int height) 
    : Widget("window"), title(title), running(false) {
    setSize(width, height);
    windows.push_back(this);
    
    // Initialize SDL if needed
    initSDL();
}

Window::~Window() {
    windows.erase(std::remove(windows.begin(), windows.end(), this), windows.end());
    
    if (sdlWindow) {
        SDL_DestroyWindow(sdlWindow);
        sdlWindow = nullptr;
    }
    
    if (windows.empty() && g_sdlInitialized) {
        if (g_context.font) {
            TTF_CloseFont(g_context.font);
            g_context.font = nullptr;
        }
        TTF_Quit();
        SDL_Quit();
        g_sdlInitialized = false;
    }
}

Window& Window::setTitle(const std::string& title) {
    this->title = title;
    if (sdlWindow) {
        SDL_SetWindowTitle(sdlWindow, title.c_str());
    }
    return *this;
}

void Window::show() {
    if (!sdlWindow) {
        sdlWindow = SDL_CreateWindow(
            title.c_str(),
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            width, height,
            SDL_WINDOW_SHOWN
        );
        
        if (!sdlWindow) {
            throw std::runtime_error("Failed to create window: " + std::string(SDL_GetError()));
        }
        
        g_context.renderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED);
        if (!g_context.renderer) {
            throw std::runtime_error("Failed to create renderer: " + std::string(SDL_GetError()));
        }
    }
    
    running = true;
    render();
}

void Window::close() {
    running = false;
    Event event{EventType::WindowClose, this, {}};
    emit(event);
}

void Window::render() {
    if (!g_context.renderer) return;
    
    // Clear screen
    SDL_SetRenderDrawColor(g_context.renderer, 
        g_context.backgroundColor.r, 
        g_context.backgroundColor.g, 
        g_context.backgroundColor.b, 
        g_context.backgroundColor.a);
        SDL_RenderClear(g_context.renderer);
    
    // Render all children
    for (auto& child : children) {
        child->render();
    }
    
    // Present
    SDL_RenderPresent(g_context.renderer);
}

void Window::runEventLoop() {
    g_eventLoopRunning = true;
    SDL_Event event;
    
    // Track focused widget for text input
    static Widget* focusedWidget = nullptr;
    static std::string inputBuffer;
    
    // Track mouse state for click detection
    static bool mouseWasPressed = false;
    
    while (g_eventLoopRunning) {
        // Process all pending events
        while (SDL_PollEvent(&event)) {
            // Handle window events
            if (event.type == SDL_QUIT) {
                stopEventLoop();
                break;
            }
            
            // Find which window the event belongs to
            Window* targetWindow = nullptr;
            for (Window* window : windows) {
                if (window->sdlWindow && SDL_GetWindowID(window->sdlWindow) == event.window.windowID) {
                    targetWindow = window;
                    break;
                }
            }
            
            if (!targetWindow) continue;
            
            // Handle different event types
            switch (event.type) {
                case SDL_WINDOWEVENT:
                    if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
                        targetWindow->close();
                    }
                    break;
                    
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        mouseWasPressed = true;
                        
                        // Find widget under mouse
                        int mouseX = event.button.x;
                        int mouseY = event.button.y;
                        
                        // Check for TextInput widgets to focus
                        Widget* clickedWidget = findWidgetAt(targetWindow, mouseX, mouseY);
                        if (clickedWidget) {
                            // Update focus
                            if (dynamic_cast<TextInput*>(clickedWidget)) {
                                focusedWidget = clickedWidget;
                                inputBuffer = dynamic_cast<TextInput*>(clickedWidget)->getText();
                                SDL_StartTextInput();
                            } else {
                                focusedWidget = nullptr;
                                SDL_StopTextInput();
                            }
                        } else {
                            focusedWidget = nullptr;
                            SDL_StopTextInput();
                        }
                    }
                    break;
                    
                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT && mouseWasPressed) {
                        mouseWasPressed = false;
                        
                        // Find widget under mouse and trigger click
                        int mouseX = event.button.x;
                        int mouseY = event.button.y;
                        
                        Widget* clickedWidget = findWidgetAt(targetWindow, mouseX, mouseY);
                        if (Button* button = dynamic_cast<Button*>(clickedWidget)) {
                            button->click();
                        }
                    }
                    break;
                    
                case SDL_TEXTINPUT:
                    if (focusedWidget && dynamic_cast<TextInput*>(focusedWidget)) {
                        inputBuffer += event.text.text;
                        dynamic_cast<TextInput*>(focusedWidget)->setText(inputBuffer);
                        targetWindow->render();
                    }
                    break;
                    
                case SDL_KEYDOWN:
                    if (focusedWidget && dynamic_cast<TextInput*>(focusedWidget)) {
                        if (event.key.keysym.sym == SDLK_BACKSPACE && !inputBuffer.empty()) {
                            inputBuffer.pop_back();
                            dynamic_cast<TextInput*>(focusedWidget)->setText(inputBuffer);
                            targetWindow->render();
                        } else if (event.key.keysym.sym == SDLK_RETURN) {
                            // Submit on Enter
                            Event enterEvent{EventType::KeyPress, focusedWidget, {{"key", "enter"}}};
                            focusedWidget->emit(enterEvent);
                        }
                    }
                    break;
                    
                case SDL_MOUSEMOTION:
                    // Update hover states by re-rendering
                    targetWindow->render();
                    break;
            }
        }
        
        // Small delay to prevent 100% CPU usage
        SDL_Delay(16); // ~60 FPS
    }
}

void Window::stopEventLoop() {
    g_eventLoopRunning = false;
    for (auto* window : windows) {
        window->running = false;
    }
}

// Helper function to find widget at coordinates
Widget* Window::findWidgetAt(Widget* root, int x, int y) {
    if (!root || !root->isVisible()) return nullptr;
    
    // Calculate absolute position
    int absX = root->getX();
    int absY = root->getY();
    Widget* p = root->parent;
    while (p) {
        absX += p->getX();
        absY += p->getY();
        p = p->parent;
    }
    
    // Check children first (top to bottom)
    for (auto it = root->children.rbegin(); it != root->children.rend(); ++it) {
        if (Widget* found = findWidgetAt(it->get(), x, y)) {
            return found;
        }
    }
    
    // Check this widget
    if (x >= absX && x < absX + root->getWidth() &&
        y >= absY && y < absY + root->getHeight()) {
        return root;
    }
    
    return nullptr;
}

} // namespace gui

