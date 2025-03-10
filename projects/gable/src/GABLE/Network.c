/**
 * @file GABLE/Network.c
 */

#include <GABLE/Engine.h>
#include <GABLE/Timer.h>
#include <GABLE/InterruptContext.h>
#include <GABLE/Network.h>

// Platform-Specific ///////////////////////////////////////////////////////////////////////////////

#if defined(GABLE_LINUX)
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <fcntl.h>

    #define GABLE_INVALID_SOCKET -1

    typedef int                 GABLE_Socket;
#else
    #error "The GABLE Engine's network interface is not yet implemented for this platform."
#endif

// GABLE Network Context Structure /////////////////////////////////////////////////////////////////

typedef struct GABLE_NetworkContext
{
    Uint8                           m_NetRAM[GABLE_NETRAM_SIZE];    ///< @brief The GABLE Engine's network RAM.
    Uint8                           m_Packet[GABLE_NET_PACKET_SIZE];///< @brief The GABLE Engine's network packet buffer.
    GABLE_Socket                    m_Socket;                       ///< @brief The GABLE Engine's network socket.
    GABLE_NetworkTransferControl    m_NTC;                          ///< @brief The GABLE Engine's network transfer control register.
    Uint8                           m_NTS;                          ///< @brief The GABLE Engine's network transfer size register.
    Uint16                          m_ByteCounter;                  ///< @brief The packet byte counter.
    Uint16                          m_TimeoutCounter;               ///< @brief The timeout counter of the current transfer.
} GABLE_NetworkContext;

// Static Function Prototypes //////////////////////////////////////////////////////////////////////

static void GABLE_HandleNetworkTransfer (GABLE_NetworkContext* p_Network, GABLE_Engine* p_Engine);

// Static Functions ////////////////////////////////////////////////////////////////////////////////

