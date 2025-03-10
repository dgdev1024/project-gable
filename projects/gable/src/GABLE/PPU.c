/**
 * @file GABLE/PPU.c
 */

#include <GABLE/Engine.h>
#include <GABLE/InterruptContext.h>
#include <GABLE/PPU.h>

// Static Constants ////////////////////////////////////////////////////////////////////////////////

static const Uint32 GABLE_PPU_DMG_PALETTE[4] =
{
    0x000000FF, 0x808080FF, 0xC0C0C0FF, 0xFFFFFFFF
};

// GABLE PPU Structure /////////////////////////////////////////////////////////////////////////////

typedef struct GABLE_PPU
{

    // Memory Buffers
    Uint32                      m_ScreenBuffer[GABLE_PPU_SCREEN_BUFFER_SIZE];     ///< @brief The screen buffer.
    Uint8                       m_VRAM0[GABLE_PPU_VRAM_BANK_SIZE];                ///< @brief The first VRAM bank.
    Uint8                       m_VRAM1[GABLE_PPU_VRAM_BANK_SIZE];                ///< @brief The second VRAM bank.
    GABLE_Object                m_OAM[GABLE_PPU_OAM_OBJECT_COUNT];                ///< @brief The object attribute memory (OAM) buffer.
    Uint8                       m_BgCRAM[GABLE_PPU_CRAM_SIZE];                    ///< @brief The background color RAM (CRAM) buffer.
    Uint8                       m_ObjCRAM[GABLE_PPU_CRAM_SIZE];                   ///< @brief The object color RAM (CRAM) buffer.
    Uint8*                      m_VRAM;                                           ///< @brief A pointer to the current VRAM bank.

    // Hardware Registers
    GABLE_DisplayControl        m_LCDC;                                           ///< @brief The display control register.
    GABLE_DisplayStatus         m_STAT;                                           ///< @brief The display status register.
    Uint8                       m_SCY;                                            ///< @brief The scroll Y register.
    Uint8                       m_SCX;                                            ///< @brief The scroll X register.
    Uint8                       m_LY;                                             ///< @brief The LCDC Y-coordinate register.
    Uint8                       m_LYC;                                            ///< @brief The LY compare register.
    Uint8                       m_DMA;                                            ///< @brief The OAM DMA register - the high byte of the source address in the DMA transfer process.
    Uint8                       m_BGP;                                            ///< @brief The background palette data register (DMG only).
    Uint8                       m_OBP0;                                           ///< @brief The object palette 0 data register (DMG only).
    Uint8                       m_OBP1;                                           ///< @brief The object palette 1 data register (DMG only).
    Uint8                       m_WY;                                             ///< @brief The window position Y register.
    Uint8                       m_WX;                                             ///< @brief The window position X register.
    Uint8                       m_VBK;                                            ///< @brief The VRAM bank number register.
    Uint8                       m_HDMA1;                                          ///< @brief The GDMA / HDMA source high byte register. Write-only.
    Uint8                       m_HDMA2;                                          ///< @brief The GDMA / HDMA source low byte register. Write-only.
    Uint8                       m_HDMA3;                                          ///< @brief The GDMA / HDMA destination high byte register. Write-only.
    Uint8                       m_HDMA4;                                          ///< @brief The GDMA / HDMA destination low byte register. Write-only.
    GABLE_HDMAControl           m_HDMA5;                                          ///< @brief The GDMA / HDMA transfer control register.
    GABLE_PaletteSpecification  m_BGPI;                                           ///< @brief The background palette specification register.
    GABLE_PaletteSpecification  m_OBPI;                                           ///< @brief The object palette specification register.

    // Note: The `BGPD` and `OBPD` registers are not stored in the PPU structure as accessing these
    // registers indexes the color RAM buffers directly, through the indices in their respective
    // specification registers.

    /**
     * @brief The object priority register. This register is used to determine how priority is assigned
     *        to objects in the object attribute memory (OAM) buffer.
     * 
     * - If this register is non-zero, then the priority of objects is determined by their X position.
     *   Objects with smaller X positions have higher priority. Objects with the same X position are
     *   assigned priority based on their index in the OAM buffer.
     * 
     * - If this register is zero, then the priority of objects is determined by their index in the OAM
     *   buffer. Objects with lower indices have higher priority.
     * 
     * - If the `GRPM` register is set to 0, then the PPU is in DMG graphics mode, and the priority of
     *   objects is determined by their X position, regardless of the value of this register.
     */
    Uint8                       m_OPRI;

    /**
     * @brief The graphics mode register. This register controls the graphics mode of the PPU.
     * 
     * - If this register is zero, then the PPU is in DMG graphics mode. In this mode, the PPU simulates
     *   the graphics hardware of the original DMG Game Boy, with a limited color palette (using the
     *   background and object palette data registers) and no access to the extended color palette of
     *   the Game Boy Color.
     * 
     * - If this register is non-zero, then the PPU is in CGB graphics mode. In this mode, the PPU has
     *   access to the extended color palette and additional features of the Game Boy Color's PPU.
     */
    Uint8                       m_GRPM;

    // Pixel Fetcher
    GABLE_PixelFetcher          m_PixelFetcher;                                   ///< @brief The PPU's pixel-fetcher unit.

    // Internal Registers - Window Line Counter
    Uint8                       m_WindowLine;                                     ///< @brief The current line being rendered within the window layer.

    // Internal Registers - Current Dot
    Uint16                      m_CurrentDot;                                     ///< @brief The current dot of the scanline being rendered.

    // Internal Registers - OAM DMA Transfer
    Uint16                      m_ODMASource;                                     ///< @brief The source address of the OAM DMA transfer.
    Uint16                      m_ODMADestination;                                ///< @brief The destination address of the OAM DMA transfer.
    Uint8                       m_ODMATicks;                                      ///< @brief The number of ticks elapsed in the OAM DMA transfer.
    Uint8                       m_ODMADelay;                                      ///< @brief The number of ticks to delay the OAM DMA transfer once initiated.

    // Internal Registers - HDMA Transfer
    Uint16                      m_HDMASource;                                     ///< @brief The source address of the HDMA transfer.
    Uint16                      m_HDMADestination;                                ///< @brief The destination address of the HDMA transfer.
    Uint8                       m_HDMABlocksLeft;                                 ///< @brief The number of 16-byte blocks left to transfer in the HDMA process.

    // Internal Registers - Object Scan
    Uint8                       m_LineObjectIndices[GABLE_PPU_OAM_OBJECT_COUNT];  ///< @brief The indices of the objects found on the current scanline.
    Uint8                       m_LineObjectCount;                                ///< @brief The number of objects found on the current scanline.

    // Frame Rendered Callback
    GABLE_FrameRenderedCallback m_FrameRenderedCallback;                          ///< @brief The callback function to invoke when a frame is rendered.

} GABLE_PPU;

// Static Function Prototypes - Misc. Helper Functions /////////////////////////////////////////////

static Bool GABLE_IsWindowVisible (GABLE_PPU* p_PPU);
static void GABLE_IncrementLY (GABLE_PPU* p_PPU, GABLE_Engine* p_Engine);

// Static Function Prototypes - Object Scan ////////////////////////////////////////////////////////

static void GABLE_ClearLineObjects (GABLE_PPU* p_PPU);
static void GABLE_FindLineObject (GABLE_PPU* p_PPU);

// Static Function Prototypes - Pixel Transfer /////////////////////////////////////////////////////

static void GABLE_PushColor (GABLE_PixelFetcher* p_Fetcher, Uint32 p_Color);
static void GABLE_PopColor (GABLE_PixelFetcher* p_Fetcher, Uint32* p_Color);
static Bool GABLE_TryAddPixel (GABLE_PPU* p_PPU, GABLE_PixelFetcher* p_Fetcher);
static void GABLE_ShiftNextPixel (GABLE_PPU* p_PPU, GABLE_PixelFetcher* p_Fetcher);
static Uint32 GABLE_FetchObjectPixel (GABLE_PPU* p_PPU, GABLE_PixelFetcher* p_Fetcher, Uint8 p_Bit, Uint8 p_ColorIndex, Uint32 p_RGBAColorValue, Uint8 p_BGWindowPriority);
static void GABLE_FetchBackgroundTileNumber (GABLE_PPU* p_PPU, GABLE_PixelFetcher* p_Fetcher);
static void GABLE_FetchWindowTileNumber (GABLE_PPU* p_PPU, GABLE_PixelFetcher* p_Fetcher);
static void GABLE_FetchObjectTileNumber (GABLE_PPU* p_PPU, GABLE_PixelFetcher* p_Fetcher);
static void GABLE_FetchObjectTileData (GABLE_PPU* p_PPU, GABLE_PixelFetcher* p_Fetcher, Uint8 p_Offset);
static void GABLE_FetchTileNumber (GABLE_PPU* p_PPU, GABLE_PixelFetcher* p_Fetcher);
static void GABLE_FetchTileDataLow (GABLE_PPU* p_PPU, GABLE_PixelFetcher* p_Fetcher);
static void GABLE_FetchTileDataHigh (GABLE_PPU* p_PPU, GABLE_PixelFetcher* p_Fetcher);
static void GABLE_FetchPushPixels (GABLE_PPU* p_PPU, GABLE_PixelFetcher* p_Fetcher);
static void GABLE_FetchSleep (GABLE_PPU* p_PPU, GABLE_PixelFetcher* p_Fetcher);
static void GABLE_TickPixelFetcher (GABLE_PPU* p_PPU, GABLE_PixelFetcher* p_Fetcher);
static void GABLE_ResetPixelFetcher (GABLE_PPU* p_PPU, GABLE_PixelFetcher* p_Fetcher);

// Static Function Prototypes - PPU State Machine //////////////////////////////////////////////////

static void GABLE_TickHorizontalBlank (GABLE_PPU* p_PPU, GABLE_Engine* p_Engine);
static void GABLE_TickVerticalBlank (GABLE_PPU* p_PPU, GABLE_Engine* p_Engine);
static void GABLE_TickObjectScan (GABLE_PPU* p_PPU, GABLE_Engine* p_Engine);
static void GABLE_TickPixelTransfer (GABLE_PPU* p_PPU, GABLE_Engine* p_Engine);

// Static Function Prototypes - HDMA Transfer //////////////////////////////////////////////////////

static void GABLE_TickHDMA (GABLE_PPU* p_PPU, GABLE_Engine* p_Engine);

// Static Functions - Misc. Helper Functions ///////////////////////////////////////////////////////

Bool GABLE_IsWindowVisible (GABLE_PPU* p_PPU)
{
    return  p_PPU->m_LCDC.m_WindowEnable == 1 && 
            p_PPU->m_WX <= 166 && 
            p_PPU->m_WY < GABLE_PPU_SCREEN_HEIGHT;
}

