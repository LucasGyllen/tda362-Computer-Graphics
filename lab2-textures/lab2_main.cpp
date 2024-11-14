#include <GL/glew.h>
#include <stb_image.h>
#include <cstdlib>
#include <iostream>
#include <labhelper.h>
#include <imgui.h>
#include <imgui_impl_sdl_gl3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

using namespace glm;

// Various globals
SDL_Window* g_window = nullptr;

int mag = 1;
int mini = 5;
float anisotropy = 16.0f;
float camera_pan = 0.f;
bool showUI = false;

// Shader program
GLuint shaderProgram;

// Scene objects
GLuint positionBuffer1, texcoordBuffer1, indexBuffer1, vertexArrayObject1; // Quad 1 (Asphalt)
GLuint positionBuffer2, texcoordBuffer2, indexBuffer2, vertexArrayObject2; // Quad 2 (Explosion)
GLuint texture1, texture2;

// Function to initialize
void initialize() {
    ENSURE_INITIALIZE_ONLY_ONCE();

    ///////////////////////////////////////////////////////////////////////////
    // Create the vertex array and buffer objects for Quad 1 (Asphalt texture)
    ///////////////////////////////////////////////////////////////////////////
    glGenVertexArrays(1, &vertexArrayObject1);
    glBindVertexArray(vertexArrayObject1);

    const float positions1[] = {
        -10.0f, 0.0f, -10.0f,  // v0
        -10.0f, 0.0f, -330.0f, // v1
        10.0f,  0.0f, -330.0f, // v2
        10.0f,  0.0f, -10.0f   // v3
    };

    glGenBuffers(1, &positionBuffer1);
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions1), positions1, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(0);

    //float array texcoords[], where each pair represents the(u, v) coordinates for each vertex of the quad.
    float texcoords1[] = {
        0.0f, 0.0f,
        0.0f, 15.0f,
        1.0f, 15.0f,
        1.0f, 0.0f
    };

    //glGenBuffers creates a buffer ID, glBindBuffer binds it as the current array buffer, 
    // and glBufferData sends the texcoords data to the GPU.
    glGenBuffers(1, &texcoordBuffer1);
    glBindBuffer(GL_ARRAY_BUFFER, texcoordBuffer1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords1), texcoords1, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(1);

    const int indices1[] = {
        0, 1, 3,
        1, 2, 3
    };

    glGenBuffers(1, &indexBuffer1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer1);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices1), indices1, GL_STATIC_DRAW);

    ///////////////////////////////////////////////////////////////////////////
    // Create the vertex array and buffer objects for Quad 2 (Explosion texture)
    ///////////////////////////////////////////////////////////////////////////
    glGenVertexArrays(1, &vertexArrayObject2);
    glBindVertexArray(vertexArrayObject2);

    const float positions2[] = {
        2.0f,  0.0f,  -20.0f,
        2.0f,  10.0f, -20.0f,
        12.0f, 10.0f, -20.0f,
        12.0f, 0.0f,  -20.0f
    };

    glGenBuffers(1, &positionBuffer2);
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions2), positions2, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(0);

    float texcoords2[] = {
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f
    };

    glGenBuffers(1, &texcoordBuffer2);
    glBindBuffer(GL_ARRAY_BUFFER, texcoordBuffer2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords2), texcoords2, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(1);

    const int indices2[] = {
        0, 1, 3,
        1, 2, 3
    };

    glGenBuffers(1, &indexBuffer2);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer2);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices2), indices2, GL_STATIC_DRAW);

    ///////////////////////////////////////////////////////////////////////////
    // Load Shader Program
    ///////////////////////////////////////////////////////////////////////////
    shaderProgram = labhelper::loadShaderProgram("../lab2-textures/simple.vert", "../lab2-textures/simple.frag");

    ///////////////////////////////////////////////////////////////////////////
    // Load Texture 1 (Asphalt)
    ///////////////////////////////////////////////////////////////////////////
    int w, h, comp;
    unsigned char* image1 = stbi_load("../scenes/textures/asphalt.jpg", &w, &h, &comp, STBI_rgb_alpha);
    if (!image1) {
        std::cerr << "Failed to load asphalt texture" << std::endl;
        return;
    }

    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
    glGenerateMipmap(GL_TEXTURE_2D); //Task 5
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(image1);

    ///////////////////////////////////////////////////////////////////////////
    // Load Texture 2 (Explosion)
    ///////////////////////////////////////////////////////////////////////////
    unsigned char* image2 = stbi_load("../scenes/textures/explosion.png", &w, &h, &comp, STBI_rgb_alpha);
    if (!image2) {
        std::cerr << "Failed to load explosion texture" << std::endl;
        return;
    }

    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(image2);
}

// Display function
void display(void) {
    int w, h;
    SDL_GetWindowSize(g_window, &w, &h);
    glViewport(0, 0, w, h);

    glClearColor(0.2f, 0.2f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_CULL_FACE); 
    //glEnable(GL_CULL_FACE);

    glDisable(GL_DEPTH_TEST);
    glUseProgram(shaderProgram);

    float fovy = radians(45.0f);
    float aspectRatio = float(w) / float(h);
    mat4 projectionMatrix = perspective(fovy, aspectRatio, 0.01f, 400.0f);
    int loc = glGetUniformLocation(shaderProgram, "projectionMatrix");
    glUniformMatrix4fv(loc, 1, GL_FALSE, &projectionMatrix[0][0]);

    loc = glGetUniformLocation(shaderProgram, "cameraPosition");
    glUniform3f(loc, camera_pan, 10, 0);

    // Draw Quad 1 (Asphalt)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glBindVertexArray(vertexArrayObject1);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // Draw Quad 2 (Explosion)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glBindVertexArray(vertexArrayObject2);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glDisable(GL_BLEND);

    glUseProgram(0);
}

void gui() {
    // Begin ImGui frame
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

    ImGui::SliderFloat("Anisotropic filtering", &anisotropy, 1.0f, 16.0f, "Number of samples: %.0f");
    ImGui::Dummy({ 0, 20 });
    ImGui::SliderFloat("Camera Panning", &camera_pan, -1.0f, 1.0f);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
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