void GABLE_HandleNetworkTransfer (GABLE_NetworkContext* p_Network, GABLE_Engine* p_Engine)
{
    // Are we sending or receiving data?
    if (p_Network->m_NTC.m_TransferDirection == GABLE_NTD_SEND)
    {
    #if defined(GABLE_LINUX)
        // Send the packet buffer (or the next part of it) to the network, in a non-blocking manner.
        ssize_t l_BytesSent = send(
            p_Network->m_Socket, 
            &p_Network->m_Packet[p_Network->m_ByteCounter], 
            GABLE_NET_PACKET_SIZE - p_Network->m_ByteCounter, 
            MSG_DONTWAIT
        );

        // Check if the send operation was successful.
        if (l_BytesSent == -1)
        {
            // If `errno` is `EAGAIN` or `EWOULDBLOCK`, then the socket is still waiting to send data.
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // In this case, update the timeout counter and see if we need to time out.
                p_Network->m_TimeoutCounter++;
                if (p_Network->m_TimeoutCounter >= GABLE_NET_TIMEOUT_TICKS)
                {
                    // If we have timed out, set the transfer status to timeout, end the transfer, and request an interrupt.
                    p_Network->m_NTC.m_TransferStatus = GABLE_NTS_TIMEOUT;
                    p_Network->m_NTC.m_TransferEnable = 0;
                    GABLE_RequestInterrupt(GABLE_GetInterruptContext(p_Engine), GABLE_INT_NET);
                    return;
                }
            }

            // Otherwise, an error occured.
            GABLE_perror("Failed to send data over the network");

            // Set the transfer status to error, end the transfer, and request an interrupt.
            p_Network->m_NTC.m_TransferStatus = GABLE_NTS_ERROR;
            p_Network->m_NTC.m_TransferEnable = 0;
            GABLE_RequestInterrupt(GABLE_GetInterruptContext(p_Engine), GABLE_INT_NET);
            return;
        }

        // Update the byte counter to reflect the number of bytes sent. If the updated byte counter
        // equals or exceeds `NTS` + 4, then the transfer is complete.
        p_Network->m_ByteCounter += l_BytesSent;
        if (p_Network->m_ByteCounter >= p_Network->m_NTS + 4)
        {
            // Set the transfer status to ready, end the transfer, and request an interrupt.
            p_Network->m_NTC.m_TransferStatus = GABLE_NTS_READY;
            p_Network->m_NTC.m_TransferEnable = 0;
            GABLE_RequestInterrupt(GABLE_GetInterruptContext(p_Engine), GABLE_INT_NET);
        }
        else
        {
            // If the transfer is not complete, then we will continue to send data in subsequent ticks.
            // Increment the timeout counter to prevent the transfer from timing out.
            p_Network->m_TimeoutCounter++;
            if (p_Network->m_TimeoutCounter >= GABLE_NET_TIMEOUT_TICKS)
            {
                // If we have timed out, set the transfer status to timeout, end the transfer, and request an interrupt.
                p_Network->m_NTC.m_TransferStatus = GABLE_NTS_TIMEOUT;
                p_Network->m_NTC.m_TransferEnable = 0;
                GABLE_RequestInterrupt(GABLE_GetInterruptContext(p_Engine), GABLE_INT_NET);
                return;
            }
        }
    #endif
    }
    else
    {
    #if defined(GABLE_LINUX)
        // Receive data from the network, in a non-blocking manner.
        ssize_t l_BytesReceived = recv(
            p_Network->m_Socket, 
            &p_Network->m_Packet[p_Network->m_ByteCounter], 
            GABLE_NET_PACKET_SIZE - p_Network->m_ByteCounter, 
            MSG_DONTWAIT
        );

        // Check if the receive operation was successful.
        if (l_BytesReceived == -1)
        {
            // If `errno` is `EAGAIN` or `EWOULDBLOCK`, then the socket is still waiting to receive data.
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // In this case, update the timeout counter and see if we need to time out.
                p_Network->m_TimeoutCounter++;
                if (p_Network->m_TimeoutCounter >= GABLE_NET_TIMEOUT_TICKS)
                {
                    // If we have timed out, set the transfer status to timeout, end the transfer, and request an interrupt.
                    p_Network->m_NTC.m_TransferStatus = GABLE_NTS_TIMEOUT;
                    p_Network->m_NTC.m_TransferEnable = 0;
                    GABLE_RequestInterrupt(GABLE_GetInterruptContext(p_Engine), GABLE_INT_NET);
                    return;
                }

                // Otherwise, an error occured.
                GABLE_perror("Failed to send data over the network");
    
                // Set the transfer status to error, end the transfer, and request an interrupt.
                p_Network->m_NTC.m_TransferStatus = GABLE_NTS_ERROR;
                p_Network->m_NTC.m_TransferEnable = 0;
                GABLE_RequestInterrupt(GABLE_GetInterruptContext(p_Engine), GABLE_INT_NET);
                return;
            }
        }

        // Update the byte counter to reflect the number of bytes received. If the updated byte counter
        // equals or exceeds `NTS` + 4, then the transfer is complete.
        p_Network->m_ByteCounter += l_BytesReceived;
        if (p_Network->m_ByteCounter >= p_Network->m_NTS + 4)
        {
            // Set the transfer status to ready, end the transfer, and request an interrupt.
            p_Network->m_NTC.m_TransferStatus = GABLE_NTS_READY;
            p_Network->m_NTC.m_TransferEnable = 0;
            GABLE_RequestInterrupt(GABLE_GetInterruptContext(p_Engine), GABLE_INT_NET);
        }
        else
        {
            // If the transfer is not complete, then we will continue to receive data in subsequent ticks.
            // Increment the timeout counter to prevent the transfer from timing out.
            p_Network->m_TimeoutCounter++;
            if (p_Network->m_TimeoutCounter >= GABLE_NET_TIMEOUT_TICKS)
            {
                // If we have timed out, set the transfer status to timeout, end the transfer, and request an interrupt.
                p_Network->m_NTC.m_TransferStatus = GABLE_NTS_TIMEOUT;
                p_Network->m_NTC.m_TransferEnable = 0;
                GABLE_RequestInterrupt(GABLE_GetInterruptContext(p_Engine), GABLE_INT_NET);
                return;
            }
        }
    #endif
    }
}

// Public Functions ////////////////////////////////////////////////////////////////////////////////

GABLE_NetworkContext* GABLE_CreateNetworkContext ()
{
    GABLE_NetworkContext* l_Network = GABLE_calloc(1, GABLE_NetworkContext);
    GABLE_pexpect(l_Network, "Failed to allocate memory for network context");

    l_Network->m_Socket = GABLE_INVALID_SOCKET;

    return l_Network;
}

void GABLE_DestroyNetworkContext (GABLE_NetworkContext* p_Network)
{
    if (p_Network != NULL)
    {
        GABLE_CloseConnection(p_Network);
        GABLE_free(p_Network);
    }
}