void GABLE_IncrementLY (GABLE_PPU* p_PPU, GABLE_Engine* p_Engine)
{
    
    // Check to see if the internal window line counter needs to be incremented, as well.
    if (
        GABLE_IsWindowVisible(p_PPU) == true &&
        p_PPU->m_LY >= p_PPU->m_WY &&
        p_PPU->m_LY < p_PPU->m_WY + GABLE_PPU_SCREEN_HEIGHT
    )
    {
        p_PPU->m_WindowLine++;
    }

    // Increment the LY register. Set the line coincidence bit in the STAT register if LY matches LYC.
    p_PPU->m_STAT.m_LineCoincidence = (++p_PPU->m_LY == p_PPU->m_LYC);

    // If the coincidence bit has been set, and its STAT source is enabled, then request an `LCD_STAT`
    // interrupt.
    if (p_PPU->m_STAT.m_LineCoincidence == 1 && p_PPU->m_STAT.m_LineCoincidenceStatSource == 1)
    {
        GABLE_RequestInterrupt(GABLE_GetInterruptContext(p_Engine), GABLE_INT_LCD_STAT);
    }

}

// Static Functions - Object Scan //////////////////////////////////////////////////////////////////

void GABLE_ClearLineObjects (GABLE_PPU* p_PPU)
{
    p_PPU->m_LineObjectCount = 0;
}

void GABLE_FindLineObject (GABLE_PPU* p_PPU)
{

    // There is a limit of 10 objects per scanline. Don't process more than 10 objects.
    if (p_PPU->m_LineObjectCount >= GABLE_PPU_OBJECTS_PER_SCANLINE)
    {
        return;
    }

    // Check the `LCDC` register for the current object height.
    Uint8 l_ObjectHeight = (p_PPU->m_LCDC.m_ObjectSize == 1) ? 16 : 8;

    // The index of the object to seek in the OAM buffer depends on the current scanline dot.
    Uint8 l_ObjectIndex = (p_PPU->m_CurrentDot / 2) % GABLE_PPU_OAM_OBJECT_COUNT;

    // Pointer to the object in the OAM buffer.
    const GABLE_Object* l_Object = &p_PPU->m_OAM[l_ObjectIndex];

    // Check if the object is visible on the current scanline.
    Bool l_ObjectIsVisible =
        l_Object->m_X > 0 &&
        p_PPU->m_LY + 16 >= l_Object->m_Y &&
        p_PPU->m_LY + 16 < l_Object->m_Y + l_ObjectHeight;

    // If the object is visible, add it to the list of objects on the current scanline.
    if (l_ObjectIsVisible == true)
    {
        p_PPU->m_LineObjectIndices[p_PPU->m_LineObjectCount++] = l_ObjectIndex;
    }

    // If `GRPM` is set to zero (DMG mode), or if `OPRI` is non-zero (priority by X position), then
    // the objects need to be sorted by their X position:
    // - Objects with smaller X positions have higher priority.
    // - Objects with the same X position are assigned priority based on their index in the OAM buffer.
    if (p_PPU->m_GRPM == 0 || p_PPU->m_OPRI != 0)
    {
        // Sort the objects by their X position.
        for (Uint8 i = 0; i < p_PPU->m_LineObjectCount; i++)
        {
            for (Uint8 j = i + 1; j < p_PPU->m_LineObjectCount; j++)
            {
                // Get the objects to compare.
                const GABLE_Object* l_ObjectA = &p_PPU->m_OAM[p_PPU->m_LineObjectIndices[i]];
                const GABLE_Object* l_ObjectB = &p_PPU->m_OAM[p_PPU->m_LineObjectIndices[j]];

                // Compare the X positions of the objects.
                if (l_ObjectA->m_X > l_ObjectB->m_X)
                {
                    // Swap the object indices.
                    Uint8 l_Temp = p_PPU->m_LineObjectIndices[i];
                    p_PPU->m_LineObjectIndices[i] = p_PPU->m_LineObjectIndices[j];
                    p_PPU->m_LineObjectIndices[j] = l_Temp;
                }
            }
        }
    }

}

// Static Functions - Pixel Transfer ///////////////////////////////////////////////////////////////

void GABLE_PushColor (GABLE_PixelFetcher* p_Fetcher, Uint32 p_Color)
{
    p_Fetcher->m_PixelFIFO.m_Buffer[p_Fetcher->m_PixelFIFO.m_Tail] = p_Color;
    p_Fetcher->m_PixelFIFO.m_Tail = (p_Fetcher->m_PixelFIFO.m_Tail + 1) % GABLE_PPU_PIXEL_FIFO_SIZE;
    p_Fetcher->m_PixelFIFO.m_Size++;
}

void GABLE_PopColor (GABLE_PixelFetcher* p_Fetcher, Uint32* p_Color)
{
    *p_Color = p_Fetcher->m_PixelFIFO.m_Buffer[p_Fetcher->m_PixelFIFO.m_Head];
    p_Fetcher->m_PixelFIFO.m_Head = (p_Fetcher->m_PixelFIFO.m_Head + 1) % GABLE_PPU_PIXEL_FIFO_SIZE;
    p_Fetcher->m_PixelFIFO.m_Size--;
}

Bool GABLE_TryAddPixel (GABLE_PPU* p_PPU, GABLE_PixelFetcher* p_Fetcher)
{

    // The pixel fetcher's FIFO is considered full if it contains more than 8 pixels, enough pixels
    // to render the current line of the current tile.
    if (p_Fetcher->m_PixelFIFO.m_Size > 8)
    {
        return false;
    }

    // Get the fetched tile's attributes.
    GABLE_TileAttributes l_TileAttributes = p_Fetcher->m_FetchedBGW.m_TileAttributes;

    // Offset the pixel fetcher's X-coordinate by the scroll X register. Ensure that the resultant
    // X-coordinate is within the screen's bounds.
    Int32 l_OffsetX = p_Fetcher->m_FetchingX - (8 - (p_PPU->m_SCX % 8));
    if (l_OffsetX < 0) { return true; }

    // Iterate over the eight pixels which need to be shifted into the pixel FIFO.
    for (Uint8 i = 0; i < 8; ++i)
    {

        // Which bit of the tile data high and low bytes need to be added?
        Uint8 l_Bit = (l_TileAttributes.m_HorizontalFlip == 1) ? 7 - i : i;

        // Grab the proper bit from the tile data low and high bytes.
        Uint8 l_LowBit = (p_Fetcher->m_FetchedBGW.m_TileDataLow >> l_Bit) & 1;
        Uint8 l_HighBit = (p_Fetcher->m_FetchedBGW.m_TileDataHigh >> l_Bit) & 1;

        // Calculate the color index of the pixel.
        Uint8 l_ColorIndex = (l_HighBit << 1) | l_LowBit;

        // If the `GRPM` register is set to 1, then the PPU is in CGB graphics mode. Retrieve the
        // color from the background color RAM.
        Uint32 l_RGBAColorValue = 0;
        if (p_PPU->m_GRPM == 1)
        {
            l_RGBAColorValue = GABLE_GetBackgroundColor(
                p_PPU,
                l_TileAttributes.m_PaletteIndex,
                l_ColorIndex,
                NULL
            );
        }
        
        // If the `GRPM` register is set to 0, then the PPU is in DMG graphics mode. The color
        // should not be fetched if `LCDC` bit 0 is clear.
        else if (p_PPU->m_LCDC.m_BGWEnableOrPriority == true)
        {
            // Get the proper two bits from the `BGP` register.
            Uint8 l_BitPair = (p_PPU->m_BGP >> (l_ColorIndex * 2)) & 0b11;

            // Get the RGBA color value from the DMG palette.
            l_RGBAColorValue = GABLE_PPU_DMG_PALETTE[l_BitPair];
        }

        // Otherwise, this is DMG mode where the background/window layer is disabled. The pixel is
        // transparent.
        else
        {
            l_RGBAColorValue = GABLE_PPU_DMG_PALETTE[0];
        }

        // If the object layer is enabled, and there is at least one object residing on this pixel,
        // then fetch the object pixel's color.
        if (p_PPU->m_LCDC.m_ObjectEnable == true)
        {
            l_RGBAColorValue = GABLE_FetchObjectPixel(
                p_PPU,
                p_Fetcher,
                l_Bit,
                l_ColorIndex,
                l_RGBAColorValue,
                p_PPU->m_LCDC.m_BGWEnableOrPriority
            );
        }

        // Shift the pixel into the pixel FIFO.
        GABLE_PushColor(p_Fetcher, l_RGBAColorValue);
        p_Fetcher->m_QueueX++;

    }

}

void GABLE_ShiftNextPixel (GABLE_PPU* p_PPU, GABLE_PixelFetcher* p_Fetcher)
{

    // Only shift a pixel from the FIFO if it's full.
    if (p_Fetcher->m_PixelFIFO.m_Size > 8)
    {

        // Pop the pixel's color from the FIFO.
        Uint32 l_RGBAColorValue = 0;
        GABLE_PopColor(p_Fetcher, &l_RGBAColorValue);

        // Ensure that the pixel is within the bounds of the screen buffer.
        if (p_Fetcher->m_LineX >= (p_PPU->m_SCX % 8))
        {

            // Determine the index of the pixel in the screen buffer.
            Uint32 l_ScreenIndex = p_Fetcher->m_PushedX + (p_PPU->m_LY * GABLE_PPU_SCREEN_WIDTH);

            // Emplace the pixel into the screen buffer. Advance the fetcher's pushed X-coordinate.
            p_PPU->m_ScreenBuffer[l_ScreenIndex] = l_RGBAColorValue;
            p_Fetcher->m_PushedX++;

        }

        // Move the fetcher's X-coordinate to the next pixel.
        p_Fetcher->m_LineX++;

    }

}

