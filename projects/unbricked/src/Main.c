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
    s_Ball = GABLE_LoadDataFromFile(s_Engine, "Ball", "assets/unbricked/ball-data.bin", 0);
}

static const Uint8 BRICK_LEFT = 0x05;
static const Uint8 BRICK_RIGHT = 0x06;
static const Uint8 BLANK_TILE = 0x08;

static void UB_GetTileByPixel ()
{
    // First, we need to divide by 8 to convert the pixel position to a tile position.
    // After this, we want to multiply the Y position by 32.
    // These operations effectively cancel out, so we only need to mask the Y value.
    G_LD_R8_R8(G_A, G_C);
    G_AND_A_N8(0b11111000);
    G_LD_R8_R8(G_L, G_A);
    G_LD_R8_N8(G_H, 0);

    // Now we have the position * 8 in HL.
    G_ADD_HL_R16(G_HL); // position * 16
    G_ADD_HL_R16(G_HL); // position * 32

    // Convert the X position into an offset.
    G_LD_R8_R8(G_A, G_B);
    G_SRL_R8(G_A); // a / 2
    G_SRL_R8(G_A); // a / 4
    G_SRL_R8(G_A); // a / 8

    // Add the two offsets together.
    G_ADD_A_R8(G_L);
    G_LD_R8_R8(G_L, G_A);
    G_ADC_A_R8(G_H);
    G_SUB_A_R8(G_L);
    G_LD_R8_R8(G_H, G_A);

    // Add the offset to the tilemap's base address, and we are done!
    G_LD_R16_N16(G_BC, 0x9800);
    G_ADD_HL_R16(G_BC);
    G_RET(G_NOCOND);
}

static void UB_IsWallTile ()
{
    G_CP_A_N8(0x00);
    if (G_RET(G_COND_Z)) { return; }

    G_CP_A_N8(0x01);
    if (G_RET(G_COND_Z)) { return; }

    G_CP_A_N8(0x02);
    if (G_RET(G_COND_Z)) { return; }

    G_CP_A_N8(0x04);
    if (G_RET(G_COND_Z)) { return; }

    G_CP_A_N8(0x05);
    if (G_RET(G_COND_Z)) { return; }

    G_CP_A_N8(0x06);
    if (G_RET(G_COND_Z)) { return; }

    G_CP_A_N8(0x07);
    G_RET(G_NOCOND);
}

static void UB_CheckAndHandleBrick ()
{
    UB_CheckAndHandleBrick__Left:
        G_LD_R8_HL(G_A);
        G_CP_A_N8(BRICK_LEFT);
        G_JR_GOTO(G_COND_NZ, UB_CheckAndHandleBrick__Right);
        G_LD_HL_N8(BLANK_TILE);
        G_INC_R16(G_HL);
        G_LD_HL_N8(BLANK_TILE);

    UB_CheckAndHandleBrick__Right:
        G_CP_A_N8(BRICK_RIGHT);
        if (G_RET(G_COND_NZ)) { return; }
        G_LD_HL_N8(BLANK_TILE);
        G_DEC_R16(G_HL);
        G_LD_HL_N8(BLANK_TILE);
        G_RET(G_NOCOND);
}

enum
{
    G_RSRESET
    G_RB(VelX)
    G_RB(VelY)
};

enum
{
    G_RSRESET
    G_RB(KeysCurrent)
    G_RB(KeysNew)
};

