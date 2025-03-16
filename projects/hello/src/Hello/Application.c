/**
 * @file Hello/Application.c
 */

#include <SDL2/SDL.h>
#include <Hello/Application.h>

// Private Constants ///////////////////////////////////////////////////////////////////////////////

// Window Size Constants (10:9 Aspect Ratio)
#define HELLO_WINDOW_WIDTH          800
#define HELLO_WINDOW_HEIGHT         720

// Window Title
#define HELLO_WINDOW_TITLE          "Hello, World!"

// Audio Constants
#define HELLO_AUDIO_SAMPLE_SIZE 2048
#define HELLO_AUDIO_SAMPLE_QUEUE_TARGET 65536

// Application Context Structure ///////////////////////////////////////////////////////////////////

typedef struct Hello_Application
{

    // SDL Members
    SDL_Window*         m_Window;
    SDL_Renderer*       m_Renderer;
    SDL_Texture*        m_RenderTarget;
    SDL_AudioDeviceID   m_AudioDevice;

    // Engine Context
    GABLE_Engine*       m_Engine;

    // Audio Buffer
    Float32             m_AudioBuffer[HELLO_AUDIO_SAMPLE_SIZE];
    Uint32              m_AudioBufferIndex;

    // Application State
    Bool                m_Running;

} Hello_Application;

// Static Function Prototypes //////////////////////////////////////////////////////////////////////

static void Hello_OnKeyDown (Hello_Application* p_App, SDL_Keycode p_Keycode);
static void Hello_OnKeyUp (Hello_Application* p_App, SDL_Keycode p_Keycode);
static void Hello_OnGamepadButtonDown (Hello_Application* p_App, SDL_GameControllerButton p_Button);
static void Hello_OnGamepadButtonUp (Hello_Application* p_App, SDL_GameControllerButton p_Button);
static void Hello_HandleEvents (GABLE_Engine* p_Engine);
static void Hello_Update (GABLE_Engine* p_Engine);
static void Hello_Render (GABLE_Engine* p_Engine);
static void Hello_OnFrameRendered (GABLE_Engine* p_Engine, GABLE_PPU* p_PPU);
static void Hello_OnAudioMixed (GABLE_Engine* p_Engine, const GABLE_AudioSample* p_Sample);
static Bool Hello_OnVerticalBlank (GABLE_Engine* p_Engine);
static Bool Hello_OnDisplayStatus (GABLE_Engine* p_Engine);
static Bool Hello_OnTimer (GABLE_Engine* p_Engine);
static Bool Hello_OnNetwork (GABLE_Engine* p_Engine);
static Bool Hello_OnJoypad (GABLE_Engine* p_Engine);
static Bool Hello_OnRealtime (GABLE_Engine* p_Engine);

// Static Functions - SDL Event Handlers ///////////////////////////////////////////////////////////

static void Hello_OnKeyDown (Hello_Application* p_App, SDL_Keycode p_Keycode)
{
    // Key-mappings for Game Boy buttons:
    // - Up: Arrow Up Key, W Key
    // - Down: Arrow Down Key, S Key
    // - Left: Arrow Left Key, A Key
    // - Right: Arrow Right Key, D Key
    // - A: Z Key, J Key
    // - B: X Key, K Key
    // - Start: Spacebar, Enter Key
    // - Select: Shift Keys

    switch (p_Keycode)
    {
        case SDLK_ESCAPE:
            p_App->m_Running = false;
            break;
        case SDLK_UP:
        case SDLK_w:
            GABLE_PressButton(p_App->m_Engine, GABLE_JB_UP);
            break;
        case SDLK_DOWN:
        case SDLK_s:
            GABLE_PressButton(p_App->m_Engine, GABLE_JB_DOWN);
            break;
        case SDLK_LEFT:
        case SDLK_a:
            GABLE_PressButton(p_App->m_Engine, GABLE_JB_LEFT);
            break;
        case SDLK_RIGHT:
        case SDLK_d:
            GABLE_PressButton(p_App->m_Engine, GABLE_JB_RIGHT);
            break;
        case SDLK_z:
        case SDLK_j:
            GABLE_PressButton(p_App->m_Engine, GABLE_JB_A);
            break;
        case SDLK_x:
        case SDLK_k:
            GABLE_PressButton(p_App->m_Engine, GABLE_JB_B);
            break;
        case SDLK_SPACE:
        case SDLK_RETURN:
            GABLE_PressButton(p_App->m_Engine, GABLE_JB_START);
            break;
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:
            GABLE_PressButton(p_App->m_Engine, GABLE_JB_SELECT);
            break;
    }
}

