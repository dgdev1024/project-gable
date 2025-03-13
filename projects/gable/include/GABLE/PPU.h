/**
 * @file     GABLE/PPU.h
 * @brief    The GABLE Engine's PPU structure and functions.
 * 
 * The GABLE Engine's PPU component seeks to simulate the Game Boy's internal pixel-processing unit
 * (PPU) hardware. The PPU is responsible for rendering graphics consisting of background and window
 * tilemaps and free-roaming objects to a 160x144 pixel screen buffer. The PPU also manages buffers
 * for storing tile, tilemap and tile attribute data, a buffer for storing object attribute data and
 * buffers for storing color palette data.
 * 
 * The PPU is responsible for the following memory buffers:
 * 
 * - The Screen Buffer: A 160x144 pixel buffer for storing the rendered graphics output. Each pixel
 *   is represented by a 32-bit unsigned integer, in RGBA format (`0xRRGGBBAA`).
 * - The Video Memory (`VRAM`): A 16 KB buffer for storing tile data, tilemap data and tile
 *   attribute data. The buffer is split into two 8 KB banks, which can be swapped in and out of
 *   memory by writing to the `VBK` hardware register. The buffer is accessible from addresses
 *   `0x8000` to `0x9FFF` in the engine's memory map. Each VRAM bank is partitioned as follows:
 *       - Relative address `0x0000` to `0x17FF`: Tile data buffer - contains data for 384 8x8 pixel
 *         tiles, each of which is represented by 16 bytes of data. The means by which the PPU
 *         addresses this buffer for fetching tiles for the background and window layers is
 *         determined by bit 4 of the display control (`LCDC`) register.
 *       - In VRAM bank 0, relative address `0x1800` to `0x1FFF`: Tilemap data buffer - contains
 *         data for two 32x32-byte tilemaps, used to index tiles in the tile data buffer to be
 *         rendered to the background and window layers. The region of the buffer used for selecting
 *         tiles for the background and window layers are determined by bits 3 and 6 of the `LCDC`
 *         register, respectively.
 *       - In VRAM bank 1, relative address `0x1800` to `0x1FFF`: Tile attribute data buffer -
 *         contains data for two 32x32-byte tile attribute maps, representing the attributes of the
 *         respective tile indices in the tilemap data buffer. This bank is not accessible if the
 *         `GRPM` register is set to 0, indicating that the PPU is in DMG graphics mode.
 * - The Object Attribute Memory (`OAM`): A 160-byte buffer for storing object attribute data. Each
 *   object attribute occupies 4 bytes of data, and the buffer is accessible from addresses `0xFE00`
 *   to `0xFE9F` in the engine's memory map. The PPU uses this buffer to determine which objects are
 *   visible on the current scanline. The buffer is divided into 40 4-byte object attribute entries,
 *   with each byte therein representing the following:
 *       - Byte 0: The object's Y-coordinate on the screen.
 *       - Byte 1: The object's X-coordinate on the screen, minus 8 pixels.
 *       - Byte 2: The object's tile index in the tile data buffer.
 *       - Byte 3: The object's attributes. More on tile attributes later.
 * - The Color RAM (`CRAM`) buffers: Two 64-byte buffers for storing color palette data, one for the
 *   background layer and one for the object layer. The buffers each store eight distinct color
 *   palettes, with each palette containing four colors. Each color is represented two bytes of data,
 *   arranged in bitwise little-endian format as follows: `0bRRRRRGGG`, `0bGGBBBBB0`. The buffers are
 *   accessible by using the palette specification registers `BGPI` and `OBPI` to select an index
 *   within the buffer, and the palette data registers `BGPD` and `OBPD` to access the color data
 *   at that index in the CRAM buffer. These buffers are not accessible if the `GRPM` register is set
 *   to 0, indicating that the PPU is in DMG graphics mode.
 * 
 * The PPU component comes equipped with the following hardware registers:
 * 
 * - `LCDC` (Display Control): The display control register is used to control the PPU's display
 *   settings. The bits of this register control the following settings:
 *       - Bit 0 - BG/Window Display or Master Priority: This setting has different effects depending
 *         on the PPU's graphics mode (set in the `GRPM` register):
 *             - In DMG graphics mode (GRPM = 0), this bit controls whether the background and window
 *               layers are displayed at all.
 *             - In CGB graphics mode (GRPM = 1), this bit controls whether the background and window
 *               layers may have priority over the object layer.
 *       - Bit 1 - Object Display: Set to display objects on the screen; clear otherwise.
 *       - Bit 2 - Object Size: Set to use 8x16 pixel objects; clear to use 8x8 pixel objects.
 *       - Bit 3 - Background Tilemap Address: If set, the PPU will select tiles from the second
 *         tilemap in VRAM (relative address `0x1C00` to `0x1FFF`) for rendering the background layer;
 *         otherwise, it will select tiles from the first tilemap in VRAM (relative address `0x1800`
 *         to `0x1BFF`).
 *       - Bit 4 - Background/Window Tile Data Address: If set, the PPU map tile indices 0-127 to
 *         tiles in block 0 of the tile data area in the current VRAM bank (relative address `0x0000`
 *         to `0x07FF`); otherwise, it maps tile indices 0-127 to tiles in block 2 of the tile data
 *         area in the current VRAM bank (relative address `0x1000` to `0x17FF`). Indices 128-255
 *         are always mapped to block 1 (relative address `0x0800` to `0x0FFF`). The object layer
 *         always uses block 0 and 1.
 *       - Bit 5 - Window Display: Set to display the window layer; clear otherwise.
 *       - Bit 6 - Window Tilemap Address: If set, the PPU will select tiles from the second tilemap
 *         in VRAM (relative address `0x1C00` to `0x1FFF`) for rendering the window layer; otherwise,
 *         it will select tiles from the first tilemap in VRAM (relative address `0x1800` to `0x1BFF`).
 *       - Bit 7 - Display Enable: Set to enable the PPU; clear to disable the PPU.
 *             - If the PPU is disabled, the screen buffer is cleared to white.
 *             - If the PPU is enabled, it cannot be disabled outside of VBLANK mode.
 * - `STAT` (Display Status): The display status register is used to indicate the PPU's current
 *   display mode, and to control the request of the `LCD_STAT` interrupt. The bits of this register
 *   control the following settings:
 *       - Bits 0-1 - Display Mode: Indicates the PPU's current display mode. More on display modes
 *         later. Read-only.
 *       - Bit 2 - Line Coincidence: Set if the current scanline (`LY`) matches the value in the
 *         line compare (`LYC`) register; clear otherwise. Read-only.
 *       - Bit 3 - Mode 0 Interrupt Request: Set to request the `LCD_STAT` interrupt when the PPU
 *         enters Mode 0 (`HBLANK`); clear otherwise.
 *       - Bit 4 - Mode 1 Interrupt Request: Set to request the `LCD_STAT` interrupt when the PPU
 *         enters Mode 1 (`VBLANK`); clear otherwise.
 *       - Bit 5 - Mode 2 Interrupt Request: Set to request the `LCD_STAT` interrupt when the PPU
 *         enters Mode 2 (`OAM_SCAN`); clear otherwise.
 *       - Bit 6 - Line Coincidence Interrupt Request: Set to request the `LCD_STAT` interrupt when
 *         the line coincidence bit is set; clear otherwise.
 * - `SCY` (Scroll Y): The scroll Y register is used to set the background scroll position on the
 *   screen. The value in this register is used to determine the Y-coordinate of the top-left corner
 *   of the visible portion of the background layer.
 * - `SCX` (Scroll X): The scroll X register is used to set the background scroll position on the
 *   screen. The value in this register is used to determine the X-coordinate of the top-left corner
 *   of the visible portion of the background layer.
 * - `LY` (LCDC Y-Coordinate, Current Scanline): The LCDC Y-coordinate register is used to store the
 *   current scanline being rendered by the PPU. The value in this register is incremented every time
 *   the PPU finishes rendering a scanline, and is reset to 0 when the PPU exits VBLANK mode. Read-only.
 * - `LYC` (LY Compare, Line Compare): The LY compare register is used to set the value against which
 *   the PPU compares the current scanline (`LY`) to determine if the line coincidence bit in the
 *   `STAT` register should be set.
 * - `DMA` (OAM DMA Start Address): The OAM DMA register written to in order to initiate an OAM DMA
 *   transfer. Writing a byte to this register triggers the transfer, copying data from an address
 *   in which the byte that was written to this register is the high byte, and the low byte starts
 *   from 0x00. More on direct-memory access processes later.
 * - `BGP` (Background Palette Data): The background palette data register is used to set the color
 *   palette for the background layer. This register is ignored if the `GRPM` register is set to 1,
 *   indicating that the PPU is in CGB graphics mode. The bits of this register control the following
 *   settings:
 *       - Bits 0-1 - Color 0: The color of palette index 0.
 *       - Bits 2-3 - Color 1: The color of palette index 1.
 *       - Bits 4-5 - Color 2: The color of palette index 2.
 *       - Bits 6-7 - Color 3: The color of palette index 3.
 * - `OBP0` and `OBP1` (Object Palette Data): The object palette data registers are used to set the
 *   color palettes for the object layer. These registers are ignored if the `GRPM` register is set to
 *   1, indicating that the PPU is in CGB graphics mode. The bits of these registers control the
 *   following settings:
 *       - Bits 0-1 - Color 0: The color of palette index 0.
 *       - Bits 2-3 - Color 1: The color of palette index 1.
 *       - Bits 4-5 - Color 2: The color of palette index 2.
 *       - Bits 6-7 - Color 3: The color of palette index 3.
 * - `WY` (Window Position Y): The window position Y register is used to set the Y-coordinate of the
 *   top-left corner of the visible portion of the window layer.
 * - `WX` (Window Position X): The window position X register is used to set the X-coordinate of the
 *   top-left corner of the visible portion of the window layer. The value in this register is offset
 *   by 7 pixels to the right when rendering the window layer.
 * - `VBK` (VRAM Bank Number): The VRAM bank number register is used to select the current VRAM bank
 *   to access. The value in this register is used to determine which VRAM bank is accessible from
 *   addresses `0x8000` to `0x9FFF`. The value in this register is masked with 0x01 to determine the
 *   bank number.
 * - `HDMA1` and `HDMA2` (GDMA / HDMA Source, High and Low Bytes): The GDMA / HDMA source registers
 *   are used to set the source address for a GDMA / HDMA transfer. The value in these registers is
 *   used to determine the source address from which data is copied to VRAM. The source address is
 *   calculated as follows: `0x0000` to `0x7FFF` is the source address, with the high byte being
 *   the value in `HDMA1` and the low byte being the value in `HDMA2`. Write-only.
 * - `HDMA3` and `HDMA4` (GDMA / HDMA Destination, High and Low Bytes): The GDMA / HDMA destination
 *   registers are used to set the destination address for a GDMA / HDMA transfer. The value in these
 *   registers is used to determine the destination address to which data is copied from VRAM. The
 *   destination address is calculated as follows: `0x8000` to `0x9FFF` is the destination address,
 *   with the high byte being the value in `HDMA3` and the low byte being the value in `HDMA4`.
 *   Write-only.
 * - `HDMA5` (GDMA / HDMA Transfer, Mode, Start): The GDMA / HDMA transfer register is used to start
 *   a GDMA / HDMA transfer. More on direct-memory access processes later. The bits of this register 
 *   control the following settings:
 *       - Bits 0-6 - Transfer Length: The number of 16-byte blocks to transfer from the source to
 *         the destination. The value in this register is incremented by 1 before each transfer.
 *       - Bit 7 - Transfer Mode: Set to enable HDMA mode; clear to enable GDMA mode.
 *       - Writing to this register triggers the transfer.
 * - `BGPI` and `OBPI` (Background and Object Palette Specification): The background and object
 *   palette specification registers are used to set the index of the color in the background and
 *   object color RAM buffers to access, as well as whether or not the index is auto-incremented
 *   with each attempted access. These registers are ignored if the `GRPM` register is set to 0,
 *   indicating that the PPU is in DMG graphics mode. The bits of these registers control the
 *   following settings:
 *      - Bits 0-5 - Palette Index: The index of the color in the color RAM buffer to access.
 *      - Bit 7 - Auto-Increment: Set to increment the palette index with each attempted access;
 *        clear otherwise.
 * - `BGPD` and `OBPD` (Background and Object Palette Data): The background and object palette data
 *   registers are used to access a byte in the background and object color RAM buffers. The index of
 *   the byte accessed is determined by the index setting in the `BGPI` and `OBPI` registers, respectively.
 *   The value in these registers is masked with 0x3F to determine the index of the byte to access.
 *   These registers are ignored if the `GRPM` register is set to 0, indicating that the PPU is in DMG
 *   graphics mode.
 * - `OPRI` (Object Priority): The object priority register is used to set the priority of the object
 *   layer. If zero, then an object's priority is determined by its X position (the smaller the X
 *   position, the higher the priority); otherwise, its priority is determined by its index in the
 *   OAM buffer (the lower the index, the higher the priority). If the `GRPM` register is set to 0,
 *   indicating that the PPU is in DMG graphics mode, this register is ignored, with the former
 *   priority setting being used.
 * - `GRPM` (PPU Graphics Mode): The PPU graphics mode register is used to set the PPU's graphics mode.
 *   Setting this register to 0 indicates that the PPU is in DMG graphics mode, while setting it to
 *   any non-zero value indicates that the PPU is in CGB graphics mode.
 * 
 * The PPU component is responsible for the following hardware interrupts:
 * 
 * - `VBLANK` (Vertical Blank Interrupt): The PPU requests this interrupt when its display mode
 *   changes to Mode 1 (`VBLANK`). This interrupt is typically used to update the game's engine state
 *   and game logic.
 * - `LCD_STAT` (LCD Status Interrupt): The PPU requests this interrupt when certain conditions are
 *   met, as determined by the `STAT` register. This interrupt is typically used to implement
 *   real-time graphics effects and special rendering techniques.
 * 
 * The process of the PPU rendering graphics to the screen buffer is carried out over a number of
 * engine cycles (a rate of 4 MHz), a unit of time which will be referred to in the PPU as "dots".
 * The process of the PPU rendering graphics to the screen buffer is carried out over four distinct
 * display modes, each of which is responsible for a different aspect of the rendering process. The
 * modes are listed as follows, in order of execution:
 * 
 * - Mode 2: Object Scan (`OAM_SCAN`): The PPU scans the Object Attribute Memory (OAM) buffer to
 *   determine which objects are visible on the current scanline. This mode lasts for 80 dots, with
 *   each object taking 2 dots to process. Up to ten objects may be visible on a single scanline.
 * - Mode 3: Pixel Transfer (`PIXEL_TRANSFER`): The PPU transfers pixel data from the tile data
 *   buffers to the screen buffer. This mode lasts anywhere from 172 to 289 dots, depending on the
 *   number of visible objects on the current scanline. The actual duration is determined by the
 *   following factors:
 *       - The PPU outputs 1 pixel to the screen buffer every dot. Since a visible scanline is 160
 *         pixels wide, this accounts for 160 dots of the mode's minimum duration.
 *       - At the start of the mode, the PPU performs two tile fetches, one of which is for the
 *         first tile on the scanline, and the other of which is discarded. These fetches account
 *         for the other 12 dots of the mode's minimum duration.
 *       - At the start of the mode, a number of dots equal to `SCX` % 8 are spent idling, accounting 
 *         for the number of pixels on the leftmost tile which are not visible. This accounts adds 
 *         up to 7 dots of the mode's duration.
 *       - After the last non-window pixel is output before encountering the visible portion of the
 *         window layer, the PPU spends 6 additional dots in this mode preparing the pixel fetcher 
 *         to render the visible portion of the window layer.
 *       - An additional 6 to 11 dots are spent for each object drawn on the scanline - even partial
 *         objects. This accounts for the remaining dots of the mode's duration. The process of
 *         determining the number of dots spent on each object is as follows:
 *             - For this process, only the leftmost pixel of the object is accounted for; we'll
 *               call this "The Pixel".
 *             - Determine if The Pixel is within a background layer tile or a window layer tile.
 *               This is affected by the background scroll and window position registers.
 *             - If The Pixel is not already occupied by another object:
 *                - Incur a dot-penalty equal to the number of pixels strictly to the right of The
 *                  Pixel in its tile, minus 2. Do not incur a dot-penalty if the result is negative.
 *             - Spend the minimum of 6 dots on fetching the object's tile data.
 * - Mode 0: Horizontal Blank (`HBLANK`): At this point, the PPU has finished rendering the current
 *   scanline. The PPU enters this mode to idle for the remaining dots of the scanline. This mode
 *   lasts for the remaining dots of the scanline after the end of Mode 3. The total duration of
 *   rendering a scanline is 456 dots. Once this mode is complete:
 *       - If the scanline just rendered was the last visible scanline of the frame, the PPU enters
 *         the Vertical Blank mode.
 *       - Otherwise, the PPU advances to the next scanline and enters Mode 2.
 * - Mode 1: Vertical Blank (`VBLANK`): The PPU has finished rendering the last visible scanline of
 *   the frame. The PPU enters this mode to idle for the remaining dots of the frame (ten scanlines'
 *   worth of dots, 4,560 dots). During this mode, the entirety of VRAM, OAM and CRAM are accessible
 *   to the engine. Once this mode is complete, the PPU resets to the first scanline of the frame and
 *   enters Mode 2. The entire process of rendering a frame takes 456 dots per scanline * 144 visible
 *   scanlines, plus 4,560 dots of VBLANK, for a total of 70,224 dots.
 * 
 * The PPU component comes equipped with a pixel-fetcher unit, which is responsible for fetching
 * pixel data from the tile data buffers and preparing them for rendering to the screen buffer. The
 * pixel-fetcher unit places these pixels into one of two first-in-first-out (FIFO) buffers: one for
 * the background/window layer and one for the object (sprite) layer. The pixel-fetcher unit operates
 * in the following steps:
 * 
 * 1. **Get Tile Number**:
 *    - The PPU fetches the tile number from the background or window tile map. The tile number is used
 *      to identify which tile's pixel data to fetch next. The tile map is a 32x32 grid of tile numbers,
 *      each representing an 8x8 pixel tile.
 * 
 * 2. **Get Tile Data Low**:
 *    - The PPU fetches the low byte of the tile data for the current tile and line. Each tile is 8x8
 *      pixels, and each line of the tile is represented by two bytes (low and high). The low byte contains
 *      the least significant bits (LSBs) of the color indices for the pixels in the current line.
 * 
 * 3. **Get Tile Data High**:
 *    - The PPU fetches the high byte of the tile data for the current tile and line. The high byte contains
 *      the most significant bits (MSBs) of the color indices for the pixels in the current line.
 * 
 * 4. **Push Pixels**:
 *    - The PPU combines the low and high bytes of the tile data to form the final color indices for the pixels.
 *      These indices are then pushed into the pixel FIFO (First-In-First-Out) buffer. The pixel FIFO holds the
 *      pixel data temporarily before it is rendered to the screen. There are two FIFO buffers: one for the
 *      background/window layer and one for the object (sprite) layer.
 * 
 * 5. **Sleep**:
 *    - The PPU enters a sleep state for a few cycles to allow the pixel FIFO to process the fetched pixels.
 *      This step ensures that the pixel FIFO has enough time to push the pixels to the screen buffer, preventing
 *      any delays or glitches in the rendering process.
 * 
 * The pixel-fetcher unit operates in synchronization with the PPU's rendering cycles, ensuring that the pixel
 * data is fetched and processed in time for rendering. Each step in the pixel fetching process takes a specific
 * number of cycles to complete, resulting in smooth and accurate graphics on the Game Boy's screen:
 *    - Get Tile Number: 2 cycles
 *    - Get Tile Data Low: 2 cycles
 *    - Get Tile Data High: 2 cycles
 *    - Push Pixels: 2 cycles
 *    - Sleep: 2 cycles
 * 
 * The PPU component also manages a number of direct-memory access (DMA) processes, which are used to transfer
 * data from one memory location to another without involving the parent engine. The PPU's DMA processes 
 * are as follows:
 * 
 * - OAM DMA Transfer: The OAM DMA transfer is used to copy data from the engine's memory to the Object
 *   Attribute Memory (OAM) buffer. This process is initiated by writing a byte to the `DMA` register. This
 *   byte is the high byte of the source address, with the low byte starting at 0x00. The transfer copies
 *   160 bytes of data from the source address to the OAM buffer, starting at address 0xFE00. The transfer
 *   takes 160 machine cycles to complete. The OAM DMA transfer is used to update the OAM buffer with new
 *   object attribute data without having to wait for a `VBLANK` period.
 * 
 * - General-Purpose DMA (GDMA): The GDMA process is used to instantly copy data from a memory location
 *   into the Video RAM (VRAM) buffer. This process is initiated by first writing the source address to the
 *   `HDMA1` and `HDMA2` registers, the destination address to the `HDMA3` and `HDMA4` registers, the
 *   transfer length to bits 0-6 of the `HDMA5` register, and 0 to the transfer mode bit (bit 7) of the
 *   `HDMA5` register. Writing to the `HDMA5` register triggers the transfer. The GDMA process copies data
 *   from the source address to the destination address all at once, in a blocking manner. The GDMA process
 *   is used to quickly transfer large amounts of data to the VRAM buffer, such as tile data or tilemap data
 *   without having to wait for a `VBLANK` period.
 * 
 * - Horizontal Blank DMA (HDMA): The HDMA process is used to copy data from a memory location into the VRAM
 *   buffer during the horizontal blank period (`HBLANK`). As with the GDMA process, the HDMA process is
 *   prepared by writing the source and destination addresses to `HDMA1` through `HDMA4`, the transfer
 *   length to bits 0-6 of the `HDMA5` register, and 1 to the transfer mode bit (bit 7) of the `HDMA5` register.
 *   Writing to the `HDMA5` register triggers the transfer. The HDMA process copies data from the source
 *   address to the destination address in blocks of 16 bytes, with the transfer length determining the number
 *   of blocks to transfer (with 0 = 1 block, up to 0x7F blocks for a maximum of 0x7F * 16 = 2,048 bytes).
 *   The HDMA process transfers one block of data at the start of each `HBLANK` period, allowing for a
 *   non-blocking transfer of data during the frame rendering process.
 */