Uint32 GABLE_FetchObjectPixel (GABLE_PPU* p_PPU, GABLE_PixelFetcher* p_Fetcher, Uint8 p_Bit, Uint8 p_ColorIndex, Uint32 p_RGBAColorValue, Uint8 p_BGWindowPriority)
{

    // The `p_ColorIndex` parameter contains the index of the color used to render a background-
    // or window-layer tile over which one or more of the objects fetched by the pixel fetcher may
    // be residing.
    //
    // Store that old color index here, as we may need to restore it later.
    Uint8 l_OldColorIndex = p_ColorIndex;

    // Iterate over the indices of the objects fetched by the pixel fetcher as it was processing the
    // current pixel.
    for (Count i = 0; i < p_Fetcher->m_FetchedOBJ.m_ObjectCount; ++i)
    {

        // Point to the object in the OAM buffer.
        const GABLE_Object* l_Object = &p_PPU->m_OAM[p_Fetcher->m_FetchedOBJ.m_ObjectIndices[i]];

        // Calculate the X-coordinate of the object's left edge on the screen.
        // Ensure the object is within the bounds of the screen.
        Uint8 l_ObjectX = (l_Object->m_X - 8) + (p_PPU->m_SCX % 8);
        if (l_ObjectX + 8 < p_Fetcher->m_QueueX)
        {
            continue;
        }

        // Calculate the offset of the current pixel within the object's tile data.
        // Ensure the offset is within the bounds of the tile data.
        Int8 l_Offset = p_Fetcher->m_QueueX - l_ObjectX;
        if (l_Offset < 0 || l_Offset >= 8)
        {
            continue;
        }

        // Correct the provided `p_Bit` parameter to account for the object's X-flip attribute.
        p_Bit = (l_Object->m_Attributes.m_HorizontalFlip == 1) ? 7 - l_Offset : l_Offset;

        // Get the proper bit of the object's tile data low and high bytes.
        Uint8 l_LowBit = (p_Fetcher->m_FetchedOBJ.m_TileDataLow[i] >> p_Bit) & 1;
        Uint8 l_HighBit = (p_Fetcher->m_FetchedOBJ.m_TileDataHigh[i] >> p_Bit) & 1;

        // Calculate the color index of the pixel.
        Uint8 l_ColorIndex = (l_HighBit << 1) | l_LowBit;

        // If the color index is zero, then the pixel is transparent and does not overwrite the
        // background or window layer.
        if (l_ColorIndex == 0)
        {
            continue;
        }
        
        // Check if one of the following conditions is met:
        // - The old color index is zero.
        // - The object's BGW priority attribute is zero.
        // - The provided `p_BGWindowPriority` parameter is zero.
        if (
            l_OldColorIndex == 0 ||
            l_Object->m_Attributes.m_Priority == 0 ||
            p_BGWindowPriority == 0
        )
        {
            // Is the graphics mode set to CGB mode?
            if (p_PPU->m_GRPM == 1)
            {
                p_RGBAColorValue = GABLE_GetObjectColor(
                    p_PPU,
                    l_Object->m_Attributes.m_PaletteIndex,
                    l_ColorIndex,
                    NULL
                );
            }

            // Otherwise, the graphics mode is set to DMG mode.
            else
            {
                // Get the proper two bits from the `OBP0` or `OBP1` register.
                Uint8 l_BitPair = (l_Object->m_Attributes.m_DMGPalette == 0) ?
                    (p_PPU->m_OBP0 >> (l_ColorIndex * 2)) & 0b11 :
                    (p_PPU->m_OBP1 >> (l_ColorIndex * 2)) & 0b11;

                // Get the RGBA color value from the DMG palette.
                p_RGBAColorValue = GABLE_PPU_DMG_PALETTE[l_BitPair];
            }

            if (p_ColorIndex == 0) { break; }
        }

    }

    // Return the RGBA color value of the pixel.
    return p_RGBAColorValue;

}

void GABLE_FetchBackgroundTileNumber (GABLE_PPU* p_PPU, GABLE_PixelFetcher* p_Fetcher)
{

    // Determine the relative starting address of the tile map used to render the background layer.
    Uint16 l_TileMapAddress = (p_PPU->m_LCDC.m_BGTilemapAddress == 0) ? 0x1800 : 0x1C00;

    // Determine the source Y position of the tile.
    Uint8 l_TileY = p_Fetcher->m_MapY / 8;

    // Use the pixel fetcher's map coordinates to determine the target address offset.
    Uint16 l_TargetOffset = (l_TileY * 32) + (p_Fetcher->m_MapX / 8);
    Uint16 l_TargetAddress = l_TileMapAddress + l_TargetOffset;

    // Fetch the tilemap number from VRAM bank 0, and the tile attributes from VRAM bank 1.
    // However, if access to VRAM is not currently permitted (display mode is pixel transfer), then
    // these values are set to garbage (0xFF).
    if (p_PPU->m_STAT.m_DisplayMode != GABLE_DM_PIXEL_TRANSFER)
    {
        p_Fetcher->m_FetchedBGW.m_TileIndex = p_PPU->m_VRAM0[l_TargetAddress];
        p_Fetcher->m_FetchedBGW.m_TileAttributes.m_Value = p_PPU->m_VRAM1[l_TargetAddress];
    }
    else
    {
        p_Fetcher->m_FetchedBGW.m_TileIndex = 0xFF;
        p_Fetcher->m_FetchedBGW.m_TileAttributes.m_Value = 0xFF;
    }

}

void GABLE_FetchWindowTileNumber (GABLE_PPU* p_PPU, GABLE_PixelFetcher* p_Fetcher)
{

    // Only fetch a window tile number if the following conditions are met:
    // - The window layer is currently enabled and visible.
    // - The fetcher's next X coordinate is within the bounds of the window layer.
    // - The `LY` register is within the bounds of the window layer.
    if (
        GABLE_IsWindowVisible(p_PPU) == true &&
        p_Fetcher->m_FetchingX + 7 >= p_PPU->m_WX &&
        p_Fetcher->m_FetchingX + 7 < (p_PPU->m_WX + GABLE_PPU_SCREEN_HEIGHT + 14) &&
        p_PPU->m_LY >= p_PPU->m_WY &&
        p_PPU->m_LY < p_PPU->m_WY + GABLE_PPU_SCREEN_WIDTH
    )
    {

        // Determine the relative starting address of the tile map used to render the window layer.
        Uint16 l_TileMapAddress = (p_PPU->m_LCDC.m_WindowTilemapAddress == 0) ? 0x1800 : 0x1C00;

        // Determine the source Y position of the tile.
        Uint8 l_TileY = p_PPU->m_WindowLine / 8;

        // Use the pixel fetcher's map coordinates to determine the target address offset.
        Uint16 l_TargetOffset = (l_TileY * 32) + ((p_Fetcher->m_FetchingX + 7 - p_PPU->m_WX) / 8);
        Uint16 l_TargetAddress = l_TileMapAddress + l_TargetOffset;

        // Fetch the tilemap number from VRAM bank 0, and the tile attributes from VRAM bank 1.
        // However, if access to VRAM is not currently permitted (display mode is pixel transfer), then
        // these values are set to garbage (0xFF).
        if (p_PPU->m_STAT.m_DisplayMode != GABLE_DM_PIXEL_TRANSFER)
        {
            p_Fetcher->m_FetchedBGW.m_TileIndex = p_PPU->m_VRAM0[l_TargetAddress];
            p_Fetcher->m_FetchedBGW.m_TileAttributes.m_Value = p_PPU->m_VRAM1[l_TargetAddress];
        }
        else
        {
            p_Fetcher->m_FetchedBGW.m_TileIndex = 0xFF;
            p_Fetcher->m_FetchedBGW.m_TileAttributes.m_Value = 0xFF;
        }

    }

}

void GABLE_FetchObjectTileNumber (GABLE_PPU* p_PPU, GABLE_PixelFetcher* p_Fetcher)
{
    // Iterate over the indices of the objects found on the current scanline.
    for (Uint8 i = 0; i < p_PPU->m_LineObjectCount; ++i)
    {

        // Point to the object in the OAM buffer.
        const GABLE_Object* l_Object = &p_PPU->m_OAM[p_PPU->m_LineObjectIndices[i]];

        // Calculate the object's scrolling-adjusted X-coordinate.
        Int16 l_ObjectX = (l_Object->m_X - 8) + (p_PPU->m_SCX % 8);

        // Ensure that the object's tile data will fit into the pixel fetcher's FIFO.
        if (
            (l_ObjectX >= p_Fetcher->m_FetchingX && l_ObjectX < p_Fetcher->m_FetchingX + 8) ||
            (l_ObjectX + 8 >= p_Fetcher->m_FetchingX && l_ObjectX + 8 < p_Fetcher->m_FetchingX + 8)
        )
        {

            // If it does, then add the object's index to the list of objects whose tile data needs
            // to be fetched.
            p_Fetcher->m_FetchedOBJ.m_ObjectIndices[p_Fetcher->m_FetchedOBJ.m_ObjectCount++] = 
                p_PPU->m_LineObjectIndices[i];

            // Limit 3 objects per pixel.
            if (p_Fetcher->m_FetchedOBJ.m_ObjectCount >= 3)
            {
                break;
            }

        }

    }
}

void GABLE_FetchObjectTileData (GABLE_PPU* p_PPU, GABLE_PixelFetcher* p_Fetcher, Uint8 p_Offset)
{

    // Determine the current object size.
    Uint8 l_ObjectHeight = (p_PPU->m_LCDC.m_ObjectSize == 1) ? 16 : 8;

    // Iterate over the indices of the objects found on this pixel, whose tile data needs to be fetched.
    for (Uint8 i = 0; i < p_Fetcher->m_FetchedOBJ.m_ObjectCount; ++i)
    {

        // Point to the object in the OAM buffer.
        const GABLE_Object* l_Object = &p_PPU->m_OAM[p_Fetcher->m_FetchedOBJ.m_ObjectIndices[i]];

        // Get the object's Y position on the screen. Adjust the position according to the object's
        // Y-flip attribute.
        Uint8 l_ObjectY = ((p_PPU->m_LY + 16) - l_Object->m_Y) * 2;
        if (l_Object->m_Attributes.m_VerticalFlip == 1)
        {
            l_ObjectY = ((l_ObjectHeight * 2) - 2) - l_ObjectY;
        }

        // Get the object's tile index, with its low bit cleared if tall objects (8x16) are being used.
        Uint8 l_TileIndex = l_Object->m_TileIndex & ((l_ObjectHeight == 16) ? 0xFE : 0xFF);

        // Calculate the target address of the object's tile data.
        Uint16 l_TargetAddress = (l_TileIndex * 16) + (l_ObjectY + p_Offset);

        // Fetch the proper tile data (low if `p_Offset` is 0, high if `p_Offset` is non-zero).
        // The above-calculated address is relative to the start of the VRAM bank, so permission to
        // access the VRAM bank will be checked for.
        if (p_Offset == 0)
        {
            p_Fetcher->m_FetchedOBJ.m_TileDataLow[i] = 
                (p_PPU->m_STAT.m_DisplayMode != GABLE_DM_PIXEL_TRANSFER) ?
                p_PPU->m_VRAM[l_TargetAddress] : 0xFF;
        }
        else
        {
            p_Fetcher->m_FetchedOBJ.m_TileDataHigh[i] = 
                (p_PPU->m_STAT.m_DisplayMode != GABLE_DM_PIXEL_TRANSFER) ?
                p_PPU->m_VRAM[l_TargetAddress] : 0xFF;
        }
        
    }

}

