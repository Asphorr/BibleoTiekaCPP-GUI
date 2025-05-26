#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <unordered_map>

// Forward declare SDL types to avoid including SDL headers in the interface
struct SDL_Window;
struct SDL_Renderer;

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
    MouseEnter,
    MouseLeave,
    FocusGained,
    FocusLost,
    WindowClose,
    WindowResize
};

// Event structure
struct Event {
    EventType type;
    Widget* source;
    std::unordered_map<std::string, std::string> data;
    
    // Helper methods for common event data
    std::string getText() const {
        auto it = data.find("text");
        return it != data.end() ? it->second : "";
    }
    
    std::string getKey() const {
        auto it = data.find("key");
        return it != data.end() ? it->second : "";
    }
    
    int getX() const {
        auto it = data.find("x");
        return it != data.end() ? std::stoi(it->second) : 0;
    }
    
    int getY() const {
        auto it = data.find("y");
        return it != data.end() ? std::stoi(it->second) : 0;
    }
};

// Color structure
struct Color {
    uint8_t r, g, b, a;
    
    Color(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = 255)
        : r(r), g(g), b(b), a(a) {}
    
    // Predefined colors
    static Color Black() { return Color(0, 0, 0); }
    static Color White() { return Color(255, 255, 255); }
    static Color Red() { return Color(255, 0, 0); }
    static Color Green() { return Color(0, 255, 0); }
    static Color Blue() { return Color(0, 0, 255); }
    static Color Gray() { return Color(128, 128, 128); }
    static Color LightGray() { return Color(200, 200, 200); }
    static Color DarkGray() { return Color(64, 64, 64); }
};

// Style structure for customizing widget appearance
struct Style {
    Color backgroundColor;
    Color foregroundColor;
    Color borderColor;
    Color hoverColor;
    Color pressedColor;
    Color disabledColor;
    int borderWidth;
    int padding;
    std::string fontFamily;
    int fontSize;
    
    Style() : 
        backgroundColor(240, 240, 240),
        foregroundColor(0, 0, 0),
        borderColor(180, 180, 180),
        hoverColor(220, 220, 220),
        pressedColor(200, 200, 200),
        disabledColor(160, 160, 160),
        borderWidth(1),
        padding(5),
        fontFamily("Arial"),
        fontSize(14) {}
};

// Base widget class
class Widget {
protected:
    std::string id;
    int x, y, width, height;
    bool visible;
    bool enabled;
    bool focused;
    Widget* parent;
    std::vector<std::unique_ptr<Widget>> children;
    Style style;
    
public:
    Widget(const std::string& id = "");
    virtual ~Widget() = default;
    
    // Property setters with method chaining
    Widget& setPosition(int x, int y);
    Widget& setSize(int width, int height);
    Widget& setVisible(bool visible);
    Widget& setEnabled(bool enabled);
    Widget& setFocused(bool focused);
    Widget& setStyle(const Style& style);
    
    // Property getters
    std::string getId() const { return id; }
    int getX() const { return x; }
    int getY() const { return y; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    bool isVisible() const { return visible; }
    bool isEnabled() const { return enabled; }
    bool isFocused() const { return focused; }
    const Style& getStyle() const { return style; }
    Widget* getParent() const { return parent; }
    
    // Absolute position calculation
    int getAbsoluteX() const;
    int getAbsoluteY() const;
    
    // Child management
    Widget& add(std::unique_ptr<Widget> child);
    Widget* find(const std::string& id);
    Widget* findAt(int x, int y);
    void remove(const std::string& id);
    void removeAll();
    const std::vector<std::unique_ptr<Widget>>& getChildren() const { return children; }
    
    // Event handling
    using EventHandler = std::function<void(const Event&)>;
    Widget& on(EventType type, EventHandler handler);
    void off(EventType type);
    
    // Virtual methods
    virtual void render() = 0;
    virtual void update(double deltaTime) {}
    virtual bool handleEvent(const Event& event) { return false; }
    
protected:
    std::unordered_map<EventType, std::vector<EventHandler>> eventHandlers;
    void emit(const Event& event);
    
    friend class Window;
    friend class Container;
};

// Button widget
class Button : public Widget {
private:
    std::string text;
    bool pressed;
    bool hover;
    
public:
    Button(const std::string& text = "", const std::string& id = "");
    