#pragma once
#include <GABLE/DataStore.h>

// Constants ///////////////////////////////////////////////////////////////////////////////////////

/** @brief The width of the screen buffer, in pixels. */
#define GABLE_PPU_SCREEN_WIDTH 160

/** @brief The height of the screen buffer, in pixels. */
#define GABLE_PPU_SCREEN_HEIGHT 144

/** @brief The number of pixels in the screen buffer. */
#define GABLE_PPU_SCREEN_BUFFER_SIZE (GABLE_PPU_SCREEN_WIDTH * GABLE_PPU_SCREEN_HEIGHT)

/** @brief The size of a bank of video RAM (VRAM), in bytes. */
#define GABLE_PPU_VRAM_BANK_SIZE 0x2000

/** @brief The size of the tile data bank partition in the video RAM (VRAM) buffers, in bytes. */
#define GABLE_PPU_VRAM_TILE_DATA_PARTITION_SIZE 0x1800

/**
 * @brief The size of one of the two 32x32-byte tilemap and tile attribute maps in the video RAM (VRAM)
 * buffers, in bytes.
 */
#define GABLE_PPU_VRAM_TILEMAP_SIZE 0x0400

/** 
 * @brief The size of the tilemap and tile attribute map partitions in the video RAM (VRAM), in 
 * bytes. 
 */