static void Hello_OnKeyUp (Hello_Application* p_App, SDL_Keycode p_Keycode)
{
    switch (p_Keycode)
    {
        case SDLK_UP:
        case SDLK_w:
            GABLE_ReleaseButton(p_App->m_Engine, GABLE_JB_UP);
            break;
        case SDLK_DOWN:
        case SDLK_s:
            GABLE_ReleaseButton(p_App->m_Engine, GABLE_JB_DOWN);
            break;
        case SDLK_LEFT:
        case SDLK_a:
            GABLE_ReleaseButton(p_App->m_Engine, GABLE_JB_LEFT);
            break;
        case SDLK_RIGHT:
        case SDLK_d:
            GABLE_ReleaseButton(p_App->m_Engine, GABLE_JB_RIGHT);
            break;
        case SDLK_z:
        case SDLK_j:
            GABLE_ReleaseButton(p_App->m_Engine, GABLE_JB_A);
            break;
        case SDLK_x:
        case SDLK_k:
            GABLE_ReleaseButton(p_App->m_Engine, GABLE_JB_B);
            break;
        case SDLK_SPACE:
        case SDLK_RETURN:
            GABLE_ReleaseButton(p_App->m_Engine, GABLE_JB_START);
            break;
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:
            GABLE_ReleaseButton(p_App->m_Engine, GABLE_JB_SELECT);
            break;
    }
}

static void Hello_OnGamepadButtonDown (Hello_Application* p_App, SDL_GameControllerButton p_Button)
{
    // Gamepad-mappings for Game Boy buttons:
    // - Up: D-Pad Up
    // - Down: D-Pad Down
    // - Left: D-Pad Left
    // - Right: D-Pad Right
    // - A: Button A
    // - B: Button B
    // - Start: Button Start
    // - Select: Button Select/Back

    switch (p_Button)
    {
        case SDL_CONTROLLER_BUTTON_DPAD_UP:
            GABLE_PressButton(p_App->m_Engine, GABLE_JB_UP);
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            GABLE_PressButton(p_App->m_Engine, GABLE_JB_DOWN);
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            GABLE_PressButton(p_App->m_Engine, GABLE_JB_LEFT);
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            GABLE_PressButton(p_App->m_Engine, GABLE_JB_RIGHT);
            break;
        case SDL_CONTROLLER_BUTTON_A:
            GABLE_PressButton(p_App->m_Engine, GABLE_JB_A);
            break;
        case SDL_CONTROLLER_BUTTON_B:
            GABLE_PressButton(p_App->m_Engine, GABLE_JB_B);
            break;
        case SDL_CONTROLLER_BUTTON_START:
            GABLE_PressButton(p_App->m_Engine, GABLE_JB_START);
            break;
        case SDL_CONTROLLER_BUTTON_BACK:
            GABLE_PressButton(p_App->m_Engine, GABLE_JB_SELECT);
            break;
    }
}

