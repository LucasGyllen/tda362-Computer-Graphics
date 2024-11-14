#include <GL/glew.h>
#include <stb_image.h>
#include <cstdlib>
#include <labhelper.h>
#include <imgui.h>
#include <imgui_impl_sdl_gl3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>

using namespace glm;

// Globals
SDL_Window* g_window = nullptr;
GLuint shaderProgram;
GLuint positionBuffer, colorBuffer, indexBuffer, texcoordBuffer, vertexArrayObject;
GLuint texture;
GLuint vertBuffer2, indexBuffer2, texcoordBuffer2, vertexArrayObject2;
GLuint texture2;
int mag = 1, mini = 5;
float anisotropy = 16.0f, camera_pan = 0.f;
bool showUI = false;

void initialize() {
    ENSURE_INITIALIZE_ONLY_ONCE();

    // Load and compile shaders
    shaderProgram = labhelper::loadShaderProgram("../lab2-textures/simple.vert", "../lab2-textures/simple.frag");

    // Create and bind VAO for main object
    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);
    const float positions[] = { -10.0f, 0.0f, -10.0f, -10.0f, 0.0f, -330.0f, 10.0f, 0.0f, -330.0f, 10.0f, 0.0f, -10.0f };
    glGenBuffers(1, &positionBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    float texcoords[] = { 0.0f, 0.0f, 0.0f, 15.0f, 1.0f, 15.0f, 1.0f, 0.0f };
    glGenBuffers(1, &texcoordBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, texcoordBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    const int indices[] = { 0, 1, 3, 1, 2, 3 };
    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Load the main texture (asphalt)
    int w, h, comp;
    unsigned char* image = stbi_load("../scenes/textures/asphalt.jpg", &w, &h, &comp, STBI_rgb_alpha);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
    stbi_image_free(image);

    // Explosion quad setup
    const float verts2[] = { 2.0f, 0.0f, -20.0f, 2.0f, 10.0f, -20.0f, 12.0f, 10.0f, -20.0f, 12.0f, 0.0f, -20.0f };
    float texcoords2[] = { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f };
    const int indices2[] = { 0, 1, 3, 1, 2, 3 };
    glGenBuffers(1, &vertBuffer2);
    glBindBuffer(GL_ARRAY_BUFFER, vertBuffer2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts2), verts2, GL_STATIC_DRAW);
    glGenBuffers(1, &texcoordBuffer2);
    glBindBuffer(GL_ARRAY_BUFFER, texcoordBuffer2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords2), texcoords2, GL_STATIC_DRAW);
    glGenBuffers(1, &indexBuffer2);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer2);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices2), indices2, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vertexArrayObject2);
    glBindVertexArray(vertexArrayObject2);
    glBindBuffer(GL_ARRAY_BUFFER, vertBuffer2);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, texcoordBuffer2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer2);

    // Load explosion texture
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    unsigned char* image2 = stbi_load("../scenes/textures/explosion.png", &w, &h, &comp, STBI_rgb_alpha);
    if (!image2) {
        std::cerr << "Failed to load explosion texture" << std::endl;
        return;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
    stbi_image_free(image2);

    // Set shader texture uniform
    glUseProgram(shaderProgram);
    int textureLocation = glGetUniformLocation(shaderProgram, "colortexture");
    glUniform1i(textureLocation, 0);
    glUseProgram(0);
}

void display() {
    // Viewport and clear settings
    int w, h;
    SDL_GetWindowSize(g_window, &w, &h);
    glViewport(0, 0, w, h);
    glClearColor(0.2f, 0.2f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glUseProgram(shaderProgram);

    // Set projection and camera position
    mat4 projectionMatrix = perspective(radians(45.0f), float(w) / float(h), 0.01f, 400.0f);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projectionMatrix"), 1, GL_FALSE, &projectionMatrix[0][0]);
    glUniform3f(glGetUniformLocation(shaderProgram, "cameraPosition"), camera_pan, 10, 0);

    // Draw main quad with asphalt texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(vertexArrayObject);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // Draw explosion quad with blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glBindVertexArray(vertexArrayObject2);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glDisable(GL_BLEND);

    glUseProgram(0);
}

void gui() {
    ImGui::PushID("mag");
    ImGui::Text("Magnification");
    ImGui::RadioButton("GL_NEAREST", &mag, 0);
    ImGui::RadioButton("GL_LINEAR", &mag, 1);
    ImGui::PopID();

    ImGui::PushID("mini");
    ImGui::Text("Minification");
    ImGui::RadioButton("GL_NEAREST", &mini, 0);
    ImGui::RadioButton("GL_LINEAR", &mini, 1);
    ImGui::RadioButton("GL_NEAREST_MIPMAP_NEAREST", &mini, 2);
    ImGui::RadioButton("GL_NEAREST_MIPMAP_LINEAR", &mini, 3);
    ImGui::RadioButton("GL_LINEAR_MIPMAP_NEAREST", &mini, 4);
    ImGui::RadioButton("GL_LINEAR_MIPMAP_LINEAR", &mini, 5);
    ImGui::PopID();

    ImGui::SliderFloat("Anisotropic filtering", &anisotropy, 1.0, 16.0);
    ImGui::SliderFloat("Camera Panning", &camera_pan, -1.0, 1.0);
}

int main(int argc, char* argv[]) {
    g_window = labhelper::init_window_SDL("OpenGL Lab 2");

    initialize();

    bool stopRendering = false;
    while (!stopRendering) {
        ImGui_ImplSdlGL3_NewFrame(g_window);
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSdlGL3_ProcessEvent(&event);
            if (event.type == SDL_QUIT || (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE)) {
                stopRendering = true;
            }
            else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_g) {
                showUI = !showUI;
            }
        }

        display();
        if (showUI) gui();
        ImGui::Render();
        SDL_GL_SwapWindow(g_window);
    }

    labhelper::shutDown(g_window);
    return 0;
}