#define GABLE_PPU_VRAM_TILEMAP_PARTITION_SIZE (GABLE_PPU_VRAM_TILEMAP_SIZE * 2)

/** @brief The number of object entries in the object attribute memory (OAM). */
#define GABLE_PPU_OAM_OBJECT_COUNT 40

/** @brief The size of the object attribute memory (OAM) buffer, in bytes. */
#define GABLE_PPU_OAM_SIZE (GABLE_PPU_OAM_OBJECT_COUNT * 4)

/** @brief The maximum number of objects visible on the current scanline. */
#define GABLE_PPU_OBJECTS_PER_SCANLINE 10

/** @brief The size of the color RAM (CRAM) buffers, in bytes. */
#define GABLE_PPU_CRAM_SIZE 64

/** @brief The number of color palettes in the color RAM (CRAM) buffers. */
#define GABLE_PPU_CRAM_PALETTE_COUNT 8

/** @brief The number of colors in a color palette in the color RAM (CRAM) buffers. */
#define GABLE_PPU_CRAM_PALETTE_COLOR_COUNT 4

/** @brief The number of bytes in a color in the color RAM (CRAM) buffers. */
#define GABLE_PPU_BYTES_PER_COLOR 2

/**
 * @brief The number of bytes in a color palette in the color RAM (CRAM) buffers. Each color is
 * represented by two bytes of data.
 */
#define GABLE_PPU_BYTES_PER_PALETTE (GABLE_PPU_BYTES_PER_COLOR * GABLE_PPU_CRAM_PALETTE_COLOR_COUNT)

/** @brief The total colors in a color RAM (CRAM) buffer. */
#define GABLE_PPU_CRAM_COLOR_COUNT (GABLE_PPU_CRAM_PALETTE_COUNT * GABLE_PPU_CRAM_PALETTE_COLOR_COUNT)

/** @brief The maximum size of the pixel fetcher's FIFO buffers. */
#define GABLE_PPU_PIXEL_FIFO_SIZE 32

/** @brief The number of lines (including `VBLANK` lines) in a frame. */
#define GABLE_LINES_PER_FRAME 154

/** @brief The number of visible scanlines in a frame. */
#define GABLE_VISIBLE_LINES 144

/** @brief The number of PPU cycles needed to process a scanline (including the `HBLANK` period). */
#define GABLE_DOTS_PER_LINE 456

/** @brief The number of PPU cycles needed to process a complete frame (including the `VBLANK` period). */
#define GABLE_DOTS_PER_FRAME 70224

/** @brief The number of tiles that can be stored in the tile data region of a VRAM bank. */
#define GABLE_PPU_VRAM_TILE_COUNT 384

// Typedefs and Forward Declarations ///////////////////////////////////////////////////////////////

/** @brief A forward-declaration of the GABLE Engine structure. */
typedef struct GABLE_Engine GABLE_Engine;

/** @brief A forward-declaration of the GABLE Engine's pixel-processing unit (PPU) component. */
typedef struct GABLE_PPU GABLE_PPU;

/** @brief A pointer to a callback function that is called when a frame is rendered by the PPU. */
typedef void (*GABLE_FrameRenderedCallback) (GABLE_Engine*, GABLE_PPU*);

// Display Mode Enumeration ////////////////////////////////////////////////////////////////////////

/**
 * @brief An enumeration representing the PPU's display modes.
 */
typedef enum GABLE_DisplayMode
{
    GABLE_DM_HORIZONTAL_BLANK = 0,  ///< @brief The PPU is in horizontal blank mode (Mode 0, `HBLANK`).
    GABLE_DM_VERTICAL_BLANK = 1,    ///< @brief The PPU is in vertical blank mode (Mode 1, `VBLANK`).
    GABLE_DM_OBJECT_SCAN = 2,       ///< @brief The PPU is scanning for objects on the current scanline (Mode 2, `OAM_SCAN`).
    GABLE_DM_PIXEL_TRANSFER = 3     ///< @brief The PPU is transferring pixel data to the screen buffer (Mode 3, `PIXEL_TRANSFER`).
} GABLE_DisplayMode;

// `LCD_STAT` Source Enumeration ///////////////////////////////////////////////////////////////////

