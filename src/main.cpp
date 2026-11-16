#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <chrono>
#include <iostream>
#include <thread>
#include "App.h"

int main() {
    if (!glfwInit()) { std::cerr << "glfwInit failed\n"; return 1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    // full screen na primarnom monitoru, u njegovoj rezoluciji
    GLFWmonitor* mon = glfwGetPrimaryMonitor();
    const GLFWvidmode* vm = glfwGetVideoMode(mon);
    glfwWindowHint(GLFW_RED_BITS, vm->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, vm->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, vm->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, vm->refreshRate);
    GLFWwindow* win = glfwCreateWindow(vm->width, vm->height, "Lincstation", mon, nullptr);
    if (!win) { std::cerr << "Cannot create window (OpenGL 3.3 core required)\n"; glfwTerminate(); return 1; }

    glfwMakeContextCurrent(win);
    glfwSwapInterval(0);                                  // limiter radimo sami (75 FPS)
    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);  // sistemski kursor sakriven, crta se naš

    if (!gladLoadGL(glfwGetProcAddress)) { std::cerr << "gladLoadGL failed\n"; return 1; }

    {   // App (teksture, VAO/VBO, glifovi) mora da bude uništen PRE glfwTerminate(), dok kontekst još postoji
    App app(win);
    if (!app.init()) { return 1; }
    glfwSetWindowUserPointer(win, &app);

    glfwSetKeyCallback(win, [](GLFWwindow* w, int key, int, int action, int) {
        static_cast<App*>(glfwGetWindowUserPointer(w))->onKey(key, action);
    });
    glfwSetCharCallback(win, [](GLFWwindow* w, unsigned c) {
        static_cast<App*>(glfwGetWindowUserPointer(w))->onChar(c);
    });
    glfwSetCursorPosCallback(win, [](GLFWwindow* w, double x, double y) {
        static_cast<App*>(glfwGetWindowUserPointer(w))->onMouseMove(x, y);
    });
    glfwSetMouseButtonCallback(win, [](GLFWwindow* w, int b, int a, int) {
        static_cast<App*>(glfwGetWindowUserPointer(w))->onMouseButton(b, a);
    });

    // početna pozicija miša
    double cx, cy;
    glfwGetCursorPos(win, &cx, &cy);
    app.onMouseMove(cx, cy);

    using clk = std::chrono::steady_clock;
    const auto frame = std::chrono::microseconds(1'000'000 / 75);   // 75 FPS
    auto next = clk::now(), prev = next;

    while (!glfwWindowShouldClose(win)) {
        next += frame;
        auto now = clk::now();
        float dt = std::chrono::duration<float>(now - prev).count();
        prev = now;

        glfwPollEvents();
        app.update(dt);
        app.draw();
        glfwSwapBuffers(win);

        // limiter: spavaj do ~1 ms pre kraja frejma, pa čekaj aktivno (tačnije na Windows-u)
        std::this_thread::sleep_until(next - std::chrono::milliseconds(1));
        while (clk::now() < next) {}
    }

    }   // ~App: ovde se oslobađaju svi OpenGL objekti

    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}
