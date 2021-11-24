# ImGui-events

Testing user input events under Linux using Dear ImGui, OpenGL and GLFW.

Converting low level GLFW keyboard and mouse events to higher level events (**Clicks**) using circular buffers.

Logging higher level input events to ImGui example dialog when the user acting from background (not from ImGui).

## Building from source and running
First make sure that OpenGL and GLFW libraries are installed then enter:

$ cd examples/example_glfw_opengl2

$ make

$ ./example_glfw_opengl2
