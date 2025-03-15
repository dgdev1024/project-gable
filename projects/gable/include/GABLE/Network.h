/**
 * @file     GABLE/Network.h
 * @brief    Contains the GABLE Engine's network interface and its associated functions.
 * 
 * Instead of simulating the Game Boy's bitwise, byte-by-byte serial communication protocol, the
 * GABLE Engine opts to implement a networking interface which allows the engine to communicate with
 * external sources such as other GABLE Engine instances, game servers, and other networked devices.
 * 
 * The network interface component is equipped with the following memory buffer:
 * 
 * - The Network RAM (`NETRAM`): A 4 KB memory buffer, split into 16 banks of 256 bytes each, which
 *   is used to store data received from and sent to external sources. Memory in the current bank of
 *   NETRAM can be read from and written to by the game software by accessing the memory range
 *   `$E000` - `$E0FF` in the GABLE Engine's memory map, and by the network interface component
 *   itself when sending and receiving data. The current bank of NETRAM can be selected by writing
 *   to bits 0-3 of the `NTC` hardware register. The NETRAM is not accessible from the address bus
 *   while the network interface is busy.
 * 
 * The network interface component provides the following memory-mapped hardware registers:
 * 
 * - `NTC` (Network Transfer Control): This register controls how the network interface component
 *   sends or receives data to and from external sources, which bank of NETRAM is currently selected,
 *   and the status of the network interface. The bits of this register are defined as follows:
 *     - Bits 0-3 - NETRAM Bank Select: Selects the current bank of NETRAM to read from or write to.
 *       Read-only if the network interface is busy.
 *     - Bit 4 - Transfer Direction: Set to send data from NETRAM to an external source; clear to
 *       receive data from an external source to NETRAM. Read-only if the network interface is busy.
 *     - Bit 5-6 - Transfer Status: Indicates the current status of the network interface component.
 *       Read-only. The possible values are:
 *         - `0b00` - Ready: The network interface is ready to send or receive data.
 *         - `0b01` - Busy: The network interface is currently sending or receiving data.
 *         - `0b10` - Timeout: The network interface timed out while sending or receiving data.
 *         - `0b11` - Error: An error occurred while sending or receiving data.
 *     - Bit 7 - Transfer Enable: Indicates whether or not a network transfer has been requested or
 *       is currently in progress. Set to initiate a network transfer; clear to cancel a network
 *       transfer or to indicate that no transfer is in progress.
 * 
 * - `NTS` (Network Transfer Size): This register controls the size of the main data section of the
 *   packet to send or receive, in bytes. The size of the packet's header section is fixed. More on 
 *   this later.
 * 
 * The network interface component is responsible for requesting the following interrupt:
 * 
 * - Network Transfer Complete Interrupt (`NET`): This interrupt is requested when the network
 *   interface component has completed a data transfer, or when a data transfer error has occurred.
 *   This interrupt is typically used to handle network communication events or network errors.
 * 
 * The network interface component sends and receives data in the form of packets, which are
 * structured as follows:
 * 
 * - Packet Header (4 Bytes) - A fixed-size header which contains the following fields:
 *       - Magic Number (1 Byte): A byte value which identifies the packet as a GABLE Engine packet.
 *         This value is always `0x6B`.
 *       - Major Version (1 Byte): The major version number of the GABLE Engine instance which sent
 *         the packet.
 *       - Minor Version (1 Byte): The minor version number of the GABLE Engine instance which sent
 *         the packet.
 *       - Packet Size (1 Byte): The total size of the packet, in bytes. This is set to the value of
 *         the `NTS` register when the packet is sent.
 * - Packet Data (Variable Size, Up to 256 Bytes) - The main data section of the packet, which
 *   contains the data to send or receive.
 * 
 * When a network transfer is first initiated (by writing `1` to the `NTC` register's Transfer Enable
 * bit), the following processes occur:
 * 
 * - An internal byte buffer, called the "packet buffer", is set up. The 260-byte packet buffer is
 *   divided into two sections: the 4-byte header section and the 256-byte data section. The first
 *   four bytes of the packet buffer make up the header section, with byte 3 set to the value of the
 *   `NTS` register. The contents of the current bank of NETRAM are copied into the data section of
 *   the packet buffer.
 * - An internal byte counter, called the "packet byte counter", is set to `0`. This counter is used
 *   to keep track of the current byte being sent or received. This counter is incremented every
 *   time a chunk of data is sent or received, and is incremented by the size of the chunk. When
 *   this counter reaches (4 + `NTS`) bytes, the network transfer is complete.
 * - Another internal counter, the "timeout counter", is set to `0`. This counter is used to keep
 *   track of the number of network divider ticks (more on this later) that have passed since the
 *   network transfer was initiated. If this counter reaches a certain threshold, the network
 *   transfer is considered to have timed out, and the network interface component will set bits
 *   5-6 of the `NTC` register to `0b10` (Timeout) and request the `NET` interrupt.
 * - The network interface component sets bits 4-5 of the `NTC` register to `0b01` (Busy) to indicate
 *   that a network transfer is in progress.
 * 
 * The network interface component contains an internal divider register which is incremented every
 * time bit 14 (or 15 in double-speed mode) of the engine timer's 16-bit divider register changes
 * from high to low. This happens at a frequency of 128 Hz, at which rate the following events occur:
 * 
 * - Check bit 7 of the `NTC` register. If this bit is set, a network transfer is in progress. If this
 *   bit is clear, the network interface component does nothing.
 * - Check bit 4 of the `NTC` register. If this bit is set, the network interface component is sending
 *   data to an external source. If this bit is clear, the network interface component is receiving
 *   data from an external source.
 * - The network interface will attempt to transfer a chunk of data from the packet buffer to/from
 *   the external source. The interface will transfer as many bytes as it can in one tick, up to
 *   however many bytes are left in the packet buffer. The network interface will increment the packet
 *   byte counter by the number of bytes transferred.
 * - If the packet byte counter reaches (4 + `NTS`) bytes, the network transfer is complete. The network
 *   interface component will set bits 4-5 of the `NTC` register to `0b00` (Ready) to indicate that the
 *   network transfer is complete, and will request the `NET` interrupt.
 * - If the network transfer is not complete, the network interface component will increment the timeout
 *   counter by one. If the timeout counter reaches a certain threshold, the network transfer is considered
 *   to have timed out, and the network interface component will set bits 5-6 of the `NTC` register to
 *   `0b10` (Timeout) and request the `NET` interrupt.
 * - Otherwise, the network interface will attempt to transfer the remaining data in the packet buffer
 *   in subsequent ticks.
 * 
 * The network interface component conducts network transfers over a non-blocking TCP/IP socket. The
 * network interface component will attempt to send or receive data in chunks of up to a maximum size
 * of 4 bytes plus the value of the `NTS` register. The network interface component will attempt to
 * send or receive as many bytes as it can in one tick, up to the maximum chunk size. The network
 * interface component will continue to send or receive data in subsequent ticks until the entire
 * packet buffer has been sent or received.
 */

