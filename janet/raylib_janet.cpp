#include <janet.h>

#include "raylib.h"

static unsigned char color_component(Janet value) {
    int32_t channel = janet_getinteger(value);
    if (channel < 0 || channel > 255) {
        janet_panicf("Color channel out of range: %d", channel);
    }
    return static_cast<unsigned char>(channel);
}

static Color color_from_args(Janet *argv, int32_t offset) {
    return Color{
        color_component(argv[offset]),
        color_component(argv[offset + 1]),
        color_component(argv[offset + 2]),
        color_component(argv[offset + 3])
    };
}

static Janet cfun_init_window(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 3);
    int width = janet_getinteger(argv, 0);
    int height = janet_getinteger(argv, 1);
    const char *title = janet_getcstring(argv, 2);
    InitWindow(width, height, title);
    return janet_wrap_nil();
}

static Janet cfun_set_target_fps(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 1);
    int fps = janet_getinteger(argv, 0);
    SetTargetFPS(fps);
    return janet_wrap_nil();
}

static Janet cfun_window_should_close(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 0);
    return janet_wrap_boolean(WindowShouldClose());
}

static Janet cfun_begin_drawing(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 0);
    BeginDrawing();
    return janet_wrap_nil();
}

static Janet cfun_end_drawing(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 0);
    EndDrawing();
    return janet_wrap_nil();
}

static Janet cfun_clear_background(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 4);
    ClearBackground(color_from_args(argv, 0));
    return janet_wrap_nil();
}

static Janet cfun_draw_text(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 8);
    const char *text = janet_getcstring(argv, 0);
    int pos_x = janet_getinteger(argv, 1);
    int pos_y = janet_getinteger(argv, 2);
    int font_size = janet_getinteger(argv, 3);
    DrawText(text, pos_x, pos_y, font_size, color_from_args(argv, 4));
    return janet_wrap_nil();
}

static Janet cfun_draw_circle(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 7);
    int center_x = janet_getinteger(argv, 0);
    int center_y = janet_getinteger(argv, 1);
    float radius = static_cast<float>(janet_getnumber(argv, 2));
    DrawCircle(center_x, center_y, radius, color_from_args(argv, 3));
    return janet_wrap_nil();
}

static Janet cfun_draw_rectangle(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 8);
    int pos_x = janet_getinteger(argv, 0);
    int pos_y = janet_getinteger(argv, 1);
    int width = janet_getinteger(argv, 2);
    int height = janet_getinteger(argv, 3);
    DrawRectangle(pos_x, pos_y, width, height, color_from_args(argv, 4));
    return janet_wrap_nil();
}

static Janet cfun_get_frame_time(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 0);
    return janet_wrap_number(static_cast<double>(GetFrameTime()));
}

static Janet cfun_close_window(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 0);
    CloseWindow();
    return janet_wrap_nil();
}

static Janet cfun_is_key_down(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 1);
    int key = janet_getinteger(argv, 0);
    return janet_wrap_boolean(IsKeyDown(key));
}

static Janet cfun_get_mouse_position(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 0);
    Vector2 pos = GetMousePosition();
    JanetTuple *tuple = janet_tuple_begin(2);
    janet_tuple_put(tuple, 0, janet_wrap_number(pos.x));
    janet_tuple_put(tuple, 1, janet_wrap_number(pos.y));
    return janet_wrap_tuple(janet_tuple_end(tuple));
}

static const JanetReg cfuns[] = {
    {"init-window", cfun_init_window, "(init-window width height title)\nInitialize a raylib window."},
    {"set-target-fps", cfun_set_target_fps, "(set-target-fps fps)\nSet target FPS."},
    {"window-should-close", cfun_window_should_close, "(window-should-close)\nCheck if the window should close."},
    {"begin-drawing", cfun_begin_drawing, "(begin-drawing)\nBegin drawing."},
    {"end-drawing", cfun_end_drawing, "(end-drawing)\nEnd drawing."},
    {"clear-background", cfun_clear_background, "(clear-background r g b a)\nClear background."},
    {"draw-text", cfun_draw_text, "(draw-text text x y size r g b a)\nDraw text."},
    {"draw-circle", cfun_draw_circle, "(draw-circle x y radius r g b a)\nDraw circle."},
    {"draw-rectangle", cfun_draw_rectangle, "(draw-rectangle x y width height r g b a)\nDraw rectangle."},
    {"get-frame-time", cfun_get_frame_time, "(get-frame-time)\nGet frame delta time."},
    {"close-window", cfun_close_window, "(close-window)\nClose raylib window."},
    {"is-key-down", cfun_is_key_down, "(is-key-down key)\nCheck if key is down."},
    {"get-mouse-position", cfun_get_mouse_position, "(get-mouse-position)\nGet mouse position tuple."},
    {nullptr, nullptr, nullptr}
};

JANET_MODULE_ENTRY(JanetTable *env) {
    janet_cfuns(env, nullptr, cfuns);
    janet_def(env, "KEY_UP", janet_wrap_integer(KEY_UP), "Arrow up key.");
    janet_def(env, "KEY_DOWN", janet_wrap_integer(KEY_DOWN), "Arrow down key.");
    janet_def(env, "KEY_LEFT", janet_wrap_integer(KEY_LEFT), "Arrow left key.");
    janet_def(env, "KEY_RIGHT", janet_wrap_integer(KEY_RIGHT), "Arrow right key.");
    janet_def(env, "KEY_SPACE", janet_wrap_integer(KEY_SPACE), "Space key.");
    janet_def(env, "KEY_ENTER", janet_wrap_integer(KEY_ENTER), "Enter key.");
    janet_def(env, "KEY_ESCAPE", janet_wrap_integer(KEY_ESCAPE), "Escape key.");
}
