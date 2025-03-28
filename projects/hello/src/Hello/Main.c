#include <SDL2/SDL.h>
#include <GABLE/GABLE.h>

// Static Members //////////////////////////////////////////////////////////////////////////////////

static          SDL_Window*         s_Window = NULL;
static          SDL_Renderer*       s_Renderer = NULL;
static          SDL_Texture*        s_RenderTarget = NULL;
static          GABLE_Engine*       s_Engine = NULL;
static const    GABLE_DataHandle*   s_TileData = NULL;
static const    GABLE_DataHandle*   s_TileMap = NULL;

// Static Functions - SDL Input Events /////////////////////////////////////////////////////////////

static void H_OnKeyDown (SDL_KeyboardEvent* p_Event)
{
    switch (p_Event->keysym.sym)
    {
        case SDLK_ESCAPE:
            exit(0);
            break;
    }
}

// Static Functions - SDL Application Loop /////////////////////////////////////////////////////////

static void H_HandleEvents ()
{
    SDL_Event l_Event;
    while (SDL_PollEvent(&l_Event))
    {
        switch (l_Event.type)
        {
            case SDL_KEYDOWN:
                H_OnKeyDown(&l_Event.key);
                break;
            case SDL_QUIT:
                exit(0);
                break;
        }
    }
}

static void H_Render ()
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

static void H_OnFrameRendered (GABLE_Engine* p_Engine, GABLE_PPU* p_PPU)
{
    H_HandleEvents();
}

// Static Functions - Interrupt Callbacks //////////////////////////////////////////////////////////

static Bool H_OnVerticalBlank (GABLE_Engine* p_Engine)
{
    H_Render();
    return G_RETI(p_Engine);
}

// Static Functions - Init, Main, and Exit /////////////////////////////////////////////////////////

static void H_AtStart ()
{
    SDL_Init(SDL_INIT_EVERYTHING);

    s_Window = SDL_CreateWindow("Hello", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 720, 
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    s_Renderer = SDL_CreateRenderer(s_Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    s_RenderTarget = SDL_CreateTexture(s_Renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
        GABLE_PPU_SCREEN_WIDTH, GABLE_PPU_SCREEN_HEIGHT);
    s_Engine = GABLE_CreateEngine();
    s_TileData = GABLE_LoadDataFromFile(s_Engine, "s_TileData", "assets/hello/tile-data.bin", 0);
    s_TileMap = GABLE_LoadDataFromFile(s_Engine, "s_TileMap", "assets/hello/tile-maps.bin", 0);

    if (s_TileData == NULL || s_TileMap == NULL)
    {
        exit(1);
    }
    
    GABLE_SetFrameRenderedCallback(s_Engine, H_OnFrameRendered);
    GABLE_SetInterruptHandler(s_Engine, GABLE_INT_VBLANK, H_OnVerticalBlank);
}

static void H_Main ()
{
    // Shut down audio circuitry.
    G_LD_R8_N8(G_A, G_AUDENA_OFF);
    G_LDH_A8_A(G_NR52);

    // Enable VBlank interrupt.
    G_LD_R8_N8(G_A, G_IEF_VBLANK);
    G_LDH_A8_A(G_IE);
    G_EI();

    // Do not turn the LCD off outside of VBlank
    G_CALL_FUNC(G_NOCOND, G_WaitVBlank());

    // Turn the LCD off.
    G_LD_R8_N8(G_A, G_LCDCF_OFF);
    G_LDH_A8_A(G_LCDC);

    // Set graphics mode to DMG.
    G_LDH_A8_A(G_GRPM);

    // Copy the tile data.
    G_LD_R16_N16(G_DE, s_TileData->m_Address);
    G_LD_R16_N16(G_HL, 0x9000);
    G_LD_R16_N16(G_BC, s_TileData->m_Length);
    G_CALL_FUNC(G_NOCOND, G_CopyBytes());

    // Copy the tile map.
    G_LD_R16_N16(G_DE, s_TileMap->m_Address);
    G_LD_R16_N16(G_HL, 0x9800);
    G_LD_R16_N16(G_BC, s_TileMap->m_Length);
    G_CALL_FUNC(G_NOCOND, G_CopyBytes());

    // Turn the LCD on.
    G_LD_R8_N8(G_A, G_LCDCF_ON | G_LCDCF_BGON);   // LCD On, BG On
    G_LDH_A8_A(G_LCDC);

    // During the first (blank) frame, initialize display registers.
    G_LD_R8_N8(G_A, 0b11100100);
    G_LDH_A8_A(G_BGP);

    do
    {
        
    } while (G_JR(G_NOCOND));
}

static void H_AtExit ()
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
    atexit(H_AtExit);
    H_AtStart();
    H_Main();

    return 0;
}