    Button& setText(const std::string& text);
    std::string getText() const { return text; }
    
    void render() override;
    bool handleEvent(const Event& event) override;
    void click();
};

// Label widget
class Label : public Widget {
private:
    std::string text;
    bool autoSize;
    
public:
    Label(const std::string& text = "", const std::string& id = "");
    
    Label& setText(const std::string& text);
    Label& setAutoSize(bool autoSize);
    std::string getText() const { return text; }
    bool getAutoSize() const { return autoSize; }
    
    void render() override;
};

// TextInput widget
class TextInput : public Widget {
private:
    std::string text;
    std::string placeholder;
    size_t cursorPosition;
    size_t selectionStart;
    size_t selectionEnd;
    bool password;
    int maxLength;
    
public:
    TextInput(const std::string& placeholder = "", const std::string& id = "");
    
    TextInput& setText(const std::string& text);
    TextInput& setPlaceholder(const std::string& placeholder);
    TextInput& setPassword(bool password);
    TextInput& setMaxLength(int maxLength);
    
    std::string getText() const { return text; }
    std::string getPlaceholder() const { return placeholder; }
    bool isPassword() const { return password; }
    int getMaxLength() const { return maxLength; }
    
    void render() override;
    bool handleEvent(const Event& event) override;
    
private:
    void insertText(const std::string& str);
    void deleteChar(bool forward);
    void moveCursor(int direction);
};

// CheckBox widget
class CheckBox : public Widget {
private:
    std::string text;
    bool checked;
    
public:
    CheckBox(const std::string& text = "", bool checked = false, const std::string& id = "");
    
    CheckBox& setText(const std::string& text);
    CheckBox& setChecked(bool checked);
    std::string getText() const { return text; }
    bool isChecked() const { return checked; }
    
    void render() override;
    bool handleEvent(const Event& event) override;
    void toggle();
};

// RadioButton widget
class RadioButton : public Widget {
private:
    std::string text;
    std::string group;
    bool checked;
    
public:
    RadioButton(const std::string& text = "", const std::string& group = "", const std::string& id = "");
    
    RadioButton& setText(const std::string& text);
    RadioButton& setGroup(const std::string& group);
    RadioButton& setChecked(bool checked);
    
    std::string getText() const { return text; }
    std::string getGroup() const { return group; }
    bool isChecked() const { return checked; }
    
    void render() override;
    bool handleEvent(const Event& event) override;
    
private:
    void uncheckOthersInGroup();
};

// ComboBox widget
class ComboBox : public Widget {
private:
    std::vector<std::string> items;
    int selectedIndex;
    bool dropped;
    
public:
    ComboBox(const std::string& id = "");
    
    ComboBox& addItem(const std::string& item);
    ComboBox& setItems(const std::vector<std::string>& items);
    ComboBox& setSelectedIndex(int index);
    
    std::vector<std::string> getItems() const { return items; }
    int getSelectedIndex() const { return selectedIndex; }
    std::string getSelectedItem() const;
    
    void render() override;
    bool handleEvent(const Event& event) override;
};

// Slider widget
class Slider : public Widget {
private:
    double minValue;
    double maxValue;
    double value;
    double step;
    bool vertical;
    bool dragging;
    
public:
    Slider(double minValue = 0, double maxValue = 100, double value = 50, const std::string& id = "");
    
    Slider& setRange(double minValue, double maxValue);
    Slider& setValue(double value);
    Slider& setStep(double step);
    Slider& setVertical(bool vertical);
    
    double getMinValue() const { return minValue; }
    double getMaxValue() const { return maxValue; }
    double getValue() const { return value; }
    double getStep() const { return step; }
    bool isVertical() const { return vertical; }
    
    void render() override;
    bool handleEvent(const Event& event) override;
};

// ProgressBar widget
class ProgressBar : public Widget {
private:
    double minValue;
    double maxValue;
    double value;
    bool showText;
    std::string textFormat; // e.g., "{value}%", "{value}/{max}"
    
public:
    ProgressBar(double minValue = 0, double maxValue = 100, const std::string& id = "");
    