void GABLE_FetchTileNumber (GABLE_PPU* p_PPU, GABLE_PixelFetcher* p_Fetcher)
{

    // Clear the fetched object count.
    p_Fetcher->m_FetchedOBJ.m_ObjectCount = 0;

    // Check the graphics mode to determine the order in which the tile numbers are fetched.
    if (p_PPU->m_GRPM == 0)
    {
        // In DMG mode, if `LCDC` bit 0 is clear, then the background/window layer is not rendered
        // at all.
        if (p_PPU->m_LCDC.m_BGWEnableOrPriority == true)
        {
            // Fetch the tile number for the background layer.
            GABLE_FetchBackgroundTileNumber(p_PPU, p_Fetcher);

            // If the window layer is enabled, then fetch the tile number for the window layer.
            if (p_PPU->m_LCDC.m_WindowEnable == true)
            {
                GABLE_FetchWindowTileNumber(p_PPU, p_Fetcher);
            }
        }

        // If the object layer is enabled, and there are objects on the current scanline, then fetch
        // the tile number for an object.
        if (p_PPU->m_LCDC.m_ObjectEnable == true && p_PPU->m_LineObjectCount > 0)
        {
            GABLE_FetchObjectTileNumber(p_PPU, p_Fetcher);
        }
    }
    else
    {
        // In CGB mode, if `LCDC` bit 0 is clear, then the background/window layer does not have
        // priority over the object layer; all objects are rendered on top of the background/window
        // layer, provided they are visible on the current scanline and the object layer is enabled.
        //
        // Otherwise, the background/window layer may have priority over the object layer.
        if (p_PPU->m_LCDC.m_BGWEnableOrPriority == false)
        {
            // If the object layer is enabled, and there are objects on the current scanline, then fetch
            // the tile number for an object.
            if (p_PPU->m_LCDC.m_ObjectEnable == true && p_PPU->m_LineObjectCount > 0)
            {
                GABLE_FetchObjectTileNumber(p_PPU, p_Fetcher);
            }

            // Fetch the tile number for the background layer.
            GABLE_FetchBackgroundTileNumber(p_PPU, p_Fetcher);

            // If the window layer is enabled, then fetch the tile number for the window layer.
            if (p_PPU->m_LCDC.m_WindowEnable == true)
            {
                GABLE_FetchWindowTileNumber(p_PPU, p_Fetcher);
            }
        }
        else
        {
            // Fetch the tile number for the background layer.
            GABLE_FetchBackgroundTileNumber(p_PPU, p_Fetcher);

            // If the window layer is enabled, then fetch the tile number for the window layer.
            if (p_PPU->m_LCDC.m_WindowEnable == true)
            {
                GABLE_FetchWindowTileNumber(p_PPU, p_Fetcher);
            }

            // If the object layer is enabled, and there are objects on the current scanline, then fetch
            // the tile number for an object.
            if (p_PPU->m_LCDC.m_ObjectEnable == true && p_PPU->m_LineObjectCount > 0)
            {
                GABLE_FetchObjectTileNumber(p_PPU, p_Fetcher);
            }
        }
    }

    // Advance the pixel fetcher's X-coordinate by 8 pixels, then move to the next state.
    p_Fetcher->m_FetchingX += 8;
    p_Fetcher->m_Mode = GABLE_PFM_TILE_DATA_LOW;

}

void GABLE_FetchTileDataLow (GABLE_PPU* p_PPU, GABLE_PixelFetcher* p_Fetcher)
{

    // Get the index of the tile which needs to be fetched.
    Uint8 l_TileIndex = p_Fetcher->m_FetchedBGW.m_TileIndex;

    // Determine the target address to begin fetching the tile data from.
    // Adjust this address, if needed, based on the `LCDC` BGW tile data address bit.
    Uint16 l_TargetAddress = (l_TileIndex * 16) + p_Fetcher->m_TileDataOffset;
    if (l_TileIndex < 128 && p_PPU->m_LCDC.m_BGWindowTileDataAddress == 1)
    {
        l_TargetAddress += 0x1000;
    }

    // Fetch the low byte of the tile data. The above-calculated address is relative to the start
    // of the VRAM bank, so permission to access the VRAM bank will be checked for.
    Bool l_Read = GABLE_ReadVRAMByte(p_PPU, l_TargetAddress, &p_Fetcher->m_FetchedBGW.m_TileDataLow);
    GABLE_expect(l_Read == true, "Failed to read tile data low byte from VRAM");

    // If there is an object residing on this pixel, fetch that object's tile data as well.
    GABLE_FetchObjectTileData(p_PPU, p_Fetcher, 0);

    // Move to the next state.
    p_Fetcher->m_Mode = GABLE_PFM_TILE_DATA_HIGH;

}

void GABLE_FetchTileDataHigh (GABLE_PPU* p_PPU, GABLE_PixelFetcher* p_Fetcher)
{

    // The high byte of the tile data is fetched in the same manner as the low byte.
    Uint8 l_TileIndex = p_Fetcher->m_FetchedBGW.m_TileIndex;
    Uint16 l_TargetAddress = (l_TileIndex * 16) + p_Fetcher->m_TileDataOffset + 1;
    if (l_TileIndex < 128 && p_PPU->m_LCDC.m_BGWindowTileDataAddress == 1)
    {
        l_TargetAddress += 0x1000;
    }

    // Fetch the high byte of the tile data.
    Bool l_Read = GABLE_ReadVRAMByte(p_PPU, l_TargetAddress, &p_Fetcher->m_FetchedBGW.m_TileDataHigh);
    GABLE_expect(l_Read == true, "Failed to read tile data high byte from VRAM");

    // If there is an object residing on this pixel, fetch that object's tile data as well.
    GABLE_FetchObjectTileData(p_PPU, p_Fetcher, 1);

    // Move to the next state.
    p_Fetcher->m_Mode = GABLE_PFM_SLEEP;

}

void GABLE_FetchPushPixels (GABLE_PPU* p_PPU, GABLE_PixelFetcher* p_Fetcher)
{

    // In this mode, continuous attempts are made to push a pixel to the pixel FIFO buffer.
    if (GABLE_TryAddPixel(p_PPU, p_Fetcher) == true)
    {
        // Once successful, return to the tile number state.
        p_Fetcher->m_Mode = GABLE_PFM_TILE_NUMBER;
    }

}

void GABLE_FetchSleep (GABLE_PPU* p_PPU, GABLE_PixelFetcher* p_Fetcher)
{
    // Here, the fetcher sleeps for two dots before pushing the pixels to the screen buffer.
    p_Fetcher->m_Mode = GABLE_PFM_PUSH_PIXELS;
}

void GABLE_TickPixelFetcher (GABLE_PPU* p_PPU, GABLE_PixelFetcher* p_Fetcher)
{

    // Calculate the absolute position of the pixel to be processed, in the 256x256 pixel tilemap.
    p_Fetcher->m_MapY = p_PPU->m_LY + p_PPU->m_SCY;
    p_Fetcher->m_MapX = p_Fetcher->m_FetchingX + p_PPU->m_SCX;

    // Calculate the offset address of the tile data to be fetched.
    p_Fetcher->m_TileDataOffset = (p_Fetcher->m_MapY % 8) * 2;

    // During each even-numbered dot in the pixel transfer mode, the pixel fetcher will work
    // to keep supplying its pixel FIFO buffer with new pixels to draw to the screen buffer.
    if (p_PPU->m_CurrentDot % 2 == 0)
    {

        // Run the appropriate function in the pixel-fetcher's state machine.
        switch (p_Fetcher->m_Mode)
        {
            case GABLE_PFM_TILE_NUMBER: GABLE_FetchTileNumber(p_PPU, p_Fetcher); break;
            case GABLE_PFM_TILE_DATA_LOW: GABLE_FetchTileDataLow(p_PPU, p_Fetcher); break;
            case GABLE_PFM_TILE_DATA_HIGH: GABLE_FetchTileDataHigh(p_PPU, p_Fetcher); break;
            case GABLE_PFM_PUSH_PIXELS: GABLE_FetchPushPixels(p_PPU, p_Fetcher); break;
            case GABLE_PFM_SLEEP: GABLE_FetchSleep(p_PPU, p_Fetcher); break;
        }

    }

    // In any event, try to shift a pixel from the FIFO buffer into the screen buffer.
    GABLE_ShiftNextPixel(p_PPU, p_Fetcher);

}

void GABLE_ResetPixelFetcher (GABLE_PPU* p_PPU, GABLE_PixelFetcher* p_Fetcher)
{

    // Reset the pixel FIFO buffer.
    p_Fetcher->m_PixelFIFO.m_Head = 0;
    p_Fetcher->m_PixelFIFO.m_Tail = 0;
    p_Fetcher->m_PixelFIFO.m_Size = 0;

}

// Static Functions - PPU State Machine ////////////////////////////////////////////////////////////

void GABLE_TickHorizontalBlank (GABLE_PPU* p_PPU, GABLE_Engine* p_Engine)
{

    // Increment the current dot.
    p_PPU->m_CurrentDot++;

    // If the current dot is at least 456, then the horizontal blank is complete.
    if (p_PPU->m_CurrentDot >= 456)
    {

        // Increment the LY register.
        GABLE_IncrementLY(p_PPU, p_Engine);

        // If all 144 visible scanlines of the current frame have been rendered, then the vertical
        // blank period has begun.
        if (p_PPU->m_LY >= GABLE_PPU_SCREEN_HEIGHT)
        {
            // Move to the vertical blank state and request the `VBLANK` interrupt.
            p_PPU->m_STAT.m_DisplayMode = GABLE_DM_VERTICAL_BLANK;
            GABLE_RequestInterrupt(GABLE_GetInterruptContext(p_Engine), GABLE_INT_VBLANK);

            // If the `LCD_STAT` interrupt source is enabled for the vertical blank period, then
            // request the `LCD_STAT` interrupt as well.
            if (p_PPU->m_STAT.m_VerticalBlankStatSource == true)
            {
                GABLE_RequestInterrupt(GABLE_GetInterruptContext(p_Engine), GABLE_INT_LCD_STAT);
            }

            // If the frame rendered callback is provided, call it here.
            if (p_PPU->m_FrameRenderedCallback != NULL)
            {
                p_PPU->m_FrameRenderedCallback(p_Engine);
            }
        }

        // If there are still visible scanlines to render, then move to the object scan state.
        else
        {
            p_PPU->m_STAT.m_DisplayMode = GABLE_DM_OBJECT_SCAN;

            // If its stat source is set, request the `LCD_STAT` interrupt.
            if (p_PPU->m_STAT.m_ObjectScanStatSource == true)
            {
                GABLE_RequestInterrupt(GABLE_GetInterruptContext(p_Engine), GABLE_INT_LCD_STAT);
            }
        }

        // Reset the current dot.
        p_PPU->m_CurrentDot = 0;

    }

}

void GABLE_TickVerticalBlank (GABLE_PPU* p_PPU, GABLE_Engine* p_Engine)
{
    
    // Increment the current dot.
    p_PPU->m_CurrentDot++;

    // If the current dot is at least 456, then the vertical blank is complete.
    if (p_PPU->m_CurrentDot >= 456)
    {

        // Increment the LY register.
        GABLE_IncrementLY(p_PPU, p_Engine);

        // If all 154 scanlines have been processed, then the vertical blank period is complete.
        if (p_PPU->m_LY >= GABLE_LINES_PER_FRAME)
        {

            // Reset the LY register and window line counter.
            p_PPU->m_LY = 0;
            p_PPU->m_WindowLine = 0;

            // Move to the object scan state and begin processing the next frame.
            p_PPU->m_STAT.m_DisplayMode = GABLE_DM_OBJECT_SCAN;

            // If its stat source is set, request the `LCD_STAT` interrupt.
            if (p_PPU->m_STAT.m_ObjectScanStatSource == true)
            {
                GABLE_RequestInterrupt(GABLE_GetInterruptContext(p_Engine), GABLE_INT_LCD_STAT);
            }

        }

        // Reset the current dot.
        p_PPU->m_CurrentDot = 0;

    }
}

