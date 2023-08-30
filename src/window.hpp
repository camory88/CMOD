#ifndef WINDOW_HPP
#define WINDOW_HPP

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <thread>
#include <functional>
#include "Modules.h"

static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

bool isRunning = true;
bool canCloseProccess = false;

static bool color_menu = false;

static bool testing_menu = true;
static bool glow_menu = true;
static bool weapon_menu = true;
static bool aimbot_menu = true;
static bool misc_menu = true;

static void CheatMeun(Memory &mem)
{
    if (testing_menu)
    {
        ImGui::Begin("Lobby");
        for (int i = 0; i < 120; ++i)
        {
            uint64_t entityPTR = GetEntitys(mem, i);
            if (entityPTR == 0x0)
                continue;

            Entity entity(entityPTR);
            if(!entity.isPlayer(mem))
                continue;
            char name[33] = { 0 };
            entity.get_Name(mem,i,name);

            char class_name[33] = { 0 };
            entity.get_class_name(mem,class_name);

            ImGui::Text(class_name);
            ImGui::SameLine();
            ImGui::Text(name);
        }
        //ImGui::PopStyleColor();
        ImGui::End();
    }
    if (aimbot_menu)
    {
        ImGui::Begin("Aimbot");
        ImGui::Text("AimBot ----------");
        ImGui::SameLine();
        ImGui::Checkbox("No Recoil with aimbot", &settings::aimbot_noRecoil);
        ImGui::SameLine();
        ImGui::Checkbox("Team cheack", &settings::teamCheack);
        ImGui::Text("FOV:%.1f", settings::FOV);
        ImGui::SameLine();
        ImGui::SliderFloat("FOV", &settings::FOV, 10.f, 1000.f);
        ImGui::Text("Smothnes:%1.f", settings::rcs_aimbot);
        ImGui::SameLine();
        ImGui::SliderFloat("Smothnes", &settings::rcs_aimbot, 2.f, 100.f);
        ImGui::Text("Aim bone:%i", settings::aimBone);
        ImGui::SameLine();
        ImGui::SliderInt("AimBone", &settings::aimBone, 0, 8);
        ImGui::Text("Dist:%i", settings::AimDist);
        ImGui::SameLine();
        ImGui::SliderInt("Dist", &settings::AimDist, 10, 600);

        ImGui::End();
    }
    if (weapon_menu)
    {
        ImGui::Begin("weapon");
        ImGui::Checkbox("Recoil", &settings::RSC);
        ImGui::SameLine();
        ImGui::SliderFloat("RCS", &settings::recoilcontrol, 2.f, 100.f);

        ImGui::End();
    }
    if (glow_menu)
    {
        ImGui::Begin("Glowing functions");

        ImGui::End();
    }
    if (color_menu)
    {
        ImGui::Begin("Glow Color CONFIG");

        ImGui::ColorPicker4("Player", (float *)&settings::player_Glow_color, ImGuiColorEditFlags_NoInputs);
        ImGui::ColorPicker4("Hand", (float *)&settings::hand_glow_color, ImGuiColorEditFlags_NoInputs);
        ImGui::ColorPicker4("Weapon", (float *)&settings::weapon_glow_color, ImGuiColorEditFlags_NoInputs);
        ImGui::ColorPicker4("Aim Target", (float *)&settings::target_glow_color, ImGuiColorEditFlags_NoInputs);
        ImGui::End();
    }

    if (misc_menu)
    {
        ImGui::Begin("Misc");

        
        ImGui::Checkbox("auto jumpGraple for path", &settings::autojumpPath);
        ImGui::End();
    }
}
void StatMenu(Memory &mem)
{
    ImGui::Begin("Cheats List");

    ImGui::Checkbox("Aim Bot", &settings::aimbot);
    ImGui::SameLine();
    ImGui::Checkbox("weapon", &weapon_menu);
    ImGui::SameLine();
    ImGui::Checkbox("Glow", &glow_menu);
    ImGui::SameLine();
    ImGui::Checkbox("Glow Color config", &color_menu);
    ImGui::SameLine();
    ImGui::Checkbox("Misc", &misc_menu);
    ImGui::SameLine();
    ImGui::Text(settings::GameMode);
    ImGui::End();

    CheatMeun(mem);
    CGlobalVars GlobalVars = mem.Read<CGlobalVars>(baseAddress + offsets::OFFSET_GlobalVars);
    mem.Read<CGlobalVars>(baseAddress + offsets::OFFSET_GlobalVars, GlobalVars);
    localPTR = mem.Read<uint64_t>(baseAddress + offsets::OFFSET_LOCAL_ENT);
    localWeponPTR = WeaponXEntity(getWeapon(mem)).ptr;
    // printf("------------------------\n");
    entity_loop(mem);
    localPlayer_function(mem);
      
}




// Main code
int RunWindow(Memory &mem)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // GL 3.0 + GLSL 130
    const char *glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

    // Create window with graphics context
    GLFWwindow *window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform Windows
    //  io.ConfigViewportsNoAutoMerge = true;
    //  io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle &style = ImGui::GetStyle();

    style.WindowPadding = ImVec2(15, 15);
    style.WindowRounding = 5.0f;
    style.FramePadding = ImVec2(5, 5);
    style.FrameRounding = 4.0f;
    style.ItemSpacing = ImVec2(12, 8);
    style.ItemInnerSpacing = ImVec2(8, 6);
    style.IndentSpacing = 25.0f;
    style.ScrollbarSize = 15.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabMinSize = 5.0f;
    style.GrabRounding = 3.0f;

    style.Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.19f, 0.18f, 0.21f, 1.00f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    while (!glfwWindowShouldClose(window))
    {

        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).

        StatMenu(mem);

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow *backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
    }
    isRunning = false;

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    UI = false;
    canCloseProccess = true;

    return 0;
}
#endif