    ProgressBar& setRange(double minValue, double maxValue);
    ProgressBar& setValue(double value);
    ProgressBar& setShowText(bool showText);
    ProgressBar& setTextFormat(const std::string& format);
    
    double getMinValue() const { return minValue; }
    double getMaxValue() const { return maxValue; }
    double getValue() const { return value; }
    double getPercentage() const;
    
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
    bool stretch; // Stretch widgets to container width
    
public:
    VerticalLayout(int spacing = 10, int padding = 10, bool stretch = false);
    void apply(Widget* container) override;
};

class HorizontalLayout : public Layout {
private:
    int spacing;
    int padding;
    bool stretch; // Stretch widgets to container height
    
public:
    HorizontalLayout(int spacing = 10, int padding = 10, bool stretch = false);
    void apply(Widget* container) override;
};

class GridLayout : public Layout {
private:
    int rows;
    int cols;
    int spacing;
    int padding;
    
public:
    GridLayout(int rows, int cols, int spacing = 10, int padding = 10);
    void apply(Widget* container) override;
};

// Container widget
class Container : public Widget {
private:
    std::unique_ptr<Layout> layout;
    bool autoResize;
    
public:
    Container(const std::string& id = "");
    
    Container& setLayout(std::unique_ptr<Layout> layout);
    Container& setAutoResize(bool autoResize);
    void applyLayout();
    
    void render() override;
    bool handleEvent(const Event& event) override;
};

// Panel widget (container with border and background)
class Panel : public Container {
private:
    std::string title;
    
public:
    Panel(const std::string& title = "", const std::string& id = "");
    
    Panel& setTitle(const std::string& title);
    std::string getTitle() const { return title; }
    
    void render() override;
};

// Window class
class Window : public Widget {
private:
    std::string title;
    bool running;
    bool resizable;
    bool fullscreen;
    SDL_Window* sdlWindow;
    SDL_Renderer* sdlRenderer;
    static std::vector<Window*> windows;
    static bool eventLoopRunning;
    
    // Helper methods
    static Widget* findWidgetAt(Widget* root, int x, int y);
    void processSDLEvent(const SDL_Event& sdlEvent);
    
public:
    Window(const std::string& title = "Window", int width = 800, int height = 600);
    ~Window();
    
    Window& setTitle(const std::string& title);
    Window& setResizable(bool resizable);
    Window& setFullscreen(bool fullscreen);
    Window& setIcon(const std::string& iconPath);
    
    std::string getTitle() const { return title; }
    bool isResizable() const { return resizable; }
    bool isFullscreen() const { return fullscreen; }
    bool isRunning() const { return running; }
    
    void show();
    void hide();
    void close();
    void center();
    void maximize();
    void minimize();
    
    void render() override;
    void clear();
    void present();
    
    SDL_Renderer* getRenderer() const { return sdlRenderer; }
    
    static void runEventLoop();
    static void stopEventLoop();
    static void processEvents();
    static Window* getActiveWindow();
};

// Dialog boxes
class MessageBox {
public:
    enum Type {
        Info,
        Warning,
        Error,
        Question
    };
    
    enum Buttons {
        OK = 1,
        OKCancel = 2,
        YesNo = 3,
        YesNoCancel = 4
    };
    
    static int show(const std::string& title, const std::string& message, 
                    Type type = Info, Buttons buttons = OK);
};

// Utility functions
namespace utils {
    // Create widgets with fluent interface
    template<typename T, typename... Args>
    std::unique_ptr<T> create(Args&&... args) {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }
    
    // Load image
    SDL_Texture* loadImage(SDL_Renderer* renderer, const std::string& path);
    
    // Text utilities
    void getTextSize(const std::string& text, int fontSize, int& width, int& height);
    
    // Color conversion
    SDL_Color toSDLColor(const Color& color);
    Color fromSDLColor(const SDL_Color& color);
}

// Timer class for animations and delayed actions
class Timer {
private:
    std::function<void()> callback;
    double interval;
    double elapsed;
        bool repeating;
    bool active;
    
public:
    Timer(double interval, std::function<void()> callback, bool repeating = false);
    