void GABLE_TickNetworkContext (GABLE_NetworkContext* p_Network, GABLE_Engine* p_Engine)
{
    GABLE_expect(p_Network != NULL, "Network context is NULL!");
    GABLE_expect(p_Engine != NULL, "Engine context is NULL!");

    // Ensure that there is a network connection.
    if (p_Network->m_Socket == GABLE_INVALID_SOCKET)
    {
        return;
    }

    Bool l_NetworkTick = GABLE_CheckTimerDividerBit(GABLE_GetTimer(p_Engine), 14);
    if (l_NetworkTick == false || p_Network->m_NTC.m_TransferEnable == 0)
    {
        return;
    }

    // Handle the network transfer.
    GABLE_HandleNetworkTransfer(p_Network, p_Engine);
}

Bool GABLE_Connect (GABLE_NetworkContext* p_Context, const char* p_Address, Uint16 p_Port)
{
    GABLE_expect(p_Context != NULL, "Network context is NULL!");
    GABLE_expect(p_Address != NULL, "Address string is NULL!");

    // Ensure that the address is not empty.
    if (p_Address[0] == '\0')
    {
        GABLE_error("Address string is blank.");
        return false;
    }

    // Ensure that the port is not zero.
    if (p_Port == 0)
    {
        GABLE_error("Port number is zero.");
        return false;
    }

    // Close the network socket if it is already open.
    GABLE_CloseConnection(p_Context);

    #if defined(GABLE_LINUX)
    {

        // In case the address string given is actually a hostname, we may need to resolve this
        // to an IP address. We will use the `getaddrinfo` function to do this.
        //
        // First, prepare the hints and address info structures.
        struct addrinfo l_Hints = { 0 };
        l_Hints.ai_family       = AF_UNSPEC;    // Use IPv4 or IPv6, whichever is available.
        l_Hints.ai_socktype     = SOCK_STREAM;  // We are using a TCP stream socket.
        l_Hints.ai_protocol     = IPPROTO_TCP;  // We are using the TCP protocol.

        // Now resolve the hostname to one or more IP addresses.
        // If the address is already an IP address, then this will be the only result.
        struct addrinfo* l_AddrInfo = NULL;
        int l_Result = getaddrinfo(p_Address, NULL, &l_Hints, &l_AddrInfo);
        if (l_Result != 0)
        {
            GABLE_error("Failed to resolve '%s' - %s", p_Address, gai_strerror(l_Result));
            return false;
        }

        // Iterate through the results to find a suitable address.
        struct addrinfo* l_Current = l_AddrInfo;
        while (l_Current != NULL)
        {
            // Ensure that the address family is either IPv4 or IPv6.
            if (l_Current->ai_family != AF_INET && l_Current->ai_family != AF_INET6)
            {
                l_Current = l_Current->ai_next;
                continue;
            }

            // Try to open a socket with the current address, using the `socket` function.
            p_Context->m_Socket = socket(
                l_Current->ai_family,
                l_Current->ai_socktype,
                l_Current->ai_protocol
            );
            if (p_Context->m_Socket != GABLE_INVALID_SOCKET)
            {
                break;
            }

            // If the socket could not be opened, then try the next address.
            l_Current = l_Current->ai_next;
        }

        // Ensure that a socket was successfully opened.
        if (p_Context->m_Socket == GABLE_INVALID_SOCKET)
        {
            GABLE_perror("Failed to open a socket for '%s:%u'", p_Address, p_Port);
            freeaddrinfo(l_AddrInfo);
            return false;
        }

        // Prepare the address structure for the remote host.
        struct sockaddr_storage l_RemoteAddr = { 0 };
        if (l_Current->ai_family == AF_INET)
        {
            struct sockaddr_in* l_RemoteAddr4 = (struct sockaddr_in*) &l_RemoteAddr;
            l_RemoteAddr4->sin_family = AF_INET;
            l_RemoteAddr4->sin_port = htons(p_Port);
            memcpy(&l_RemoteAddr4->sin_addr, &((struct sockaddr_in*) l_Current->ai_addr)->sin_addr, sizeof(struct in_addr));
        }
        else if (l_Current->ai_family == AF_INET6)
        {
            struct sockaddr_in6* l_RemoteAddr6 = (struct sockaddr_in6*) &l_RemoteAddr;
            l_RemoteAddr6->sin6_family = AF_INET6;
            l_RemoteAddr6->sin6_port = htons(p_Port);
            memcpy(&l_RemoteAddr6->sin6_addr, &((struct sockaddr_in6*) l_Current->ai_addr)->sin6_addr, sizeof(struct in6_addr));
        }

        // Attempt to connect to the remote host using the `connect` function.
        if (connect(p_Context->m_Socket, (struct sockaddr*) &l_RemoteAddr, sizeof(l_RemoteAddr)) == -1)
        {
            GABLE_perror("Failed to connect to '%s:%u'", p_Address, p_Port);
            close(p_Context->m_Socket);
            p_Context->m_Socket = GABLE_INVALID_SOCKET;
            freeaddrinfo(l_AddrInfo);
            return false;
        }

        // Free the address info structure.
        freeaddrinfo(l_AddrInfo);

    }
    #endif

    return true;
}

