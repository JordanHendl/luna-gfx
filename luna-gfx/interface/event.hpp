#pragma once
#include <functional>
#include <string>
#include <vector>
#include <memory>
namespace luna {
namespace gfx {
class Event;
enum class Key;
enum class MouseButton;

/** Enum to describe a key press.
 */
enum class Key {
  None,
  ESC,
  F1,
  F2,
  F3,
  F4,
  F5,
  F6,
  F7,
  F8,
  F9,
  F10,
  F11,
  F12,
  Backtick,
  One,
  Two,
  Three,
  Four,
  Five,
  Six,
  Seven,
  Eight,
  Nine,
  Zero,
  Hyphen,
  Equals,
  Backspace,
  Tab,
  Q,
  W,
  E,
  R,
  T,
  Y,
  U,
  I,
  O,
  P,
  LBracket,
  RBracket,
  Backslash,
  Capslock,
  A,
  S,
  D,
  F,
  G,
  H,
  J,
  K,
  L,
  Semicolon,
  Quote,
  Return,
  LShift,
  RShift,
  Z,
  X,
  C,
  V,
  B,
  N,
  M,
  Comma,
  Period,
  Forwardslash,
  LCtrl,
  LAlt,
  Space,
  RAlt,
  Fn,
  RCtrl,
  Left,
  Up,
  Right,
  Down
};

/** Enum to describe a mouse button action.
 */
enum class MouseButton {
  None,
  LeftClick,
  MiddleClick,
  RightClick,
  WheelUp,
  WheelDown,
  Button01,
  Button02
};

/** Object to describe an IO event sent by the underlying API.
 */
class Event {
 public:
  enum class Type {
    None,
    KeyDown,
    KeyUp,
    MouseButtonDown,
    MouseButtonUp,
    MouseWheelUp,
    MouseWheelDown,
    JoystickAxis,
    JoystickButton,
    JoystickInitialize,
    WindowExit,
  };

  inline Event();
  inline Event(Event::Type type, Key key);
  inline Event(Event::Type type, MouseButton button);
  inline Event(const Event& event);
  inline ~Event() = default;
  [[nodiscard]] inline auto type() const -> Type;
  [[nodiscard]] inline auto key() const -> Key;
  [[nodiscard]] inline auto button() const -> MouseButton;