#pragma once
#include <GABLE/Common.h>

// Constants ///////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief The maximum size of a packet sent or received by the GABLE Engine's network interface, in
 *        bytes - four bytes for the header and 256 bytes for the data.
 */
#define GABLE_NET_PACKET_SIZE 260

/**
 * @brief The number of banks in the GABLE Engine's network RAM.
 */
#define GABLE_NET_RAM_BANKS 16

/**
 * @brief The number of ticks to wait before timing out a network transfer.
 */
#define GABLE_NET_TIMEOUT_TICKS 1024

// Typedefs and Forward Declarations ///////////////////////////////////////////////////////////////

/**
 * @brief A forward declaration of the GABLE Engine structure.
 */
typedef struct GABLE_Engine GABLE_Engine;

/**
 * @brief The GABLE Engine's network interface structure.
 */
typedef struct GABLE_NetworkContext GABLE_NetworkContext;

// Network Transfer Direction Enumeration //////////////////////////////////////////////////////////

/**
 * @brief Enumerates the possible directions of a network transfer.
 */
typedef enum GABLE_NetworkTransferDirection
{
    GABLE_NTD_RECEIVE = 0,  ///< @brief Receive data from an external source.
    GABLE_NTD_SEND          ///< @brief Send data to an external source.
} GABLE_NetworkTransferDirection;

// Network Transfer Status Enumeration /////////////////////////////////////////////////////////////

/**
 * @brief Enumerates the possible statuses of a network transfer.
 */
typedef enum GABLE_NetworkTransferStatus
{
    GABLE_NTS_READY = 0,    ///< @brief The network interface is ready to send or receive data.
    GABLE_NTS_BUSY,         ///< @brief The network interface is currently sending or receiving data.
    GABLE_NTS_TIMEOUT,      ///< @brief The network interface timed out while sending or receiving data.
    GABLE_NTS_ERROR         ///< @brief An error occurred while sending or receiving data.
} GABLE_NetworkTransferStatus;

// Network Transfer Control Union //////////////////////////////////////////////////////////////////

/**
 * @brief A union representing the GABLE Engine's network transfer control (`NTC`) register.
 */
typedef union GABLE_NetworkTransferControl
{
    struct
    {
        Uint8 m_BankSelect : 4;         ///< @brief The NETRAM bank select bits.
        Uint8 m_TransferDirection : 1;  ///< @brief The transfer direction bit
        Uint8 m_TransferStatus : 2;     ///< @brief The transfer status bits.
        Uint8 m_TransferEnable : 1;     ///< @brief The transfer enable bit.
    };
    Uint8 m_Register;                   ///< @brief The raw register value.
} GABLE_NetworkTransferControl;

