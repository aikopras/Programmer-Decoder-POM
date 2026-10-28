//
//  TCPSocket.m
//  Programmer Decoder-POM
//
//  Created by Aiko Pras on 27-10-26.
//  Copyright (c) 2026 by Aiko Pras. All rights reserved.
//
//  2026: Reusable TCP client connection, based on Apple's Network framework. See TCPSocket.h.
//
//  Notes on the implementation:
//  - All Network framework events are handled on the main queue. The networking itself is done by macOS;
//    the main thread only receives the results and never waits.
//  - If a connection cannot be established, the Network framework enters the "waiting" state and would
//    try again after a network change. This class gives up instead, and reports TCP_SOCKET_FAILED.
//  - Manual memory management (MRC): the handlers do not retain self (__block variables). Instead, self
//    is retained when a connection is opened, and released when the final event of that connection
//    ("cancelled") has been delivered. According to the Network framework documentation, outstanding
//    sends and receives complete before that event, so the handlers never refer to a released object.
//

#import "TCPSocket.h"
#import <Network/Network.h>
#include <string.h>

@implementation TCPSocketClass {
  nw_connection_t _connection;           // The current connection; nil if there is no connection
  NSMutableData *_pendingData;           // Data passed to sendBytes:length: while connecting; nil if none
}

@synthesize delegate = _delegate;
@synthesize state = _state;
@synthesize lastErrorCode = _lastErrorCode;


// ******************************************************************************************
// **************************** Some C type definitions and declarations ********************
// ******************************************************************************************
#define MAX_RECEIVE_LENGTH 4096          // Maximum number of bytes delivered per receive call


// ******************************************************************************************
// ************************************** PUBLIC METHODS ************************************
// ******************************************************************************************
- (void)openToHost:(NSString *)host port:(NSString *)port timeout:(uint32_t)seconds {
  // Only one connection at a time: close a previous connection first
  [self close];
  _lastErrorCode = 0;
  // The endpoint consists of the host (name or IP address) plus the port number
  nw_endpoint_t endpoint = NULL;
  if ((host != nil) && (port != nil)) endpoint = nw_endpoint_create_host([host UTF8String], [port UTF8String]);
  if (endpoint == NULL) {[self setState:TCP_SOCKET_FAILED reason:@"invalid host or port"]; return;}
  // Plain TCP (no TLS), with a limited time to establish the connection
  nw_parameters_t parameters = nw_parameters_create_secure_tcp(NW_PARAMETERS_DISABLE_PROTOCOL,
    ^(nw_protocol_options_t tcpOptions) {nw_tcp_options_set_connection_timeout(tcpOptions, seconds);});
  nw_connection_t connection = nw_connection_create(endpoint, parameters);
  nw_release(endpoint);
  nw_release(parameters);
  if (connection == NULL) {[self setState:TCP_SOCKET_FAILED reason:@"could not create connection"]; return;}
  _connection = connection;
  // Keep this object alive until the final event of this connection has been delivered (see top of file)
  [self retain];
  __block TCPSocketClass *blockSelf = self;
  __block nw_connection_t blockConnection = connection;
  // Handle all events on the main queue, so the delegate methods are called on the main thread
  nw_connection_set_queue(connection, dispatch_get_main_queue());
  nw_connection_set_state_changed_handler(connection, ^(nw_connection_state_t nwState, nw_error_t error) {
    [blockSelf connection:blockConnection changedState:nwState error:error];
  });
  nw_connection_start(connection);
  [self setState:TCP_SOCKET_CONNECTING reason:nil];
}


