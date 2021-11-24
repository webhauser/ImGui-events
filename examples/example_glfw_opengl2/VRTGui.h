#ifndef VRTGui_H
#define VRTGui_H

#include "imgui.h"
#include <GLFW/glfw3.h>

/*
button: GLFW_MOUSE_BUTTON_LEFT, GLFW_MOUSE_BUTTON_RIGHT
action: GLFW_PRESS, GLFW_RELEASE
mods:   GLFW_MOD_SHIFT		0x0001
mods:   GLFW_MOD_CONTROL	0x0002
mods:   GLFW_MOD_ALT		0x0004
mods:   GLFW_MOD_CONTROL	0x0008
*/
struct t_mouse_input {
	int type;		// 0=mouse key, 1=mouse move
	int action;		// 0=release, 1=press, 2=click (high level)
	int button;		// 0=left,  1=right
	int mods;
	double x,y;		// mouse position
};

/*
action: GLFW_PRESS, GLFW_REPEAT, GLFW_RELEASE
mods:   GLFW_MOD_SHIFT		0x0001
mods:   GLFW_MOD_CONTROL	0x0002
mods:   GLFW_MOD_ALT		0x0004
mods:   GLFW_MOD_CONTROL	0x0008
*/
struct t_keyboard_input {
	int action;		// 0=release, 1=press, 2=click (high level)
	int code;		// keyboard scancode with modifier keys
	int mods;
};

struct t_event {
	char what;	// k=keyboard, m=mouse
	double time;	// timestamp
	union {
		struct t_mouse_input mouse;
		struct t_keyboard_input keyboard;
	} uni;
};

enum EditorMode {
	VISUALIZER,
	WORLD_EDITOR,
	SHAPE_EDITOR,
	IMAGE_EDITOR,
	SOUND_EDITOR,
	KEYBOARD_EDITOR,
	LAYOUT_EDITOR,
	RESOURCE_EDITOR,
	PALETTE_EDITOR,
	//
	NUM_EDITORS
};

class VRTGui {
private:
	GLFWwindow* window;
	
	void menu_default();

	// Areas in Editor Mode
	void render_menu();
	void render_toolbar();
	void render_statusbar();
	void render_movebar();

public:
	EditorMode mode;

	struct t_menu_flags {
		bool is_log;
		// Editor menu
		bool is_editor[9];
		// About menu
		bool is_about;
		
		void clear() {
			is_log=false;
			// Editor menu
			clear_editor_flags(NUM_EDITORS);
			// About menu
			is_about=false;
		}

		void clear_editor_flags(int j) {
			for(int i=0; i<NUM_EDITORS; i++) {
				if(i!=j) is_editor[i]=false;
			}
		}
	} menu;

	VRTGui(GLFWwindow* _window): window(_window), mode(EditorMode::VISUALIZER) {
		menu.clear();
	}
	
	~VRTGui() {}
	
	inline void setMode(EditorMode _mode) { mode = _mode; }

	static void print_event(struct t_event &event);
	
	void render();	
};

#endif