static void Hello_OnGamepadButtonUp (Hello_Application* p_App, SDL_GameControllerButton p_Button)
{
    switch (p_Button)
    {
        case SDL_CONTROLLER_BUTTON_DPAD_UP:
            GABLE_ReleaseButton(p_App->m_Engine, GABLE_JB_UP);
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            GABLE_ReleaseButton(p_App->m_Engine, GABLE_JB_DOWN);
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            GABLE_ReleaseButton(p_App->m_Engine, GABLE_JB_LEFT);
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            GABLE_ReleaseButton(p_App->m_Engine, GABLE_JB_RIGHT);
            break;
        case SDL_CONTROLLER_BUTTON_A:
            GABLE_ReleaseButton(p_App->m_Engine, GABLE_JB_A);
            break;
        case SDL_CONTROLLER_BUTTON_B:
            GABLE_ReleaseButton(p_App->m_Engine, GABLE_JB_B);
            break;
        case SDL_CONTROLLER_BUTTON_START:
            GABLE_ReleaseButton(p_App->m_Engine, GABLE_JB_START);
            break;
        case SDL_CONTROLLER_BUTTON_BACK:
            GABLE_ReleaseButton(p_App->m_Engine, GABLE_JB_SELECT);
            break;
    }
}

// Static Functions - Application Loop /////////////////////////////////////////////////////////////

static void Hello_HandleEvents (GABLE_Engine* p_Engine)
{
    // Get the application context
    Hello_Application* l_App = (Hello_Application*) GABLE_GetUserdata(p_Engine);
    
    // Handle SDL Events
    SDL_Event l_Event;
    while (SDL_PollEvent(&l_Event))
    {
        switch (l_Event.type)
        {
            case SDL_QUIT:
                l_App->m_Running = false;
                break;
            case SDL_KEYDOWN:
                Hello_OnKeyDown(l_App, l_Event.key.keysym.sym);
                break;
            case SDL_KEYUP:
                Hello_OnKeyUp(l_App, l_Event.key.keysym.sym);
                break;
            case SDL_CONTROLLERBUTTONDOWN:
                Hello_OnGamepadButtonDown(l_App, l_Event.cbutton.button);
                break;
            case SDL_CONTROLLERBUTTONUP:
                Hello_OnGamepadButtonUp(l_App, l_Event.cbutton.button);
                break;
        }
    }
}

static void Hello_Update (GABLE_Engine* p_Engine)
{
    // Get the application context
    Hello_Application* l_App = (Hello_Application*) GABLE_GetUserdata(p_Engine);
}

static void Hello_Render (GABLE_Engine* p_Engine)
{
    // Get the application context
    Hello_Application* l_App = (Hello_Application*) GABLE_GetUserdata(p_Engine);

    // Lock the render target for writing.
    Uint32* l_Pixels = NULL;
    Int32   l_Pitch = 0;
    SDL_LockTexture(l_App->m_RenderTarget, NULL, (void**) &l_Pixels, &l_Pitch);

    // Copy the PPU's framebuffer to the render target.
    SDL_memcpy(l_Pixels, GABLE_GetScreenBuffer(p_Engine), GABLE_PPU_SCREEN_BUFFER_SIZE * sizeof(Uint32));

    // Unlock the render target.
    SDL_UnlockTexture(l_App->m_RenderTarget);

    // Clear, then render the render target to the window.
    SDL_RenderClear(l_App->m_Renderer);
    SDL_RenderCopy(l_App->m_Renderer, l_App->m_RenderTarget, NULL, NULL);
    SDL_RenderPresent(l_App->m_Renderer);
}

// Static Functions - Engine Callbacks /////////////////////////////////////////////////////////////

static void Hello_OnFrameRendered (GABLE_Engine* p_Engine, GABLE_PPU* p_PPU)
{
    // Get the application context
    Hello_Application* l_App = (Hello_Application*) GABLE_GetUserdata(p_Engine);

    // Handle SDL Events
    Hello_HandleEvents(p_Engine);

    // Check to see if the audio queue needs more samples.
    Uint32 l_QueuedAudio = SDL_GetQueuedAudioSize(l_App->m_AudioDevice);
    if (l_QueuedAudio < HELLO_AUDIO_SAMPLE_QUEUE_TARGET)
    {
        SDL_QueueAudio(l_App->m_AudioDevice, l_App->m_AudioBuffer, l_App->m_AudioBufferIndex * sizeof(Float32));
        l_App->m_AudioBufferIndex = 0;
    }
}

