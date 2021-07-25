/*
* Copyright 2021 Conquer Space
*/
#include "client/systems/systurnsavewindow.h"


#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "engine/gui.h"

void conquerspace::client::systems::SysTurnSaveWindow::Init() {
}

void conquerspace::client::systems::SysTurnSaveWindow::DoUI(int delta_time) {
        // Turn window
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x, 30),
                            ImGuiCond_Always, ImVec2(1.f, 0.f));
    ImGui::SetNextWindowSize(ImVec2(150, 65), ImGuiCond_Always);
    bool to_show = true;
    ImGui::Begin("TS window", &to_show, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                     ImGuiWindowFlags_AlwaysAutoResize | window_flags);
    // Show date
    ImGui::TextFmt("Date: {} Speed: {}", GetApp().GetUniverse().date.GetDate(), tick_speed);
    if (ImGui::Button("<<")) {
        // Slower
        if (tick_speed > 0) {
            tick_speed--;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button(to_tick ? "Running" : "Paused")) {
        TogglePlayState();
    }
    ImGui::SameLine();
    if (ImGui::Button(">>")) {
        // Faster
        if (tick_speed < tick_speeds.size() - 1) {
            tick_speed++;
        }
    }
    ImGui::End();
}

void conquerspace::client::systems::SysTurnSaveWindow::DoUpdate(int delta_time) {
    if (!ImGui::GetIO().WantCaptureKeyboard) {
        if (GetApp().ButtonIsReleased(GLFW_KEY_SPACE)) {
            TogglePlayState();
        }
    }
    // Update tick
    if (to_tick && GetApp().GetTime() - last_tick >
                                            static_cast<float>(tick_speeds[tick_speed])/1000.f) {
        GetApp().GetUniverse().EnableTick();
        last_tick = GetApp().GetTime();
    }
}


void conquerspace::client::systems::SysTurnSaveWindow::TogglePlayState() {
    to_tick = !to_tick;
}