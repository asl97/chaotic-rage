// This file is part of Chaotic Rage (c) 2010 Josh Heidenreich
//
// kate: tab-width 4; indent-width 4; space-indent off; word-wrap off;

#pragma once
#include <iostream>
#include <ios>
#include <iomanip>
#include <SDL.h>
#include "rage.h"


enum NetMsgType {
	NOTHING =       0x00,
	
	INFO_REQ =      0x01,		// [C] Req for server info
	INFO_RESP =     0x02,		// [S] Server info  (map, gametype, etc)
	
	JOIN_REQ =      0x03,		// [C] Join request
	JOIN_OKAY =     0x04,		// [S] Join accepted
	JOIN_DENY =     0x05,		// [S] Join denied
	JOIN_ACK =      0x06,		// [C] Client ack of join accept, server to send data
	JOIN_DONE =     0x07,		// [S] Sending of data is complete
	
	CHAT_REQ =      0x08,		// [C] Client -> server w/ chat msg
	CHAT_MSG =      0x09,		// [S] Server -> all w/ chat msg
	
	CLIENT_STATE =  0x0A,		// [C] Keyboard (buttons) and mouse (buttons, position) status -> server
	
	UNIT_ADD =      0x0B,		// [S] A unit has been added
	UNIT_UPDATE =   0x0C,		// [S] Unit params have changed
	UNIT_REM =      0x0D,		// [S] Unit has been removed
	
	WALL_UPDATE =   0x0E,		// [S] Wall params have changed
	WALL_REM =      0x0F,		// [S] Wall has been removed
	
	PG_ADD =        0x10,		// [S] Particle Generator has been added
	PG_REM =        0x11,		// [S] Particle Generator has been removed
	
	PLAYER_DROP =   0x12,		// [S] Player dropped. too laggy -> all clients
	QUIT_REQ =      0x13,		// [C] Player want's to leave -> server
	PLAYER_QUIT =   0x14,		// [S] Player has left -> all clients
	
	BOTTOM =        0x15,		// -- not really a message --
};

extern int msgtype_len [];

class NetMsg {
	public:
		Uint8 type;
		Uint8 *data;
		unsigned int size;
		unsigned int seq;
		
	public:
		NetMsg (NetMsgType type, unsigned int size);
		~NetMsg();
};


void dumpPacket(Uint8* data, int size);


