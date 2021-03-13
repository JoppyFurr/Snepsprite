#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <GL/gl3w.h>
#include <SDL2/SDL.h>

#include "imgui.h"
#include "examples/imgui_impl_sdl.h"
#include "examples/imgui_impl_opengl3.h"

#define BORDER_SIZE 8

/* Global state */
SDL_Window *window = NULL;
SDL_GLContext gl_context = NULL;
int host_width;
int host_height;

uint8_t palette [16] = { 0 };

const char *palette_strings [16] = { "0", "1", "2", "3",
                                     "4", "5", "6", "7",
                                     "8", "9", "A", "B",
                                     "C", "D", "E", "F"
};

/*
 * Main menu bar (top)
 */
void menu_bar (void)
{
    if (ImGui::BeginMainMenuBar ())
    {
        if (ImGui::BeginMenu ("File"))
        {
            if (ImGui::MenuItem ("New"))
            {
            }

            ImGui::EndMenu ();
        }

        ImGui::EndMainMenuBar ();
    }
}


/*
 * Palette bar (bottom)
 */
void palette_bar (void)
{
    /* Calculate button size */
    uint32_t button_width = ((host_width * 0.90f) - (8 * 17)) / 16;
    uint32_t button_height = (host_height * 0.05f) - (8 * 2);

    /* Enforce minimum height */
    button_height = (button_height > 20) ? button_height : 20;

    /* Calculate palette-bar size based on button size */
    uint32_t width = (16 * button_width) + (17 * BORDER_SIZE);
    uint32_t height = button_height + (2 * BORDER_SIZE);

    ImGui::SetNextWindowPos (ImVec2 ((host_width - width) / 2, (host_height - height) - 16));
    ImGui::SetNextWindowSize (ImVec2(width, height));

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoMove     | ImGuiWindowFlags_NoScrollbar;

    ImGui::Begin ("palette", NULL, window_flags);

    for (uint32_t i = 0; i < 16; i++)
    {
        ImGui::Button (palette_strings [i], ImVec2 (button_width, button_height));
        if (i < 15)
        {
            ImGui::SameLine ();
        }
    }

    ImGui::End ();
}

/*
 * Main GUI loop.
 */
int main_gui_loop (void)
{
    bool running = true;

    while (running)
    {
        SDL_GetWindowSize (window, &host_width, &host_height);
        SDL_Event event;

        /* Handle input */
        while (SDL_PollEvent (&event))
        {
            ImGui_ImplSDL2_ProcessEvent (&event);

            if (event.type == SDL_QUIT)
            {
                running = false;
            }
        }

        /* Render */
        ImGui_ImplOpenGL3_NewFrame ();
        ImGui_ImplSDL2_NewFrame (window);
        ImGui::NewFrame ();

        menu_bar ();
        palette_bar ();

        /* Draw to HW */
        ImGui::Render ();
        SDL_GL_MakeCurrent (window, gl_context);
        glViewport (0, 0, (int) ImGui::GetIO ().DisplaySize.x, (int) ImGui::GetIO ().DisplaySize.y);
        glClearColor (0.0, 0.0, 0.0, 0.0);
        glClear (GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData (ImGui::GetDrawData ());
        SDL_GL_SwapWindow (window);
    }

    return 0;
}


/*
 * Entry point
 */
int main (int argc, char **argv)
{
    if (SDL_Init (SDL_INIT_EVERYTHING) == -1)
    {
        fprintf (stderr, "SDL_Init failure: %s\n", SDL_GetError ());
        return EXIT_FAILURE;
    }

    /* Create a window */
    SDL_GL_SetAttribute (SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute (SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute (SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute (SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute (SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute (SDL_GL_CONTEXT_MINOR_VERSION, 2);
    window = SDL_CreateWindow ("Snepsprite", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               640, 480, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    if (window == NULL)
    {
        fprintf (stderr, "SDL_CreateWindow failure: %s\n", SDL_GetError ());
        SDL_Quit ();
        return EXIT_FAILURE;
    }

    gl_context = SDL_GL_CreateContext (window);
    if (gl_context == NULL)
    {
        fprintf (stderr, "SDL_GL_CreateContext failure: %s\n", SDL_GetError ());
        SDL_Quit ();
        return EXIT_FAILURE;
    }
    SDL_GL_SetSwapInterval (1);

    /* GL Loader */
    if (gl3wInit () != 0)
    {
        fprintf (stderr, "gl3wInit failed.\n");
        SDL_Quit ();
        return EXIT_FAILURE;
    }

    ImGui::CreateContext ();
    ImGui::GetIO ().IniFilename = NULL;
    ImGui_ImplSDL2_InitForOpenGL (window, gl_context);
    ImGui_ImplOpenGL3_Init ();

    /* Style */
    ImGui::GetStyle ().FrameRounding = 2.0f;

    main_gui_loop ();

    ImGui_ImplOpenGL3_Shutdown ();
    ImGui_ImplSDL2_Shutdown ();
    ImGui::DestroyContext ();

    SDL_GL_DeleteContext (gl_context);
    SDL_DestroyWindow (window);
    SDL_Quit ();

    return EXIT_SUCCESS;
}
