#include "common.h"

#ifdef SERVER
#include <vector>
#include "packets.h"

ENetHost *serverHost = NULL;
Client* clients[PLAYER_PER_TEAM*2];
int countClients = 0;
bool waitingPlayers;

Cvar* dedicated;
Cvar* version;
Cvar* developper;
Cvar* s_masterserver;
Cvar* g_gravity;

bool is_in_game(int idUser)
{
	for (int i = 0; i < PLAYER_PER_TEAM * 2; i++)
	{
		if (clients[i] != NULL)
		{
			if (clients[i]->id == idUser)
			{
				return true;
			}
		}
	}
	return false;
}

void fatal_error(char* error)
{
	printf(error);
	Server_Cleanup();
	exit(EXIT_FAILURE);
}

void Server_Send_Game_State()
{

}

Client* add_client()
{
	Client* c = NULL;
	for (int i = 0; i < PLAYER_PER_TEAM * 2; i++)
	{
		if (clients[i] == NULL)
		{
			c = new Client;
			c->clientNumber = i;
			c->authed = false;
			clients[i] = c;
			return c;
		}
	}
	return NULL;
}

void Server_Disconnect_Player(int clientNumber, int reason)
{
	Client c = *clients[clientNumber];
	enet_peer_disconnect(c.peer, reason);
}

void Server_Send_To(ENetPeer* peer, const uint8 *source, uint32 length, uint8 channelNo, uint32 flag)
{
	uint8* data = new uint8[length];
	memcpy(data, source, length);

	ENetPacket *packet = enet_packet_create(data, length, flag);
	if (enet_peer_send(peer, channelNo, packet) < 0)
	{
		delete[] data;
		Print_Error(1, "[NETWORK]Error while sending packet");
	}

	delete[] data;
}

void Server_Send_Infos(int clientNumber)
{
	PacketHeader packet(1);
	Server_Send_To(clients[clientNumber]->peer, reinterpret_cast<uint8 *>(&packet), sizeof(packet), CHL_S2C, ENET_PACKET_FLAG_RELIABLE);
	Print("Send infos");
}

void Server_Frame()
{
	ENetEvent event;
	bool eventProcessed = false;
	while (!eventProcessed)
	{
		if (enet_host_check_events(serverHost, &event) <= 0)
		{
			if (enet_host_service(serverHost, &event, 5) <= 0)
				break;
			eventProcessed = true;
		}
		switch (event.type)
		{
			case ENET_EVENT_TYPE_CONNECT:
			{
				Print("Client connected, waiting for informations");
				Client* c = add_client();
				if (c == NULL)
				{
					Print("Can't accept anymore connection");
					enet_peer_disconnect(event.peer, DISCONNECT_MAXCLIENTS);
					break;
				}
				c->peer = event.peer;
				c->peer->data = (void*)c->clientNumber;
				break;
			}

			case ENET_EVENT_TYPE_RECEIVE:
			{
				Print("Event : packet");
				Server_Handle_Packet(event.packet, (int)event.peer->data, event.channelID);
				break;
			}

			case ENET_EVENT_TYPE_DISCONNECT:
			{
				Print("Event : disconnect");
				break;
			}

			default:
				break;
		}
	}
	Server_Send_Game_State();
}

void Server_Handle_Packet(ENetPacket* packet, int client, int chan)
{
	Print("Received packet form %d on chan %d", client, chan);
	PacketHeader *header = reinterpret_cast<PacketHeader*>(packet->data);

	switch(header->cmd)
	{
	case C2S_CHECK:
		PacketChecking *pkt = reinterpret_cast<PacketChecking*>(packet->data);
		if (pkt->version != VERSION)
		{
			//Send packet wrong version
		}

		if (waitingPlayers)
		{
			if (is_in_game(pkt->idUser))
			{
				//Send packet already in game
			}
			clients[client]->id = pkt->idUser;
			clients[client]->authed = true;
			/* Send informations :
			- Map
			- Team
			- How much players
			*/
		}
		break;
	}
}

bool Server_Init(int argc, char** argv)
{
	/*
	Parser les arguments
	*/
	char* date = __DATE__;
	char* version_str = (char*)malloc(strlen(PRODUCT_NAME " client 1 (%s)") + 1);
	
	dedicated = Cvar_Set("dedicated", "1", CVAR_READ_ONLY, "If this is dedicated server");

	developper = Cvar_Set("developer", "1", CVAR_READ_ONLY, "If the developper mod is on");
	g_gravity = Cvar_Set("g_gravity", "800", CVAR_CHEATS, "Gravity of the game");

	Command_Exec("exec server.cfg");

	ENetAddress address;
	address.host = ENET_HOST_ANY;
	address.port = PORT_SERVER_DEFAULT;

	serverHost = enet_host_create(&address, PLAYER_PER_TEAM*2, 3, 0, 0);
	if (!serverHost)
	{
		fatal_error("cannot start the server");
	}

	atexit(enet_deinitialize);
	atexit(Server_Cleanup);
	enet_time_set(0);

	for (;;)
	{
		Server_Frame();
		//Console_Frame();
	}
		
	return true;
}

void Server_Cleanup(void)
{
	enet_host_destroy(serverHost);
}

int main(int argc, char** argv)
{
	/*
	Set mode
	Load map
	Wait for everyone
	Wait for pick
	Start
	*/
	if (enet_initialize() < 0)
	{
		fatal_error("Error while starting enet");
	}

	Console_Init();
	Cvar_Init();
	Command_Init();

	Server_Init(argc, argv);
}

#endif