void GABLE_TickObjectScan (GABLE_PPU* p_PPU, GABLE_Engine* p_Engine)
{

    // Increment the current dot. Save the old dot tick.
    Uint16 l_Dot = p_PPU->m_CurrentDot++;

    // If the incremented dot is at least 80, then the object scan is complete. Move to the pixel
    // transfer state.
    if (p_PPU->m_CurrentDot >= 80)
    {
        p_PPU->m_STAT.m_DisplayMode = GABLE_DM_PIXEL_TRANSFER;        
    }
    else if (l_Dot % 2 == 0)
    {
        // Find the next object on the current scanline.
        GABLE_FindLineObject(p_PPU);
    }

}

void GABLE_TickPixelTransfer (GABLE_PPU* p_PPU, GABLE_Engine* p_Engine)
{

    // Tick the pixel fetcher.
    GABLE_TickPixelFetcher(p_PPU, &p_PPU->m_PixelFetcher);

    // Increment the current dot.
    p_PPU->m_CurrentDot++;

    // If the pixel fetcher has pushed enough pixels to the screen buffer to fill a scanline, then
    // the pixel transfer is complete. Move to the horizontal blank state.
    if (p_PPU->m_PixelFetcher.m_PushedX >= GABLE_PPU_SCREEN_WIDTH)
    {
        
        // Reset the pixel fetcher.
        GABLE_ResetPixelFetcher(p_PPU, &p_PPU->m_PixelFetcher);

        // Move to the horizontal blank state. If its stat source is set, request the `LCD_STAT`
        // interrupt.
        p_PPU->m_STAT.m_DisplayMode = GABLE_DM_HORIZONTAL_BLANK;
        if (p_PPU->m_STAT.m_HorizontalBlankStatSource == true)
        {
            GABLE_RequestInterrupt(GABLE_GetInterruptContext(p_Engine), GABLE_INT_LCD_STAT);
        }

        // At the start of each H-Blank period, another block of HDMA data is transferred.
        GABLE_TickHDMA(p_PPU, p_Engine);

    }

}

// Static Functions - HDMA Transfer ////////////////////////////////////////////////////////////////

void GABLE_TickHDMA (GABLE_PPU* p_PPU, GABLE_Engine* p_Engine)
{
    if (p_PPU->m_HDMABlocksLeft > 0)
    {
        // If the HDMA transfer is active, then decrement the number of blocks left to transfer.
        p_PPU->m_HDMABlocksLeft--;

        // Transfer the next block of data
        for (Uint8 i = 0; i < 0x10; i++)
        {
            Uint8 l_Value = 0x00;
            GABLE_ReadByte(p_Engine, p_PPU->m_HDMASource++, &l_Value);
            GABLE_WriteVRAMByte(p_PPU, p_PPU->m_HDMADestination++, l_Value);
        }
    }
}

// Public Functions ////////////////////////////////////////////////////////////////////////////////

GABLE_PPU* GABLE_CreatePPU ()
{

    // Allocate the PPU context.
    GABLE_PPU* l_PPU = GABLE_calloc(1, GABLE_PPU);
    GABLE_pexpect(l_PPU != NULL, "Failed to allocate PPU context");

    // Initialize the PPU context.
    GABLE_ResetPPU(l_PPU);

    // Return the PPU context.
    return l_PPU;

}

void GABLE_DestroyPPU (GABLE_PPU* p_PPU)
{

    if (p_PPU != NULL)
    {
        GABLE_free(p_PPU);
    }

}

void GABLE_ResetPPU (GABLE_PPU* p_PPU)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");

    // Point the VRAM pointer to the first VRAM bank.
    p_PPU->m_VRAM = p_PPU->m_VRAM0;

    // Reset the hardware registers.
    p_PPU->m_LCDC.m_Register = 0x91;
    p_PPU->m_STAT.m_Register = 0x85;
    p_PPU->m_SCY = 0x00;
    p_PPU->m_SCX = 0x00;
    p_PPU->m_LY = 0x00;
    p_PPU->m_LYC = 0x00;
    p_PPU->m_DMA = 0x00;
    p_PPU->m_BGP = 0xFC;
    p_PPU->m_OBP0 = 0xFF;
    p_PPU->m_OBP1 = 0xFF;
    p_PPU->m_WY = 0x00;
    p_PPU->m_WX = 0x00;
    p_PPU->m_VBK = 0x00;
    p_PPU->m_HDMA1 = 0x00;
    p_PPU->m_HDMA2 = 0x00;
    p_PPU->m_HDMA3 = 0x00;
    p_PPU->m_HDMA4 = 0x00;
    p_PPU->m_HDMA5.m_Register = 0x00;
    p_PPU->m_BGPI.m_Register = 0xFC;
    p_PPU->m_OBPI.m_Register = 0xFF;
    p_PPU->m_OPRI = 0xFF;
    p_PPU->m_GRPM = 0x00;

    // Initialize the color RAM buffers. Initialize all palettes to the default DMG palette.
    for (Count i = 0; i < GABLE_PPU_CRAM_SIZE; i += 8)
    {
        p_PPU->m_BgCRAM[i    ] = 0b11111111; p_PPU->m_ObjCRAM[i    ] = 0b11111111;
        p_PPU->m_BgCRAM[i + 1] = 0b11111110; p_PPU->m_ObjCRAM[i + 1] = 0b11111110;
        p_PPU->m_BgCRAM[i + 2] = 0b10011100; p_PPU->m_ObjCRAM[i + 2] = 0b10011100;
        p_PPU->m_BgCRAM[i + 3] = 0b11100110; p_PPU->m_ObjCRAM[i + 3] = 0b11100110;
        p_PPU->m_BgCRAM[i + 4] = 0b01011010; p_PPU->m_ObjCRAM[i + 4] = 0b01011010;
        p_PPU->m_BgCRAM[i + 5] = 0b11010110; p_PPU->m_ObjCRAM[i + 5] = 0b11010110;
        p_PPU->m_BgCRAM[i + 6] = 0b00001000; p_PPU->m_ObjCRAM[i + 6] = 0b00001000;
        p_PPU->m_BgCRAM[i + 7] = 0b01000010; p_PPU->m_ObjCRAM[i + 7] = 0b01000010;
    }
}

void GABLE_TickPPU (GABLE_PPU* p_PPU, GABLE_Engine* p_Engine)
{

    GABLE_expect(p_PPU, "PPU context is NULL!");
    GABLE_expect(p_Engine, "Engine context is NULL!");
    
    // Run the appropriate PPU state machine based on the current PPU display mode.
    switch (p_PPU->m_STAT.m_DisplayMode)
    {
        case GABLE_DM_HORIZONTAL_BLANK: GABLE_TickHorizontalBlank(p_PPU, p_Engine); break;
        case GABLE_DM_VERTICAL_BLANK:   GABLE_TickVerticalBlank(p_PPU, p_Engine); break;
        case GABLE_DM_OBJECT_SCAN:      GABLE_TickObjectScan(p_PPU, p_Engine); break;
        case GABLE_DM_PIXEL_TRANSFER:   GABLE_TickPixelTransfer(p_PPU, p_Engine); break;
    }

}

void GABLE_TickODMA (GABLE_PPU* p_PPU, GABLE_Engine* p_Engine)
{

    // Check to see if the DMA transfer is active.
    if (p_PPU->m_ODMATicks >= 0xA0)
    {
        return;
    }

    // If the ODMA transfer has been initiated, check to see if there is a delay still before
    // actually starting the transfer.
    if (p_PPU->m_ODMADelay > 0)
    {
        p_PPU->m_ODMADelay--;
        return;
    }

    // If the ODMA transfer is active, then transfer the next byte of data.
    Uint8 l_Value = 0x00;
    GABLE_ReadByte(p_Engine, p_PPU->m_ODMASource + p_PPU->m_ODMATicks, &l_Value);
    GABLE_WriteOAMByte(p_PPU, p_PPU->m_ODMADestination + p_PPU->m_ODMATicks, l_Value);

    // Increment the number of ticks.
    p_PPU->m_ODMATicks++;

}

// Public Functions - Memory Access ////////////////////////////////////////////////////////////////

Bool GABLE_ReadVRAMByte (const GABLE_PPU* p_PPU, Uint16 p_Address, Uint8* p_Value)
{

    GABLE_expect(p_PPU, "PPU context is NULL!");
    GABLE_expect(p_Value, "Value pointer is NULL!");

    // If, for some reason, the VRAM pointer is NULL, return `false`.
    if (p_PPU->m_VRAM == NULL)
    {
        GABLE_error("Current VRAM bank pointer is NULL. Write to 'VBK' register to correct this.");
        return false;
    }
    
    // If a relative address (`0x0000` to `0x1FFF`) is provided, then it can be assumed that VRAM is
    // being accessed from GABLE's address bus. If the PPU is currently transferring pixels to the
    // screen buffer (i.e., in the pixel transfer state), then the VRAM buffer is locked and cannot be
    // accessed. In this case, the byte returned is `0xFF`.
    //
    // This check is not performed if this read is being conducted as part of an OAM DMA transfer.
    if (p_Address < GABLE_PPU_VRAM_BANK_SIZE && p_PPU->m_ODMATicks >= 0xA0)
    {
        if (p_PPU->m_STAT.m_DisplayMode == GABLE_DM_PIXEL_TRANSFER)
        {
            *p_Value = 0xFF;
            return true;
        }
    }

    // If an absolute address (`0x8000` to `0x9FFF`) is provided, then it can be assumed that the
    // VRAM is being accessed internally by the PPU. In this case, the VRAM buffer is always accessible; 
    // correct the provided address to a relative address.
    else if (p_Address >= GABLE_GB_VRAM_START && p_Address <= GABLE_GB_VRAM_END)
    {
        p_Address -= GABLE_GB_VRAM_START;
    }

    // If the address is out of bounds, return `false`.
    else
    {
        GABLE_error("VRAM read address $%04X is out of bounds.", p_Address);
        return false;
    }

    // Read the byte from the current VRAM bank.
    *p_Value = p_PPU->m_VRAM[p_Address];
    return true;

}

Bool GABLE_ReadOAMByte (const GABLE_PPU* p_PPU, Uint16 p_Address, Uint8* p_Value)
{

    GABLE_expect(p_PPU, "PPU context is NULL!");
    GABLE_expect(p_Value, "Value pointer is NULL!");
    
    // If a relative address (`0x0000` to `0x009F`) is provided, then it can be assumed that the OAM
    // buffer is being accessed from GABLE's address bus. In this case, the OAM buffer can only be
    // accessed during the `VBLANK` or `HBLANK` states.
    //
    // This check is not performed if this read is being conducted as part of an OAM DMA transfer.
    if (p_Address < GABLE_PPU_OAM_SIZE && p_PPU->m_ODMATicks >= 0xA0)
    {
        if (
            p_PPU->m_STAT.m_DisplayMode != GABLE_DM_VERTICAL_BLANK && 
            p_PPU->m_STAT.m_DisplayMode != GABLE_DM_HORIZONTAL_BLANK
        )
        {
            *p_Value = 0xFF;
            return true;
        }
    }

    // If an absolute address (`0xFE00` to `0xFE9F`) is provided, then it can be assumed that the OAM
    // buffer is being accessed internally by the PPU. In this case, the OAM buffer is always accessible;
    // correct the provided address to a relative address.
    else if (p_Address >= GABLE_GB_OAM_START && p_Address <= GABLE_GB_OAM_END)
    {
        p_Address -= GABLE_GB_OAM_START;
    }

    // If the address is out of bounds, return `false`.
    else
    {
        GABLE_error("OAM read address $%04X is out of bounds.", p_Address);
        return false;
    }

    // Cast the OAM buffer to a byte array and read the byte from that array.
    const Uint8* l_OAM = (const Uint8*) p_PPU->m_OAM;
    *p_Value = l_OAM[p_Address];
    return true;

}

