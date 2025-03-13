/**
 * @file Scaffold/Application.c
 */

#include <SDL2/SDL.h>
#include <GABLE/GABLE.h>
#include <Scaffold/Application.h>

// Application Context Structure ///////////////////////////////////////////////////////////////////

typedef struct Game_Application
{
    SDL_Window*         m_Window;
    SDL_Renderer*       m_Renderer;
    SDL_Texture*        m_RenderTarget;
    GABLE_Engine*       m_Engine;
    SDL_AudioDeviceID   m_AudioDevice;
    Float32             m_AudioBuffer[GAME_AUDIO_SAMPLE_BUFFER_SIZE];
    Index               m_AudioCursor;
    Bool                m_Running;
} Game_Application;

// Static Variables ////////////////////////////////////////////////////////////////////////////////

/**
 * @brief The number of engine instances created.
 * 
 * This is used to determine whether or not SDL has been initialized, and to ensure that SDL is only
 * initialized and quit once.
 */
static Uint32 s_EngineInstanceCount = 0;

// Static Function Prototypes - Event Loop Functions ///////////////////////////////////////////////

static void Game_OnKeyDown (Game_Application* p_App, SDL_Keycode p_Keycode);
static void Game_OnKeyUp (Game_Application* p_App, SDL_Keycode p_Keycode);
static void Game_OnGamepadButtonDown (Game_Application* p_App, SDL_GameControllerButton p_Button);
static void Game_OnGamepadButtonUp (Game_Application* p_App, SDL_GameControllerButton p_Button);
static void Game_HandleEvents (Game_Application* p_App);
static void Game_Update (Game_Application* p_App);
static void Game_RenderFrame (Game_Application* p_App);

// Static Function Prototypes - Frame and Mix Handlers /////////////////////////////////////////////

static void Game_OnFrameRendered (GABLE_Engine* p_Engine, GABLE_PPU* p_PPU);
static void Game_OnAudioMix (GABLE_Engine* p_Engine, const GABLE_AudioSample* p_Sample);

// Static Function Prototypes - Interrupt Handlers /////////////////////////////////////////////////

static Bool Game_HandleVerticalBlank (GABLE_Engine* p_Engine);
static Bool Game_HandleLCDStat (GABLE_Engine* p_Engine);
static Bool Game_HandleTimer (GABLE_Engine* p_Engine);
static Bool Game_HandleJoypad (GABLE_Engine* p_Engine);
static Bool Game_HandleRealtime (GABLE_Engine* p_Engine);
static Bool Game_HandleNetwork (GABLE_Engine* p_Engine);

// Static Functions - Event Loop Functions /////////////////////////////////////////////////////////

void Game_OnKeyDown (Game_Application* p_App, SDL_Keycode p_Keycode)
{
    // Button mappings for Game Boy controls
    // - Up:     `Up Arrow` or `W`
    // - Down:   `Down Arrow` or `S`
    // - Left:   `Left Arrow` or `A`
    // - Right:  `Right Arrow` or `D`
    // - A:      `Z` or `J`
    // - B:      `X` or `K`
    // - Start:  `Enter` or `Space`
    // - Select: `Right Shift` or `Left Shift`

    // Handle the key down event here.
    switch (p_Keycode)
    {
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
        case SDLK_RETURN:
        case SDLK_SPACE:
            GABLE_PressButton(p_App->m_Engine, GABLE_JB_START);
            break;
        case SDLK_RSHIFT:
        case SDLK_LSHIFT:
            GABLE_PressButton(p_App->m_Engine, GABLE_JB_SELECT);
            break;
        case SDLK_ESCAPE:
            p_App->m_Running = false;
            break;
    }
}

void Game_OnKeyUp (Game_Application* p_App, SDL_Keycode p_Keycode)
{
    // Handle the key up event here.
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
        case SDLK_RETURN:
        case SDLK_SPACE:
            GABLE_ReleaseButton(p_App->m_Engine, GABLE_JB_START);
            break;
        case SDLK_RSHIFT:
        case SDLK_LSHIFT:
            GABLE_ReleaseButton(p_App->m_Engine, GABLE_JB_SELECT);
            break;
    }
}