/**
 * @brief An enumeration representing the sources from which the `LCD_STAT` interrupt may be requested.
 */
typedef enum GABLE_DisplayStatusSource
{
    GABLE_DSS_HORIZONTAL_BLANK = 1,  ///< @brief The `LCD_STAT` interrupt is requested when the PPU enters Mode 0 (`HBLANK`).
    GABLE_DSS_VERTICAL_BLANK,        ///< @brief The `LCD_STAT` interrupt is requested when the PPU enters Mode 1 (`VBLANK`).
    GABLE_DSS_OBJECT_SCAN,           ///< @brief The `LCD_STAT` interrupt is requested when the PPU enters Mode 2 (`OAM_SCAN`).
    GABLE_DSS_LINE_COINCIDENCE       ///< @brief The `LCD_STAT` interrupt is requested when the line coincidence bit changes from low to high.
} GABLE_DisplayStatusSource;

// Pixel Fetcher Mode Enumeration //////////////////////////////////////////////////////////////////

/**
 * @brief An enumeration representing the modes of the PPU's internal pixel-fetcher unit.
 */
typedef enum GABLE_PixelFetchMode
{
    GABLE_PFM_TILE_NUMBER,           ///< @brief The pixel-fetcher is fetching the tile number for the current pixel.
    GABLE_PFM_TILE_DATA_LOW,         ///< @brief The pixel-fetcher is fetching the low byte of the tile data for the current pixel.
    GABLE_PFM_TILE_DATA_HIGH,        ///< @brief The pixel-fetcher is fetching the high byte of the tile data for the current pixel.
    GABLE_PFM_PUSH_PIXELS,           ///< @brief The pixel-fetcher is pushing the pixel data to the pixel FIFO buffer.
    GABLE_PFM_SLEEP                  ///< @brief The pixel-fetcher is idling to allow the pixel FIFO to process the fetched pixels.
} GABLE_PixelFetchMode;

// Object Priority Mode Enumeration ////////////////////////////////////////////////////////////////

/**
 * @brief An enumeration representing the priority modes for objects in the object attribute memory (OAM).
 */
typedef enum GABLE_ObjectPriorityMode
{
    GABLE_OPM_OAM_INDEX = 0,            ///< @brief The PPU uses the object's index in the OAM buffer to determine its priority. Objects with a lower index in OAM have higher priority over other objects.
    GABLE_OPM_X_POSITION                ///< @brief The PPU uses the object's X position to determine its priority; objects with a smaller X position have higher priority over other objects, with ties broken by the object's index in OAM.
} GABLE_ObjectPriorityMode;

// Graphics Mode Enumeration ///////////////////////////////////////////////////////////////////////

/**
 * @brief An enumeration representing the PPU's graphics modes.
 */
typedef enum GABLE_GraphicsMode
{
    GABLE_GM_DMG = 0,  ///< @brief The PPU is in DMG graphics mode.
    GABLE_GM_CGB       ///< @brief The PPU is in CGB graphics mode.
} GABLE_GraphicsMode;

// Bits Per Pixel Enumeration //////////////////////////////////////////////////////////////////////

/**
 * @brief An enumeration representing the possible bit depths for tile data in the PPU's VRAM.
 */
typedef enum GABLE_BitsPerPixel
{
    GABLE_1BPP,         ///< @brief 1 bit per pixel (8 bytes per tile).
    GABLE_2BPP          ///< @brief 2 bits per pixel (16 bytes per tile).
} GABLE_BitsPerPixel;

// Preset Color Enumeration ////////////////////////////////////////////////////////////////////////

/**
 * @brief An enumeration representing the preset colors available to the PPU.
 */
typedef enum GABLE_Color
{
    GABLE_COLOR_BLACK = 0,      ///< @brief The color black.
    GABLE_COLOR_DARK_GRAY,      ///< @brief The color dark gray.
    GABLE_COLOR_GRAY,           ///< @brief The color gray.
    GABLE_COLOR_LIGHT_GRAY,     ///< @brief The color light gray.
    GABLE_COLOR_WHITE,          ///< @brief The color white.
    GABLE_COLOR_RED,            ///< @brief The color red.
    GABLE_COLOR_ORANGE,         ///< @brief The color orange.
    GABLE_COLOR_BROWN,          ///< @brief The color brown.
    GABLE_COLOR_YELLOW,         ///< @brief The color yellow.
    GABLE_COLOR_GREEN,          ///< @brief The color green.
    GABLE_COLOR_CYAN,           ///< @brief The color cyan.
    GABLE_COLOR_BLUE,           ///< @brief The color blue.
    GABLE_COLOR_PURPLE,         ///< @brief The color purple.
    GABLE_COLOR_MAGENTA,        ///< @brief The color magenta.
    GABLE_COLOR_PINK,           ///< @brief The color pink.
    GABLE_COLOR_GOLD,           ///< @brief The color gold.
    GABLE_COLOR_SILVER,         ///< @brief The color silver.
    GABLE_COLOR_BRONZE,         ///< @brief The color bronze.

    GABLE_COLOR_COUNT           ///< @brief The number of preset colors.
} GABLE_Color;

// Display Control Union ///////////////////////////////////////////////////////////////////////////

/**
 * @brief A union representing the PPU's `LCDC` display control register.
 */
typedef union GABLE_DisplayControl
{
    struct
    {
        /**
         * @brief This bit has different effects depending on the PPU's graphics mode (set in the `GRPM`
         * register):
         * 
         * - In DMG graphics mode (GRPM = 0), this bit controls whether the background and window layers
         *   are displayed at all.
         * 
         * - In CGB graphics mode (GRPM = 1), this bit controls whether the background and window layers
         *   may have priority over the object layer.
         */
        Uint8 m_BGWEnableOrPriority : 1;
        Uint8 m_ObjectEnable : 1;       ///< @brief Set to display objects on the screen; clear otherwise.
        Uint8 m_ObjectSize : 1;         ///< @brief Set to use 8x16 pixel objects ("tall objects"); clear to use 8x8 pixel objects.

        /**
         * @brief This bit controls which tilemap in VRAM the PPU selects tiles from for rendering
         *        the background layer:
         * 
         * - If set, the PPU will select tiles from the second tilemap in VRAM (relative address
         *   `0x1C00` to `0x1FFF`).
         * 
         * - Otherwise, it will select tiles from the first tilemap in VRAM (relative address `0x1800`
         *   to `0x1BFF`).
         */
        Uint8 m_BGTilemapAddress : 1;

        /**
         * @brief This bit controls which tile data area ("block") in VRAM the PPU maps tile indices
         *        0-127 to for rendering the background and window layers:
         * 
         * - If set, the PPU maps tile indices 0-127 to tiles in block 0 of the tile data area in the
         *   current VRAM bank (relative address `0x0000` to `0x07FF`).
         * 
         * - Otherwise, it maps tile indices 0-127 to tiles in block 2 of the tile data area in the
         *   current VRAM bank (relative address `0x1000` to `0x17FF`).
         * 
         * - Indices 128-255 are always mapped to block 1 (relative address `0x0800` to `0x0FFF`).
         * 
         * - The object layer always uses block 0 and 1.
         */
        Uint8 m_BGWindowTileDataAddress : 1;

        Uint8 m_WindowEnable : 1;       ///< @brief Set to display the window layer; clear otherwise.

        /**
         * @brief This bit controls which tilemap in VRAM the PPU selects tiles from for rendering
         *        the window layer:
         * 
         * - If set, the PPU will select tiles from the second tilemap in VRAM (relative address
         *   `0x1C00` to `0x1FFF`).
         * 
         * - Otherwise, it will select tiles from the first tilemap in VRAM (relative address `0x1800`
         *   to `0x1BFF`).
         */
        Uint8 m_WindowTilemapAddress : 1;

        /**
         * @brief This bit controls whether the PPU is enabled:
         * 
         * - If set, the PPU is enabled and may render graphics to the screen buffer.
         * 
         * - If clear, the PPU is disabled and the screen buffer is cleared to white.
         * 
         * @warning If the PPU is currently enabled, it cannot be disabled outside of VBLANK mode!
         */
        Uint8 m_DisplayEnable : 1;
    };

    Uint8 m_Register;  ///< @brief The raw register value.
} GABLE_DisplayControl;

// Display Status Union ////////////////////////////////////////////////////////////////////////////

/**
 * @brief A union representing the PPU's `STAT` display status register.
 */
typedef union GABLE_DisplayStatus
{
    struct
    {
        Uint8 m_DisplayMode : 2;  ///< @brief The PPU's current display mode. Read-only.

        /**
         * @brief This bit indicates whether the current scanline (`LY`) register equals the value
         *        in the line compare (`LYC`) register. Read-only.
         */
        Uint8 m_LineCoincidence : 1;

        /**
         * @brief Set to request the `LCD_STAT` interrupt when the PPU enters Mode 0 (`HBLANK`);
         *        clear otherwise.
         */
        Uint8 m_HorizontalBlankStatSource : 1;

        /**
         * @brief Set to request the `LCD_STAT` interrupt when the PPU enters Mode 1 (`VBLANK`);
         *        clear otherwise.
         */
        Uint8 m_VerticalBlankStatSource : 1;

        /**
         * @brief Set to request the `LCD_STAT` interrupt when the PPU enters Mode 2 (`OAM_SCAN`);
         *        clear otherwise.
         */
        Uint8 m_ObjectScanStatSource : 1;

        /**
         * @brief Set to request the `LCD_STAT` interrupt when the line coincidence bit changes from
         *        low to high; clear otherwise.
         */
        Uint8 m_LineCoincidenceStatSource : 1;

    };

    Uint8 m_Register;  ///< @brief The raw register value.
} GABLE_DisplayStatus;

