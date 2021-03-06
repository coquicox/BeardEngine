#include "common.h"
#include "client.h"
#include "client_input.h"
#include "packets.h"

#ifndef SERVER

ENetHost *hostClient = NULL;
ENetPeer *peerClient = NULL;
int connectionAttemps = 0;
unsigned int timeoutTimer = 0;
unsigned int inputSent = 0;
bool connecting = false, connected = false;

void Client_Send_Input(int actualTime)
{
	if (!connected)
		return;
	if (actualTime - inputSent > 33)
	{
		//Send input
		PacketInput pkt = PacketInput();
		Client_Send(reinterpret_cast<uint8*>(&pkt), sizeof(PacketInput), CHL_C2S, ENET_PACKET_FLAG_RELIABLE);
		inputSent = actualTime;
	}
}

void Client_Init()
{
	if (enet_initialize() != 0)
	{
		Print_Error(1, "Cannot start enet");
		exit(1);
	}

	Command_Add("connect", Client_Connect_f);

	atexit(Client_Cleanup);
	atexit(enet_deinitialize);

	Client_Init_Input();
	Print("client initialized");
}

void Client_Connect(const char* serverAddress, int port)
{
	if (port <= 0)
	{
		port = PORT_SERVER_DEFAULT;
	}
	
	ENetAddress address;
	address.port = port;

	if (!serverAddress)
		return;
	enet_address_set_host(&address, serverAddress);

	if (!hostClient)
		hostClient = enet_host_create(NULL, 2, 3, 0, 0);

	if (hostClient)
	{
		peerClient = enet_host_connect(hostClient, &address, 3, 0);
		enet_host_flush(hostClient);
		connecting = true;
		connectionAttemps = 0;
		timeoutTimer = SDL_GetTicks() + 3000;
		Print("Trying to connect to %s:%d", serverAddress, port);
	}
	else
	{
		Print_Error(1, "Cannot connect to the server");
	}
}

void Client_Abort_Connection()
{
	if (!peerClient)
		return;
	if (peerClient->state != ENET_PEER_STATE_DISCONNECTED)
		enet_peer_reset(peerClient);
	peerClient = NULL;
	connecting = false;
}

void Client_S2C()
{
	if (!hostClient || (!connecting && !peerClient))
		return;
	if (connecting && timeoutTimer < SDL_GetTicks())
	{
		Print("Retrying to connect...");
		timeoutTimer = SDL_GetTicks() + 3000;
		connectionAttemps++;
		if (connectionAttemps > 3)
		{
			Print("Cannot connect to the server");
			Client_Abort_Connection();
			return;
		}
	}

	ENetEvent event;

	while (enet_host_service(hostClient, &event, 0) > 0)
	{
		switch (event.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
		{
			connecting = false;
			connected = true;
			Client_Send_Request_Connect();
			Print("Connected");
		}
			break;
		case ENET_EVENT_TYPE_RECEIVE:
			Client_Handle_Packet(event.channelID, event.packet);
			//enet_packet_destroy(event.packet);
			break;

		case ENET_EVENT_TYPE_DISCONNECT:
			Print("Connection lost.\n", event.peer->data);
			connected = false;
			connecting = false;
			/* Reset the peer's client information. */
			event.peer->data = NULL;
		}
	}
}

void Client_Send_Request_Connect()
{
	PacketChecking packet = PacketChecking();
	packet.version = VERSION;
	packet.idUser = 123456;
	Client_Send(reinterpret_cast<uint8 *>(&packet), sizeof(packet), CHL_C2S, ENET_PACKET_FLAG_RELIABLE);
	Print("Sending request");
}

void Client_Handle_Packet(enet_uint8 chanel, ENetPacket* packet)
{
	PacketHeader *header = reinterpret_cast<PacketHeader*>(packet->data);
	switch (header->cmd)
	{
	case S2C_GAMEINFOS:
	{
		PacketSendGameInfos *pkt = reinterpret_cast<PacketSendGameInfos*>(packet->data);

		delete pkt;
	}
		break;

	case S2C_WAITFORSTART:
		Print("Waiting for game starting...");
		break;

	case S2C_SNAPSHOT:
	{
		PacketSnapshot *pkt = reinterpret_cast<PacketSnapshot*>(packet->data);
		Print("Snapshot received. Server time : %d\n Entity count : %d", pkt->s.serverTime ,pkt->s.countEntity);
		//Client_Unpack_Snapshot(pkt->snapshot);
		delete pkt;
	}
		break;

	default:
		Print("Cannot handle packet received on chanel %d with packet id %d", chanel, header->cmd);
		break;
	}
}

void Client_Send(const uint8 *source, uint32 length, uint8 channelNo, uint32 flag)
{
	if (!connected)
		return;

	uint8* data = new uint8[length];
	memcpy(data, source, length);

	ENetPacket *packet = enet_packet_create(data, length, flag);
	if (enet_peer_send(peerClient, channelNo, packet) < 0)
	{
		delete[] data;
		delete packet;
		Print_Error(1, "[NETWORK]Error while sending packet");
	}

	delete[] data;
}

void Client_Disconnect()
{
	if (peerClient)
	{
		enet_peer_disconnect(peerClient, DISCONNECT);
		enet_host_flush(hostClient);
		enet_peer_reset(peerClient);
	}
}

void Client_Cleanup()
{
	Client_Abort_Connection();
	Client_Disconnect();
	if (hostClient)
	{
		enet_host_destroy(hostClient);
		hostClient = NULL;
	}
}

void Client_Connect_f()
{
	if (Command_Argc() == 1)
	{
		Print("usage : connect <host> [port]");
		return;
	}
	else if (Command_Argc() == 2)
	{
		Client_Connect(Command_Argv(1), 0);
	}
	else if (Command_Argc() >= 3)
	{
		Client_Connect(Command_Argv(1), atoi(Command_Argv(2)));
	}
}

#endif
