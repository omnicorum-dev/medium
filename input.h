//
// Created by Nico Russo on 4/18/26.
//

#ifndef MEDIUM_INPUT_H
#define MEDIUM_INPUT_H

#include <map>
#include <medium.h>

/* Mouse Buttons */
#define MED_MOUSE_BUTTON_1         0
#define MED_MOUSE_BUTTON_2         1
#define MED_MOUSE_BUTTON_3         2
#define MED_MOUSE_BUTTON_4         3
#define MED_MOUSE_BUTTON_5         4
#define MED_MOUSE_BUTTON_6         5
#define MED_MOUSE_BUTTON_7         6
#define MED_MOUSE_BUTTON_8         7
#define MED_MOUSE_BUTTON_LAST      MED_MOUSE_BUTTON_8
#define MED_MOUSE_BUTTON_LEFT      MED_MOUSE_BUTTON_1
#define MED_MOUSE_BUTTON_RIGHT     MED_MOUSE_BUTTON_2
#define MED_MOUSE_BUTTON_MIDDLE    MED_MOUSE_BUTTON_3

#define MED_MOUSE_SCROLL           8

/* Printable keys */
#define MED_KEY_SPACE              32
#define MED_KEY_APOSTROPHE         39  /* ' */
#define MED_KEY_COMMA              44  /* , */
#define MED_KEY_MINUS              45  /* - */
#define MED_KEY_PERIOD             46  /* . */
#define MED_KEY_SLASH              47  /* / */
#define MED_KEY_0                  48
#define MED_KEY_1                  49
#define MED_KEY_2                  50
#define MED_KEY_3                  51
#define MED_KEY_4                  52
#define MED_KEY_5                  53
#define MED_KEY_6                  54
#define MED_KEY_7                  55
#define MED_KEY_8                  56
#define MED_KEY_9                  57
#define MED_KEY_SEMICOLON          59  /* ; */
#define MED_KEY_EQUAL              61  /* = */
#define MED_KEY_A                  65
#define MED_KEY_B                  66
#define MED_KEY_C                  67
#define MED_KEY_D                  68
#define MED_KEY_E                  69
#define MED_KEY_F                  70
#define MED_KEY_G                  71
#define MED_KEY_H                  72
#define MED_KEY_I                  73
#define MED_KEY_J                  74
#define MED_KEY_K                  75
#define MED_KEY_L                  76
#define MED_KEY_M                  77
#define MED_KEY_N                  78
#define MED_KEY_O                  79
#define MED_KEY_P                  80
#define MED_KEY_Q                  81
#define MED_KEY_R                  82
#define MED_KEY_S                  83
#define MED_KEY_T                  84
#define MED_KEY_U                  85
#define MED_KEY_V                  86
#define MED_KEY_W                  87
#define MED_KEY_X                  88
#define MED_KEY_Y                  89
#define MED_KEY_Z                  90
#define MED_KEY_LEFT_BRACKET       91  /* [ */
#define MED_KEY_BACKSLASH          92  /* \ */
#define MED_KEY_RIGHT_BRACKET      93  /* ] */
#define MED_KEY_GRAVE_ACCENT       96  /* ` */
#define MED_KEY_WORLD_1            161 /* non-US #1 */
#define MED_KEY_WORLD_2            162 /* non-US #2 */

/* Function keys */
#define MED_KEY_ESCAPE             256
#define MED_KEY_ENTER              257
#define MED_KEY_TAB                258
#define MED_KEY_BACKSPACE          259
#define MED_KEY_INSERT             260
#define MED_KEY_DELETE             261
#define MED_KEY_RIGHT              262
#define MED_KEY_LEFT               263
#define MED_KEY_DOWN               264
#define MED_KEY_UP                 265
#define MED_KEY_PAGE_UP            266
#define MED_KEY_PAGE_DOWN          267
#define MED_KEY_HOME               268
#define MED_KEY_END                269
#define MED_KEY_CAPS_LOCK          280
#define MED_KEY_SCROLL_LOCK        281
#define MED_KEY_NUM_LOCK           282
#define MED_KEY_PRINT_SCREEN       283
#define MED_KEY_PAUSE              284
#define MED_KEY_F1                 290
#define MED_KEY_F2                 291
#define MED_KEY_F3                 292
#define MED_KEY_F4                 293
#define MED_KEY_F5                 294
#define MED_KEY_F6                 295
#define MED_KEY_F7                 296
#define MED_KEY_F8                 297
#define MED_KEY_F9                 298
#define MED_KEY_F10                299
#define MED_KEY_F11                300
#define MED_KEY_F12                301
#define MED_KEY_F13                302
#define MED_KEY_F14                303
#define MED_KEY_F15                304
#define MED_KEY_F16                305
#define MED_KEY_F17                306
#define MED_KEY_F18                307
#define MED_KEY_F19                308
#define MED_KEY_F20                309
#define MED_KEY_F21                310
#define MED_KEY_F22                311
#define MED_KEY_F23                312
#define MED_KEY_F24                313
#define MED_KEY_F25                314
#define MED_KEY_KP_0               320
#define MED_KEY_KP_1               321
#define MED_KEY_KP_2               322
#define MED_KEY_KP_3               323
#define MED_KEY_KP_4               324
#define MED_KEY_KP_5               325
#define MED_KEY_KP_6               326
#define MED_KEY_KP_7               327
#define MED_KEY_KP_8               328
#define MED_KEY_KP_9               329
#define MED_KEY_KP_DECIMAL         330
#define MED_KEY_KP_DIVIDE          331
#define MED_KEY_KP_MULTIPLY        332
#define MED_KEY_KP_SUBTRACT        333
#define MED_KEY_KP_ADD             334
#define MED_KEY_KP_ENTER           335
#define MED_KEY_KP_EQUAL           336
#define MED_KEY_LEFT_SHIFT         340
#define MED_KEY_LEFT_CONTROL       341
#define MED_KEY_LEFT_ALT           342
#define MED_KEY_LEFT_SUPER         343
#define MED_KEY_RIGHT_SHIFT        344
#define MED_KEY_RIGHT_CONTROL      345
#define MED_KEY_RIGHT_ALT          346
#define MED_KEY_RIGHT_SUPER        347
#define MED_KEY_MENU               348

