#pragma once

#include "Packet.h"
#include "RandomGenerator.h"
#include <queue>
#include <vector>

using namespace std;

class Server
{
public:
	Server(int id, int max_packets, RandomGenerator *rnd);
	~Server();
	void Initialize();
	vector<Packet> GetPacketsInFifoAtCurrTime(double sim_time);
	double GetFirstPacketArrivalTime();
	int GetId();

private:
	int id;
	int max_packets;
	int generated_packet_count;
	RandomGenerator *rnd;
	queue<Packet> fifo_buffer;

	void CreatePacketAndAddToFifo();
};