static void Hello_OnAudioMixed (GABLE_Engine* p_Engine, const GABLE_AudioSample* p_Sample)
{
    // Get the application context
    Hello_Application* l_App = (Hello_Application*) GABLE_GetUserdata(p_Engine);

    // Copy the audio samples to the buffer, if there is room.
    if (l_App->m_AudioBufferIndex + 2 <= HELLO_AUDIO_SAMPLE_SIZE)
    {
        l_App->m_AudioBuffer[l_App->m_AudioBufferIndex++] = p_Sample->m_Left;
        l_App->m_AudioBuffer[l_App->m_AudioBufferIndex++] = p_Sample->m_Right;
    }
}

// Static Functions - Interrupt Handlers ///////////////////////////////////////////////////////////

static Bool Hello_OnVerticalBlank (GABLE_Engine* p_Engine)
{
    // Update and render the application.
    Hello_Update(p_Engine);
    Hello_Render(p_Engine);

    // Re-enable interrupts and return.
    return GABLE_ReturnFromInterrupt(p_Engine);
}

static Bool Hello_OnDisplayStatus (GABLE_Engine* p_Engine)
{
    // Re-enable interrupts and return.
    return GABLE_ReturnFromInterrupt(p_Engine);
}

static Bool Hello_OnTimer (GABLE_Engine* p_Engine)
{
    // Re-enable interrupts and return.
    return GABLE_ReturnFromInterrupt(p_Engine);
}

static Bool Hello_OnNetwork (GABLE_Engine* p_Engine)
{
    // Re-enable interrupts and return.
    return GABLE_ReturnFromInterrupt(p_Engine);
}

static Bool Hello_OnJoypad (GABLE_Engine* p_Engine)
{
    // Re-enable interrupts and return.
    return GABLE_ReturnFromInterrupt(p_Engine);
}

static Bool Hello_OnRealtime (GABLE_Engine* p_Engine)
{
    // Re-enable interrupts and return.
    return GABLE_ReturnFromInterrupt(p_Engine);
}

// Public Functions ////////////////////////////////////////////////////////////////////////////////