#define MED_MOD_SHIFT           0x0001
#define MED_MOD_CONTROL         0x0002
#define MED_MOD_ALT             0x0004
#define MED_MOD_SUPER           0x0008
#define MED_MOD_CAPS_LOCK       0x0010
#define MED_MOD_NUM_LOCK        0x0020

/* Gamepad Inputs */
#define MED_JOYSTICK_1             0
#define MED_JOYSTICK_2             1
#define MED_JOYSTICK_3             2
#define MED_JOYSTICK_4             3
#define MED_JOYSTICK_5             4
#define MED_JOYSTICK_6             5
#define MED_JOYSTICK_7             6
#define MED_JOYSTICK_8             7
#define MED_JOYSTICK_9             8
#define MED_JOYSTICK_10            9
#define MED_JOYSTICK_11            10
#define MED_JOYSTICK_12            11
#define MED_JOYSTICK_13            12
#define MED_JOYSTICK_14            13
#define MED_JOYSTICK_15            14
#define MED_JOYSTICK_16            15
#define MED_JOYSTICK_LAST          MED_JOYSTICK_16

#define MED_GAMEPAD_BUTTON_A               0
#define MED_GAMEPAD_BUTTON_B               1
#define MED_GAMEPAD_BUTTON_X               2
#define MED_GAMEPAD_BUTTON_Y               3
#define MED_GAMEPAD_BUTTON_LEFT_BUMPER     4
#define MED_GAMEPAD_BUTTON_RIGHT_BUMPER    5
#define MED_GAMEPAD_BUTTON_BACK            6
#define MED_GAMEPAD_BUTTON_START           7
#define MED_GAMEPAD_BUTTON_GUIDE           8
#define MED_GAMEPAD_BUTTON_LEFT_THUMB      9
#define MED_GAMEPAD_BUTTON_RIGHT_THUMB     10
#define MED_GAMEPAD_BUTTON_DPAD_UP         11
#define MED_GAMEPAD_BUTTON_DPAD_RIGHT      12
#define MED_GAMEPAD_BUTTON_DPAD_DOWN       13
#define MED_GAMEPAD_BUTTON_DPAD_LEFT       14
#define MED_GAMEPAD_BUTTON_LAST            MED_GAMEPAD_BUTTON_DPAD_LEFT

#define MED_GAMEPAD_BUTTON_CROSS       MED_GAMEPAD_BUTTON_A
#define MED_GAMEPAD_BUTTON_CIRCLE      MED_GAMEPAD_BUTTON_B
#define MED_GAMEPAD_BUTTON_SQUARE      MED_GAMEPAD_BUTTON_X
#define MED_GAMEPAD_BUTTON_TRIANGLE    MED_GAMEPAD_BUTTON_Y


#define MED_GAMEPAD_AXIS_LEFT_X        0
#define MED_GAMEPAD_AXIS_LEFT_Y        1
#define MED_GAMEPAD_AXIS_RIGHT_X       2
#define MED_GAMEPAD_AXIS_RIGHT_Y       3
#define MED_GAMEPAD_AXIS_LEFT_TRIGGER  4
#define MED_GAMEPAD_AXIS_RIGHT_TRIGGER 5
#define MED_GAMEPAD_AXIS_LAST          MED_GAMEPAD_AXIS_RIGHT_TRIGGER

/* Event Types */
#define MED_RELEASE                0
#define MED_PRESS                  1
#define MED_REPEAT                 2