- (void)sendBytes:(const uint8_t *)bytes length:(NSUInteger)length {
  if ((_connection == nil) || (length == 0)) return;
  // Data offered while the connection is being established is kept, and sent as soon as the connection is ready
  if (_state == TCP_SOCKET_CONNECTING) {
    if (_pendingData == nil) _pendingData = [[NSMutableData alloc] init];
    [_pendingData appendBytes:bytes length:length];
    return;
  }
  if (_state != TCP_SOCKET_CONNECTED) return;
  // dispatch_data_create with the default destructor copies the bytes, so the caller may reuse its buffer
  dispatch_data_t content = dispatch_data_create(bytes, length, NULL, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
  __block TCPSocketClass *blockSelf = self;
  __block nw_connection_t blockConnection = _connection;
  nw_connection_send(_connection, content, NW_CONNECTION_DEFAULT_MESSAGE_CONTEXT, true, ^(nw_error_t error) {
    // An error while sending means that the connection broke down
    if ((error != nil) && (blockConnection == blockSelf->_connection)) [blockSelf connectionBroke:blockConnection error:error];
  });
  dispatch_release(content);
}


- (void)close {
  if (_connection == nil) return;               // Nothing to close
  nw_connection_t connection = _connection;
  _connection = nil;                            // From now on, events of this connection are ignored
  nw_connection_cancel(connection);             // Closes the TCP connection (TCP FIN); the last event will be "cancelled"
  [self discardPendingData];
  [self setState:TCP_SOCKET_CLOSED reason:nil];
}


- (BOOL)isOpen {
  return ((_state == TCP_SOCKET_CONNECTING) || (_state == TCP_SOCKET_CONNECTED));
}


// ******************************************************************************************
// ************************************** INTERNAL METHODS **********************************
// ******************************************************************************************
- (void)setState:(tcpSocketState_t)state reason:(NSString *)reason {
  _state = state;
  [_delegate tcpSocket:self didChangeState:state reason:reason];
}


- (void)connection:(nw_connection_t)connection changedState:(nw_connection_state_t)nwState error:(nw_error_t)error {
  if (nwState == nw_connection_state_cancelled) {
    // Final event of this connection: release the connection and this object (see openToHost:port:timeout:)
    nw_release(connection);
    [self release];
    return;
  }
  if (connection != _connection) return;        // Event of a connection that has already been closed
  switch (nwState) {
    case nw_connection_state_ready:
      _state = TCP_SOCKET_CONNECTED;            // Set first, so the data kept while connecting can be sent
      [self sendPendingData];
      [self setState:TCP_SOCKET_CONNECTED reason:nil];
      if (connection == _connection) [self receiveOn:connection];   // Unless the delegate closed the connection
      break;
    case nw_connection_state_waiting:           // Could not connect; the framework would retry later, we give up
    case nw_connection_state_failed:
      [self connectionBroke:connection error:error];
      break;
    default:                                    // invalid, preparing: nothing to do
      break;
  }
}


- (void)receiveOn:(nw_connection_t)connection {
  __block TCPSocketClass *blockSelf = self;
  __block nw_connection_t blockConnection = connection;
  nw_connection_receive(connection, 1, MAX_RECEIVE_LENGTH,
    ^(dispatch_data_t content, nw_content_context_t context, bool isComplete, nw_error_t error) {
      // Pass the received bytes to the delegate (the data may consist of several regions)
      if ((content != nil) && (blockConnection == blockSelf->_connection)) {
        dispatch_data_apply(content, ^bool(dispatch_data_t region, size_t offset, const void *buffer, size_t size) {
          [blockSelf->_delegate tcpSocket:blockSelf didReceiveBytes:(const uint8_t *)buffer length:size];
          return (blockConnection == blockSelf->_connection);   // Stop if the delegate closed the connection
        });
      }
      if (blockConnection != blockSelf->_connection) return;    // Connection closed in the meantime
      if (error != nil) {[blockSelf connectionBroke:blockConnection error:error]; return;}
      if (isComplete) {[blockSelf remoteClosed:blockConnection]; return;}
      [blockSelf receiveOn:blockConnection];                     // Wait for more data
    });
}


// The connection could not be established, or broke down: cancel it and inform the delegate
- (void)connectionBroke:(nw_connection_t)connection error:(nw_error_t)error {
  tcpSocketState_t newState = (_state == TCP_SOCKET_CONNECTED) ? TCP_SOCKET_LOST : TCP_SOCKET_FAILED;
  NSString *reason = [self reasonForError:error];
  _connection = nil;
  nw_connection_cancel(connection);
  [self discardPendingData];
  [self setState:newState reason:reason];
}


// The remote side closed the connection
- (void)remoteClosed:(nw_connection_t)connection {
  _connection = nil;
  nw_connection_cancel(connection);
  [self discardPendingData];
  [self setState:TCP_SOCKET_CLOSED reason:@"closed by remote side"];
}


// Send the data that was kept while the connection was being established (see sendBytes:length:)
- (void)sendPendingData {
  if (_pendingData == nil) return;
  NSMutableData *data = _pendingData;
  _pendingData = nil;
  [self sendBytes:[data bytes] length:[data length]];
  [data release];
}


- (void)discardPendingData {
  [_pendingData release];
  _pendingData = nil;
}


- (NSString *)reasonForError:(nw_error_t)error {
  _lastErrorCode = 0;
  if (error == nil) return @"unknown reason";
  int code = nw_error_get_error_code(error);
  if (nw_error_get_error_domain(error) == nw_error_domain_posix) {
    _lastErrorCode = code;
    return [NSString stringWithUTF8String:strerror(code)];
  }
  return [NSString stringWithFormat:@"network error %d", code];
}


- (void)dealloc {
  [_pendingData release];
  [super dealloc];
}

@end