    void start();
    void stop();
    void reset();
    void update(double deltaTime);
    
    bool isActive() const { return active; }
    double getInterval() const { return interval; }
    double getElapsed() const { return elapsed; }
};

// Animation support
class Animation {
public:
    enum EasingType {
        Linear,
        EaseIn,
        EaseOut,
        EaseInOut,
        Bounce,
        Elastic
    };
    
private:
    Widget* target;
    std::string property; // "x", "y", "width", "height", "opacity"
    double startValue;
    double endValue;
    double duration;
    double elapsed;
    EasingType easing;
    std::function<void()> onComplete;
    bool active;
    
public:
    Animation(Widget* target, const std::string& property, 
              double endValue, double duration, EasingType easing = Linear);
    
    Animation& setOnComplete(std::function<void()> callback);
    
    void start();
    void stop();
    void update(double deltaTime);
    
    bool isActive() const { return active; }
    
private:
    double ease(double t);
};

// Menu system
class MenuItem {
private:
    std::string text;
    std::string id;
    std::string shortcut;
    bool enabled;
    bool checkable;
    bool checked;
    std::vector<std::unique_ptr<MenuItem>> subItems;
    std::function<void()> onClick;
    
public:
    MenuItem(const std::string& text, const std::string& id = "");
    
    MenuItem& setText(const std::string& text);
    MenuItem& setShortcut(const std::string& shortcut);
    MenuItem& setEnabled(bool enabled);
    MenuItem& setCheckable(bool checkable);
    MenuItem& setChecked(bool checked);
    MenuItem& setOnClick(std::function<void()> callback);
    MenuItem& addSubItem(std::unique_ptr<MenuItem> item);
    
    const std::string& getText() const { return text; }
    const std::string& getId() const { return id; }
    const std::string& getShortcut() const { return shortcut; }
    bool isEnabled() const { return enabled; }
    bool isCheckable() const { return checkable; }
    bool isChecked() const { return checked; }
    bool hasSubItems() const { return !subItems.empty(); }
    const std::vector<std::unique_ptr<MenuItem>>& getSubItems() const { return subItems; }
    
    void click();
};

class Menu : public Widget {
private:
    std::vector<std::unique_ptr<MenuItem>> items;
    bool visible;
    int highlightedIndex;
    
public:
    Menu(const std::string& id = "");
    
    Menu& addItem(std::unique_ptr<MenuItem> item);
    Menu& addSeparator();
    
    void show(int x, int y);
    void hide();
    
    void render() override;
    bool handleEvent(const Event& event) override;
};

class MenuBar : public Widget {
private:
    std::vector<std::pair<std::string, std::unique_ptr<Menu>>> menus;
    int activeMenuIndex;
    
public:
    MenuBar(const std::string& id = "menubar");
    
    MenuBar& addMenu(const std::string& title, std::unique_ptr<Menu> menu);
    
    void render() override;
    bool handleEvent(const Event& event) override;
};

// TabControl widget
class TabControl : public Widget {
private:
    struct Tab {
        std::string title;
        std::unique_ptr<Widget> content;
        bool closable;
    };
    
    std::vector<Tab> tabs;
    int activeTabIndex;
    
public:
    TabControl(const std::string& id = "");
    
    TabControl& addTab(const std::string& title, std::unique_ptr<Widget> content, bool closable = false);
    TabControl& removeTab(int index);
    TabControl& setActiveTab(int index);
    
    int getTabCount() const { return tabs.size(); }
    int getActiveTabIndex() const { return activeTabIndex; }
    
    void render() override;
    bool handleEvent(const Event& event) override;
};

// ScrollBar widget
class ScrollBar : public Widget {
private:
    double minValue;
    double maxValue;
    double value;
    double pageSize;
    bool vertical;
    bool dragging;
    
public:
    ScrollBar(bool vertical = true, const std::string& id = "");
    