#define MED_HAT_CENTERED           0
#define MED_HAT_UP                 1
#define MED_HAT_RIGHT              2
#define MED_HAT_DOWN               4
#define MED_HAT_LEFT               8
#define MED_HAT_RIGHT_UP           (MED_HAT_RIGHT | MED_HAT_UP)
#define MED_HAT_RIGHT_DOWN         (MED_HAT_RIGHT | MED_HAT_DOWN)
#define MED_HAT_LEFT_UP            (MED_HAT_LEFT  | MED_HAT_UP)
#define MED_HAT_LEFT_DOWN          (MED_HAT_LEFT  | MED_HAT_DOWN)

class Input {
public:
    Medium* medium = nullptr;

    using EventCallback = std::function<void()>;
    using GlobalEventCallback = std::function<void(int, int, int, double, double)>;

    virtual void initializeInput(Medium* medium_) {
        medium = medium_;
    }

    virtual ~Input() = default;

    virtual bool isKeyPressed(int keycode) = 0;
    virtual bool isMouseButtonPressed(int button) = 0;
    virtual std::pair<float, float> getMousePosition() = 0;
    virtual float getMouseX() = 0;
    virtual float getMouseY() = 0;

    void registerEventCallback(int eventObject, int eventType, const EventCallback& callback) {
        eventCallbacks[{eventObject, eventType}] = callback;
    }

    void registerGlobalCallback(const GlobalEventCallback& callback) {
        globalCallback = callback;
    }

protected:
    virtual void eventCallbackImpl(int eventObject, int eventType) {
        auto it = eventCallbacks.find({eventObject, eventType});
        if (it != eventCallbacks.end()) {
            it->second();
        }
    }

    virtual void globalCallbackImpl(int object, int action, int mods, double x, double y) {
        if (globalCallback)
            globalCallback(object, action, mods, x, y);
    }

    GlobalEventCallback globalCallback;
    std::map<std::pair<int, int>, EventCallback> eventCallbacks;
};

inline char keycodeToChar(const int keycode, const int mods)
{
    const bool shift = (mods & MED_MOD_SHIFT) != 0;
    const bool caps  = (mods & MED_MOD_CAPS_LOCK) != 0;

    // Letters: Shift XOR Caps Lock = uppercase
    if (keycode >= MED_KEY_A && keycode <= MED_KEY_Z)
    {
        bool upper = shift ^ caps;
        return static_cast<char>((upper ? 'A' : 'a') + (keycode - MED_KEY_A));
    }

    // Number row
    switch (keycode)
    {
        case MED_KEY_0: return shift ? ')' : '0';
        case MED_KEY_1: return shift ? '!' : '1';
        case MED_KEY_2: return shift ? '@' : '2';
        case MED_KEY_3: return shift ? '#' : '3';
        case MED_KEY_4: return shift ? '$' : '4';
        case MED_KEY_5: return shift ? '%' : '5';
        case MED_KEY_6: return shift ? '^' : '6';
        case MED_KEY_7: return shift ? '&' : '7';
        case MED_KEY_8: return shift ? '*' : '8';
        case MED_KEY_9: return shift ? '(' : '9';

        case MED_KEY_SPACE:        return ' ';
        case MED_KEY_APOSTROPHE:   return shift ? '"' : '\'';
        case MED_KEY_COMMA:        return shift ? '<' : ',';
        case MED_KEY_MINUS:        return shift ? '_' : '-';
        case MED_KEY_PERIOD:       return shift ? '>' : '.';
        case MED_KEY_SLASH:        return shift ? '?' : '/';
        case MED_KEY_SEMICOLON:    return shift ? ':' : ';';
        case MED_KEY_EQUAL:        return shift ? '+' : '=';
        case MED_KEY_LEFT_BRACKET: return shift ? '{' : '[';
        case MED_KEY_BACKSLASH:    return shift ? '|' : '\\';
        case MED_KEY_RIGHT_BRACKET:return shift ? '}' : ']';
        case MED_KEY_GRAVE_ACCENT: return shift ? '~' : '`';

        // Keypad (Num Lock assumed on if present)
        case MED_KEY_KP_0: return '0';
        case MED_KEY_KP_1: return '1';
        case MED_KEY_KP_2: return '2';
        case MED_KEY_KP_3: return '3';
        case MED_KEY_KP_4: return '4';
        case MED_KEY_KP_5: return '5';
        case MED_KEY_KP_6: return '6';
        case MED_KEY_KP_7: return '7';
        case MED_KEY_KP_8: return '8';
        case MED_KEY_KP_9: return '9';
        case MED_KEY_KP_DECIMAL: return '.';
        case MED_KEY_KP_DIVIDE:  return '/';
        case MED_KEY_KP_MULTIPLY:return '*';
        case MED_KEY_KP_SUBTRACT:return '-';
        case MED_KEY_KP_ADD:     return '+';
        case MED_KEY_KP_EQUAL:   return '=';

        default:
            return '\0'; // non-printable key
    }
}

#endif //MEDIUM_INPUT_H