 private:
  Type event_type;
  Key event_key;
  MouseButton event_button;
};

[[nodiscard]] inline static auto to_string(const Event::Type& event) -> std::string;
[[nodiscard]] inline static auto to_string(const Event& event) -> std::string;
[[nodiscard]] inline static auto to_string(const Key& key) -> std::string;
[[nodiscard]] inline static auto to_string(const MouseButton& button) -> std::string;

/** Object to handle subscribing to events from the library. Registered callbacks are
 * destroyed after this object is deconstructed.
 */
class EventRegister {
 public:
  EventRegister();
  EventRegister(EventRegister&& mv);
  ~EventRegister();
  auto operator=(EventRegister&& mv) -> EventRegister&;
  auto add(std::function<void(const Event&)> callback) -> void;
  auto reset() -> void;
 private:
  struct EventRegisterData;
  std::unique_ptr<EventRegisterData> m_data;
};

auto poll_events() -> void;

Event::Event(Event::Type type, Key key) {
  this->event_type = type;
  this->event_key = key;
}

Event::Event(Event::Type type, MouseButton button) {
  this->event_type = type;
  this->event_button = button;
}

Event::Event() {
  this->event_type = Event::Type::None;
  this->event_button = MouseButton::None;
  this->event_key = Key::None;
}

Event::Event(const Event& event) {
  this->event_key = event.event_key;
  this->event_type = event.event_type;
}

auto Event::button() const -> MouseButton { return this->event_button; }

auto Event::type() const -> Event::Type { return this->event_type; }

auto Event::key() const -> Key { return this->event_key; }

auto to_string(const Event& event) -> std::string {
  switch (event.type()) {
    case Event::Type::MouseButtonDown:
    case Event::Type::MouseButtonUp:
    case Event::Type::MouseWheelDown:
    case Event::Type::MouseWheelUp:
      return to_string(event.type());
    case Event::Type::KeyDown:
    case Event::Type::KeyUp:
      return (std::string(to_string(event.type())) + std::string(" ") +
              std::string(to_string(event.key())));
    case Event::Type::WindowExit:
      return std::string("Window Exit");
    default:
      return std::string("Unknown");
  }
}

auto to_string(const Event::Type& type) -> std::string {
  switch (type) {
    case Event::Type::KeyDown:
      return std::string("Key Down");
    case Event::Type::KeyUp:
      return std::string("Key Up");
    case Event::Type::MouseButtonDown:
      return std::string("Mouse Button Down");
    case Event::Type::MouseButtonUp:
      return std::string("Mouse Button Up");
    case Event::Type::MouseWheelUp:
      return std::string("Mouse Wheel Up");
    case Event::Type::MouseWheelDown:
      return std::string("Mouse Wheel Down");
    default:
      return "Unknown";
  }
}

auto to_string(const Key& key) -> std::string {
  switch (key) {
    case Key::Q:
      return std::string("Q");
    case Key::W:
      return std::string("W");
    case Key::E:
      return std::string("E");
    case Key::R:
      return std::string("R");
    case Key::T:
      return std::string("T");
    case Key::Y:
      return std::string("Y");
    case Key::U:
      return std::string("U");
    case Key::I:
      return std::string("I");
    case Key::O:
      return std::string("O");
    case Key::P:
      return std::string("P");
    case Key::A:
      return std::string("A");
    case Key::S:
      return std::string("S");
    case Key::D:
      return std::string("D");
    case Key::F:
      return std::string("F");
    case Key::G:
      return std::string("G");
    case Key::H:
      return std::string("H");
    case Key::J:
      return std::string("J");
    case Key::K:
      return std::string("K");
    case Key::L:
      return std::string("L");
    case Key::Z:
      return std::string("Z");
    case Key::X:
      return std::string("X");
    case Key::C:
      return std::string("C");
    case Key::V:
      return std::string("V");
    case Key::B:
      return std::string("B");
    case Key::N:
      return std::string("N");
    case Key::M:
      return std::string("M");
    case Key::Backslash:
      return std::string("Backslash");
    case Key::Backspace:
      return std::string("Backspace");
    case Key::Backtick:
      return std::string("Backtick");
    case Key::Capslock:
      return std::string("Caps lock");
    case Key::Comma:
      return std::string("Comma");
    case Key::ESC:
      return std::string("Escape");
    case Key::Equals:
      return std::string("Equals");
    case Key::One:
      return std::string("1");
    case Key::Two:
      return std::string("2");
    case Key::Three:
      return std::string("3");
    case Key::Four:
      return std::string("4");
    case Key::Five:
      return std::string("5");
    case Key::Six:
      return std::string("6");
    case Key::Seven:
      return std::string("7");
    case Key::Eight:
      return std::string("8");
    case Key::Nine:
      return std::string("9");
    case Key::Zero:
      return std::string("0");
    case Key::Forwardslash:
      return std::string("Forward Slash");
    default:
      return "Unknown";
  }
}

auto to_string(const MouseButton& button) -> std::string {
  switch (button) {
    case MouseButton::LeftClick:
      return std::string("Left Click");
    case MouseButton::RightClick:
      return std::string("Right Click");
    case MouseButton::MiddleClick:
      return std::string("Middle Click");
    case MouseButton::WheelUp:
      return std::string("Mouse Wheel Up");
    case MouseButton::WheelDown:
      return std::string("Mouse Wheel Down");
    default:
      return "Unknown";
  }
}
}
}  // namespace luna