    ScrollBar& setRange(double minValue, double maxValue);
    ScrollBar& setValue(double value);
    ScrollBar& setPageSize(double pageSize);
    
    double getValue() const { return value; }
    double getPageSize() const { return pageSize; }
    
    void render() override;
    bool handleEvent(const Event& event) override;
};

// ScrollableContainer widget
class ScrollableContainer : public Container {
private:
    std::unique_ptr<ScrollBar> verticalScrollBar;
    std::unique_ptr<ScrollBar> horizontalScrollBar;
    int contentWidth;
    int contentHeight;
    int scrollX;
    int scrollY;
    
public:
    ScrollableContainer(const std::string& id = "");
    
    ScrollableContainer& setContentSize(int width, int height);
    void updateScrollBars();
    
    void render() override;
    bool handleEvent(const Event& event) override;
};

// ListBox widget
class ListBox : public Widget {
private:
    std::vector<std::string> items;
    int selectedIndex;
    int scrollOffset;
    int itemHeight;
    bool multiSelect;
    std::vector<int> selectedIndices;
    
public:
    ListBox(const std::string& id = "");
    
    ListBox& addItem(const std::string& item);
    ListBox& setItems(const std::vector<std::string>& items);
    ListBox& setSelectedIndex(int index);
    ListBox& setMultiSelect(bool multiSelect);
    ListBox& clearSelection();
    
    std::vector<std::string> getItems() const { return items; }
    int getSelectedIndex() const { return selectedIndex; }
    std::string getSelectedItem() const;
    std::vector<int> getSelectedIndices() const { return selectedIndices; }
    std::vector<std::string> getSelectedItems() const;
    
    void render() override;
    bool handleEvent(const Event& event) override;
};

// TreeView widget
class TreeNode {
private:
    std::string text;
    std::string id;
    bool expanded;
    bool selected;
    TreeNode* parent;
    std::vector<std::unique_ptr<TreeNode>> children;
    void* userData;
    
public:
    TreeNode(const std::string& text, const std::string& id = "");
    
    TreeNode& setText(const std::string& text);
    TreeNode& setExpanded(bool expanded);
    TreeNode& setSelected(bool selected);
    TreeNode& setUserData(void* data);
    TreeNode& addChild(std::unique_ptr<TreeNode> child);
    
    const std::string& getText() const { return text; }
    const std::string& getId() const { return id; }
    bool isExpanded() const { return expanded; }
    bool isSelected() const { return selected; }
    bool hasChildren() const { return !children.empty(); }
    void* getUserData() const { return userData; }
    TreeNode* getParent() const { return parent; }
    const std::vector<std::unique_ptr<TreeNode>>& getChildren() const { return children; }
    
    TreeNode* find(const std::string& id);
    void toggle();
};

class TreeView : public Widget {
private:
    std::vector<std::unique_ptr<TreeNode>> roots;
    TreeNode* selectedNode;
    int scrollOffset;
    int nodeHeight;
    int indentSize;
    
public:
    TreeView(const std::string& id = "");
    
    TreeView& addRoot(std::unique_ptr<TreeNode> root);
    TreeView& setNodeHeight(int height);
    TreeView& setIndentSize(int size);
    
    TreeNode* getSelectedNode() const { return selectedNode; }
    TreeNode* findNode(const std::string& id);
    
    void render() override;
    bool handleEvent(const Event& event) override;
    
private:
    void renderNode(TreeNode* node, int& y, int indent);
    TreeNode* getNodeAt(int y);
};

// Table widget
class TableColumn {
private:
    std::string title;
    int width;
    bool resizable;
    bool sortable;
    
public:
    TableColumn(const std::string& title, int width = 100);
    
    TableColumn& setWidth(int width);
    TableColumn& setResizable(bool resizable);
    TableColumn& setSortable(bool sortable);
    
    const std::string& getTitle() const { return title; }
    int getWidth() const { return width; }
    bool isResizable() const { return resizable; }
    bool isSortable() const { return sortable; }
};

class Table : public Widget {
private:
    std::vector<TableColumn> columns;
    std::vector<std::vector<std::string>> rows;
    int selectedRow;
    int scrollOffset;
    int rowHeight;
    bool showHeader;
    bool showGrid;
    
public:
    Table(const std::string& id = "");
    
