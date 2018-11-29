#pragma once

class Packet
{
public:
	Packet(double arrival_time, int packet_size, int destination);
	~Packet();

	double arrival_time;
	int packet_size;
	int destination;
};