#define UB_BALLVEL_STRUCT(K) \
    G_STRUCT(K) \
    G_DB(K##X) \
    G_DB(K##Y)

#define UB_KEYS_STRUCT(K) \
    G_STRUCT(K) \
    G_DB(K##Current) \
    G_DB(K##New)

enum
{
    G_DB(wFrameCounter)
    UB_KEYS_STRUCT(wKeys)
    UB_BALLVEL_STRUCT(wBallVel)
    G_UNION(MyUnion)
        G_DS(wName, 10)
        G_DS(wNickname, 10)
    G_NEXTU(MyUnion)
        G_DW(wHealth)
        G_DB(wLives)
        G_DS(wUnused0, 7)
        G_DB(wBonus)
    G_NEXTU(MyUnion)
        G_DS(wVideoBuffer, 16)
    G_ENDU(MyUnion, 20)
};

static void UB_Main ()
{
    // Don't turn the LCD off outside of VBlank
    G_CALL_FUNC(G_NOCOND, G_WaitVBlank());

    // Turn the LCD off.
    G_LD_R8_N8(G_A, G_LCDCF_OFF);
    G_LDH_A8_A(G_LCDC);

    // Copy the tile data to VRAM
    G_LD_R16_N16(G_DE, s_Tiles->m_Address);
    G_LD_R16_N16(G_HL, 0x9000);
    G_LD_R16_N16(G_BC, s_Tiles->m_Length);
    G_CALL_FUNC(G_NOCOND, G_CopyBytes());

    // Copy the tilemap to VRAM
    G_LD_R16_N16(G_DE, s_Tilemap->m_Address);
    G_LD_R16_N16(G_HL, 0x9800);
    G_LD_R16_N16(G_BC, s_Tilemap->m_Length);
    G_CALL_FUNC(G_NOCOND, G_CopyBytes());

    // Copy the paddle sprite to VRAM
    G_LD_R16_N16(G_DE, s_Paddle->m_Address);
    G_LD_R16_N16(G_HL, 0x8000);
    G_LD_R16_N16(G_BC, s_Paddle->m_Length);
    G_CALL_FUNC(G_NOCOND, G_CopyBytes());

    // Copy the ball sprite to VRAM
    G_LD_R16_N16(G_DE, s_Ball->m_Address);
    G_LD_R16_N16(G_HL, 0x8000 + s_Paddle->m_Length);
    G_LD_R16_N16(G_BC, s_Ball->m_Length);
    G_CALL_FUNC(G_NOCOND, G_CopyBytes());

    // Clear OAM.
    G_CALL_FUNC(G_NOCOND, G_ClearOAM());

    // Set up the paddle sprite.
    G_LD_R16_N16(G_HL, G_OAMRAM);
    G_LD_R8_N8(G_A, 128 + 16);
    G_LD_HLI_A();
    G_LD_R8_N8(G_A, 16 + 8);
    G_LD_HLI_A();
    G_LD_R8_N8(G_A, 0);
    G_LD_HLI_A();
    G_LD_HLI_A();

    // Set up the ball sprite.
    G_LD_R8_N8(G_A, 100 + 16);
    G_LD_HLI_A();
    G_LD_R8_N8(G_A, 32 + 8);
    G_LD_HLI_A();
    G_LD_R8_N8(G_A, 1);
    G_LD_HLI_A();
    G_LD_R8_N8(G_A, 0);
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
    G_LD_A16_A(G_WADDR(wFrameCounter));
    G_LD_A16_A(G_WADDRS(wKeys, KeysCurrent));
    G_LD_A16_A(G_WADDRS(wKeys, KeysNew));

    // Set up the ball sprite to go up and to the right.
    G_LD_R8_N8(G_A, 1);
    G_LD_A16_A(G_WADDRS(wBallVel, VelX));
    G_LD_R8_N8(G_A, -1);
    G_LD_A16_A(G_WADDRS(wBallVel, VelY));

    // Main loop
    UB_Main__Loop:
        G_CALL_FUNC(G_NOCOND, G_WaitAfterVBlank());
        G_CALL_FUNC(G_NOCOND, G_WaitVBlank());

        // Add the ball sprite's velocity to its position.
        G_LD_A_A16(G_WADDRS(wBallVel, VelX));
        G_LD_R8_R8(G_B, G_A);
        G_LD_A_A16(G_OAMRAM + 5);
        G_ADD_A_R8(G_B);
        G_LD_A16_A(G_OAMRAM + 5);
        G_LD_A_A16(G_WADDRS(wBallVel, VelY));
        G_LD_R8_R8(G_B, G_A);
        G_LD_A_A16(G_OAMRAM + 4);
        G_ADD_A_R8(G_B);
        G_LD_A16_A(G_OAMRAM + 4);

        // Check for collisions with the walls.
        UB_Main__BounceOnTop:
            G_LD_A_A16(G_OAMRAM + 4);
            G_SUB_A_N8(16 + 1);
            G_LD_R8_R8(G_C, G_A);
            G_LD_A_A16(G_OAMRAM + 5);
            G_SUB_A_N8(8);
            G_LD_R8_R8(G_B, G_A);
            G_CALL_FUNC(G_NOCOND, UB_GetTileByPixel());
            G_LD_A_RP16(G_HL);
            G_CALL_FUNC(G_NOCOND, UB_IsWallTile());
            G_JP_GOTO(G_COND_NZ, UB_Main__BounceOnRight);
            G_CALL_FUNC(G_NOCOND, UB_CheckAndHandleBrick());
            G_LD_R8_N8(G_A, 1);
            G_LD_A16_A(G_WADDRS(wBallVel, VelY));

        UB_Main__BounceOnRight:
            G_LD_A_A16(G_OAMRAM + 4);
            G_SUB_A_N8(16);
            G_LD_R8_R8(G_C, G_A);
            G_LD_A_A16(G_OAMRAM + 5);
            G_SUB_A_N8(8 - 1);
            G_LD_R8_R8(G_B, G_A);
            G_CALL_FUNC(G_NOCOND, UB_GetTileByPixel());
            G_LD_A_RP16(G_HL);
            G_CALL_FUNC(G_NOCOND, UB_IsWallTile());
            G_JP_GOTO(G_COND_NZ, UB_Main__BounceOnLeft);
            G_CALL_FUNC(G_NOCOND, UB_CheckAndHandleBrick());
            G_LD_R8_N8(G_A, -1);
            G_LD_A16_A(G_WADDRS(wBallVel, VelX));

        UB_Main__BounceOnLeft:
            G_LD_A_A16(G_OAMRAM + 4);
            G_SUB_A_N8(16);
            G_LD_R8_R8(G_C, G_A);
            G_LD_A_A16(G_OAMRAM + 5);
            G_SUB_A_N8(8 + 1);
            G_LD_R8_R8(G_B, G_A);
            G_CALL_FUNC(G_NOCOND, UB_GetTileByPixel());
            G_LD_A_RP16(G_HL);
            G_CALL_FUNC(G_NOCOND, UB_IsWallTile());
            G_JP_GOTO(G_COND_NZ, UB_Main__BounceOnBottom);
            G_CALL_FUNC(G_NOCOND, UB_CheckAndHandleBrick());
            G_LD_R8_N8(G_A, 1);
            G_LD_A16_A(G_WADDRS(wBallVel, VelX));

        UB_Main__BounceOnBottom:
            G_LD_A_A16(G_OAMRAM + 4);
            G_SUB_A_N8(16 - 1);
            G_LD_R8_R8(G_C, G_A);
            G_LD_A_A16(G_OAMRAM + 5);
            G_SUB_A_N8(8);
            G_LD_R8_R8(G_B, G_A);
            G_CALL_FUNC(G_NOCOND, UB_GetTileByPixel());
            G_LD_A_RP16(G_HL);
            G_CALL_FUNC(G_NOCOND, UB_IsWallTile());
            G_JP_GOTO(G_COND_NZ, UB_Main__BounceDone);
            G_CALL_FUNC(G_NOCOND, UB_CheckAndHandleBrick());
            G_LD_R8_N8(G_A, -1);
            G_LD_A16_A(G_WADDRS(wBallVel, VelY));

        UB_Main__BounceDone:
            G_LD_A_A16(G_OAMRAM);
            G_LD_R8_R8(G_B, G_A);
            G_LD_A_A16(G_OAMRAM + 4);
            G_ADD_A_N8(6);
            G_CP_A_R8(G_B);
            G_JP_GOTO(G_COND_NZ, UB_Main__PaddleBounceDone);
            G_LD_A_A16(G_OAMRAM + 5);
            G_LD_R8_R8(G_B, G_A);
            G_LD_A_A16(G_OAMRAM + 1);
            G_SUB_A_N8(8);
            G_CP_A_R8(G_B);
            G_JP_GOTO(G_COND_NC, UB_Main__PaddleBounceDone);
            G_ADD_A_N8(8 + 16);
            G_CP_A_R8(G_B);
            G_JP_GOTO(G_COND_C, UB_Main__PaddleBounceDone);
            G_LD_R8_N8(G_A, -1);
            G_LD_A16_A(G_WADDRS(wBallVel, VelY));

        UB_Main__PaddleBounceDone:
            G_CALL_FUNC(G_NOCOND, G_UpdateKeys(G_WADDR(wKeysCurrent), G_WADDR(wKeysNew)));

        UB_Main__CheckLeft:
            G_LD_A_A16(G_WADDR(wKeysCurrent));
            G_AND_A_N8(G_PADF_LEFT);
            G_JP_GOTO(G_COND_Z, UB_Main__CheckRight);
            G_LD_A_A16(G_OAMRAM + 1);
            G_DEC_R8(G_A);
            G_CP_A_N8(15);
            G_JP_GOTO(G_COND_Z, UB_Main__Loop);
            G_LD_A16_A(G_OAMRAM + 1);
            G_JP_GOTO(G_NOCOND, UB_Main__Loop);

        UB_Main__CheckRight:
            G_LD_A_A16(G_WADDR(wKeysCurrent));
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
