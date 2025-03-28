#include <SDL2/SDL.h>
#include <GABLE/GABLE.h>

// Static Members //////////////////////////////////////////////////////////////////////////////////

static          SDL_Window*         s_Window = NULL;
static          SDL_Renderer*       s_Renderer = NULL;
static          SDL_Texture*        s_RenderTarget = NULL;
static          GABLE_Engine*       s_Engine = NULL;
static const    GABLE_DataHandle*   s_Tiles = NULL;
static const    GABLE_DataHandle*   s_Tilemap = NULL;
static const    GABLE_DataHandle*   s_Paddle = NULL;
static const    GABLE_DataHandle*   s_Ball = NULL;

// Static Functions - SDL Input Events /////////////////////////////////////////////////////////////

static void UB_OnKeyDown (SDL_KeyboardEvent* p_Event)
{
    switch (p_Event->keysym.sym)
    {
        case SDLK_UP:
        case SDLK_w:
            GABLE_PressButton(s_Engine, GABLE_JB_UP);
            break;
        case SDLK_DOWN:
        case SDLK_s:
            GABLE_PressButton(s_Engine, GABLE_JB_DOWN);
            break;
        case SDLK_LEFT:
        case SDLK_a:
            GABLE_PressButton(s_Engine, GABLE_JB_LEFT);
            break;
        case SDLK_RIGHT:
        case SDLK_d:
            GABLE_PressButton(s_Engine, GABLE_JB_RIGHT);
            break;
        case SDLK_z:
        case SDLK_j:
            GABLE_PressButton(s_Engine, GABLE_JB_A);
            break;
        case SDLK_x:
        case SDLK_k:
            GABLE_PressButton(s_Engine, GABLE_JB_B);
            break;
        case SDLK_RETURN:
        case SDLK_SPACE:
            GABLE_PressButton(s_Engine, GABLE_JB_START);
            break;
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:
            GABLE_PressButton(s_Engine, GABLE_JB_SELECT);
            break;
        case SDLK_ESCAPE:
            exit(0);
            break;
    }
}

static void UB_OnKeyUp (SDL_KeyboardEvent* p_Event)
{
    switch (p_Event->keysym.sym)
    {
        case SDLK_UP:
        case SDLK_w:
            GABLE_ReleaseButton(s_Engine, GABLE_JB_UP);
            break;
        case SDLK_DOWN:
        case SDLK_s:
            GABLE_ReleaseButton(s_Engine, GABLE_JB_DOWN);
            break;
        case SDLK_LEFT:
        case SDLK_a:
            GABLE_ReleaseButton(s_Engine, GABLE_JB_LEFT);
            break;
        case SDLK_RIGHT:
        case SDLK_d:
            GABLE_ReleaseButton(s_Engine, GABLE_JB_RIGHT);
            break;
        case SDLK_z:
        case SDLK_j:
            GABLE_ReleaseButton(s_Engine, GABLE_JB_A);
            break;
        case SDLK_x:
        case SDLK_k:
            GABLE_ReleaseButton(s_Engine, GABLE_JB_B);
            break;
        case SDLK_RETURN:
        case SDLK_SPACE:
            GABLE_ReleaseButton(s_Engine, GABLE_JB_START);
            break;
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:
            GABLE_ReleaseButton(s_Engine, GABLE_JB_SELECT);
            break;
    }
}

static void UB_OnGameControllerButtonDown (SDL_ControllerButtonEvent* p_Event)
{
    switch (p_Event->button)
    {
        case SDL_CONTROLLER_BUTTON_DPAD_UP:
            GABLE_PressButton(s_Engine, GABLE_JB_UP);
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            GABLE_PressButton(s_Engine, GABLE_JB_DOWN);
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            GABLE_PressButton(s_Engine, GABLE_JB_LEFT);
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            GABLE_PressButton(s_Engine, GABLE_JB_RIGHT);
            break;
        case SDL_CONTROLLER_BUTTON_A:
            GABLE_PressButton(s_Engine, GABLE_JB_A);
            break;
        case SDL_CONTROLLER_BUTTON_B:
            GABLE_PressButton(s_Engine, GABLE_JB_B);
            break;
        case SDL_CONTROLLER_BUTTON_START:
            GABLE_PressButton(s_Engine, GABLE_JB_START);
            break;
        case SDL_CONTROLLER_BUTTON_BACK:
            exit(0);
            break;
    }
}

