#include "VRTGui.h"


//-----------------------------------------------------------------------------
// [SECTION] Example App: Debug Log / ShowExampleAppLog()
//-----------------------------------------------------------------------------

// Usage:
//  static ExampleAppLog my_log;
//  my_log.AddLog("Hello %d world\n", 123);
//  my_log.Draw("title");
struct ExampleAppLog
{
    ImGuiTextBuffer     Buf;
    ImGuiTextFilter     Filter;
    ImVector<int>       LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
    bool                AutoScroll;  // Keep scrolling if already at the bottom.

    ExampleAppLog()
    {
        AutoScroll = true;
        Clear();
    }

    void    Clear()
    {
        Buf.clear();
        LineOffsets.clear();
        LineOffsets.push_back(0);
    }

    void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
    {
        int old_size = Buf.size();
        va_list args;
        va_start(args, fmt);
        Buf.appendfv(fmt, args);
        va_end(args);
        for (int new_size = Buf.size(); old_size < new_size; old_size++)
            if (Buf[old_size] == '\n')
                LineOffsets.push_back(old_size + 1);
    }

    void    Draw(const char* title, bool* p_open = NULL)
    {
        if (!ImGui::Begin(title, p_open))
        {
            ImGui::End();
            return;
        }

        // Options menu
        if (ImGui::BeginPopup("Options"))
        {
            ImGui::Checkbox("Auto-scroll", &AutoScroll);
            ImGui::EndPopup();
        }

        // Main window
        if (ImGui::Button("Options"))
            ImGui::OpenPopup("Options");
        ImGui::SameLine();
        bool clear = ImGui::Button("Clear");
        ImGui::SameLine();
        bool copy = ImGui::Button("Copy");
        ImGui::SameLine();
        Filter.Draw("Filter", -100.0f);

        ImGui::Separator();
        ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        if (clear)
            Clear();
        if (copy)
            ImGui::LogToClipboard();

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        const char* buf = Buf.begin();
        const char* buf_end = Buf.end();
        if (Filter.IsActive())
        {
            // In this example we don't use the clipper when Filter is enabled.
            // This is because we don't have a random access on the result on our filter.
            // A real application processing logs with ten of thousands of entries may want to store the result of
            // search/filter.. especially if the filtering function is not trivial (e.g. reg-exp).
            for (int line_no = 0; line_no < LineOffsets.Size; line_no++)
            {
                const char* line_start = buf + LineOffsets[line_no];
                const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                if (Filter.PassFilter(line_start, line_end))
                    ImGui::TextUnformatted(line_start, line_end);
            }
        }
        else
        {
            // The simplest and easy way to display the entire buffer:
            //   ImGui::TextUnformatted(buf_begin, buf_end);
            // And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
            // to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
            // within the visible area.
            // If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
            // on your side is recommended. Using ImGuiListClipper requires
            // - A) random access into your data
            // - B) items all being the  same height,
            // both of which we can handle since we an array pointing to the beginning of each line of text.
            // When using the filter (in the block of code above) we don't have random access into the data to display
            // anymore, which is why we don't use the clipper. Storing or skimming through the search result would make
            // it possible (and would be recommended if you want to search through tens of thousands of entries).
            ImGuiListClipper clipper;
            clipper.Begin(LineOffsets.Size);
            while (clipper.Step())
            {
                for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
                {
                    const char* line_start = buf + LineOffsets[line_no];
                    const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                    ImGui::TextUnformatted(line_start, line_end);
                }
            }
            clipper.End();
        }
        ImGui::PopStyleVar();

        if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);

        ImGui::EndChild();
        ImGui::End();
    }
};

static ExampleAppLog log;

// Demonstrate creating a simple log window with basic filtering.
void ShowExampleAppLog(bool* p_open)
{    
    // For the demo: add a debug button _BEFORE_ the normal log window contents
    // We take advantage of a rarely used feature: multiple calls to Begin()/End() are appending to the _same_ window.
    // Most of the contents of the window will be added by the log.Draw() call.
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Example: Log", p_open);
    if (ImGui::SmallButton("[Debug] Add 5 entries"))
    {
        static int counter = 0;
        const char* categories[3] = { "info", "warn", "error" };
        const char* words[] = { "Bumfuzzled", "Cattywampus", "Snickersnee", "Abibliophobia", "Absquatulate", "Nincompoop", "Pauciloquent" };
        for (int n = 0; n < 5; n++)
        {
            const char* category = categories[counter % IM_ARRAYSIZE(categories)];
            const char* word = words[counter % IM_ARRAYSIZE(words)];
            log.AddLog("[%05d] [%s] Hello, current time is %.1f, here's a word: '%s'\n",
                ImGui::GetFrameCount(), category, ImGui::GetTime(), word);
            counter++;
        }
    }
    ImGui::End();

    // Actually call in the regular Log helper (which will Begin() into the same window as we just did)
    log.Draw("Example: Log", p_open);
}

