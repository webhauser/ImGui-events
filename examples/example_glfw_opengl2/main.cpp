/*
* Linux platform
* GLFW, OpenGL2, ImGui
*/
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"
#include <stdio.h>
#include <GLFW/glfw3.h>

#include "VRTGui.h"
#include "CircularBuffer.h"

#define CLICK_TIME 0.30
#define KEY_TIME 0.40

CircularBuffer<t_event> lo_level_events;
CircularBuffer<t_event> hi_level_events;

VRTGui *gui;

void drawTriangle() 
{
    glColor3f(1.0, 1.0, 1.0);
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);

        glBegin(GL_TRIANGLES);
                glVertex3f(-0.7, 0.7, 0);
                glVertex3f(0.7, 0.7, 0);
                glVertex3f(0, -1, 0);
        glEnd();

    glFlush();
}

void game_render(GLFWwindow* window) {
        int display_w, display_h;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

	drawTriangle();
}

/*
* Process low level event queue 
* and generate higher level events
*/
void process_input() {
	static struct t_event event1, event2;
	double wait_time;

	switch(lo_level_events.size()) {
	case 0:	break;
	case 1: 
		event1 = lo_level_events.elem(0);
		wait_time = event1.what == 'm' ? CLICK_TIME : KEY_TIME;
		if(glfwGetTime() - event1.time > wait_time) {
			lo_level_events.removeFirst();
			hi_level_events.insert(event1);
		}
		break;
	default:
		event1 = lo_level_events.elem(0);
		event2 = lo_level_events.elem(1);
		switch(event1.what) {
		case 'm': 
			if(event1.what==event2.what && 					// Mouse events
			   event1.uni.mouse.type==0 && event2.uni.mouse.type==0 && 	// Mouse key events
			   event1.uni.mouse.button==event2.uni.mouse.button && 		// Same Mouse buttons
			   event1.uni.mouse.action==GLFW_PRESS && event2.uni.mouse.action==GLFW_RELEASE &&	// Press and Release in order
			  (event2.time - event1.time <= CLICK_TIME) ) {			// Click detected
				event2.uni.mouse.action = 2;
				hi_level_events.insert(event2);
			} else {
				hi_level_events.insert(event1);
				hi_level_events.insert(event2);
			}
			break;
		case 'k':
			if(event1.what==event2.what && 					// Keyboard events
			   event1.uni.keyboard.code==event2.uni.keyboard.code &&	// Same keys
			   event1.uni.keyboard.action==GLFW_PRESS && event2.uni.keyboard.action==GLFW_RELEASE && // Press and Release in order
			  (event2.time - event1.time <= KEY_TIME) ) {			// Keypress detected
				event2.uni.keyboard.action = 2;
				hi_level_events.insert(event2);
			} else {
				hi_level_events.insert(event1);
				hi_level_events.insert(event2);
			}
			break;
		}
		lo_level_events.removeFirst();
		lo_level_events.removeFirst();
		break;
	} 
}

/*
* Process high level event queue
*/
void game_update() {
	static struct t_event event;
	if(hi_level_events.size() > 0) {
		event = hi_level_events.elem(0);
		gui->print_event(event);
		hi_level_events.removeFirst();
	}
}

/*
* GLFW keyboard events handler
* Adds the event to the low level event buffer
*/
static void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	static struct t_event event;
	ImGuiIO& io = ImGui::GetIO();
	if(!io.WantCaptureKeyboard) {
		event.what = 'k';
		event.time = glfwGetTime();
		event.uni.keyboard.action = action;
		event.uni.keyboard.code = scancode;
		event.uni.keyboard.mods = mods;
		lo_level_events.insert(event);
	}
}

/*
* GLFW Mouse events handler
* Adds the event to the low level event buffer
*/
static void mouse_key_callback(GLFWwindow* window, int button, int action, int mods) {
	static struct t_event event;
	ImGuiIO& io = ImGui::GetIO();
	if(!io.WantCaptureMouse) {
		// Create and add event to low level buffer
		event.what = 'm';
		event.time = glfwGetTime();
		event.uni.mouse.type   = 0;
		event.uni.mouse.action = action;
		event.uni.mouse.button = button;
		event.uni.mouse.mods   = mods;
		lo_level_events.insert(event);
	}
}

/*
* GLFW Mouse events handler
* Adds the event to the low level event buffer
*/
static void mouse_move_callback(GLFWwindow* window, double xpos, double ypos) {
	static struct t_event event;
	ImGuiIO& io = ImGui::GetIO();
	if(!io.WantCaptureMouse) {
		// Create and add event to low level buffer
		event.what = 'm';
		event.time = glfwGetTime();
		event.uni.mouse.type = 1;
		event.uni.mouse.x = xpos;
		event.uni.mouse.y = ypos;
		lo_level_events.insert(event);
	}
}

/*
* System code
*/

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main(void)
{
	int count=0;
	GLFWwindow* window;
    
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return -1;

	if((window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL))==NULL) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

	/*
	* === Setup Own Keyboard and Mouse handlers ===
	*/
	glfwSetKeyCallback(window, keyboard_callback);
	glfwSetMouseButtonCallback(window, mouse_key_callback);
	glfwSetCursorPosCallback(window, mouse_move_callback);
	
	/*
	* Setup Dear ImGui context
	*/
	
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	//ImGui::StyleColorsClassic();
	ImGui::StyleColorsDark();
	
	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL2_Init();

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);		
	
    gui = new VRTGui(window);

    while (!glfwWindowShouldClose(window))
    {
	/*
	* Render the Game
	*/
	game_render(window);

        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
		
	/*
	* Display Gui 
	*/
	gui->render();
	
        ImGui::Render();

	/*
	* Update the Game
	*/
	//if(++count & 0x0F) 
	process_input();

	game_update();

        // If you are using this code with non-legacy OpenGL header/contexts (which you should not, prefer using imgui_impl_opengl3.cpp!!),
        // you may need to backup/reset/restore other state, e.g. for current shader using the commented lines below.
        //GLint last_program;
        //glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
        //glUseProgram(0);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        //glUseProgram(last_program);

        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
    }

    // Delete my gui
    delete gui;

    // Cleanup
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}