    Table& addColumn(const TableColumn& column);
    Table& addRow(const std::vector<std::string>& row);
    Table& setData(const std::vector<std::vector<std::string>>& data);
    Table& setSelectedRow(int row);
    Table& setShowHeader(bool show);
    Table& setShowGrid(bool show);
    
    int getColumnCount() const { return columns.size(); }
    int getRowCount() const { return rows.size(); }
    int getSelectedRow() const { return selectedRow; }
    std::vector<std::string> getRow(int index) const;
    
    void render() override;
    bool handleEvent(const Event& event) override;
};

// StatusBar widget
class StatusBar : public Widget {
private:
    struct Panel {
        std::string text;
        int width;
        bool autoSize;
    };
    
    std::vector<Panel> panels;
    
public:
    StatusBar(const std::string& id = "statusbar");
    
    StatusBar& addPanel(const std::string& text = "", int width = -1);
    StatusBar& setPanelText(int index, const std::string& text);
    
    void render() override;
};

// ToolBar widget
class ToolBar : public Widget {
private:
    struct Tool {
        std::string icon;
        std::string tooltip;
        std::function<void()> onClick;
        bool enabled;
        bool toggle;
        bool pressed;
    };
    
    std::vector<Tool> tools;
    int toolSize;
    bool showTooltips;
    
public:
    ToolBar(const std::string& id = "toolbar");
    
    ToolBar& addTool(const std::string& icon, const std::string& tooltip, 
                     std::function<void()> onClick, bool toggle = false);
    ToolBar& addSeparator();
    ToolBar& setToolSize(int size);
    ToolBar& setShowTooltips(bool show);
    
    void render() override;
    bool handleEvent(const Event& event) override;
};

// FileDialog
class FileDialog {
public:
    enum Mode {
        Open,
        Save,
        SelectFolder
    };
    
    static std::string show(const std::string& title, Mode mode, 
                           const std::string& defaultPath = "",
                           const std::vector<std::string>& filters = {});
};

// ColorPicker widget
class ColorPicker : public Widget {
private:
    Color color;
    bool showAlpha;
    
public:
    ColorPicker(const Color& color = Color::White(), const std::string& id = "");
    
    ColorPicker& setColor(const Color& color);
    ColorPicker& setShowAlpha(bool show);
    
    Color getColor() const { return color; }
    
    void render() override;
    bool handleEvent(const Event& event) override;
};

// DatePicker widget
class DatePicker : public Widget {
private:
    int year;
    int month;
    int day;
    bool showWeekNumbers;
    
public:
    DatePicker(const std::string& id = "");
    
    DatePicker& setDate(int year, int month, int day);
    DatePicker& setShowWeekNumbers(bool show);
    
    void getDate(int& year, int& month, int& day) const;
    std::string getDateString(const std::string& format = "YYYY-MM-DD") const;
    
    void render() override;
    bool handleEvent(const Event& event) override;
};

// Global theme management
class Theme {
private:
    std::unordered_map<std::string, Style> styles;
    std::string name;
    
public:
    Theme(const std::string& name);
    
    Theme& setStyle(const std::string& widgetType, const Style& style);
    const Style* getStyle(const std::string& widgetType) const;
    
    static void setGlobalTheme(const Theme& theme);
    static const Theme* getGlobalTheme();
    
    // Predefined themes
    static Theme Light();
    static Theme Dark();
    static Theme Blue();
};

// Application class for managing the GUI application
class Application {
private:
    static Application* instance;
    std::vector<std::unique_ptr<Window>> windows;
    std::vector<std::unique_ptr<Timer>> timers;
    std::vector<std::unique_ptr<Animation>> animations;
    bool running;
    
public:
    Application();
    ~Application();
    
    static Application* getInstance();
    
    void addWindow(std::unique_ptr<Window> window);
    void addTimer(std::unique_ptr<Timer> timer);
    void addAnimation(std::unique_ptr<Animation> animation);
    
    void run();
    void quit();
    
    void update(double deltaTime);
};

} // namespace gui

