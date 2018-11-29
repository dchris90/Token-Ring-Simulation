#include "Packet.h"
#include <iostream>
using namespace std;

Packet::Packet(double arrival_time, int packet_size, int destination)
{
	this->arrival_time = arrival_time;
	this->packet_size = packet_size;
	this->destination = destination;
}

Packet::~Packet()
{
}