// GDMA / HDMA Control Union ///////////////////////////////////////////////////////////////////////

/**
 * @brief A union representing the PPU's `HDMA5` GDMA / HDMA transfer control register.
 */
typedef union GABLE_HDMAControl
{
    struct
    {
        Uint8 m_TransferLength : 7;  ///< @brief The number of 16-byte blocks to transfer from the source to the destination.
        
        /**
         * @brief This bit determines how the transfer is carried out:
         * 
         * - Set to initiate a Horizontal Blank DMA (HDMA) transfer. In this mode, the PPU transfers
         *   one 16-byte block of data at the start of each `HBLANK` period.
         * 
         * - Clear to initiate a General DMA (GDMA) transfer. In this mode, the PPU transfers all
         *   data from the source to the destination at once, in a blocking manner.
         */
        Uint8 m_TransferMode : 1;
    };

    Uint8 m_Register;  ///< @brief The raw register value.
} GABLE_HDMAControl;

// Palette Specification Union /////////////////////////////////////////////////////////////////////

/**
 * @brief A union representing the PPU's `BGPI` and `OBPI` background and object palette specification
 * registers.
 */
typedef union GABLE_PaletteSpecification
{
    struct
    {
        Uint8 m_ByteIndex : 6;  ///< @brief The index of the color in the color RAM buffer to access.
        Uint8 : 1;              ///< @brief Unused bit.

        /**
         * @brief This bit controls whether the index of the color in the color RAM buffer is
         *        auto-incremented with each attempted access.
         * 
         * @note If this bit is set, the index is incremented even if the attempted access fails.
         */
        Uint8 m_AutoIncrement : 1;
    };

    Uint8 m_Register;  ///< @brief The raw register value.
} GABLE_PaletteSpecification;

// Tile Attributes Union ///////////////////////////////////////////////////////////////////////////

/**
 * @brief A union representing the attributes of a tile in the tile attribute map, and the 
 *        attributes of an object in the object attribute memory (OAM).
 */
typedef union GABLE_TileAttributes
{
    struct
    {
        Uint8 m_PaletteIndex : 3;   ///< @brief The index of the color palette to use (CGB only).
        Uint8 m_VRAMBank : 1;       ///< @brief The VRAM bank to use for the tile data (CGB only).
        Uint8 m_DMGPalette : 1;     ///< @brief The DMG palette to use for the tile data (DMG only).
        Uint8 m_HorizontalFlip : 1; ///< @brief Whether to flip the tile horizontally.
        Uint8 m_VerticalFlip : 1;   ///< @brief Whether to flip the tile vertically.
        Uint8 m_Priority : 1;       ///< @brief Does the background/window have priority over the object or any object on this tile?
    };

    Uint8 m_Value;  ///< @brief The raw attribute value.
} GABLE_TileAttributes;

// Object Attribute Memory (OAM) Entry Structure ///////////////////////////////////////////////////

/**
 * @brief A structure representing an entry in the object attribute memory (OAM) buffer.
 */
typedef struct GABLE_Object
{
    Uint8 m_Y;                          ///< @brief The Y-coordinate of the object on the screen.
    Uint8 m_X;                          ///< @brief The X-coordinate of the object on the screen.
    Uint8 m_TileIndex;                  ///< @brief The index of the tile in the tile data buffer.
    GABLE_TileAttributes m_Attributes;  ///< @brief The attributes of the object.
} GABLE_Object;

// CRAM Color Structure ////////////////////////////////////////////////////////////////////////////

/**
 * @brief A structure representing a two-byte RGB555 color in the color RAM (CRAM) buffer.
 * 
 * @note The RGB555 color format uses 5 bits for each of the red, green, and blue color channels,
 *       laid out as follows: `0bRRRRRGGG` `0bGGBBBBB0`.
 */
typedef struct GABLE_ColorRGB555
{
    Uint16 m_Red   : 5;  ///< @brief The red color channel.
    Uint16 m_Green : 5;  ///< @brief The green color channel.
    Uint16 m_Blue  : 5;  ///< @brief The blue color channel.
    Uint16 : 1;          ///< @brief The unused bit at the end of the color sequence.
} GABLE_ColorRGB555;

// PPU Pixel Fetcher Structure /////////////////////////////////////////////////////////////////////

/**
 * @brief A structure representing the PPU's internal pixel-fetcher unit.
 */
typedef struct GABLE_PixelFetcher
{
    
    GABLE_PixelFetchMode m_Mode;  ///< @brief The current mode of the pixel-fetcher unit.
    
    // Pixel FIFO
    struct
    {
        Uint32 m_Buffer[GABLE_PPU_PIXEL_FIFO_SIZE]; ///< @brief The pixel FIFO buffer.
        Uint8  m_Head;                              ///< @brief The index of the next available slot in the buffer.
        Uint8  m_Tail;                              ///< @brief The index of the next slot to read from in the buffer.
        Uint8  m_Size;                              ///< @brief The number of pixels currently in the buffer.
    } m_PixelFIFO;

    // Fetched Tile Data - Background/Window Layer
    struct
    {
        Uint8                   m_TileIndex;        ///< @brief The number of the fetched tile in the appropriate tilemap.
        GABLE_TileAttributes    m_TileAttributes;   ///< @brief The attributes of the fetched tile.
        Uint8                   m_TileDataLow;      ///< @brief The low byte of the fetched tile data.
        Uint8                   m_TileDataHigh;     ///< @brief The high byte of the fetched tile data.
    } m_FetchedBGW;

    // Fetched Tile Data - Object Layer
    struct
    {
        Uint8                   m_ObjectIndices[3]; ///< @brief The indices of the fetched objects' tiles in the tile data buffer.
        Uint8                   m_TileDataLow[3];   ///< @brief The low bytes of the fetched objects' tile data.
        Uint8                   m_TileDataHigh[3];  ///< @brief The high bytes of the fetched objects' tile data.
        Uint8                   m_ObjectCount;      ///< @brief The number of objects fetched. Maximum of 3.
    } m_FetchedOBJ;

    // Tile Fetcher State
    Uint8 m_LineX;                                  ///< @brief The fetcher's current X-coordinate on the current scanline.
    Uint8 m_PushedX;                                ///< @brief The X-coordinate of the last pixel pushed to the screen buffer.  
    Uint8 m_FetchingX;                              ///< @brief The X-coordinate of the pixel currently being fetched.
    Uint8 m_MapY, m_MapX;                           ///< @brief The fetcher's current Y and X coordinates in the 256x256-pixel tilemap.               
    Uint8 m_TileDataOffset;                         ///< @brief The starting offset address of the tile data being fetched.
    Uint8 m_OffsetX;                                ///< @brief The X-coordinate offset of the current tile data fetch.
    Uint8 m_QueueX;                                 ///< @brief The X-coordinate of the last pixel pushed into the pixel FIFO?

} GABLE_PixelFetcher;

// Public Functions ////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Creates and initializes a new instance of the GABLE PPU structure.
 * 
 * @return A pointer to the newly created and initialized GABLE PPU structure.
 */
GABLE_PPU* GABLE_CreatePPU ();

/**
 * @brief Destroys an instance of the GABLE PPU structure.
 * 
 * @param p_PPU A pointer to the GABLE PPU structure to destroy.
 */
void GABLE_DestroyPPU (GABLE_PPU* p_PPU);

/**
 * @brief Resets the GABLE PPU structure to its initial state.
 * 
 * @param p_PPU A pointer to the GABLE PPU structure to reset.
 */
void GABLE_ResetPPU (GABLE_PPU* p_PPU);

/**
 * @brief Ticks the GABLE PPU structure, updating its components, and continuing to process the
 *        current frame.
 * 
 * @param p_PPU     A pointer to the GABLE PPU structure to tick.
 * @param p_Engine  A pointer to the GABLE Engine structure.
 * 
 * @note A pointer to the engine structure is needed to access the engine's memory map and so that
 *       the PPU can request interrupts from the engine.
 */
void GABLE_TickPPU (GABLE_PPU* p_PPU, GABLE_Engine* p_Engine);

/**
 * @brief Ticks the PPU's OAM DMA transfer process, copying the next byte of data from the engine's
 *        memory to the OAM buffer.
 * 
 * @param p_PPU     A pointer to the GABLE PPU structure.
 * @param p_Engine  A pointer to the GABLE Engine structure.
 */
void GABLE_TickODMA (GABLE_PPU* p_PPU, GABLE_Engine* p_Engine);

// Public Functions - Memory Access ////////////////////////////////////////////////////////////////

/**
 * @brief Reads a byte from the video RAM (VRAM) buffer.
 * 
 * @param p_PPU     A pointer to the GABLE PPU structure.
 * @param p_Address The address in the VRAM buffer to read from.
 * @param p_Value   A pointer to a variable to store the read byte.
 * 
 * @return `true` if the byte was read successfully; `false` otherwise.
 */
