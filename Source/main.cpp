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
bool running = true;
SDL_Window *window = NULL;
SDL_GLContext gl_context = NULL;
int host_width;
int host_height;

uint8_t active_palette_index = 0;
uint8_t palette [16] = { 0x30, 0x3f, 0x37, 0x3b, 0x0f, 0x0b, 0x00, 0x2f,
                         0x06, 0x0b, 0x01, 0x3e, 0x38, 0x0c, 0x08, 0x3c
};

const char *palette_strings [16] = { "0", "1", "2", "3",
                                     "4", "5", "6", "7",
                                     "8", "9", "A", "B",
                                     "C", "D", "E", "F"
};

/* Gui variables */
uint32_t palette_bar_height = 0;

/* 8 × 8 pixel tile */
uint8_t tile [64] = { 0 };
const char *tile_strings [64] = { "##00", "##01", "##02", "##03", "##04", "##05", "##06", "##07",
                                  "##08", "##09", "##0a", "##0b", "##0c", "##0d", "##0e", "##0f",
                                  "##10", "##11", "##12", "##13", "##14", "##15", "##16", "##17",
                                  "##18", "##19", "##1a", "##1b", "##1c", "##1d", "##1e", "##1f",
                                  "##20", "##21", "##22", "##23", "##24", "##25", "##26", "##27",
                                  "##28", "##29", "##2a", "##2b", "##2c", "##2d", "##2e", "##2f",
                                  "##30", "##31", "##32", "##33", "##34", "##35", "##36", "##37",
                                  "##38", "##39", "##3a", "##3b", "##3c", "##3d", "##3e", "##3f"
};


/*
 * Convert a 6-bit SMS colour into an ImColor.
 */
ImVec4 sms_to_imgui_colour (uint8_t colour, uint8_t hilight)
{
    uint16_t r = (0xff / 3) * ((colour & 0x03) >> 0);
    uint16_t g = (0xff / 3) * ((colour & 0x0c) >> 2);
    uint16_t b = (0xff / 3) * ((colour & 0x33) >> 4);

    if (hilight)
    {
        float scale = hilight * 0.1;
        r = (r * (1.0 - scale) + (255.0 * scale));
        g = (g * (1.0 - scale) + (255.0 * scale));
        b = (b * (1.0 - scale) + (255.0 * scale));
    }

    return (ImVec4) ImColor (r, g, b);
}


/*
 * Main menu bar (top)
 */
void menu_bar (void)
{
    if (ImGui::BeginMainMenuBar ())
    {
        if (ImGui::BeginMenu ("File"))
        {
            if (ImGui::MenuItem ("Quit"))
            {
                running = false;
            }

            ImGui::EndMenu ();
        }

        ImGui::EndMainMenuBar ();
    }
}


/*
 * Tile editing area.
 */
void editing_area (void)
{
    uint32_t free_height = host_height - palette_bar_height;
    uint32_t pixel_size = ((free_height * 0.8) - (2 * BORDER_SIZE)) / 8;
    uint32_t window_size = (8 * pixel_size) + (2 * BORDER_SIZE);

    ImGui::SetNextWindowPos (ImVec2 ((host_width - window_size) / 2, (free_height - window_size) / 2));
    ImGui::SetNextWindowSize (ImVec2 (window_size, window_size));

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoMove     | ImGuiWindowFlags_NoScrollbar;


    ImGui::Begin ("editing_area", NULL, window_flags);

    ImGui::PushStyleVar (ImGuiStyleVar_FrameRounding, 0.0f);
    ImGui::PushStyleVar (ImGuiStyleVar_ItemSpacing, ImVec2 (0.0f, 0.0f));

    for (uint32_t i = 0; i < 64; i++)
    {
        ImGui::PushStyleColor (ImGuiCol_Button,        sms_to_imgui_colour (palette [tile [i]], 0));
        ImGui::PushStyleColor (ImGuiCol_ButtonHovered, sms_to_imgui_colour (palette [tile [i]], 1));
        ImGui::PushStyleColor (ImGuiCol_ButtonActive,  sms_to_imgui_colour (palette [tile [i]], 2));

        if (ImGui::Button (tile_strings [i], ImVec2 (pixel_size, pixel_size)))
        {
            tile [i] = active_palette_index;
        }

        ImGui::PopStyleColor (3);

        if (i % 8 != 7)
        {
            ImGui::SameLine ();
        }
    }

    ImGui::PopStyleVar (2);

    ImGui::End ();
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
    palette_bar_height = height;

    ImGui::SetNextWindowPos (ImVec2 ((host_width - width) / 2, (host_height - height) - 16));
    ImGui::SetNextWindowSize (ImVec2 (width, height));

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoMove     | ImGuiWindowFlags_NoScrollbar;

    ImGui::Begin ("palette", NULL, window_flags);

    for (uint32_t i = 0; i < 16; i++)
    {
        ImGui::PushStyleColor (ImGuiCol_Button,        sms_to_imgui_colour (palette [i], 0));
        ImGui::PushStyleColor (ImGuiCol_ButtonHovered, sms_to_imgui_colour (palette [i], 1));
        ImGui::PushStyleColor (ImGuiCol_ButtonActive,  sms_to_imgui_colour (palette [i], 2));

        if (ImGui::Button (palette_strings [i], ImVec2 (button_width, button_height)))
        {
            active_palette_index = i;

            if (SDL_GetMouseState (NULL, NULL) & SDL_BUTTON (SDL_BUTTON_RIGHT))
            {
                /* Palette chooser */
            }
        }

        ImGui::PopStyleColor (3);

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
    while (running)
    {
        SDL_GetWindowSize (window, &host_width, &host_height);
        SDL_Event event;

        /* Handle input */
        while (SDL_PollEvent (&event))
        {
            /* Allow ImGui buttons to be clicked with the right mouse button */
            if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)
            {
                if (event.button.button == SDL_BUTTON_RIGHT)
                {
                    event.button.button = SDL_BUTTON_LEFT;
                }
            }

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
        editing_area ();
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