Bool GABLE_WriteVRAMByte (GABLE_PPU* p_PPU, Uint16 p_Address, Uint8 p_Value)
{

    GABLE_expect(p_PPU, "PPU context is NULL!");

    // If, for some reason, the VRAM pointer is NULL, return `false`.
    if (p_PPU->m_VRAM == NULL)
    {
        GABLE_error("Current VRAM bank pointer is NULL. Write to 'VBK' register to correct this.");
        return false;
    }

    // If a relative address (`0x0000` to `0x1FFF`) is provided, then it can be assumed that VRAM is
    // being accessed from GABLE's address bus. If the PPU is currently transferring pixels to the
    // screen buffer (i.e., in the pixel transfer state), then the VRAM buffer is locked and cannot be
    // accessed. In this case, the byte is not written.
    if (p_Address < GABLE_PPU_VRAM_BANK_SIZE)
    {
        if (p_PPU->m_STAT.m_DisplayMode == GABLE_DM_PIXEL_TRANSFER)
        {
            return true;
        }
    }

    // If an absolute address (`0x8000` to `0x9FFF`) is provided, then it can be assumed that the
    // VRAM is being accessed internally by the PPU, perhaps as part of a GDMA or HDMA transfer
    // process. In this case, the VRAM buffer is always accessible; correct the provided address to a
    // relative address.
    else if (p_Address >= GABLE_GB_VRAM_START && p_Address <= GABLE_GB_VRAM_END)
    {
        p_Address -= GABLE_GB_VRAM_START;
    }

    // If the address is out of bounds, return `false`.
    else
    {
        GABLE_error("VRAM write address $%04X is out of bounds.", p_Address);
        return false;
    }

    // Write the byte to the current VRAM bank.
    p_PPU->m_VRAM[p_Address] = p_Value;
    return true;

}

Bool GABLE_WriteOAMByte (GABLE_PPU* p_PPU, Uint16 p_Address, Uint8 p_Value)
{

    GABLE_expect(p_PPU, "PPU context is NULL!");

    // If a relative address (`0x0000` to `0x009F`) is provided, then it can be assumed that the OAM
    // buffer is being accessed from GABLE's address bus. In this case, the OAM buffer can only be
    // accessed during the `VBLANK` or `HBLANK` states.
    if (p_Address < GABLE_PPU_OAM_SIZE)
    {
        if (
            p_PPU->m_STAT.m_DisplayMode != GABLE_DM_VERTICAL_BLANK && 
            p_PPU->m_STAT.m_DisplayMode != GABLE_DM_HORIZONTAL_BLANK
        )
        {
            return true;
        }
    }

    // If an absolute address (`0xFE00` to `0xFE9F`) is provided, then it can be assumed that the OAM
    // buffer is being accessed internally by the PPU, likely as part of an OAM DMA transfer process. In
    // this case, the OAM buffer is always accessible; correct the provided address to a relative address.
    else if (p_Address >= GABLE_GB_OAM_START && p_Address <= GABLE_GB_OAM_END)
    {
        p_Address -= GABLE_GB_OAM_START;
    }

    // If the address is out of bounds, return `false`.
    else
    {
        GABLE_error("OAM write address $%04X is out of bounds.", p_Address);
        return false;
    }

    // Cast the OAM buffer to a byte array and write the byte to that array.
    Uint8* l_OAM = (Uint8*) p_PPU->m_OAM;
    l_OAM[p_Address] = p_Value;
    return true;

}

// Public Functions - Color ////////////////////////////////////////////////////////////////////////

Uint32 GABLE_GetBackgroundColor (const GABLE_PPU* p_PPU, Uint8 p_PaletteIndex, Uint8 p_ColorIndex, GABLE_Color* p_Color)
{

    GABLE_expect(p_PPU, "PPU context is NULL!");

    // Validate the color and palette indices.
    if (p_PaletteIndex >= GABLE_PPU_CRAM_PALETTE_COUNT)
    {
        GABLE_error("Invalid background palette index: %u.", p_PaletteIndex);
        return 0x00000000;
    }
    if (p_ColorIndex >= GABLE_PPU_CRAM_PALETTE_COLOR_COUNT)
    {
        GABLE_error("Invalid background color index: %u.", p_ColorIndex);
        return 0x00000000;
    }

    // Determine the start index of the color in the CRAM buffer.
    Uint8 l_StartIndex = (p_PaletteIndex * GABLE_PPU_CRAM_PALETTE_COLOR_COUNT) + (p_ColorIndex * 2);

    // Get the color data from the CRAM buffer.
    Uint8 l_ColorData[2] = { p_PPU->m_BgCRAM[l_StartIndex], p_PPU->m_BgCRAM[l_StartIndex + 1] };

    // Extract the RGB555 color data. Remember that the color data is laid out as follows:
    // `0bRRRRRGGG` `0bGGBBBBB0`
    Uint8 l_Red   = (l_ColorData[0] & 0b11111000) >> 3;
    Uint8 l_Green = ((l_ColorData[0] & 0b00000111) << 2) | ((l_ColorData[1] & 0b11000000) >> 6);
    Uint8 l_Blue  = (l_ColorData[1] & 0b00111110) >> 1;

    // If a color structure was provided, store the color data in it.
    if (p_Color != NULL)
    {
        p_Color->m_Red   = l_Red;
        p_Color->m_Green = l_Green;
        p_Color->m_Blue  = l_Blue;
    }

    // Return the RGBA color value.
    return (
        ((l_Red * 8) << 24) |
        ((l_Green * 8) << 16) |
        ((l_Blue * 8) << 8) |
        0xFF
    );
}

Uint32 GABLE_GetObjectColor (const GABLE_PPU* p_PPU, Uint8 p_PaletteIndex, Uint8 p_ColorIndex, GABLE_Color* p_Color)
{

    GABLE_expect(p_PPU, "PPU context is NULL!");

    // Validate the color and palette indices.
    if (p_PaletteIndex >= GABLE_PPU_CRAM_PALETTE_COUNT)
    {
        GABLE_error("Invalid object palette index: %u.", p_PaletteIndex);
        return 0x00000000;
    }
    if (p_ColorIndex >= GABLE_PPU_CRAM_PALETTE_COLOR_COUNT)
    {
        GABLE_error("Invalid object color index: %u.", p_ColorIndex);
        return 0x00000000;
    }

    // Determine the start index of the color in the CRAM buffer.
    Uint8 l_StartIndex = (p_PaletteIndex * GABLE_PPU_CRAM_PALETTE_COLOR_COUNT) + (p_ColorIndex * 2);

    // Get the color data from the CRAM buffer.
    Uint8 l_ColorData[2] = { p_PPU->m_ObjCRAM[l_StartIndex], p_PPU->m_ObjCRAM[l_StartIndex + 1] };

    // Extract the RGB555 color data. Remember that the color data is laid out as follows:
    // `0bRRRRRGGG` `0bGGBBBBB0`
    Uint8 l_Red   = (l_ColorData[0] & 0b11111000) >> 3;
    Uint8 l_Green = ((l_ColorData[0] & 0b00000111) << 2) | ((l_ColorData[1] & 0b11000000) >> 6);
    Uint8 l_Blue  = (l_ColorData[1] & 0b00111110) >> 1;

    // If a color structure was provided, store the color data in it.
    if (p_Color != NULL)
    {
        p_Color->m_Red   = l_Red;
        p_Color->m_Green = l_Green;
        p_Color->m_Blue  = l_Blue;
    }

    // Return the RGBA color value.
    return (
        ((l_Red * 8) << 24) |
        ((l_Green * 8) << 16) |
        ((l_Blue * 8) << 8) |
        0xFF
    );
}

Bool GABLE_SetBackgroundColor (GABLE_PPU* p_PPU, Uint8 p_PaletteIndex, Uint8 p_ColorIndex, const GABLE_Color* p_RGB555, Uint32 p_RGBA)
{

    GABLE_expect(p_PPU, "PPU context is NULL!");

    // Validate the color and palette indices.
    if (p_PaletteIndex >= GABLE_PPU_CRAM_PALETTE_COUNT)
    {
        GABLE_error("Invalid background palette index: %u.", p_PaletteIndex);
        return false;
    }
    if (p_ColorIndex >= GABLE_PPU_CRAM_PALETTE_COLOR_COUNT)
    {
        GABLE_error("Invalid background color index: %u.", p_ColorIndex);
        return false;
    }

    // Determine the start index of the color in the CRAM buffer.
    Uint8 l_StartIndex = (p_PaletteIndex * GABLE_PPU_CRAM_PALETTE_COLOR_COUNT) + (p_ColorIndex * 2);

    // Extract the RGB555 color data from the provided color structure or RGBA value.
    Uint8 l_Red = 0, l_Green = 0, l_Blue = 0;
    if (p_RGB555 != NULL)
    {
        l_Red   = p_RGB555->m_Red;
        l_Green = p_RGB555->m_Green;
        l_Blue  = p_RGB555->m_Blue;
    }
    else
    {
        l_Red   = ((p_RGBA >> 24) & 0xFF) / 8;
        l_Green = ((p_RGBA >> 16) & 0xFF) / 8;
        l_Blue  = ((p_RGBA >> 8) & 0xFF) / 8;
    }

    // Validate the color data. Correct if out of bounds.
    if (l_Red > 31)   { l_Red   = 31; }
    if (l_Green > 31) { l_Green = 31; }
    if (l_Blue > 31)  { l_Blue  = 31; }

    // Pack the RGB555 color data into the CRAM buffer. Remember that the color data is laid out as
    // follows: `0bRRRRRGGG` `0bGGBBBBB0`
    p_PPU->m_BgCRAM[l_StartIndex]     = (l_Red << 3) | (l_Green >> 2);
    p_PPU->m_BgCRAM[l_StartIndex + 1] = ((l_Green & 0b00000011) << 6) | (l_Blue << 1);

    return true;
}