Bool GABLE_ReadVRAMByte (const GABLE_PPU* p_PPU, Uint16 p_Address, Uint8* p_Value);

/**
 * @brief Writes a byte to the video RAM (VRAM) buffer.
 * 
 * @param p_PPU     A pointer to the GABLE PPU structure.
 * @param p_Address The address in the VRAM buffer to write to.
 * @param p_Value   The byte to write.
 * 
 * @return `true` if the byte was written successfully; `false` otherwise.
 */
Bool GABLE_ReadOAMByte (const GABLE_PPU* p_PPU, Uint16 p_Address, Uint8* p_Value);

/**
 * @brief Writes a byte to the video RAM (VRAM) buffer.
 * 
 * @param p_PPU     A pointer to the GABLE PPU structure.
 * @param p_Address The address in the VRAM buffer to write to.
 * @param p_Value   The byte to write.
 * 
 * @return `true` if the byte was written successfully; `false` otherwise.
 */
Bool GABLE_WriteVRAMByte (GABLE_PPU* p_PPU, Uint16 p_Address, Uint8 p_Value);

/**
 * @brief Writes a byte to the object attribute memory (OAM) buffer.
 * 
 * @param p_PPU     A pointer to the GABLE PPU structure.
 * @param p_Address The address in the OAM buffer to write to.
 * @param p_Value   The byte to write.
 * 
 * @return `true` if the byte was written successfully; `false` otherwise.
 */
Bool GABLE_WriteOAMByte (GABLE_PPU* p_PPU, Uint16 p_Address, Uint8 p_Value);

// Public Functions - Hardware Register Getters ////////////////////////////////////////////////////

/**
 * @brief Gets the value of the PPU's `LCDC` display control register.
 * 
 * @param p_PPU A pointer to the GABLE PPU structure.
 * 
 * @return The value of the PPU's `LCDC` display control register.
 */
Uint8 GABLE_ReadLCDC (const GABLE_PPU* p_PPU);

/**
 * @brief Gets the value of the PPU's `STAT` display status register.
 * 
 * @param p_PPU A pointer to the GABLE PPU structure.
 * 
 * @return The value of the PPU's `STAT` display status register.
 */
Uint8 GABLE_ReadSTAT (const GABLE_PPU* p_PPU);

/**
 * @brief Gets the value of the PPU's `SCY` scroll Y register.
 * 
 * @param p_PPU A pointer to the GABLE PPU structure.
 * 
 * @return The value of the PPU's `SCY` scroll Y register.
 */
Uint8 GABLE_ReadSCY (const GABLE_PPU* p_PPU);

/**
 * @brief Gets the value of the PPU's `SCX` scroll X register.
 * 
 * @param p_PPU A pointer to the GABLE PPU structure.
 * 
 * @return The value of the PPU's `SCX` scroll X register.
 */
Uint8 GABLE_ReadSCX (const GABLE_PPU* p_PPU);

/**
 * @brief Gets the value of the PPU's `LY` scanline register.
 * 
 * @param p_PPU A pointer to the GABLE PPU structure.
 * 
 * @return The value of the PPU's `LY` scanline register.
 */
Uint8 GABLE_ReadLY (const GABLE_PPU* p_PPU);

/**
 * @brief Gets the value of the PPU's `LYC` line compare register.
 * 
 * @param p_PPU A pointer to the GABLE PPU structure.
 * 
 * @return The value of the PPU's `LYC` line compare register.
 */
Uint8 GABLE_ReadLYC (const GABLE_PPU* p_PPU);

/**
 * @brief Gets the value of the PPU's `DMA` OAM DMA transfer register.
 * 
 * @param p_PPU A pointer to the GABLE PPU structure.
 * 
 * @return The value of the PPU's `DMA` OAM DMA transfer register.
 */
Uint8 GABLE_ReadDMA (const GABLE_PPU* p_PPU);

/**
 * @brief Gets the value of the PPU's `BGP` background palette register.
 * 
 * @param p_PPU A pointer to the GABLE PPU structure.
 * 
 * @return The value of the PPU's `BGP` background palette register.
 */
Uint8 GABLE_ReadBGP (const GABLE_PPU* p_PPU);

/**
 * @brief Gets the value of the PPU's `OBP0` object palette 0 register.
 * 
 * @param p_PPU A pointer to the GABLE PPU structure.
 * 
 * @return The value of the PPU's `OBP0` object palette 0 register.
 */
Uint8 GABLE_ReadOBP0 (const GABLE_PPU* p_PPU);

/**
 * @brief Gets the value of the PPU's `OBP1` object palette 1 register.
 * 
 * @param p_PPU A pointer to the GABLE PPU structure.
 * 
 * @return The value of the PPU's `OBP1` object palette 1 register.
 */
Uint8 GABLE_ReadOBP1 (const GABLE_PPU* p_PPU);

/**
 * @brief Gets the value of the PPU's `WY` window Y position register.
 * 
 * @param p_PPU A pointer to the GABLE PPU structure.
 * 
 * @return The value of the PPU's `WY` window Y position register.
 */
Uint8 GABLE_ReadWY (const GABLE_PPU* p_PPU);

/**
 * @brief Gets the value of the PPU's `WX` window X position register.
 * 
 * @param p_PPU A pointer to the GABLE PPU structure.
 * 
 * @return The value of the PPU's `WX` window X position register.
 */
Uint8 GABLE_ReadWX (const GABLE_PPU* p_PPU);

/**
 * @brief Gets the value of the PPU's `VBK` VRAM bank register.
 * 
 * @param p_PPU A pointer to the GABLE PPU structure.
 * 
 * @return The value of the PPU's `VBK` VRAM bank register.
 */
Uint8 GABLE_ReadVBK (const GABLE_PPU* p_PPU);

// `HDMA1` through `HDMA4` are write-only.

/**
 * @brief Gets the value of the PPU's `HDMA5` GDMA / HDMA transfer control register.
 * 
 * @param p_PPU A pointer to the GABLE PPU structure.
 * 
 * @return The value of the PPU's `HDMA5` GDMA / HDMA transfer control register.
 */
Uint8 GABLE_ReadHDMA5 (const GABLE_PPU* p_PPU);

/**
 * @brief Gets the value of the PPU's `BGPI` background color palette specification register.
 * 
 * @param p_PPU A pointer to the GABLE PPU structure.
 * 
 * @return The value of the PPU's `BGPI` background color palette specification register.
 */
Uint8 GABLE_ReadBGPI (const GABLE_PPU* p_PPU);

/**
 * @brief Gets the value of the PPU's `BGPD` background color palette data register.
 * 
 * @param p_PPU A pointer to the GABLE PPU structure.
 * 
 * @return The value of the PPU's `BGPD` background color palette data register.
 */
Uint8 GABLE_ReadBGPD (const GABLE_PPU* p_PPU);

/**
 * @brief Gets the value of the PPU's `OBPI` object color palette specification register.
 * 
 * @param p_PPU A pointer to the GABLE PPU structure.
 * 
 * @return The value of the PPU's `OBPI` object color palette specification register.
 */
Uint8 GABLE_ReadOBPI (const GABLE_PPU* p_PPU);

/**
 * @brief Gets the value of the PPU's `OBPD` object color palette data register.
 * 
 * @param p_PPU A pointer to the GABLE PPU structure.
 * 
 * @return The value of the PPU's `OBPD` object color palette data register.
 */
Uint8 GABLE_ReadOBPD (const GABLE_PPU* p_PPU);

/**
 * @brief Gets the value of the PPU's `OPRI` object priority register.
 * 
 * @param p_PPU A pointer to the GABLE PPU structure.
 * 
 * @return The value of the PPU's `OPRI` object priority register.
 */
Uint8 GABLE_ReadOPRI (const GABLE_PPU* p_PPU);

/**
 * @brief Gets the value of the PPU's `GRPM` graphics mode register.
 * 
 * @param p_PPU A pointer to the GABLE PPU structure.
 * 
 * @return The value of the PPU's `GRPM` graphics mode register.
 */
Uint8 GABLE_ReadGRPM (const GABLE_PPU* p_PPU);

// Public Functions - Hardware Register Setters ////////////////////////////////////////////////////

/**
 * @brief Sets the value of the PPU's `LCDC` display control register.
 * 
 * @param p_PPU     A pointer to the GABLE PPU structure.
 * @param p_Value   The new value to set.
 */
void GABLE_WriteLCDC (GABLE_PPU* p_PPU, Uint8 p_Value);

/**
 * @brief Sets the value of the PPU's `STAT` display status register.
 * 
 * @param p_PPU     A pointer to the GABLE PPU structure.
 * @param p_Value   The new value to set.
 */
void GABLE_WriteSTAT (GABLE_PPU* p_PPU, Uint8 p_Value);

/**
 * @brief Sets the value of the PPU's `SCY` scroll Y register.
 * 
 * @param p_PPU     A pointer to the GABLE PPU structure.
 * @param p_Value   The new value to set.
 */
void GABLE_WriteSCY (GABLE_PPU* p_PPU, Uint8 p_Value);

/**
 * @brief Sets the value of the PPU's `SCX` scroll X register.
 * 
 * @param p_PPU     A pointer to the GABLE PPU structure.
 * @param p_Value   The new value to set.
 */
void GABLE_WriteSCX (GABLE_PPU* p_PPU, Uint8 p_Value);

// `LY` is read-only.