static void UB_OnGameControllerButtonUp (SDL_ControllerButtonEvent* p_Event)
{
    switch (p_Event->button)
    {
        case SDL_CONTROLLER_BUTTON_DPAD_UP:
            GABLE_ReleaseButton(s_Engine, GABLE_JB_UP);
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            GABLE_ReleaseButton(s_Engine, GABLE_JB_DOWN);
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            GABLE_ReleaseButton(s_Engine, GABLE_JB_LEFT);
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            GABLE_ReleaseButton(s_Engine, GABLE_JB_RIGHT);
            break;
        case SDL_CONTROLLER_BUTTON_A:
            GABLE_ReleaseButton(s_Engine, GABLE_JB_A);
            break;
        case SDL_CONTROLLER_BUTTON_B:
            GABLE_ReleaseButton(s_Engine, GABLE_JB_B);
            break;
        case SDL_CONTROLLER_BUTTON_START:
            GABLE_ReleaseButton(s_Engine, GABLE_JB_START);
            break;
    }
}

// Static Functions - SDL Application Loop /////////////////////////////////////////////////////////

static void UB_HandleEvents ()
{
    SDL_Event l_Event;
    while (SDL_PollEvent(&l_Event))
    {
        switch (l_Event.type)
        {
            case SDL_KEYDOWN:
                UB_OnKeyDown(&l_Event.key);
                break;
            case SDL_KEYUP:
                UB_OnKeyUp(&l_Event.key);
                break;
            case SDL_CONTROLLERBUTTONDOWN:
                UB_OnGameControllerButtonDown(&l_Event.cbutton);
                break;
            case SDL_CONTROLLERBUTTONUP:
                UB_OnGameControllerButtonUp(&l_Event.cbutton);
                break;
            case SDL_QUIT:
                exit(0);
                break;
        }
    }
}

static void UB_Render ()
{
    Uint32* l_Pixels = NULL;
    Int32   l_Pitch = 0;
    SDL_LockTexture(s_RenderTarget, NULL, (void**) &l_Pixels, &l_Pitch);
    SDL_memcpy(l_Pixels, GABLE_GetScreenBuffer(s_Engine), GABLE_PPU_SCREEN_BUFFER_SIZE * sizeof(Uint32));
    SDL_UnlockTexture(s_RenderTarget);
    SDL_RenderCopy(s_Renderer, s_RenderTarget, NULL, NULL);
    SDL_RenderPresent(s_Renderer);
}

// Static Functions - Frame and Mix Callbacks //////////////////////////////////////////////////////

static void UB_OnFrameRendered (GABLE_Engine* p_Engine, GABLE_PPU* p_PPU)
{
    UB_HandleEvents();
}

// Static Functions - Interrupt Callbacks //////////////////////////////////////////////////////////

static Bool UB_OnVerticalBlank (GABLE_Engine* p_Engine)
{
    UB_Render();
    return G_RETI(p_Engine);
}

// Static Functions - Init, Main, and Exit /////////////////////////////////////////////////////////