Bool GABLE_SetObjectColor (GABLE_PPU* p_PPU, Uint8 p_PaletteIndex, Uint8 p_ColorIndex, const GABLE_Color* p_RGB555, Uint32 p_RGBA)
{

    GABLE_expect(p_PPU, "PPU context is NULL!");

    // Validate the color and palette indices.
    if (p_PaletteIndex >= GABLE_PPU_CRAM_PALETTE_COUNT)
    {
        GABLE_error("Invalid object palette index: %u.", p_PaletteIndex);
        return false;
    }
    if (p_ColorIndex >= GABLE_PPU_CRAM_PALETTE_COLOR_COUNT)
    {
        GABLE_error("Invalid object color index: %u.", p_ColorIndex);
        return false;
    }

    // Determine the start index of the color in the CRAM buffer.
    Uint8 l_StartIndex = (p_PaletteIndex * GABLE_PPU_CRAM_PALETTE_COLOR_COUNT) + (p_ColorIndex * 2);

    // Extract the RGB555 color data from the provided color structure or RGBA value.
    Uint8 l_Red = 0, l_Green = 0, l_Blue = 0;
    if (p_RGB555 != NULL)
    {
        l_Red   = p_RGB555->m_Red;
        l_Green = p_RGB555->m_Green;
        l_Blue  = p_RGB555->m_Blue;
    }
    else
    {
        l_Red   = ((p_RGBA >> 24) & 0xFF) / 8;
        l_Green = ((p_RGBA >> 16) & 0xFF) / 8;
        l_Blue  = ((p_RGBA >> 8) & 0xFF) / 8;
    }

    // Validate the color data. Correct if out of bounds.
    if (l_Red > 31)   { l_Red   = 31; }
    if (l_Green > 31) { l_Green = 31; }
    if (l_Blue > 31)  { l_Blue  = 31; }

    // Pack the RGB555 color data into the CRAM buffer. Remember that the color data is laid out as
    // follows: `0bRRRRRGGG` `0bGGBBBBB0`
    p_PPU->m_ObjCRAM[l_StartIndex]     = (l_Red << 3) | (l_Green >> 2);
    p_PPU->m_ObjCRAM[l_StartIndex + 1] = ((l_Green & 0b00000011) << 6) | (l_Blue << 1);

    return true;
}

// Public Functions - Object Attribute Memory (OAM) ////////////////////////////////////////////////

GABLE_Object* GABLE_GetObject (GABLE_PPU* p_PPU, Uint8 p_Index)
{

    GABLE_expect(p_PPU, "PPU context is NULL!");

    // Validate the object index.
    if (p_Index >= GABLE_PPU_OAM_OBJECT_COUNT)
    {
        GABLE_error("Invalid object index: %u.", p_Index);
        return NULL;
    }

    // Return a pointer to the object at the provided index.
    return &p_PPU->m_OAM[p_Index];

}

void GABLE_GetObjectPosition (const GABLE_Object* p_Object, Uint8* p_X, Uint8* p_Y)
{

    GABLE_expect(p_Object, "Object is NULL!");

    // Extract the X and Y coordinates from the object's attributes.
    if (p_X != NULL) { *p_X = p_Object->m_X; }
    if (p_Y != NULL) { *p_Y = p_Object->m_Y; }

}

Uint8 GABLE_GetObjectTileIndex (const GABLE_Object* p_Object)
{

    GABLE_expect(p_Object, "Object is NULL!");

    // Extract the tile index from the object's attributes.
    return p_Object->m_TileIndex;

}

GABLE_TileAttributes GABLE_GetObjectAttributes (const GABLE_Object* p_Object)
{

    GABLE_expect(p_Object, "Object is NULL!");

    // Extract the object's attributes.
    return p_Object->m_Attributes;

}

void GABLE_SetObjectPosition (GABLE_Object* p_Object, Uint8 p_X, Uint8 p_Y)
{

    GABLE_expect(p_Object, "Object is NULL!");

    // Set the X and Y coordinates in the object's attributes.
    p_Object->m_X = p_X;
    p_Object->m_Y = p_Y;

}

void GABLE_SetObjectTileIndex (GABLE_Object* p_Object, Uint8 p_TileIndex)
{

    GABLE_expect(p_Object, "Object is NULL!");

    // Set the tile index in the object's attributes.
    p_Object->m_TileIndex = p_TileIndex;

}

void GABLE_SetObjectAttributes (GABLE_Object* p_Object, GABLE_TileAttributes p_Attributes)
{

    GABLE_expect(p_Object, "Object is NULL!");

    // Set the object's attributes.
    p_Object->m_Attributes = p_Attributes;

}

void GABLE_MoveObject (GABLE_Object* p_Object, Int8 p_DeltaX, Int8 p_DeltaY)
{

    GABLE_expect(p_Object, "Object is NULL!");

    // Move the object by the provided deltas.
    p_Object->m_X += p_DeltaX;
    p_Object->m_Y += p_DeltaY;

}

// Public Functions - Tilemaps /////////////////////////////////////////////////////////////////////

Uint8 GABLE_GetTileIndex (const GABLE_PPU* p_PPU, Uint8 p_MapIndex, Uint8 p_X, Uint8 p_Y)
{
    
    GABLE_expect(p_PPU, "PPU context is NULL!");

    // Ensure that the PPU is not in the pixel transfer state.
    if (p_PPU->m_STAT.m_DisplayMode == GABLE_DM_PIXEL_TRANSFER)
    {
        return 0xFF;
    }

    // Validate the X and Y coordinates. Correct if out of bounds.
    p_X %= 32;
    p_Y %= 32;
    
    // Using bit 0 of the `p_MapIndex` parameter, determine the start address of the tilemap
    // which is to be accessed.
    Uint16 l_StartAddress = (GABLE_bit(p_MapIndex, 0) != 0) ? 0x1C00 : 0x1800;

    // Calculate the tilemap address.
    Uint16 l_TilemapAddress = l_StartAddress + (p_Y * 32) + p_X;

    // Read the tile index from the tilemap.
    return p_PPU->m_VRAM0[l_TilemapAddress];
}

Uint8 GABLE_GetBackgroundTileIndex (const GABLE_PPU* p_PPU, Uint8 p_X, Uint8 p_Y)
{
    return GABLE_GetTileIndex(p_PPU, p_PPU->m_LCDC.m_BGTilemapAddress, p_X, p_Y);
}

Uint8 GABLE_GetWindowTileIndex (const GABLE_PPU* p_PPU, Uint8 p_X, Uint8 p_Y)
{
    return GABLE_GetTileIndex(p_PPU, p_PPU->m_LCDC.m_WindowTilemapAddress, p_X, p_Y);
}

GABLE_TileAttributes GABLE_GetTileAttributes (const GABLE_PPU* p_PPU, Uint8 p_MapIndex, Uint8 p_X, Uint8 p_Y)
{
    
    GABLE_expect(p_PPU, "PPU context is NULL!");

    // Ensure that the PPU is not in the pixel transfer state.
    if (p_PPU->m_STAT.m_DisplayMode == GABLE_DM_PIXEL_TRANSFER)
    {
        return (GABLE_TileAttributes) { .m_Value = 0x00 };
    }

    // Validate the X and Y coordinates. Correct if out of bounds.
    p_X %= 32;
    p_Y %= 32;

    // Using bit 0 of the `p_MapIndex` parameter, determine the start address of the attribute map
    // which is to be accessed.
    Uint16 l_StartAddress = (GABLE_bit(p_MapIndex, 0) != 0) ? 0x1C00 : 0x1800;

    // Calculate the attribute map address.
    Uint16 l_AttributeMapAddress = l_StartAddress + (p_Y * 32) + p_X;

    // Read the attribute byte from the attribute map.
    Uint8 l_AttributeByte = p_PPU->m_VRAM1[l_AttributeMapAddress];
}

GABLE_TileAttributes GABLE_GetBackgroundTileAttributes (const GABLE_PPU* p_PPU, Uint8 p_X, Uint8 p_Y)
{
    return GABLE_GetTileAttributes(p_PPU, p_PPU->m_LCDC.m_BGTilemapAddress, p_X, p_Y);
}

GABLE_TileAttributes GABLE_GetWindowTileAttributes (const GABLE_PPU* p_PPU, Uint8 p_X, Uint8 p_Y)
{
    return GABLE_GetTileAttributes(p_PPU, p_PPU->m_LCDC.m_WindowTilemapAddress, p_X, p_Y);
}

void GABLE_SetTileIndex (GABLE_PPU* p_PPU, Uint8 p_MapIndex, Uint8 p_X, Uint8 p_Y, Uint8 p_Tile)
{
    
    GABLE_expect(p_PPU, "PPU context is NULL!");

    // Ensure that the PPU is not in the pixel transfer state.
    if (p_PPU->m_STAT.m_DisplayMode == GABLE_DM_PIXEL_TRANSFER)
    {
        return;
    }

    // Validate the X and Y coordinates. Correct if out of bounds.
    p_X %= 32;
    p_Y %= 32;

    // Using bit 0 of the `p_MapIndex` parameter, determine the start address of the tilemap
    // which is to be accessed.
    Uint16 l_StartAddress = (GABLE_bit(p_MapIndex, 0) != 0) ? 0x1C00 : 0x1800;

    // Calculate the tilemap address.
    Uint16 l_TilemapAddress = l_StartAddress + (p_Y * 32) + p_X;

    // Write the tile index to the tilemap.
    p_PPU->m_VRAM0[l_TilemapAddress] = p_Tile;
}

void GABLE_SetBackgroundTileIndex (GABLE_PPU* p_PPU, Uint8 p_X, Uint8 p_Y, Uint8 p_Tile)
{
    GABLE_SetTileIndex(p_PPU, p_PPU->m_LCDC.m_BGTilemapAddress, p_X, p_Y, p_Tile);
}

void GABLE_SetWindowTileIndex (GABLE_PPU* p_PPU, Uint8 p_X, Uint8 p_Y, Uint8 p_Tile)
{
    GABLE_SetTileIndex(p_PPU, p_PPU->m_LCDC.m_WindowTilemapAddress, p_X, p_Y, p_Tile);
}

void GABLE_SetTileAttributes (GABLE_PPU* p_PPU, Uint8 p_MapIndex, Uint8 p_X, Uint8 p_Y, GABLE_TileAttributes p_Attributes)
{
    
    GABLE_expect(p_PPU, "PPU context is NULL!");

    // Ensure that the PPU is not in the pixel transfer state.
    if (p_PPU->m_STAT.m_DisplayMode == GABLE_DM_PIXEL_TRANSFER)
    {
        return;
    }

    // Validate the X and Y coordinates. Correct if out of bounds.
    p_X %= 32;
    p_Y %= 32;

    // Using bit 0 of the `p_MapIndex` parameter, determine the start address of the attribute map
    // which is to be accessed.
    Uint16 l_StartAddress = (GABLE_bit(p_MapIndex, 0) != 0) ? 0x1C00 : 0x1800;

    // Calculate the attribute map address.
    Uint16 l_AttributeMapAddress = l_StartAddress + (p_Y * 32) + p_X;

    // Write the attribute byte to the attribute map.
    p_PPU->m_VRAM1[l_AttributeMapAddress] = p_Attributes.m_Value;
}

void GABLE_SetBackgroundTileAttributes (GABLE_PPU* p_PPU, Uint8 p_X, Uint8 p_Y, GABLE_TileAttributes p_Attributes)
{
    GABLE_SetTileAttributes(p_PPU, p_PPU->m_LCDC.m_BGTilemapAddress, p_X, p_Y, p_Attributes);
}

