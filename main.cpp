#include <assert.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
/* #include "imgui_impl_opengl3_loader.h" */
#if defined(BOB)
#include <GL/glew.h>
#else
#include <GLES3/gl3.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

// C++ Standard Library
#include <cmath>
#include <cstring>
#include <cstdlib>

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

// ERROR ERROR
void GLAPIENTRY
MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
    ImGui::Begin("OpenGL Error Place");
    ImGui::Text("GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, severity, message
            );
    ImGui::End();
}
int main(int, char**)
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    int small = 650;
    int display_w = 1000, display_h = small;

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(
        display_w,
        display_h,                          // width, height
        "Texture as a screen",  // title
        NULL,                               // monitor
        NULL                                // share
        );

    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

#if defined(BOB)
    glewExperimental = true;
    glewInit();
#endif

    // ERROR ERROR
    // During init, enable debug output
    //glEnable              ( GL_DEBUG_OUTPUT );
    //glDebugMessageCallback( MessageCallback, 0 );

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    /* ImGui::StyleColorsClassic(); */
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Texture dimensions
    const int tex_w = 640;
    const int tex_h = 380;
    const int tex_channel_count = 1;

    // Texture buffer (CPU-side)
    uint8_t* texture_data_buffer = nullptr;

    texture_data_buffer = (uint8_t*)std::malloc(tex_channel_count * tex_w * tex_h * sizeof(uint8_t));
    std::memset(texture_data_buffer, 0, tex_channel_count * tex_w * tex_h * sizeof(uint8_t));

    // Create a single-channel texture
    unsigned int texture_channel_format;
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // 4 channel texture (RED-GREEN-BLUE-ALPHA)
    if (tex_channel_count == 4)
    {
        texture_channel_format = GL_RGBA;
    }
    // 3 channel texture (RED-GREEN-BLUE)
    else if (tex_channel_count == 3)
    {
        texture_channel_format = GL_RGB;
    }
    // 2 channel texture (RED-GREEN)
    else if (tex_channel_count == 2)
    {
        texture_channel_format = GL_RG;
    }
    // 1 channel texture (RED)
    else if (tex_channel_count == 1)
    {
        texture_channel_format = GL_RED;
    }
    else
    {
        fprintf(stderr, "NOT GOOD\n");
        return 1;
    }

    /**
     * You can use a single channel texture. Its less data to upload, but by default, ImGui renders it with the default
     * single channel color, which is RED. If you wanted to be conservative with your uploads you can:
     *
     * - use a single channel texture
     * - use a shader to remap the channel data (red value) to your screen pixel brightness as some other colors
     * - render this to a frambuffer (FBO) texture
     * - pass the FBO texture target to ImGui::Image, and not the single channel source
     */

    // Using tex_channel_count > 2 sort of tanks my framerate, so this ^^^ is probably the way to go if you need the brightness
    // color to be something OTHER than red

    glTexImage2D(GL_TEXTURE_2D, 0, texture_channel_format, tex_w, tex_h, 0, texture_channel_format, GL_UNSIGNED_BYTE, texture_data_buffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_w, tex_h, 0, GL_RGB, GL_UNSIGNED_BYTE, texture_data_buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // This is just seasoning
    int iteration = 0;

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glClearColor(0.5f, 0.2f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glViewport(0, 0, small, small);

        // Update the CPU-side texture buffer
        const int n_pixel_components = tex_channel_count * tex_h * tex_w;
        for (int i = 0; i < n_pixel_components; ++i)
        {
            texture_data_buffer[i] = (i + iteration) % 255;
        }

        // Add flavor...
        ++iteration;

        // Upload the updated buffer
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexSubImage2D(
            GL_TEXTURE_2D,
            0, // level of detail (LOD); not applicable here, not using mip-maps
            0, // bottom left corner x
            0, // bottom left corner y
            tex_w, // replacing whole texture (width)
            tex_h, // replacing whole texture (height)
            texture_channel_format,
            GL_UNSIGNED_BYTE,
            texture_data_buffer);
        glBindTexture(GL_TEXTURE_2D, 0);

        //Create a window to draw game stuff into
        ImGui::Begin("Me texture");
        ImGui::Text("Iteration %d", iteration);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Image(
                reinterpret_cast<void*>(texture), // user_texture_id,
                ImGui::GetContentRegionAvail(), //size; as big as is left in the window
                ImVec2(0, 0), // UV-coord, whole texture (bottom corner)
                ImVec2(1, 1), // UV-coord, whole texture (top corner)
                ImVec4(1,0,1,1) // tint color; make it green, not red
            );
        ImGui::End();

        // Rendering
        ImGui::Render();
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw imgui stuff to screen
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Maybe this is a good idea
    glDeleteTextures(1, &texture);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
