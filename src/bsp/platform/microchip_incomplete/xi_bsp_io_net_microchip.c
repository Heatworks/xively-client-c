/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_io_net.h>

#include "system_config.h"
#include "system_definitions.h"
#include "mqtt_tasks.h"
#include <stdbool.h>
#include "socket.h"
#include "wifi.h"

#define __XI_MICROCHIP_BSP_NET_IMPLEMENTATION__

#ifdef __XI_MICROCHIP_BSP_NET_IMPLEMENTATION__


typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef void* PTR_BASE;
typedef uint8_t BOOL;

/*enum
{
    TCP_OPEN_ROM_HOST,
    TCP_PURPOSE_GENERIC_TCP_CLIENT,
    INVALID_SOCKET
};
*/
/* fake microchip network API just for gcc compilation purposes */
//int TCPOpen( DWORD host, int host_type, uint16_t port, int purpose );

//BOOL TCPIsPutReady( intptr_t socket );
//WORD TCPPutArray( intptr_t socket, const uint8_t* buf, size_t count );
//WORD TCPGetArray( intptr_t socket, uint8_t* buf, size_t count );

//BOOL TCPIsConnected( intptr_t socket );
//BOOL TCPWasReset( intptr_t socket );

//void TCPDisconnect( intptr_t socket );
//void TCPClose( intptr_t socket );

#endif /* __XI_FAKE_MICROCHIP_BSP_NET_IMPLEMENTATION__ */


