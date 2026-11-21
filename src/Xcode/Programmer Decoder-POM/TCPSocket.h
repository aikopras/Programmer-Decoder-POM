//
//  TCPSocket.h
//  Programmer Decoder-POM
//
//  Created by Aiko Pras on 27-10-26.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  2026: Reusable TCP client connection, based on Apple's Network framework.
//  It replaces the NSStream based connections, since getStreamsToHostWithName:port:inputStream:outputStream:
//  will be deprecated. The class has no knowledge of DCC, RS-bus or Lenz; it only connects, sends,
//  receives and reports. It can therefore be used unchanged in other applications.
//
//  Usage:
//  - create the object, set its delegate and call openToHost:port:timeout:
//  - the delegate is informed of every state change and of all received data
//  - all delegate methods are called on the main thread
//  - call close to close the connection (the delegate receives TCP_SOCKET_CLOSED)
//  - data passed to sendBytes:length: while the connection is being established is kept, and sent as soon
//    as the connection is ready (as NSStream did). In all other states than connecting or connected,
//    data passed to sendBytes:length: is discarded
//  The project should link Network.framework.
//
//  History:
//  27-10-2026: first version, for dccmon (OSX-DCC-Monitor)
//  28-10-2026: data sent while connecting is kept until the connection is ready
//

#import <Foundation/Foundation.h>

@class TCPSocketClass;

// The TCP socket may be in any of the following states
typedef enum
{ TCP_SOCKET_CLOSED = 0,      // No connection (initial state, after close, or after the remote side closed)
  TCP_SOCKET_CONNECTING = 1,  // The connection is being established
  TCP_SOCKET_CONNECTED = 2,   // The connection is established; data can be sent and received
  TCP_SOCKET_FAILED = 3,      // The connection could not be established (no response, refused, ...)
  TCP_SOCKET_LOST = 4,        // The connection was established, but broke down afterwards
} tcpSocketState_t;


// The object that uses the socket should implement the following two methods
@protocol TCPSocketDelegate <NSObject>
// Called after every state change. For TCP_SOCKET_FAILED and TCP_SOCKET_LOST, reason contains a description
// of the error (see also lastErrorCode); for TCP_SOCKET_CLOSED, reason is nil if the connection was closed
// by calling close, and "closed by remote side" if the other side closed the connection
- (void)tcpSocket:(TCPSocketClass *)socket didChangeState:(tcpSocketState_t)state reason:(NSString *)reason;
// Called for every chunk of data received. The bytes are only valid during this call
- (void)tcpSocket:(TCPSocketClass *)socket didReceiveBytes:(const uint8_t *)bytes length:(NSUInteger)length;
@end


@interface TCPSocketClass : NSObject

// The following methods should be used
- (void)openToHost:(NSString *)host port:(NSString *)port timeout:(uint32_t)seconds;
- (void)sendBytes:(const uint8_t *)bytes length:(NSUInteger)length;
- (void)close;
- (BOOL)isOpen;                                        // YES while connecting or connected

@property (assign) id <TCPSocketDelegate> delegate;    // Not retained
@property (readonly) tcpSocketState_t state;
@property (readonly) int lastErrorCode;                // POSIX error code (errno) of the last failure, 0 if none

@end