Hello_Application* Hello_CreateApplication ()
{
    // Initialize SDL, if it hasn't been already.
    if (SDL_WasInit(0) == 0)
    {
        GABLE_expect(SDL_Init(SDL_INIT_EVERYTHING) == 0, 
            "Failed to initialize SDL - %s", SDL_GetError());
    }

    // Allocate the application context.
    Hello_Application* l_App = GABLE_calloc(1, Hello_Application);
    GABLE_pexpect(l_App != NULL, "Failed to allocate application context");

    // Create the SDL window.
    l_App->m_Window = SDL_CreateWindow(
        HELLO_WINDOW_TITLE, 
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, 
        HELLO_WINDOW_WIDTH, 
        HELLO_WINDOW_HEIGHT, 
        SDL_WINDOW_SHOWN
    );
    GABLE_expect(l_App->m_Window != NULL, "Failed to create SDL window - %s", SDL_GetError());

    // Create the SDL renderer. Vertical sync is needed to properly throttle the engine. For best
    // results, use a 60Hz display.
    l_App->m_Renderer = SDL_CreateRenderer(
        l_App->m_Window, 
        -1, 
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    GABLE_expect(l_App->m_Renderer != NULL, "Failed to create SDL renderer - %s", SDL_GetError());
    
    // Create the streaming renderer target texture.
    l_App->m_RenderTarget = SDL_CreateTexture(
        l_App->m_Renderer, 
        SDL_PIXELFORMAT_RGBA8888, 
        SDL_TEXTUREACCESS_STREAMING, 
        GABLE_PPU_SCREEN_WIDTH, 
        GABLE_PPU_SCREEN_HEIGHT
    );
    GABLE_expect(l_App->m_RenderTarget != NULL, "Failed to create SDL texture - %s", SDL_GetError());

    // Prepare the audio device.
    SDL_AudioSpec l_DesiredSpec = {0}, l_ObtainedSpec = {0};
    l_DesiredSpec.freq = GABLE_AUDIO_SAMPLE_RATE;
    l_DesiredSpec.format = AUDIO_F32;
    l_DesiredSpec.channels = 2;
    l_DesiredSpec.samples = 1024;
    l_DesiredSpec.callback = NULL;
    l_App->m_AudioDevice = SDL_OpenAudioDevice(NULL, 0, &l_DesiredSpec, &l_ObtainedSpec, 0);
    GABLE_expect(l_App->m_AudioDevice != 0, "Failed to open SDL audio device - %s", SDL_GetError());

    // Create the GABLE Engine instance.
    l_App->m_Engine = GABLE_CreateEngine();

    // Set up the renderer's clear color.
    SDL_SetRenderDrawColor(l_App->m_Renderer, 255, 255, 255, 255);

    return l_App;
}

void Hello_DestroyApplication (Hello_Application* p_App)
{
    if (p_App != NULL)
    {

        // Pause the audio device.
        SDL_PauseAudioDevice(p_App->m_AudioDevice, 1);

        // Destroy the GABLE Engine instance.
        if (p_App->m_Engine != NULL)        { GABLE_DestroyEngine(p_App->m_Engine); }

        // Close the audio device.
        if (p_App->m_AudioDevice != 0)      { SDL_CloseAudioDevice(p_App->m_AudioDevice); }

        // Destroy the SDL structures.
        if (p_App->m_RenderTarget != NULL)  { SDL_DestroyTexture(p_App->m_RenderTarget); }
        if (p_App->m_Renderer != NULL)      { SDL_DestroyRenderer(p_App->m_Renderer); }
        if (p_App->m_Window != NULL)        { SDL_DestroyWindow(p_App->m_Window); }

        // Free the application context.
        GABLE_free(p_App);

    }

    // Quit SDL, if it hasn't been quit already.
    if (SDL_WasInit(0) != 0) { SDL_Quit(); }
}

void Hello_StartApplication (Hello_Application* p_App)
{
    GABLE_expect(p_App != NULL, "Application context is NULL!");

    // Set the application context as the engine's userdata.
    GABLE_SetUserdata(p_App->m_Engine, p_App);

    // Set the engine's PPU and APU callbacks.
    GABLE_SetFrameRenderedCallback(p_App->m_Engine, Hello_OnFrameRendered);
    GABLE_SetAudioMixCallback(p_App->m_Engine, Hello_OnAudioMixed);

    // Set the engine's interrupt handlers.
    GABLE_SetInterruptHandler(p_App->m_Engine, GABLE_INT_VBLANK, Hello_OnVerticalBlank);
    GABLE_SetInterruptHandler(p_App->m_Engine, GABLE_INT_LCD_STAT, Hello_OnDisplayStatus);
    GABLE_SetInterruptHandler(p_App->m_Engine, GABLE_INT_TIMER, Hello_OnTimer);
    GABLE_SetInterruptHandler(p_App->m_Engine, GABLE_INT_NET, Hello_OnNetwork);
    GABLE_SetInterruptHandler(p_App->m_Engine, GABLE_INT_JOYPAD, Hello_OnJoypad);
    GABLE_SetInterruptHandler(p_App->m_Engine, GABLE_INT_RTC, Hello_OnRealtime);

    // Enable the engine's interrupts as needed, and the interrupt master enable bit.
    GABLE_SetInterruptMasterEnable(p_App->m_Engine, true);
    GABLE_SetInterruptEnable(p_App->m_Engine, GABLE_INT_VBLANK, true);

    // Start the audio device.
    SDL_PauseAudioDevice(p_App->m_AudioDevice, 0);

    // Start the application loop.
    p_App->m_Running = true;
    while (p_App->m_Running == true)
    {
        GABLE_CycleEngine(p_App->m_Engine, 1);
    }
}