#ifdef __cplusplus
extern "C" {
#endif

xi_bsp_io_net_state_t xi_bsp_io_net_create_socket( xi_bsp_socket_t* xi_socket )
{
    //( void )xi_socket;    
    *xi_socket = socket(AF_INET, SOCK_STREAM, SOCKET_FLAGS_SSL);
    if (*xi_socket < 0) {
		SYS_CONSOLE_PRINT("Socket creation error\r\n");
		close(*xi_socket);
		return XI_BSP_IO_NET_STATE_ERROR;
	}
    return XI_BSP_IO_NET_STATE_OK;
}

xi_bsp_io_net_state_t
xi_bsp_io_net_connect( xi_bsp_socket_t* xi_socket, const char* host, uint16_t port )
{
    struct sockaddr_in addr_in;

	addr_in.sin_family = AF_INET;
	addr_in.sin_port = _htons(port);
	addr_in.sin_addr.s_addr = mqttData.resolvedMqttIp.Val;

	/* Create secure socket. */
	if (*xi_socket < 0) {
		*xi_socket = socket(AF_INET, SOCK_STREAM, SOCKET_FLAGS_SSL);
	}

	/* Check if socket was created successfully. */
	if (*xi_socket < 0) {
		SYS_CONSOLE_PRINT("Socket creation error\r\n");
		close(*xi_socket);
		return XI_BSP_IO_NET_STATE_ERROR;
	}
    SYS_CONSOLE_PRINT("Xively socket %d created.\r\n", *xi_socket);
	/* If success, connect to socket. */
	if (connect(*xi_socket, (struct sockaddr *)&addr_in, sizeof(struct sockaddr_in)) != SOCK_ERR_NO_ERROR) {
		SYS_CONSOLE_PRINT("Socket connection error\r\n");
		return XI_BSP_IO_NET_STATE_ERROR;
	}

	/* Success. */
	return XI_BSP_IO_NET_STATE_OK;

    if ( INVALID_SOCKET == *xi_socket )
    {
        return XI_BSP_IO_NET_STATE_ERROR;
    }

    return XI_BSP_IO_NET_STATE_OK;
}

xi_bsp_io_net_state_t xi_bsp_io_net_connection_check( xi_bsp_socket_t xi_socket,
                                                      const char* host,
                                                      uint16_t port )
{
    ( void )xi_socket;
    ( void )host;
    ( void )port;

    /* no connect post action needed */

    return XI_BSP_IO_NET_STATE_OK;
}

xi_bsp_io_net_state_t xi_bsp_io_net_write( xi_bsp_socket_t xi_socket,
                                           int* out_written_count,
                                           const uint8_t* buf,
                                           size_t count )
{
    if ( wifiData.socketStatus == SOCKET_ERROR)
    {
        SYS_CONSOLE_PRINT( "connection reset by peer\r\n" );

        return XI_BSP_IO_NET_STATE_CONNECTION_RESET;
    }
    
    // if the socket is ready, we will give it a request to send data.  The data is NOT sent immediately.
    //so, we return "zero bytes sent, NET State Busy."
    //This will result in a future call to this function when we must NOT call "send" to the socket, and instead, return how many bytes have been transmitted.
    if(wifiData.socketTxReady){
        send((SOCKET) xi_socket, buf, count, 0);
        *out_written_count = 0;
        wifiData.socketTxReady = false;
        wifiData.socketTxCompleted = false;
        return XI_BSP_IO_NET_STATE_BUSY;
    }    
    else if(wifiData.socketTxCompleted){    //we're not ready, but TX completed.  This means the full transaction has taken place.
        *out_written_count = wifiData.socketNumBytesSent;
        wifiData.socketTxReady = true;
        SYS_CONSOLE_PRINT( "Xively wrote: %d bytes\r\n", *out_written_count );
    }       

    return XI_BSP_IO_NET_STATE_OK;
}

xi_bsp_io_net_state_t xi_bsp_io_net_read( xi_bsp_socket_t xi_socket,
                                          int* out_read_count,
                                          uint8_t* buf,
                                          size_t count )
{
    if ( wifiData.socketStatus == SOCKET_ERROR)
    {
        SYS_CONSOLE_PRINT( "connection reset by peer\r\n" );
        return XI_BSP_IO_NET_STATE_CONNECTION_RESET;
    }
    
    //same logic as TX
    if(wifiData.socketRxReady){
        recv((SOCKET) xi_socket, buf, count, 10000);    //10 second timeout??  what should we use here
        *out_read_count = 0;
        wifiData.socketRxReady = false;
        wifiData.socketRxCompleted = false;
        return XI_BSP_IO_NET_STATE_BUSY;
    }    
    else if(wifiData.socketRxCompleted){    //we're not ready, but TX completed.  This means the full transaction has taken place.
        *out_read_count = wifiData.socketNumBytesReceived;
        wifiData.socketRxReady = true;
        SYS_CONSOLE_PRINT( "Xively read: %d bytes\r\n", *out_read_count );
    }

    return XI_BSP_IO_NET_STATE_OK;
}

xi_bsp_io_net_state_t xi_bsp_io_net_close_socket( xi_bsp_socket_t* xi_socket )
{
    /* close the connection & the socket */    
    close(xi_socket);
    return XI_BSP_IO_NET_STATE_OK;
}

xi_bsp_io_net_state_t xi_bsp_io_net_select( xi_bsp_socket_events_t* socket_events_array,
                                            size_t socket_events_array_size,
                                            long timeout )
{
    /* unused at least for now */
    ( void )timeout;
        
    /* translate the library socket events settings to the set's of events used by posix
     * select */
    size_t socket_id = 0;
    //MJW To Do:  will xively ever open more than 1 socket?  Seems like this is implied here, but our main wifi.c loop assumes a single socket connection.
    for ( socket_id = 0; socket_id < socket_events_array_size; ++socket_id )
    {
        xi_bsp_socket_events_t* socket_events = &socket_events_array[socket_id];

        if ( 1 == socket_events->in_socket_want_connect )
        {            
            //if ( TRUE == TCPIP_TCP_IsConnected( socket_events->xi_socket ) )
            if(wifiData.socketStatus == SOCKET_CONNECTED)
            {
                socket_events->out_socket_connect_finished = 1;
            }
            continue;
        }

//        if ( 1 == socket_events->in_socket_want_read &&
//             ( TCPIP_TCP_GetIsReady( socket_events->xi_socket ) > 0 ||
//               0 == TCPIP_TCP_IsConnected( socket_events->xi_socket ) ) )
        if((1 == socket_events->in_socket_want_read) && (wifiData.socketStatus == SOCKET_CONNECTED))
        {
            socket_events->out_socket_can_read = 1;
            continue;
        }

//        if ( 1 == socket_events->in_socket_want_write &&
//             ( TCPIP_TCP_PutIsReady( socket_events->xi_socket ) > 0 ||
//               0 == TCPIP_TCP_IsConnected( socket_events->xi_socket ) ) )
        if((1 == socket_events->in_socket_want_write) && (wifiData.socketStatus == SOCKET_CONNECTED))
        {
            socket_events->out_socket_can_write = 1;
            continue;
        }

        if ( 1 == socket_events->in_socket_want_error )
        {
            socket_events->out_socket_error = 1;
            continue;
        }
    }

    return XI_BSP_IO_NET_STATE_OK;
}

#ifdef __cplusplus
}
#endif