// Public Functions ////////////////////////////////////////////////////////////////////////////////

/**
 * @brief      Creates a new GABLE Engine network interface instance.
 * 
 * @return     A pointer to the new GABLE Engine network interface instance.
 */
GABLE_NetworkContext* GABLE_CreateNetworkContext ();

/**
 * @brief      Resets a GABLE Engine network interface instance.
 * 
 * @param      p_Context  A pointer to the GABLE Engine network interface instance to reset.
 */
void GABLE_ResetNetworkContext (GABLE_NetworkContext* p_Context);

/**
 * @brief      Destroys a GABLE Engine network interface instance.
 * 
 * @param      p_Context  A pointer to the GABLE Engine network interface instance to destroy.
 */
void GABLE_DestroyNetworkContext (GABLE_NetworkContext* p_Context);

/**
 * @brief      Ticks the GABLE Engine's network interface component.
 * 
 * @param      p_Context  A pointer to the GABLE Engine network interface instance.
 * @param      p_Engine   A pointer to the GABLE Engine instance.
 */
void GABLE_TickNetworkContext (GABLE_NetworkContext* p_Context, GABLE_Engine* p_Engine);

// Public Functions - Memory Access ////////////////////////////////////////////////////////////////

/**
 * @brief      Reads a byte from the GABLE Engine's network RAM.
 * 
 * @param      p_Context  A pointer to the GABLE Engine network interface instance.
 * @param      p_Address  The address in the network RAM to read from.
 * @param      p_Value    A pointer to the variable to store the byte read from the network RAM.
 * 
 * @return     `true` if the byte was read successfully; `false` otherwise.
 */
Bool GABLE_ReadNetworkRAMByte (const GABLE_NetworkContext* p_Context, Uint16 p_Address, Uint8* p_Value);

/**
 * @brief      Writes a byte to the GABLE Engine's network RAM.
 * 
 * @param      p_Context  A pointer to the GABLE Engine network interface instance.
 * @param      p_Address  The address in the network RAM to write to.
 * @param      p_Value    The byte to write to the network RAM.
 * 
 * @return     `true` if the byte was written successfully; `false` otherwise.
 */
Bool GABLE_WriteNetworkRAMByte (GABLE_NetworkContext* p_Context, Uint16 p_Address, Uint8 p_Value);

// Public Functions - Hardware Register Getters ////////////////////////////////////////////////////

/**
 * @brief      Gets the value of the `NTC` (Network Transfer Control) register.
 * 
 * @param      p_Context  A pointer to the GABLE Engine network interface instance.
 * 
 * @return     The value of the `NTC` register.
 */
Uint8 GABLE_ReadNTC (const GABLE_NetworkContext* p_Context);

/**
 * @brief      Gets the value of the `NTS` (Network Transfer Size) register.
 * 
 * @param      p_Context  A pointer to the GABLE Engine network interface instance.
 * 
 * @return     The value of the `NTS` register.
 */
Uint8 GABLE_ReadNTS (const GABLE_NetworkContext* p_Context);

// Public Functions - Hardware Register Setters ////////////////////////////////////////////////////

/**
 * @brief      Sets the value of the `NTC` (Network Transfer Control) register.
 * 
 * @param      p_Context  A pointer to the GABLE Engine network interface instance.
 * @param      p_Value    The new value of the `NTC` register.
 */
void GABLE_WriteNTC (GABLE_NetworkContext* p_Context, Uint8 p_Value);

/**
 * @brief      Sets the value of the `NTS` (Network Transfer Size) register.
 * 
 * @param      p_Context  A pointer to the GABLE Engine network interface instance.
 * @param      p_Value    The new value of the `NTS` register.
 */
void GABLE_WriteNTS (GABLE_NetworkContext* p_Context, Uint8 p_Value);

// Public Functions - High-Level Functions /////////////////////////////////////////////////////////

/**
 * @brief      Connects the GABLE Engine's network interface component to a remote host.
 * 
 * @param      p_Engine   A pointer to the GABLE Engine instance.
 * @param      p_Address  The IP address (or hostname) of the remote host to connect to.
 * @param      p_Port     The port number of the remote host to connect to.
 * 
 * @return     `true` if the connection was successful; `false` otherwise.
 */
Bool GABLE_Connect (GABLE_Engine* p_Engine, const char* p_Address, Uint16 p_Port);

/**
 * @brief      Closes the connection of the GABLE Engine's network interface component, if one exists.
 * 
 * @param      p_Engine   A pointer to the GABLE Engine instance.
 */
void GABLE_CloseConnection (GABLE_Engine* p_Engine);