void GABLE_CloseConnection (GABLE_NetworkContext* p_Context)
{
    GABLE_expect(p_Context != NULL, "Network context is NULL!");   

    // Close the network socket.
    #if defined(GABLE_LINUX)
    if (p_Context->m_Socket != GABLE_INVALID_SOCKET)
    {
        close(p_Context->m_Socket);
        p_Context->m_Socket = GABLE_INVALID_SOCKET;
    }
    #endif
}

void GABLE_RequestNetworkTransfer (GABLE_NetworkContext* p_Context)
{
    GABLE_expect(p_Context != NULL, "Network context is NULL!");

    // Enable the network transfer and set the status to busy.
    p_Context->m_NTC.m_TransferEnable = 1;
    p_Context->m_NTC.m_TransferStatus = GABLE_NTS_BUSY;

    // Reset the timeout and byte counters.
    p_Context->m_TimeoutCounter = 0;
    p_Context->m_ByteCounter = 0;

    // Set up the packet buffer.
    p_Context->m_Packet[0] = 0x6B;
    p_Context->m_Packet[1] = 0x01;
    p_Context->m_Packet[2] = 0x00;
    p_Context->m_Packet[3] = p_Context->m_NTS;
    
    // Copy the current bank of NETRAM into the packet buffer.
    memcpy(
        &p_Context->m_Packet[4],
        &p_Context->m_NetRAM[p_Context->m_NTC.m_BankSelect * GABLE_NETRAM_BANK_SIZE],
        GABLE_NETRAM_BANK_SIZE
    );
}

void GABLE_CancelNetworkTransfer (GABLE_NetworkContext* p_Context)
{
    GABLE_expect(p_Context != NULL, "Network context is NULL!");

    // Disable the network transfer and set the status to ready.
    p_Context->m_NTC.m_TransferEnable = 0;
    p_Context->m_NTC.m_TransferStatus = GABLE_NTS_READY;

    // Reset the timeout and byte counters.
    p_Context->m_TimeoutCounter = 0;
    p_Context->m_ByteCounter = 0;

    // Clear the packet buffer.
    memset(p_Context->m_Packet, 0, GABLE_NET_PACKET_SIZE);
}

GABLE_NetworkTransferStatus GABLE_GetNetworkTransferStatus (const GABLE_NetworkContext* p_Context)
{
    GABLE_expect(p_Context != NULL, "Network context is NULL!");

    return p_Context->m_NTC.m_TransferStatus;
}

GABLE_NetworkTransferDirection GABLE_GetNetworkTransferDirection (const GABLE_NetworkContext* p_Context)
{
    GABLE_expect(p_Context != NULL, "Network context is NULL!");

    return p_Context->m_NTC.m_TransferDirection;
}

Uint8 GABLE_GetNetworkRAMBank (const GABLE_NetworkContext* p_Context)
{
    GABLE_expect(p_Context != NULL, "Network context is NULL!");

    return p_Context->m_NTC.m_BankSelect;
}

Uint8 GABLE_GetNetworkTransferSize (const GABLE_NetworkContext* p_Context)
{
    GABLE_expect(p_Context != NULL, "Network context is NULL!");

    return p_Context->m_NTS;
}

void GABLE_SetNetworkTransferDirection (GABLE_NetworkContext* p_Context, GABLE_NetworkTransferDirection p_Direction)
{
    GABLE_expect(p_Context != NULL, "Network context is NULL!");

    // Read-only if the network interface is busy.
    if (p_Context->m_NTC.m_TransferStatus == GABLE_NTS_BUSY)
    {
        return;
    }

    p_Context->m_NTC.m_TransferDirection = p_Direction;
}