/**
 * @brief Sets the value of the PPU's `LYC` line compare register.
 * 
 * @param p_PPU     A pointer to the GABLE PPU structure.
 * @param p_Value   The new value to set.
 */
void GABLE_WriteLYC (GABLE_PPU* p_PPU, Uint8 p_Value);

/**
 * @brief Sets the value of the PPU's `DMA` OAM DMA transfer register. The value written to this
 *        register is the upper byte of the source address for the OAM DMA transfer, and writing
 *        to this register initiates the transfer.
 * 
 * @param p_PPU     A pointer to the GABLE PPU structure.
 * @param p_Value   The new value to set.
 */
void GABLE_WriteDMA (GABLE_PPU* p_PPU, Uint8 p_Value);

/**
 * @brief Sets the value of the PPU's `BGP` background palette register.
 * 
 * @param p_PPU     A pointer to the GABLE PPU structure.
 * @param p_Value   The new value to set.
 */
void GABLE_WriteBGP (GABLE_PPU* p_PPU, Uint8 p_Value);

/**
 * @brief Sets the value of the PPU's `OBP0` object palette 0 register.
 * 
 * @param p_PPU     A pointer to the GABLE PPU structure.
 * @param p_Value   The new value to set.
 */
void GABLE_WriteOBP0 (GABLE_PPU* p_PPU, Uint8 p_Value);

/**
 * @brief Sets the value of the PPU's `OBP1` object palette 1 register.
 * 
 * @param p_PPU     A pointer to the GABLE PPU structure.
 * @param p_Value   The new value to set.
 */
void GABLE_WriteOBP1 (GABLE_PPU* p_PPU, Uint8 p_Value);

/**
 * @brief Sets the value of the PPU's `WY` window Y position register.
 * 
 * @param p_PPU     A pointer to the GABLE PPU structure.
 * @param p_Value   The new value to set.
 */
void GABLE_WriteWY (GABLE_PPU* p_PPU, Uint8 p_Value);

/**
 * @brief Sets the value of the PPU's `WX` window X position register.
 * 
 * @param p_PPU     A pointer to the GABLE PPU structure.
 * @param p_Value   The new value to set.
 */
void GABLE_WriteWX (GABLE_PPU* p_PPU, Uint8 p_Value);

/**
 * @brief Sets the value of the PPU's `VBK` VRAM bank register.
 * 
 * @param p_PPU     A pointer to the GABLE PPU structure.
 * @param p_Value   The new value to set.
 */
void GABLE_WriteVBK (GABLE_PPU* p_PPU, Uint8 p_Value);

/**
 * @brief Sets the value of the PPU's `HDMA1` HDMA source high address register.
 * 
 * @param p_PPU     A pointer to the GABLE PPU structure.
 * @param p_Value   The new value to set.
 */
void GABLE_WriteHDMA1 (GABLE_PPU* p_PPU, Uint8 p_Value);

/**
 * @brief Sets the value of the PPU's `HDMA2` HDMA source low address register.
 * 
 * @param p_PPU     A pointer to the GABLE PPU structure.
 * @param p_Value   The new value to set.
 */
void GABLE_WriteHDMA2 (GABLE_PPU* p_PPU, Uint8 p_Value);

/**
 * @brief Sets the value of the PPU's `HDMA3` HDMA destination high address register.
 * 
 * @param p_PPU     A pointer to the GABLE PPU structure.
 * @param p_Value   The new value to set.
 */
void GABLE_WriteHDMA3 (GABLE_PPU* p_PPU, Uint8 p_Value);

/**
 * @brief Sets the value of the PPU's `HDMA4` HDMA destination low address register.
 * 
 * @param p_PPU     A pointer to the GABLE PPU structure.
 * @param p_Value   The new value to set.
 */
void GABLE_WriteHDMA4 (GABLE_PPU* p_PPU, Uint8 p_Value);

/**
 * @brief Sets the value of the PPU's `HDMA5` GDMA / HDMA transfer control register.
 * 
 * @param p_PPU     A pointer to the GABLE PPU structure.
 * @param p_Engine  A pointer to the GABLE Engine structure.
 * @param p_Value   The new value to set.
 * 
 * @note The engine pointer is needed to read from the address bus if a GDMA transfer is requested.
 */
void GABLE_WriteHDMA5 (GABLE_PPU* p_PPU, GABLE_Engine* p_Engine, Uint8 p_Value);

/**
 * @brief Sets the value of the PPU's `BGPI` background color palette specification register.
 * 
 * @param p_PPU     A pointer to the GABLE PPU structure.
 * @param p_Value   The new value to set.
 */
void GABLE_WriteBGPI (GABLE_PPU* p_PPU, Uint8 p_Value);

/**
 * @brief Sets the value of the PPU's `BGPD` background color palette data register.
 * 
 * @param p_PPU     A pointer to the GABLE PPU structure.
 * @param p_Value   The new value to set.
 */
void GABLE_WriteBGPD (GABLE_PPU* p_PPU, Uint8 p_Value);

/**
 * @brief Sets the value of the PPU's `OBPI` object color palette specification register.
 * 
 * @param p_PPU     A pointer to the GABLE PPU structure.
 * @param p_Value   The new value to set.
 */
void GABLE_WriteOBPI (GABLE_PPU* p_PPU, Uint8 p_Value);

/**
 * @brief Sets the value of the PPU's `OBPD` object color palette data register.
 * 
 * @param p_PPU     A pointer to the GABLE PPU structure.
 * @param p_Value   The new value to set.
 */
void GABLE_WriteOBPD (GABLE_PPU* p_PPU, Uint8 p_Value);

/**
 * @brief Sets the value of the PPU's `OPRI` object priority register.
 * 
 * @param p_PPU     A pointer to the GABLE PPU structure.
 * @param p_Value   The new value to set.
 */
void GABLE_WriteOPRI (GABLE_PPU* p_PPU, Uint8 p_Value);

/**
 * @brief Sets the value of the PPU's `GRPM` graphics mode register.
 * 
 * @param p_PPU     A pointer to the GABLE PPU structure.
 * @param p_Value   The new value to set.
 */
void GABLE_WriteGRPM (GABLE_PPU* p_PPU, Uint8 p_Value);

// Public Functions - High-Level Functions /////////////////////////////////////////////////////////

/**
 * @brief Sets the function to be called when the PPU finishes rendering the visible portion of the
 *        current frame.
 * 
 * @param p_Engine   A pointer to the GABLE Engine structure.
 * @param p_Callback A pointer to the function to call when the frame is rendered.
 * 
 * @note  This callback function is called at the start of the `VBLANK` period, regardless of whether
 *        the `VBLANK` interrupt is enabled or disabled.
 */
void GABLE_SetFrameRenderedCallback (GABLE_Engine* p_Engine, GABLE_FrameRenderedCallback p_Callback);

/**
 * @brief Gets the PPU's screen buffer, containing the RGBA color values of the pixels to be displayed
 *        on the screen.
 * 
 * @param p_Engine A pointer to the GABLE Engine structure.
 * 
 * @return A pointer to the screen buffer.
 */
const Uint32* GABLE_GetScreenBuffer (GABLE_Engine* p_Engine);

/**
 * @brief Waits until the end of the current vertial blanking period (`VBLANK`).
 * 
 * @param p_Engine A pointer to the GABLE Engine structure.
 */
void GABLE_WaitUntilAfterVerticalBlank (GABLE_Engine* p_Engine);

/**
 * @brief Waits for the PPU to enter the vertical blanking period (`VBLANK`).
 * 
 * @param p_Engine A pointer to the GABLE Engine structure.
 */
void GABLE_WaitForVerticalBlank (GABLE_Engine* p_Engine);

/**
 * @brief Uploads tile data to the tile data region in the current bank of the video RAM (VRAM).
 * 
 * @param p_Engine              A pointer to the GABLE Engine structure.
 * @param p_BPP                 The enumerated bit depth of the tile data (`GABLE_1BPP`, `GABLE_2BPP`).
 * @param p_SourceAddress       The address in the engine's memory map to copy the tile data from.
 * @param p_DestinationIndex    The index of the first tile in the tile data region to copy the data to.
 * @param p_TileCount           The number of tiles to copy.
 * 
 * @note The tile data is copied to the tile data region in the current bank of the video RAM (VRAM),
 *       and the destination index is set according to the `LCDC` register's tile data address bit.
 */
void GABLE_UploadTileData (GABLE_Engine* p_Engine, GABLE_BitsPerPixel p_BPP, Uint16 p_SourceAddress, Uint8 p_DestinationIndex, Uint8 p_TileCount);

/**
 * @brief Retrieves some information about a tile in the tile map at the specified index.
 * 
 * - If the VRAM bank number `VBK` is 0, then the information is the index of the tile in the tile
 *   data region, found in VRAM bank 0.
 * 
 * - If the VRAM bank number `VBK` is 1, then the information is the attributes of the tile in the
 *   tile attribute map, found in VRAM bank 1.
 * 
 * @param p_Engine        A pointer to the GABLE Engine structure.
 * @param p_TilemapIndex  The index of the tile in the tile map.
 * @param p_X             The X-coordinate of the tile in the tile map.
 * @param p_Y             The Y-coordinate of the tile in the tile map.
 * 
 * @return The requested tile information (tile data index if `VBK` is 0, tile attributes if `VBK` is 1).
 * 
 * @note If the VRAM bank number `VBK` is 0, the tile data index is returned as a byte value. Use
 *       only the `m_Value` member of the returned `GABLE_TileAttributes` union to access this value.
 */