void Game_OnGamepadButtonDown (Game_Application* p_App, SDL_GameControllerButton p_Button)
{
    // Handle the gamepad button down event here.
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

void Game_OnGamepadButtonUp (Game_Application* p_App, SDL_GameControllerButton p_Button)
{
    // Handle the gamepad button up event here.
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

void Game_HandleEvents (Game_Application* p_App)
{
    SDL_Event l_Event;
    while (SDL_PollEvent(&l_Event))
    {
        switch (l_Event.type)
        {
            case SDL_KEYDOWN:
                Game_OnKeyDown(p_App, l_Event.key.keysym.sym);
                break;
            case SDL_KEYUP:
                Game_OnKeyUp(p_App, l_Event.key.keysym.sym);
                break;
            case SDL_CONTROLLERBUTTONDOWN:
                Game_OnGamepadButtonDown(p_App, l_Event.cbutton.button);
                break;
            case SDL_CONTROLLERBUTTONUP:
                Game_OnGamepadButtonUp(p_App, l_Event.cbutton.button);
                break;
            case SDL_QUIT:
                p_App->m_Running = false;
                break;
        }
    }
}

void Game_Update (Game_Application* p_App)
{
    // Update the game state here.
    //
    // First, check to see if new audio data should be enqueued.
    Uint32 l_QueuedAudioSize = SDL_GetQueuedAudioSize(p_App->m_AudioDevice);
    if (l_QueuedAudioSize < GAME_AUDIO_SAMPLE_COUNT_TARGET)
    {
        // Enqueue the current contents of the audio buffer.
        SDL_QueueAudio(p_App->m_AudioDevice, p_App->m_AudioBuffer, 
            p_App->m_AudioCursor * sizeof(Float32));
        p_App->m_AudioCursor = 0;
    }
}

void Game_RenderFrame (Game_Application* p_App)
{
    // Lock the render target for pixel access.
    Uint32* l_Pixels = NULL;
    Int32   l_Pitch  = 0;
    SDL_LockTexture(p_App->m_RenderTarget, NULL, (void**) &l_Pixels, &l_Pitch);

    // Copy the PPU's screen buffer into the render target.
    const Uint32* l_ScreenBuffer = GABLE_GetScreenBuffer(p_App->m_Engine);
    memcpy(l_Pixels, l_ScreenBuffer, GABLE_PPU_SCREEN_BUFFER_SIZE * sizeof(Uint32));

    // Unlock the render target.
    SDL_UnlockTexture(p_App->m_RenderTarget);

    // Clear the renderer and render the render target.
    SDL_RenderClear(p_App->m_Renderer);
    SDL_RenderCopy(p_App->m_Renderer, p_App->m_RenderTarget, NULL, NULL);
    SDL_RenderPresent(p_App->m_Renderer);
}

// Static Functions - Frame and Mix Handlers ///////////////////////////////////////////////////////

void Game_OnFrameRendered (GABLE_Engine* p_Engine, GABLE_PPU* p_PPU)
{
    // Handle events on each frame, even when the `VBLANK` interrupt is disabled.
    Game_Application* l_App = (Game_Application*) GABLE_GetUserdata(p_Engine);
    Game_HandleEvents(l_App);
}

void Game_OnAudioMix (GABLE_Engine* p_Engine, const GABLE_AudioSample* p_Sample)
{
    Game_Application* l_App = (Game_Application*) GABLE_GetUserdata(p_Engine);
    if (l_App->m_AudioCursor + 2 < GAME_AUDIO_SAMPLE_BUFFER_SIZE)
    {
        l_App->m_AudioBuffer[l_App->m_AudioCursor++] = p_Sample->m_Left;
        l_App->m_AudioBuffer[l_App->m_AudioCursor++] = p_Sample->m_Right;
    }
}

// Static Functions - Interrupt Handlers ///////////////////////////////////////////////////////////

Bool Game_HandleVerticalBlank (GABLE_Engine* p_Engine)
{
    // Update and render the game state here.
    Game_Application* l_App = (Game_Application*) GABLE_GetUserdata(p_Engine);
    Game_Update(l_App);
    Game_RenderFrame(l_App);

    // Handle the vertical blank interrupt here.
    return GABLE_ReturnFromInterrupt(p_Engine);
}

Bool Game_HandleLCDStat (GABLE_Engine* p_Engine)
{
    // Handle the LCD status interrupt here.
    return GABLE_ReturnFromInterrupt(p_Engine);
}

Bool Game_HandleTimer (GABLE_Engine* p_Engine)
{
    // Handle the timer interrupt here.
    return GABLE_ReturnFromInterrupt(p_Engine);
}

Bool Game_HandleNetwork (GABLE_Engine* p_Engine)
{
    // Handle the network interrupt here.
    return GABLE_ReturnFromInterrupt(p_Engine);
}

Bool Game_HandleJoypad (GABLE_Engine* p_Engine)
{
    // Handle the joypad interrupt here.
    return GABLE_ReturnFromInterrupt(p_Engine);
}

Bool Game_HandleRealtime (GABLE_Engine* p_Engine)
{
    // Handle the real-time clock interrupt here.
    return GABLE_ReturnFromInterrupt(p_Engine);
}

// Public Functions ////////////////////////////////////////////////////////////////////////////////

Game_Application* Game_CreateApplication ()
{
    // Check if SDL needs to be initialized.
    if (s_EngineInstanceCount == 0)
    {
        GABLE_expect(SDL_Init(SDL_INIT_EVERYTHING) == 0, "Failed to initialize SDL - %s!", 
            SDL_GetError());
    }

    // Allocate the application context.
    Game_Application* l_App = GABLE_calloc(1, Game_Application);
    GABLE_pexpect(l_App, "Failed to allocate memory for the application context");

    // Create the game window.
    l_App->m_Window = SDL_CreateWindow(GAME_WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    GABLE_expect(l_App->m_Window != NULL, "Failed to create window - %s!", SDL_GetError());

    // Create the renderer. Vsync is needed to properly throttle the engine's clock.
    l_App->m_Renderer = SDL_CreateRenderer(l_App->m_Window, -1, 
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    GABLE_expect(l_App->m_Renderer != NULL, "Failed to create renderer - %s!", SDL_GetError());

    // Create the render target texture. It's a streaming texture for per-pixel access.
    l_App->m_RenderTarget = SDL_CreateTexture(l_App->m_Renderer, SDL_PIXELFORMAT_RGBA8888, 
        SDL_TEXTUREACCESS_STREAMING, GABLE_PPU_SCREEN_WIDTH, GABLE_PPU_SCREEN_HEIGHT);
    GABLE_expect(l_App->m_RenderTarget != NULL, "Failed to create render target - %s!",
        SDL_GetError());

    // Open the audio device for mixing audio samples.
    SDL_AudioSpec l_DesiredSpec = {0}, l_ObtainedSpec = {0};
    l_DesiredSpec.freq = GABLE_AUDIO_SAMPLE_RATE;                   // 44.1 kHz
    l_DesiredSpec.format = AUDIO_F32;                               // 32-bit floating-point samples
    l_DesiredSpec.channels = 2;                                     // Stereo
    l_DesiredSpec.samples = GAME_AUDIO_SAMPLE_BUFFER_SIZE / 2;      // 1024 samples
    l_DesiredSpec.callback = NULL;                                  // Callback is not needed
    l_App->m_AudioDevice = SDL_OpenAudioDevice(NULL, 0, &l_DesiredSpec, &l_ObtainedSpec, 0);
    GABLE_expect(l_App->m_AudioDevice != 0, "Failed to open audio device - %s!", SDL_GetError());   

    // Create the GABLE Engine instance.
    l_App->m_Engine = GABLE_CreateEngine();
    GABLE_expect(l_App->m_Engine != NULL, "Failed to create GABLE Engine");

    // Hook up the frame and mix handlers to the engine's PPU and APU, respectively.
    GABLE_SetFrameRenderedCallback(l_App->m_Engine, Game_OnFrameRendered);
    GABLE_SetAudioMixCallback(l_App->m_Engine, Game_OnAudioMix);

    // Hook up the interrupt handlers to the engine's interrupt context.
    GABLE_SetInterruptHandler(l_App->m_Engine, GABLE_INT_VBLANK, Game_HandleVerticalBlank);
    GABLE_SetInterruptHandler(l_App->m_Engine, GABLE_INT_LCD_STAT, Game_HandleLCDStat);
    GABLE_SetInterruptHandler(l_App->m_Engine, GABLE_INT_TIMER, Game_HandleTimer);
    GABLE_SetInterruptHandler(l_App->m_Engine, GABLE_INT_NET, Game_HandleNetwork);
    GABLE_SetInterruptHandler(l_App->m_Engine, GABLE_INT_JOYPAD, Game_HandleJoypad);
    GABLE_SetInterruptHandler(l_App->m_Engine, GABLE_INT_RTC, Game_HandleRealtime);

    // Set the engine's user data to the application context.
    GABLE_SetUserdata(l_App->m_Engine, l_App);

    // Increment the engine instance count.
    s_EngineInstanceCount++;

    return l_App;
}

void Game_DestroyApplication (Game_Application* p_App)
{
    if (p_App != NULL)
    {
        // Pause the audio playback.
        SDL_PauseAudioDevice(p_App->m_AudioDevice, 1);

        if (p_App->m_Engine != NULL)        { GABLE_DestroyEngine(p_App->m_Engine); }
        if (p_App->m_RenderTarget != NULL)  { SDL_DestroyTexture(p_App->m_RenderTarget); }
        if (p_App->m_Renderer != NULL)      { SDL_DestroyRenderer(p_App->m_Renderer); }
        if (p_App->m_Window != NULL)        { SDL_DestroyWindow(p_App->m_Window); }
        if (p_App->m_AudioDevice != 0)      { SDL_CloseAudioDevice(p_App->m_AudioDevice); }

        GABLE_free(p_App);

        // Check if SDL needs to be quit.
        if (--s_EngineInstanceCount == 0)
        {
            SDL_Quit();
        }
    }
}

void Game_StartApplication (Game_Application* p_App)
{
    GABLE_expect(p_App != NULL, "Application context is NULL");

    // Prepare the application loop.
    //
    // Start by enabling the `VBLANK` interrupt and `IME` flag, because the process of updating and
    // rendering the game happens in the `VBLANK` interrupt handler.
    GABLE_SetInterruptEnable(p_App->m_Engine, GABLE_INT_VBLANK, true);
    GABLE_SetInterruptMasterEnable(p_App->m_Engine, true);

    // Begin the audio playback.
    SDL_PauseAudioDevice(p_App->m_AudioDevice, 0);

    // Set the application running flag.
    p_App->m_Running = true;

    // Wait here for the next vertical blank period to start.
    GABLE_WaitForVerticalBlank(p_App->m_Engine);

    // Disable the PPU.
    GABLE_DisplayControl l_LCDC = GABLE_GetDisplayControl(p_App->m_Engine);
    l_LCDC.m_DisplayEnable = false;
    GABLE_SetDisplayControl(p_App->m_Engine, l_LCDC);

    // Load the color test data into the data store.
    const GABLE_DataHandle* l_ColorTestHandle = GABLE_LoadDataFromFile(p_App->m_Engine, "color_test",
        "assets/color_test.2bpp", 0);
    GABLE_expect(l_ColorTestHandle != NULL, "Failed to load color test data");

    // Load Chris's spritesheet into the data store.
    const GABLE_DataHandle* l_ChrisHandle = GABLE_LoadDataFromFile(p_App->m_Engine, "chris",
        "assets/chris.2bpp", 0);
    GABLE_expect(l_ChrisHandle != NULL, "Failed to load Chris's spritesheet");  

    // Load the tile data from the data store into the PPU's VRAM.
    GABLE_UploadTileData(p_App->m_Engine, GABLE_2BPP, l_ColorTestHandle->m_Address, 10,
        l_ColorTestHandle->m_Length / 16);
    GABLE_UploadTileData(p_App->m_Engine, GABLE_2BPP, l_ChrisHandle->m_Address, 100,
        l_ChrisHandle->m_Length / 16);

    for (Uint8 i = 1; i <= (l_ColorTestHandle->m_Length / 16); ++i)
    {
        GABLE_SetBackgroundTileInfo(p_App->m_Engine, 10 + (i % 8), 10 + (i / 8), 10 + i);
    }

    // Set up the OAM objects to render the sprite.
    GABLE_SetObjectTileIndex(p_App->m_Engine, 0, 100);
    GABLE_SetObjectTileIndex(p_App->m_Engine, 1, 101);
    GABLE_SetObjectTileIndex(p_App->m_Engine, 2, 102);
    GABLE_SetObjectTileIndex(p_App->m_Engine, 3, 103);
    GABLE_SetObjectPosition(p_App->m_Engine, 0, 32, 40);
    GABLE_SetObjectPosition(p_App->m_Engine, 1, 40, 40);
    GABLE_SetObjectPosition(p_App->m_Engine, 2, 32, 48);
    GABLE_SetObjectPosition(p_App->m_Engine, 3, 40, 48);

    // Set the background palette colors.
    GABLE_SetBackgroundColor(p_App->m_Engine, 0, 0, GABLE_LookupColorPreset(GABLE_COLOR_GOLD), NULL);
    GABLE_SetBackgroundColor(p_App->m_Engine, 0, 1, GABLE_LookupColorPreset(GABLE_COLOR_ORANGE), NULL);
    GABLE_SetBackgroundColor(p_App->m_Engine, 0, 2, GABLE_LookupColorPreset(GABLE_COLOR_SILVER), NULL);
    GABLE_SetBackgroundColor(p_App->m_Engine, 0, 3, GABLE_LookupColorPreset(GABLE_COLOR_BLACK), NULL);

    // Set the object palette colors.
    GABLE_SetObjectColor(p_App->m_Engine, 0, 1, GABLE_LookupColorPreset(GABLE_COLOR_ORANGE), NULL);
    GABLE_SetObjectColor(p_App->m_Engine, 0, 2, GABLE_LookupColorPreset(GABLE_COLOR_SILVER), NULL);
    GABLE_SetObjectColor(p_App->m_Engine, 0, 3, GABLE_LookupColorPreset(GABLE_COLOR_BLACK), NULL);

    // Re-enable the PPU. Also enable the object layer.
    l_LCDC.m_DisplayEnable = true;
    l_LCDC.m_ObjectEnable = true;
    GABLE_SetDisplayControl(p_App->m_Engine, l_LCDC);

    // Start the application loop.
    GABLE_debug("Starting the application loop");
    while (p_App->m_Running == true)
    {

        // Elapse one cycle of the engine, to simulate the Game Boy reading the opcode of the
        // next instruction to be executed.
        GABLE_CycleEngine(p_App->m_Engine, 1);

    }    
}