void VRTGui::menu_default() 
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {   
		ImGui::MenuItem("New");
		ImGui::MenuItem("Open...");
		ImGui::MenuItem("Close");
		ImGui::MenuItem("Save As...");
		ImGui::MenuItem("Log", NULL, &menu.is_log);
		if(ImGui::MenuItem("Exit")) 
			glfwSetWindowShouldClose(window,1);

		ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Editors"))
        {            
            if(ImGui::MenuItem("Visualizer", NULL, &menu.is_editor[VISUALIZER]))	{menu.clear_editor_flags(VISUALIZER);   setMode(VISUALIZER);}
            if(ImGui::MenuItem("World",    NULL, &menu.is_editor[WORLD_EDITOR])) 	{menu.clear_editor_flags(WORLD_EDITOR); setMode(WORLD_EDITOR);}
            if(ImGui::MenuItem("Shape",    NULL, &menu.is_editor[SHAPE_EDITOR])) 	{menu.clear_editor_flags(SHAPE_EDITOR); setMode(SHAPE_EDITOR);}
            if(ImGui::MenuItem("Image",    NULL, &menu.is_editor[IMAGE_EDITOR])) 	{menu.clear_editor_flags(IMAGE_EDITOR); setMode(IMAGE_EDITOR);}
            if(ImGui::MenuItem("Sound",    NULL, &menu.is_editor[SOUND_EDITOR])) 	{menu.clear_editor_flags(SOUND_EDITOR); setMode(SOUND_EDITOR);}
            if(ImGui::MenuItem("Keyboard", NULL, &menu.is_editor[KEYBOARD_EDITOR]))	{menu.clear_editor_flags(KEYBOARD_EDITOR); setMode(KEYBOARD_EDITOR);}
            if(ImGui::MenuItem("Layout",   NULL, &menu.is_editor[LAYOUT_EDITOR]))	{menu.clear_editor_flags(LAYOUT_EDITOR); setMode(LAYOUT_EDITOR);}
            if(ImGui::MenuItem("Resource", NULL, &menu.is_editor[RESOURCE_EDITOR]))	{menu.clear_editor_flags(RESOURCE_EDITOR); setMode(RESOURCE_EDITOR);}
            if(ImGui::MenuItem("Palette",  NULL, &menu.is_editor[PALETTE_EDITOR]))	{menu.clear_editor_flags(PALETTE_EDITOR); setMode(PALETTE_EDITOR);}
            ImGui::EndMenu();
        }        
        if (ImGui::BeginMenu("About"))
        {
		if(ImGui::MenuItem("About this"))
			menu.is_about=true;	

		ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if(menu.is_about) {
	ImGui::OpenPopup("Help");
	menu.is_about=false;
    }

    if (ImGui::BeginPopupModal("Help"))
    {
	ImGui::Text("1999-2021 (c) Istvan Hauser, E-Mail: webHauser@gmail.com\nVRT Visualizer for Linux v1.5.0\n");
	ImGui::Separator();
	if (ImGui::Button("OK", ImVec2(120, 0)))
		ImGui::CloseCurrentPopup();

	ImGui::EndPopup();
    }

    if(menu.is_log) ShowExampleAppLog(&menu.is_log);
}

void VRTGui::render_menu() {
	menu_default();
}

void VRTGui::render_toolbar() {}
void VRTGui::render_statusbar() {}
void VRTGui::render_movebar() {}

void VRTGui::render() {
	render_menu();
	render_toolbar();
	render_statusbar();
	render_movebar();
}

void VRTGui::print_event(struct t_event &e) {
	static const char *actions[] = { "Release", "Press", "Click" };
	switch(e.what) {
	case 'm':
		if(e.uni.mouse.type==0) log.AddLog("%f : MOUSE Key(%02X,%d) %s\n", e.time, e.uni.mouse.button, e.uni.mouse.mods, actions[e.uni.mouse.action & 3]);
		else log.AddLog("%f : MOUSE Move %f, %f\n", e.time, e.uni.mouse.x, e.uni.mouse.y);
		break;
	case 'k':
		log.AddLog("%f : KEY(%02X,%d) %s\n", e.time, e.uni.keyboard.code, e.uni.keyboard.mods, actions[e.uni.keyboard.action & 3]);
		break;
	}
}