static void UB_AtStart ()
{
    // Initialize SDL
    SDL_Init(SDL_INIT_EVERYTHING);
    s_Window = SDL_CreateWindow("Unbricked", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 720, 
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    s_Renderer = SDL_CreateRenderer(s_Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    s_RenderTarget = SDL_CreateTexture(s_Renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
        GABLE_PPU_SCREEN_WIDTH, GABLE_PPU_SCREEN_HEIGHT);

    // Initialize GABLE
    s_Engine = GABLE_CreateEngine();
    GABLE_SetFrameRenderedCallback(s_Engine, UB_OnFrameRendered);
    GABLE_SetInterruptHandler(s_Engine, GABLE_INT_VBLANK, UB_OnVerticalBlank);

    // Load asset data
    s_Tiles = GABLE_LoadDataFromFile(s_Engine, "Tiles", "assets/unbricked/tile-data.bin", 0);
    s_Tilemap = GABLE_LoadDataFromFile(s_Engine, "Tilemap", "assets/unbricked/tile-map.bin", 0);
    s_Paddle = GABLE_LoadDataFromFile(s_Engine, "Paddle", "assets/unbricked/paddle-data.bin", 0);
}

static void UB_Main ()
{
    static const Uint16 wFrameCounter = G_RAM + 0;
    static const Uint16 wCurKeys = G_RAM + 1;
    static const Uint16 wNewKeys = G_RAM + 2;

    // Don't turn the LCD off outside of VBlank
    G_WaitVBlank();

    // Turn the LCD off.
    G_LD_R8_N8(G_A, G_LCDCF_OFF);
    G_LDH_A8_A(G_LCDC);

    // Copy the tile data to VRAM
    G_LD_R16_N16(G_DE, s_Tiles->m_Address);
    G_LD_R16_N16(G_HL, 0x9000);
    G_LD_R16_N16(G_BC, s_Tiles->m_Length);
    G_CopyBytes();

    // Copy the tilemap to VRAM
    G_LD_R16_N16(G_DE, s_Tilemap->m_Address);
    G_LD_R16_N16(G_HL, 0x9800);
    G_LD_R16_N16(G_BC, s_Tilemap->m_Length);
    G_CopyBytes();

    // Copy the paddle sprite to VRAM
    G_LD_R16_N16(G_DE, s_Paddle->m_Address);
    G_LD_R16_N16(G_HL, 0x8000);
    G_LD_R16_N16(G_BC, s_Paddle->m_Length);
    G_CopyBytes();

    // Clear OAM.
    G_ClearOAM();

    // Set up the paddle sprite.
    G_LD_R16_N16(G_HL, G_OAMRAM);
    G_LD_R8_N8(G_A, 128 + 16);
    G_LD_HLI_A();
    G_LD_R8_N8(G_A, 16 + 8);
    G_LD_HLI_A();
    G_LD_R8_N8(G_A, 0);
    G_LD_HLI_A();
    G_LD_HLI_A();

    // Set up the display registers
    G_LD_R8_N8(G_A, 0b11100100);
    G_LDH_A8_A(G_BGP);
    G_LDH_A8_A(G_OBP0);

    // Enable VBlank Interrupt
    G_LD_R8_N8(G_A, G_IEF_VBLANK);
    G_LDH_A8_A(G_IE);
    G_EI();

    // Turn the LCD on
    G_LD_R8_N8(G_A, G_LCDCF_ON | G_LCDCF_BGON | G_LCDCF_OBJON);
    G_LDH_A8_A(G_LCDC);

    // Initialize RAM variables.
    G_LD_R8_N8(G_A, 0);
    G_LD_A16_A(wFrameCounter);
    G_LD_A16_A(wCurKeys);
    G_LD_A16_A(wNewKeys);

    // Main loop
    UB_Main__Loop:
        G_WaitAfterVBlank();
        G_WaitVBlank();
        G_UpdateKeys(wCurKeys, wNewKeys);

        Uint8 l_CurKeys = 0, l_NewKeys = 0;
        GABLE_ReadByte(s_Engine, wCurKeys, &l_CurKeys);
        GABLE_ReadByte(s_Engine, wNewKeys, &l_NewKeys);
        GABLE_debug("CurKeys: %02X, NewKeys: %02X", l_CurKeys, l_NewKeys);

        UB_Main__CheckLeft:
            G_LD_A_A16(wCurKeys);
            G_AND_A_N8(G_PADF_LEFT);
            G_JP_GOTO(G_COND_Z, UB_Main__CheckRight);
            G_LD_A_A16(G_OAMRAM + 1);
            G_DEC_R8(G_A);
            G_CP_A_N8(15);
            G_JP_GOTO(G_COND_Z, UB_Main__Loop);
            G_LD_A16_A(G_OAMRAM + 1);
            G_JP_GOTO(G_NOCOND, UB_Main__Loop);

        UB_Main__CheckRight:
            G_LD_A_A16(wCurKeys);
            G_AND_A_N8(G_PADF_RIGHT);
            G_JP_GOTO(G_COND_Z, UB_Main__Loop);
            G_LD_A_A16(G_OAMRAM + 1);
            G_INC_R8(G_A);
            G_CP_A_N8(105);
            G_JP_GOTO(G_COND_Z, UB_Main__Loop);
            G_LD_A16_A(G_OAMRAM + 1);
            G_JP_GOTO(G_NOCOND, UB_Main__Loop);
}

static void UB_AtExit ()
{
    if (s_Engine != NULL)       { GABLE_DestroyEngine(s_Engine); }
    if (s_RenderTarget != NULL) { SDL_DestroyTexture(s_RenderTarget); }
    if (s_Renderer != NULL)     { SDL_DestroyRenderer(s_Renderer); }
    if (s_Window != NULL)       { SDL_DestroyWindow(s_Window); }
    SDL_Quit();
}

// Main Function ///////////////////////////////////////////////////////////////////////////////////

int main ()
{
    atexit(UB_AtExit);
    UB_AtStart();
    UB_Main();

    return 0;
}
