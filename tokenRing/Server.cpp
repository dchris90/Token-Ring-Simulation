#include "Server.h"
#include <iostream>
using namespace std;

Server::Server(int id, int max_packets, RandomGenerator *rnd)
{
	this->id = id;
	this->max_packets = max_packets;
	this->rnd = rnd;
}

Server::~Server()
{
}

// during initialization:
// initialize each server with 1 packet before simulation begins
// simulation will run until a total of (max_count * N servers) packets are sent
void Server::Initialize()  
{
	CreatePacketAndAddToFifo();
}

/*
Current token_owner gets packet(s) in FIFO queue at current sim_time.

At the current sim_time, the current token_owner will send all packets in queue:
	(1) token_owner pushes incoming packet(s) assigned to it (or passing through it)
		to its FIFO queue, and processes the packet by assigning it a random destination,
		size, and arrival time.
	(2) token_owner pops packets off queue, transmits to destination host

This process repeats indefinitely (e.g., next host becomes token owner, 
and so on and so forth), until simulation reaches the threshold of the number of packets sent.
*/
vector<Packet> Server::GetPacketsInFifoAtCurrTime(double sim_time)
{
	vector<Packet> packets;

	// 1 -- Add new packets to FIFO buffer and process it
	while (generated_packet_count < max_packets && fifo_buffer.back().arrival_time < sim_time)
	{
		//cout << "\n[time " << sim_time << "] "; // comments part 3 of 4
		CreatePacketAndAddToFifo();
	}

	// 2 -- Get packets in queue
	while (fifo_buffer.size() > 0 && fifo_buffer.front().arrival_time <= sim_time)
	{
		packets.push_back(fifo_buffer.front()); 
		fifo_buffer.pop(); 
	}

	// return the packets to be transmitted
	return packets;
}

// get queue's (front) packet's arrival time
double Server::GetFirstPacketArrivalTime()
{
	if (fifo_buffer.size() > 0)
	{
		return fifo_buffer.front().arrival_time;
	}
	else
	{
		return INT_MAX;
	}

}

/*
Add new packets to FIFO buffer

For this packet in this server
	assign it a random arrival_time that will arrive at the server
	assign it a random size in the interval [64, 1518] bytes
	assign it a random destination from (0, ..., N - 1) to send it to

	Create the packet with those values

	Push the packet into server's FIFO buffer
*/
void Server::CreatePacketAndAddToFifo()
{
	int destination;

	double arrival_time = rnd->GetRandomArrivalTime();

	if (!fifo_buffer.empty())
	{
		arrival_time += fifo_buffer.back().arrival_time;
	}

	int packet_size = rnd->GetRandomPacketSize();

	while ( (destination = rnd->GetRandomServer()) == id );

	Packet pkt(arrival_time, packet_size, destination);

	// push packet to end of queue
	// (X1, first to arrive, front), X2, X3, ... , (Xn, newly pushed, back)
	fifo_buffer.push(pkt); 
	
	generated_packet_count++;

	// comments part 4 of 4
	//cout << "\ntoken_owner " << id << " has a new incoming packet to arrive on " << arrival_time
		//<< ", to be sent to Host " << destination;
}

int Server::GetId()
{
	return id; // return a number from (0, ... , N-1)
}