GABLE_TileAttributes GABLE_GetTileInfo (GABLE_Engine* p_Engine, Uint8 p_TilemapIndex, Uint8 p_X, Uint8 p_Y);
GABLE_TileAttributes GABLE_GetBackgroundTileInfo (GABLE_Engine* p_Engine, Uint8 p_X, Uint8 p_Y);
GABLE_TileAttributes GABLE_GetWindowTileInfo (GABLE_Engine* p_Engine, Uint8 p_X, Uint8 p_Y);

/**
 * @brief Sets some information about a tile in the tile map at the specified index.
 * 
 * - If the VRAM bank number `VBK` is 0, then the information is the index of the tile in the tile
 *   data region, found in VRAM bank 0.
 * 
 * - If the VRAM bank number `VBK` is 1, then the information is the attributes of the tile in the
 *   tile attribute map, found in VRAM bank 1.
 * 
 * @param p_Engine        A pointer to the GABLE Engine structure.
 * @param p_TilemapIndex  The index of the tile in the tile map.
 * @param p_X             The X-coordinate of the tile in the tile map.
 * @param p_Y             The Y-coordinate of the tile in the tile map.
 * @param p_Value         The new value to set.
 * 
 * @note If the VRAM bank number `VBK` is 1, then the information being set is the attributes of the
 *       tile in the tile attribute map. It is recommended to use a `GABLE_TileAttributes` union to
 *       configure this information, then pass its `m_Value` member to this function.
 */
void GABLE_SetTileInfo (GABLE_Engine* p_Engine, Uint8 p_TilemapIndex, Uint8 p_X, Uint8 p_Y, Uint8 p_Value);
void GABLE_SetBackgroundTileInfo (GABLE_Engine* p_Engine, Uint8 p_X, Uint8 p_Y, Uint8 p_Value);
void GABLE_SetWindowTileInfo (GABLE_Engine* p_Engine, Uint8 p_X, Uint8 p_Y, Uint8 p_Value);

/**
 * @brief Retrieves some information about an object in the object attribute memory (OAM) buffer at
 *        the specified index.
 * 
 * @param p_Engine          A pointer to the GABLE Engine structure.
 * @param p_Index           The index of the object in the OAM buffer.
 * @param p_X               A pointer to a variable to store the X-coordinate of the object.
 * @param p_Y               A pointer to a variable to store the Y-coordinate of the object.
 * @param p_TileIndex       A pointer to a variable to store the index of the object's tile.
 * @param p_Attributes      A pointer to a variable to store the attributes of the object.
 */
void GABLE_GetObjectInfo (GABLE_Engine* p_Engine, Uint8 p_Index, Uint8* p_X, Uint8* p_Y, Uint8* p_TileIndex, GABLE_TileAttributes* p_Attributes);
void GABLE_SetObjectPosition (GABLE_Engine* p_Engine, Uint8 p_Index, Uint8 p_X, Uint8 p_Y);
void GABLE_SetObjectX (GABLE_Engine* p_Engine, Uint8 p_Index, Uint8 p_X);
void GABLE_SetObjectY (GABLE_Engine* p_Engine, Uint8 p_Index, Uint8 p_Y);
void GABLE_SetObjectTileIndex (GABLE_Engine* p_Engine, Uint8 p_Index, Uint8 p_TileIndex);
void GABLE_SetObjectAttributes (GABLE_Engine* p_Engine, Uint8 p_Index, GABLE_TileAttributes p_Attributes);
void GABLE_MoveObject (GABLE_Engine* p_Engine, Uint8 p_Index, Int8 p_OffsetX, Int8 p_OffsetY);

const GABLE_ColorRGB555* GABLE_LookupColorPreset (GABLE_Color p_Color);
void GABLE_GetBackgroundColor (GABLE_Engine* p_Engine, Uint8 p_PaletteIndex, Uint8 p_ColorIndex, GABLE_ColorRGB555* p_RGB555, Uint32* p_RGBA);
void GABLE_GetObjectColor (GABLE_Engine* p_Engine, Uint8 p_PaletteIndex, Uint8 p_ColorIndex, GABLE_ColorRGB555* p_RGB555, Uint32* p_RGBA);
void GABLE_SetBackgroundColor (GABLE_Engine* p_Engine, Uint8 p_PaletteIndex, Uint8 p_ColorIndex, const GABLE_ColorRGB555* p_RGB555, const Uint32* p_RGBA);
void GABLE_SetObjectColor (GABLE_Engine* p_Engine, Uint8 p_PaletteIndex, Uint8 p_ColorIndex, const GABLE_ColorRGB555* p_RGB555, const Uint32* p_RGBA);

GABLE_DisplayControl GABLE_GetDisplayControl (GABLE_Engine* p_Engine);
GABLE_DisplayStatus GABLE_GetDisplayStatus (GABLE_Engine* p_Engine);
void GABLE_GetViewportPosition (GABLE_Engine* p_Engine, Uint8* p_X, Uint8* p_Y);
Uint8 GABLE_GetCurrentScanline (GABLE_Engine* p_Engine);
Uint8 GABLE_GetLineCompare (GABLE_Engine* p_Engine);
void GABLE_GetDMGPaletteIndices (GABLE_Engine* p_Engine, Uint8* p_BG, Uint8* p_OB0, Uint8* p_OB1);
void GABLE_GetWindowPosition (GABLE_Engine* p_Engine, Uint8* p_X, Uint8* p_Y);
Uint8 GABLE_GetVRAMBankNumber (GABLE_Engine* p_Engine);
GABLE_HDMAControl GABLE_GetHDMAControl (GABLE_Engine* p_Engine);
GABLE_PaletteSpecification GABLE_GetBackgroundPaletteSpec (GABLE_Engine* p_Engine);
GABLE_PaletteSpecification GABLE_GetObjectPaletteSpec (GABLE_Engine* p_Engine);
Uint8 GABLE_GetCurrentBackgroundColorByte (GABLE_Engine* p_Engine);
Uint8 GABLE_GetCurrentObjectColorByte (GABLE_Engine* p_Engine);
GABLE_ObjectPriorityMode GABLE_GetObjectPriorityMode (GABLE_Engine* p_Engine);
GABLE_GraphicsMode GABLE_GetGraphicsMode (GABLE_Engine* p_Engine);

void GABLE_SetDisplayControl (GABLE_Engine* p_Engine, GABLE_DisplayControl p_Value);
void GABLE_SetDisplayStatus (GABLE_Engine* p_Engine, GABLE_DisplayStatus p_Value);
void GABLE_SetViewportPosition (GABLE_Engine* p_Engine, Uint8 p_X, Uint8 p_Y);
void GABLE_SetViewportX (GABLE_Engine* p_Engine, Uint8 p_X);
void GABLE_SetViewportY (GABLE_Engine* p_Engine, Uint8 p_Y);
void GABLE_SetLineCompare (GABLE_Engine* p_Engine, Uint8 p_Value);
void GABLE_InitiateODMA (GABLE_Engine* p_Engine, Uint8 p_SourceAddressHigh);
void GABLE_SetDMGPaletteIndices (GABLE_Engine* p_Engine, Uint8 p_BG, Uint8 p_OB0, Uint8 p_OB1);
void GABLE_SetDMGBackgroundPaletteIndex (GABLE_Engine* p_Engine, Uint8 p_Index);
void GABLE_SetDMGObjectPaletteIndex (GABLE_Engine* p_Engine, Uint8 p_Palette, Uint8 p_Index);
void GABLE_SetWindowPosition (GABLE_Engine* p_Engine, Uint8 p_X, Uint8 p_Y);
void GABLE_SetWindowX (GABLE_Engine* p_Engine, Uint8 p_X);
void GABLE_SetWindowY (GABLE_Engine* p_Engine, Uint8 p_Y);
void GABLE_SetVRAMBankNumber (GABLE_Engine* p_Engine, Uint8 p_Value);
void GABLE_SetHDMAAddresses (GABLE_Engine* p_Engine, Uint16 p_SourceAddress, Uint16 p_DestinationAddress);
void GABLE_SetHDMASourceAddress (GABLE_Engine* p_Engine, Uint16 p_Address);
void GABLE_SetHDMADestinationAddress (GABLE_Engine* p_Engine, Uint16 p_Address);
void GABLE_InitiateHDMA (GABLE_Engine* p_Engine, Uint8 p_TransferLength, Bool p_IsGDMA);
void GABLE_SetBackgroundPaletteSpec (GABLE_Engine* p_Engine, Uint8 p_Index, Bool p_AutoIncrement);
void GABLE_SetObjectPaletteSpec (GABLE_Engine* p_Engine, Uint8 p_Index, Bool p_AutoIncrement);
void GABLE_SetCurrentBackgroundColorByte (GABLE_Engine* p_Engine, Uint8 p_Value);
void GABLE_SetCurrentObjectColorByte (GABLE_Engine* p_Engine, Uint8 p_Value);
void GABLE_SetObjectPriorityMode (GABLE_Engine* p_Engine, GABLE_ObjectPriorityMode p_Mode);
void GABLE_SetGraphicsMode (GABLE_Engine* p_Engine, GABLE_GraphicsMode p_Mode);