void GABLE_SetNetworkRAMBank (GABLE_NetworkContext* p_Context, Uint8 p_Bank)
{
    GABLE_expect(p_Context != NULL, "Network context is NULL!");

    // Read-only if the network interface is busy.
    if (p_Context->m_NTC.m_TransferStatus == GABLE_NTS_BUSY)
    {
        return;
    }

    p_Context->m_NTC.m_BankSelect = p_Bank;
}

void GABLE_SetNetworkTransferSize (GABLE_NetworkContext* p_Context, Uint8 p_Size)
{
    GABLE_expect(p_Context != NULL, "Network context is NULL!");

    // Read-only if the network interface is busy.
    if (p_Context->m_NTC.m_TransferStatus == GABLE_NTS_BUSY)
    {
        return;
    }

    p_Context->m_NTS = p_Size;
}

// Public Functions - Memory Access ////////////////////////////////////////////////////////////////

Bool GABLE_ReadNetworkRAMByte (const GABLE_NetworkContext* p_Context, Uint16 p_Address, Uint8* p_Value)
{
    GABLE_expect(p_Context != NULL, "Network context is NULL!");
    GABLE_expect(p_Value != NULL, "Value pointer is NULL!");

    // Check if the network interface is busy.
    if (p_Context->m_NTC.m_TransferStatus == GABLE_NTS_BUSY)
    {
        // If it is, then the NETRAM is inaccessible. Read a garbage value.
        *p_Value = 0xFF;
        return true;
    }

    // Check if the address is out of bounds.
    if (p_Address >= GABLE_NETRAM_BANK_SIZE)
    {
        GABLE_error("NetRAM address $%04X is out of bounds.", p_Address);
        return false;
    }

    // Read the byte from the current bank of NETRAM.
    *p_Value = p_Context->m_NetRAM[(p_Context->m_NTC.m_BankSelect * GABLE_NETRAM_BANK_SIZE) + p_Address];
    return true;
}

Bool GABLE_WriteNetworkRAMByte (GABLE_NetworkContext* p_Context, Uint16 p_Address, Uint8 p_Value)
{
    GABLE_expect(p_Context != NULL, "Network context is NULL!");

    // Check if the network interface is busy.
    if (p_Context->m_NTC.m_TransferStatus == GABLE_NTS_BUSY)
    {
        // If it is, then the NETRAM is inaccessible. Do nothing.
        return true;
    }

    // Check if the address is out of bounds.
    if (p_Address >= GABLE_NETRAM_BANK_SIZE)
    {
        GABLE_error("NetRAM address $%04X is out of bounds.", p_Address);
        return false;
    }

    // Write the byte to the current bank of NETRAM.
    p_Context->m_NetRAM[(p_Context->m_NTC.m_BankSelect * GABLE_NETRAM_BANK_SIZE) + p_Address] = p_Value;
    return true;
}

// Public Functions - Hardware Register Getters ////////////////////////////////////////////////////

Uint8 GABLE_ReadNTC (const GABLE_NetworkContext* p_Context)
{
    GABLE_expect(p_Context != NULL, "Network context is NULL!");

    return p_Context->m_NTC.m_Register;
}

Uint8 GABLE_ReadNTS (const GABLE_NetworkContext* p_Context)
{
    GABLE_expect(p_Context != NULL, "Network context is NULL!");

    return p_Context->m_NTS;
}

// Public Functions - Hardware Register Setters ////////////////////////////////////////////////////

void GABLE_WriteNTC (GABLE_NetworkContext* p_Context, Uint8 p_Value)
{
    GABLE_expect(p_Context != NULL, "Network context is NULL!");

    // If the network interface is busy, then all but bit 7 are read-only.
    if (p_Context->m_NTC.m_TransferStatus == GABLE_NTS_BUSY)
    {
        p_Value &= 0b10000000;
    }

    // Otherwise, only bits 5 and 6 are read-only.
    else
    {
        p_Value &= 0b10011111;
    }

    p_Context->m_NTC.m_Register |= p_Value;

    // If the transfer enable bit is set, then a network transfer is requested.
    if (p_Context->m_NTC.m_TransferEnable == 1)
    {
        GABLE_RequestNetworkTransfer(p_Context);
    }
}

void GABLE_WriteNTS (GABLE_NetworkContext* p_Context, Uint8 p_Value)
{
    GABLE_expect(p_Context != NULL, "Network context is NULL!");

    // Read-only if the network interface is busy.
    if (p_Context->m_NTC.m_TransferStatus == GABLE_NTS_BUSY)
    {
        return;
    }

    p_Context->m_NTS = p_Value;
}