void GABLE_SetWindowTileAttributes (GABLE_PPU* p_PPU, Uint8 p_X, Uint8 p_Y, GABLE_TileAttributes p_Attributes)
{
    GABLE_SetTileAttributes(p_PPU, p_PPU->m_LCDC.m_WindowTilemapAddress, p_X, p_Y, p_Attributes);
}

// Public Functions - Hardware Register Getters ////////////////////////////////////////////////////

Uint8 GABLE_ReadLCDC (const GABLE_PPU* p_PPU)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    return p_PPU->m_LCDC.m_Register;
}

Uint8 GABLE_ReadSTAT (const GABLE_PPU* p_PPU)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    return p_PPU->m_STAT.m_Register;
}

Uint8 GABLE_ReadSCY (const GABLE_PPU* p_PPU)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    return p_PPU->m_SCY;
}

Uint8 GABLE_ReadSCX (const GABLE_PPU* p_PPU)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    return p_PPU->m_SCX;
}

Uint8 GABLE_ReadLY (const GABLE_PPU* p_PPU)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    return p_PPU->m_LY;
}

Uint8 GABLE_ReadLYC (const GABLE_PPU* p_PPU)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    return p_PPU->m_LYC;
}

Uint8 GABLE_ReadDMA (const GABLE_PPU* p_PPU)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    return p_PPU->m_DMA;
}

Uint8 GABLE_ReadBGP (const GABLE_PPU* p_PPU)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    return p_PPU->m_BGP;
}

Uint8 GABLE_ReadOBP0 (const GABLE_PPU* p_PPU)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    return p_PPU->m_OBP0;
}

Uint8 GABLE_ReadOBP1 (const GABLE_PPU* p_PPU)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    return p_PPU->m_OBP1;
}

Uint8 GABLE_ReadWY (const GABLE_PPU* p_PPU)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    return p_PPU->m_WY;
}

Uint8 GABLE_ReadWX (const GABLE_PPU* p_PPU)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    return p_PPU->m_WX;
}

Uint8 GABLE_ReadVBK (const GABLE_PPU* p_PPU)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    return p_PPU->m_VBK;
}

Uint8 GABLE_ReadHDMA5 (const GABLE_PPU* p_PPU)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    return p_PPU->m_HDMA5.m_Register;
}

Uint8 GABLE_ReadBGPI (const GABLE_PPU* p_PPU)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    return p_PPU->m_BGPI.m_Register;
}

Uint8 GABLE_ReadBGPD (const GABLE_PPU* p_PPU)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    
    // Palette data cannot be read from the BGPD register if the PPU is in the pixel transfer state.
    if (p_PPU->m_STAT.m_DisplayMode == GABLE_DM_PIXEL_TRANSFER)
    {
        return 0xFF;
    }

    return p_PPU->m_BgCRAM[p_PPU->m_BGPI.m_ByteIndex];
}

Uint8 GABLE_ReadOBPI (const GABLE_PPU* p_PPU)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    return p_PPU->m_OBPI.m_Register;
}

Uint8 GABLE_ReadOBPD (const GABLE_PPU* p_PPU)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    
    // Palette data cannot be read from the OBPD register if the PPU is in the pixel transfer state.
    if (p_PPU->m_STAT.m_DisplayMode == GABLE_DM_PIXEL_TRANSFER)
    {
        return 0xFF;
    }

    return p_PPU->m_ObjCRAM[p_PPU->m_OBPI.m_ByteIndex];
}

Uint8 GABLE_ReadOPRI (const GABLE_PPU* p_PPU)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    return p_PPU->m_OPRI;
}

Uint8 GABLE_ReadGRPM (const GABLE_PPU* p_PPU)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    return p_PPU->m_GRPM;
}

// Public Functions - Hardware Register Setters ////////////////////////////////////////////////////

void GABLE_WriteLCDC (GABLE_PPU* p_PPU, Uint8 p_Value)
{
    
    GABLE_expect(p_PPU, "PPU context is NULL!");
    
    // If LCDC bit 7 is currently on, and the new value turns it off, then do not turn it off if the
    // PPU is not in vertical blank mode.
    if (
        p_PPU->m_LCDC.m_DisplayEnable == true &&
        GABLE_bit(p_Value, 7) == 0 &&
        p_PPU->m_STAT.m_DisplayMode != GABLE_DM_VERTICAL_BLANK
    )
    {
        GABLE_clearbit(p_Value, 7);
    }

    // Update the LCDC register.
    p_PPU->m_LCDC.m_Register |= p_Value;
}

void GABLE_WriteSTAT (GABLE_PPU* p_PPU, Uint8 p_Value)
{
    
    GABLE_expect(p_PPU, "PPU context is NULL!"); 

    // Update the STAT register. The lower 3 bits are read-only.
    p_PPU->m_STAT.m_Register = (p_Value & 0xF8) | (p_PPU->m_STAT.m_Register & 0x07);

}

void GABLE_WriteSCY (GABLE_PPU* p_PPU, Uint8 p_Value)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    p_PPU->m_SCY = p_Value;
}

void GABLE_WriteSCX (GABLE_PPU* p_PPU, Uint8 p_Value)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    p_PPU->m_SCX = p_Value;
}

// `LY` is read-only and cannot be written to.

void GABLE_WriteLYC (GABLE_PPU* p_PPU, Uint8 p_Value)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    p_PPU->m_LYC = p_Value;
}

void GABLE_WriteDMA (GABLE_PPU* p_PPU, Uint8 p_Value)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    p_PPU->m_DMA = p_Value;

    // Writing to the DMA register initiates an OAM DMA transfer.
    p_PPU->m_ODMADelay = 2;
    p_PPU->m_ODMADestination = GABLE_GB_OAM_START;
    p_PPU->m_ODMATicks = 0;

    // The source address starts at $XX00, where `XX` is the value written to the DMA register.
    p_PPU->m_ODMASource = (p_Value << 8);
}

void GABLE_WriteBGP (GABLE_PPU* p_PPU, Uint8 p_Value)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    p_PPU->m_BGP = p_Value;
}

void GABLE_WriteOBP0 (GABLE_PPU* p_PPU, Uint8 p_Value)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    p_PPU->m_OBP0 = p_Value;
}

void GABLE_WriteOBP1 (GABLE_PPU* p_PPU, Uint8 p_Value)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    p_PPU->m_OBP1 = p_Value;
}

void GABLE_WriteWY (GABLE_PPU* p_PPU, Uint8 p_Value)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    p_PPU->m_WY = p_Value;
}

void GABLE_WriteWX (GABLE_PPU* p_PPU, Uint8 p_Value)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    p_PPU->m_WX = p_Value;
}

void GABLE_WriteVBK (GABLE_PPU* p_PPU, Uint8 p_Value)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    p_PPU->m_VBK = p_Value;
    
    if (GABLE_bit(p_Value, 0) == 0)
    {
        p_PPU->m_VRAM = p_PPU->m_VRAM0;
    }
    else
    {
        p_PPU->m_VRAM = p_PPU->m_VRAM1;
    }
}

void GABLE_WriteHDMA1 (GABLE_PPU* p_PPU, Uint8 p_Value)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    p_PPU->m_HDMA1 = p_Value;
}

void GABLE_WriteHDMA2 (GABLE_PPU* p_PPU, Uint8 p_Value)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    p_PPU->m_HDMA2 = p_Value;
}

void GABLE_WriteHDMA3 (GABLE_PPU* p_PPU, Uint8 p_Value)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    p_PPU->m_HDMA3 = p_Value;
}

void GABLE_WriteHDMA4 (GABLE_PPU* p_PPU, Uint8 p_Value)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    p_PPU->m_HDMA4 = p_Value;
}

void GABLE_WriteHDMA5 (GABLE_PPU* p_PPU, GABLE_Engine* p_Engine, Uint8 p_Value)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    p_PPU->m_HDMA5.m_Register = p_Value;

    // Writing to the HDMA5 register initiates an HDMA transfer.
    p_PPU->m_HDMASource = (p_PPU->m_HDMA1 << 8) | (p_PPU->m_HDMA2 & 0xF0);
    p_PPU->m_HDMADestination = (p_PPU->m_HDMA3 << 8) | (p_PPU->m_HDMA4 & 0xF0);
    p_PPU->m_HDMABlocksLeft = (p_PPU->m_HDMA5.m_TransferLength + 1);
    
    if (p_PPU->m_HDMA5.m_TransferMode == 0)
    {
        // A GDMA transfer has been initiated. The transfer will begin immediately and will transfer
        // all blocks right now, at once.
        while (p_PPU->m_HDMABlocksLeft > 0)
        {
            GABLE_TickHDMA(p_PPU, p_Engine);
        }
    }

    // If the transfer mode is 1, then an HDMA transfer has been initiated. One block of data will
    // be transferred each H-Blank period.
}

void GABLE_WriteBGPI (GABLE_PPU* p_PPU, Uint8 p_Value)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    p_PPU->m_BGPI.m_Register = p_Value;
}

void GABLE_WriteBGPD (GABLE_PPU* p_PPU, Uint8 p_Value)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    
    // Palette data can only be written to the BGPD register if the PPU is not in the pixel transfer
    // state.
    if (p_PPU->m_STAT.m_DisplayMode != GABLE_DM_PIXEL_TRANSFER)
    {
        p_PPU->m_BgCRAM[p_PPU->m_BGPI.m_ByteIndex] = p_Value;
    }

    // Whether or not the write was successful, the byte index will always increment if auto-increment
    // is enabled.
    if (p_PPU->m_BGPI.m_AutoIncrement == true)
    {
        p_PPU->m_BGPI.m_ByteIndex = (p_PPU->m_BGPI.m_ByteIndex + 1) & 0x3F;
    }
}

void GABLE_WriteOBPI (GABLE_PPU* p_PPU, Uint8 p_Value)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    p_PPU->m_OBPI.m_Register = p_Value;
}

void GABLE_WriteOBPD (GABLE_PPU* p_PPU, Uint8 p_Value)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    
    // Palette data can only be written to the OBPD register if the PPU is not in the pixel transfer
    // state.
    if (p_PPU->m_STAT.m_DisplayMode != GABLE_DM_PIXEL_TRANSFER)
    {
        p_PPU->m_ObjCRAM[p_PPU->m_OBPI.m_ByteIndex] = p_Value;
    }

    // Whether or not the write was successful, the byte index will always increment if auto-increment
    // is enabled.
    if (p_PPU->m_OBPI.m_AutoIncrement == true)
    {
        p_PPU->m_OBPI.m_ByteIndex = (p_PPU->m_OBPI.m_ByteIndex + 1) & 0x3F;
    }
}

void GABLE_WriteOPRI (GABLE_PPU* p_PPU, Uint8 p_Value)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    p_PPU->m_OPRI = p_Value;
}

void GABLE_WriteGRPM (GABLE_PPU* p_PPU, Uint8 p_Value)
{
    GABLE_expect(p_PPU, "PPU context is NULL!");
    p_PPU->m_GRPM = p_Value;
}
