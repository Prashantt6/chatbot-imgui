#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>


bool show_chat_window = true;

std::vector<std::string>chattexts;
void loadchatsfromfile(const std::string& chats){
    std::ifstream file(chats);
    if(!file.is_open()){
        std::cerr<<"File not loaded";
    }
    chattexts.clear();
    std::string line ;
    
    while(std::getline (file , line )){
        chattexts.push_back(line);
    }
    file.close();
    
}

int main() {

    
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    // OpenGL version hints (3.3 Core)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 800, "IMGUI WINDOW", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        static bool show_chats_history = false;
        static bool history = false ;
        static bool send = true;

        loadchatsfromfile("data/chats.txt");

        // Chat window
        if (show_chat_window) {
            ImGui::Begin("Chatbot", &show_chat_window);
            
            ImGui::BeginChild("Chats", ImVec2(400, 300), true, ImGuiWindowFlags_HorizontalScrollbar);
            // ImGui::Text("Hello! This is your chatbot window.");
            if(!history){
                if (ImGui::Button("History")) {
                    show_chats_history = true;
                    history = true;
                }
            }
            else {
                if(ImGui::Button("Back")){
                    show_chats_history = false ;
                    history = false;
                }
            }

            ImGui::SameLine();
            ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Chats").x) * 0.5f);
            ImGui::Text("Chats");
            
           
            if(show_chats_history){
                ImGui::BeginChild("History " , ImVec2(200,300), true , ImGuiWindowFlags_HorizontalScrollbar);
                for(auto& chats : chattexts){
                ImGui::TextUnformatted(chats.c_str());
                }
                ImGui::EndChild();
             }

            ImGui::EndChild();






            ImGui::BeginChild("chat", ImVec2(400, 100), true, ImGuiWindowFlags_AlwaysHorizontalScrollbar);

            static char bigText[1024] = "";

            
            float availWidth = ImGui::GetContentRegionAvail().x;

            
            ImVec2 buttonSize(60, 80); 

            
            ImGui::InputTextMultiline(
                "##chatInput", 
                bigText, 
                IM_ARRAYSIZE(bigText), 
                ImVec2(availWidth - buttonSize.x - ImGui::GetStyle().ItemSpacing.x, buttonSize.y)
            );

            
            ImGui::SameLine();

            if (ImGui::Button("Send", buttonSize)) {
                send == true;
            }

            ImGui::EndChild();

            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
