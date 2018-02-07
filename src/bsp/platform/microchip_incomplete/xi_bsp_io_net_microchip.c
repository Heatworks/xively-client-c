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
    xi_socket = socket(AF_INET, SOCK_STREAM, SOCKET_FLAGS_SSL);
    if (xi_socket < 0) {
		SYS_CONSOLE_PRINT("Socket creation error\r\n");
		close(xi_socket);
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
	if (xi_socket < 0) {
		xi_socket = socket(AF_INET, SOCK_STREAM, SOCKET_FLAGS_SSL);
	}

	/* Check if socket was created successfully. */
	if (xi_socket < 0) {
		SYS_CONSOLE_PRINT("Socket creation error\r\n");
		close(xi_socket);
		return XI_BSP_IO_NET_STATE_ERROR;
	}

	/* If success, connect to socket. */
	if (connect(xi_socket, (struct sockaddr *)&addr_in, sizeof(struct sockaddr_in)) != SOCK_ERR_NO_ERROR) {
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
    //TO DO:  MJW how do we check if the winc1500 is "Ready" to receive more data?
    /*else if ( TCPIP_TCP_PutIsReady( xi_socket ) == 0)
    {
        //MJW changed b/c XI_BSP_IO_NET_STATE_WANT_WRITE doesn't exist??
        //return XI_BSP_IO_NET_STATE_WANT_WRITE;        
        return XI_BSP_IO_NET_STATE_BUSY;
    }*/
        
    send((SOCKET) xi_socket, buf, count, 0);
    
    //looks like "out written count" is used to monitor what has been sent.  We only know this from our socket callback.
    // how to deal with this?
    *out_written_count = wifiData.s16Sent;
    SYS_CONSOLE_PRINT( "written: %d bytes\r\n", *out_written_count );

    if ( 0 == *out_written_count )
    {
        /* treat this like EAGAIN */
        //MJW changed b/c XI_BSP_IO_NET_STATE_WANT_WRITE doesn't exist??
        //return XI_BSP_IO_NET_STATE_WANT_WRITE;
        SYS_CONSOLE_PRINT( "Wanted to write, but couldn't!\r\n");
        return XI_BSP_IO_NET_STATE_BUSY;
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

    recv((SOCKET) xi_socket, buf, count, 10000);    //10 second timeout??  what should we use here
    
    *out_read_count = wifiData.s16Received;
    
    SYS_CONSOLE_PRINT( "read: %d bytes\r\n", *out_read_count );

    if ( *out_read_count < 0 )
    {
        /*if ( TCPIP_TCP_WasReset( xi_socket ) || !TCPIP_TCP_IsConnected( xi_socket ) )
        {
            SYS_CONSOLE_PRINT( "connection reset by peer\r\n" );
            return XI_BSP_IO_NET_STATE_CONNECTION_RESET;
        }
        else
        {
            //MJW changed b/c XI_BSP_IO_NET_STATE_WANT_READ doesn't exist??
            //return XI_BSP_IO_NET_STATE_WANT_READ;
            return XI_BSP_IO_NET_STATE_BUSY;            
        }*/
        return XI_BSP_IO_NET_STATE_BUSY;            
    }
    else if ( *out_read_count == 0 )
    {
        /* treat this like EAGAIN */
        //MJW changed b/c XI_BSP_IO_NET_STATE_WANT_READ doesn't exist??
        //return XI_BSP_IO_NET_STATE_WANT_READ;
        return XI_BSP_IO_NET_STATE_BUSY;            
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
    for ( socket_id = 0; socket_id < socket_events_array_size; ++socket_id )
    {
        xi_bsp_socket_events_t* socket_events = &socket_events_array[socket_id];

        if ( 1 == socket_events->in_socket_want_connect )
        {
            /*if ( TRUE == TCPIP_TCP_IsConnected( socket_events->xi_socket ) )
            {
                socket_events->out_socket_connect_finished = 1;
            }*/
            continue;
        }

        /*if ( 1 == socket_events->in_socket_want_read &&
             ( TCPIP_TCP_GetIsReady( socket_events->xi_socket ) > 0 ||
               0 == TCPIP_TCP_IsConnected( socket_events->xi_socket ) ) )
        {
            socket_events->out_socket_can_read = 1;
            continue;
        }

        if ( 1 == socket_events->in_socket_want_write &&
             ( TCPIP_TCP_PutIsReady( socket_events->xi_socket ) > 0 ||
               0 == TCPIP_TCP_IsConnected( socket_events->xi_socket ) ) )
        {
            socket_events->out_socket_can_write = 1;
            continue;
        }*/

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
