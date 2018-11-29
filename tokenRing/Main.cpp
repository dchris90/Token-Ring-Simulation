/*
IMPLEMENTATION LOGIC: See documentation.
*/


#include <fstream>
#include <iostream>
#include "Server.h"
using namespace std;


/*
Helper Function: 
Get the next available arrival time of a packet, 
which may be far off into the future.

Used in the [if (no_tx_count == N hosts)] block
*/
double GetMinPacketArrivalTime(vector<Server*> srvs)
{
	double min_time = srvs[0]->GetFirstPacketArrivalTime();

	for (int i = 1; i < srvs.size(); i++)
	{
		if (min_time > srvs[i]->GetFirstPacketArrivalTime())
		{
			min_time = srvs[i]->GetFirstPacketArrivalTime();
		}
	}

	return min_time;
}

int main()
{
	double PROPAGATION_DELAY = 0.00001;		// 10 microseconds -> 0.00001 seconds
	double TRANSMISSION_RATE = 100000000;	// 100 Mbps -> 100000000 bits
	int TOKEN_SIZE = 24;					// not given; let it be 24 bits

	int N;							// N hosts
	int max_packets;				// sim stops when (max_packets * N servers) 
									// packets have been sent. reason: want token
									// to fully circulate at least once
	vector<Server*> servers;		// a number in (0, ..., n - 1), N hosts total
	RandomGenerator rnd;			// generate the packet's random 
									// size/destination/arrival time

	// lambda packets per second
	double lambdas[] = { 0.01, 0.05, 0.1, 0.2, 0.3, 0.5, 0.6, 0.7, 0.8, 0.9 };

	cout << "Set number of servers in the ring: ";
	cin >> N;
	rnd.SetServerCount(N);

	cout << "Set packet count per server to simulate. "
		<< "Terminates when (packetCount * N hosts) are sent. ";
	cin >> max_packets;
	
	cout << endl << endl;

	// run simulation for each lambda, there are 10 lambda's given
	for (int n = 0; n < 10; n++)
	{
		double sim_time = 0.0;
		int total_bytes_count = 0;	// accumulates bytes sent
		double total_delay = 0;		// accumulates the delays to send pkt/token
		int total_packets = 0;		// accumulates packets sent
		int no_tx_count = 0;		// if threshold reaches (no_tx_count == N), 
									// speed up simulation 
									// (otherwise program runs for long time)
									// by jumping to sim_time of next arrival

		cout << "[Initialization]" << endl;

		double lambda = lambdas[n];
		rnd.SetLambda(lambda);

		/* 
		Initialization:

		Create a ring of N servers
		For each server, initiate one packet (will arrive to server at random arrival_time)
		For each packet, assign it a random arrival time, destination, size
		*/
		for (int i = 0; i < N; i++)
		{
			Server *ts = new Server(i, max_packets, &rnd);
			ts->Initialize();
			servers.push_back(ts);
		}

		// Initialization: Randomly select a server to hold the token
		int curr_token_owner = rnd.GetRandomServer();

		// Initialization complete; simulation loop begins
		// comments part 1 of 4
		cout << "\n\n[initialization complete. 1 packet has been "
			<< "initialized at each of the " << N << " servers]" << endl;
		cout << "[simulation terminates when a total of (max_packets * N hosts) = " 
			<< max_packets * N << " packets have been sent]" << endl;

		while (true)
		{
			// Get the server that is holding the token
			Server *srv_token_owner = servers[curr_token_owner];

			// Check if this server has pkts to send; grab pkts in its queue
			vector<Packet> packets_in_queue = srv_token_owner->GetPacketsInFifoAtCurrTime(sim_time);

			/*
			This token_owner has packets to send because its queue is 
			non-empty at this current sim_time.
			
			Then, for each packet in its queue, accumulate these: 
			bytes sent, delays, sim_time, packets sent.

			Behind the scenes [see GetPacketsInFifoAtCurrTime() ]:
			(1) token_owner pushes incoming packet(s) assigned to it 
			(or passing through it) to its FIFO queue, and processes the 
			packet by assigning it a random destination, size, and arrival time.
			(2) token_owner pops packets off queue, transmits to destination host
			*/
			if (packets_in_queue.size() > 0)
			{
				int bytes_to_send_size = 0; 

				for (int i = 0; i < packets_in_queue.size(); i++)
				{
					// comments part 2 of 4
					/*
					cout << "\ntoken_owner "
						<< srv_token_owner->GetId() 
						<< " has just transmitted a packet to Host "
						<< packets_in_queue[i].destination << endl;
					cout << ". " << endl;
					cout << ".  [token passing in progress..."
						<< "waiting for next token_owner with data to send]" << endl;
					cout << ".";
					*/

					// Accumulate this packet's bytes
					bytes_to_send_size += packets_in_queue[i].packet_size;

					// number of links between token_owner and packet's destination
					int hop_count = packets_in_queue[i].destination - srv_token_owner->GetId();

					// If hop_count is negative, make it the positive negation of itself
					// hop_count may be positive or negative depending on the direction 
					// the packet is going. 
					// (E.g., from server #2 to server #0 gives (-2) hops)
					if (hop_count < 0)
					{
						hop_count += N;
					}

					// Accumulate delays of sending data to destination
					//	dQueue is the amount of time this pkt has been sitting in queue
					//	dTrans and dProp are dependent on hop_count
					//	packet size is in bytes; need to convert it to bits; 1 byte = 8 bits
					total_delay += (sim_time - packets_in_queue[i].arrival_time) + hop_count * (PROPAGATION_DELAY + 8.0 * packets_in_queue[i].packet_size / TRANSMISSION_RATE);

					// accumulate packets sent
					total_packets++;
				}

				// Accumulate total bytes (sum of [bytes_to_send_size] for each pkt)
				total_bytes_count += bytes_to_send_size;

				// Accumulate simulation time for each packet's round trip
				// Every single packet sent from token owner must return so that 
				// the token owner can release the token.
				// dTrans is alraedy in bits, need to convert packet's bytes to bits
				sim_time += N * (PROPAGATION_DELAY + 8.0 * bytes_to_send_size / TRANSMISSION_RATE);

				// Reset no_tx_count counter to 0 since server had something to send
				no_tx_count = 0;

				// Error checking: Check if all packets were sent?
				if (total_packets >= N * max_packets)
				{
					break;
				}
			} 
			else
			{
				// this token_owner has no data to send because its queue is empty 
				// at this current sim_time
				no_tx_count++;
			}

			/*
			Help speed up simulation by jumping to nearest sim_time a token_owner has data to send

			if (no_tx_count == N), then the token has circled the ring without
			anybody sending, and this means that the next arrival_rate of a packet
			is probably not near the current sim_time. 

			Then, to improve program speed, calculate the number of rounds the token will 
			need to traverse before a token_owner has something to send, jump to new time, 
			and accumulate the sim_time accordingly. Delays are not accumulated for since we
			are just transferring the token.
			*/
			if (no_tx_count == N)
			{
				// reset counter 
				no_tx_count = 0;

				// get time of first packet that simulation time arrives to 
				double next_packet_time = GetMinPacketArrivalTime(servers);

				// calculate number of rounds before simulation hits next_packet_time
				int m = (int)((next_packet_time - sim_time) / (N * (PROPAGATION_DELAY + TOKEN_SIZE / TRANSMISSION_RATE)));

				if (m > 0) // m can be quite large
				{
					// Make fast jump in simulation time
					sim_time += m * N * (PROPAGATION_DELAY + TOKEN_SIZE / TRANSMISSION_RATE);
				}

				else
				{
					// Make slow jump in simulation time (proceed like before)

					// Move token to next server
					curr_token_owner = (curr_token_owner + 1) % N;

					// Increment simulation time
					sim_time += PROPAGATION_DELAY + TOKEN_SIZE / TRANSMISSION_RATE;
				}
			}
			else
			{
				/*
				If there is no need to speed up simulation at this time (no_tx_count != N), 
				then just proceed as normal: 

				Pass the token.
				Accumulate simulation time with respect to passing the token.
				*/

				// Move token to next server
				curr_token_owner = (curr_token_owner + 1) % N;

				// Increment simulation time (only token is sent)
				sim_time += PROPAGATION_DELAY + TOKEN_SIZE / TRANSMISSION_RATE;
			}
		}

		// Print results
		cout << "\n*** End of simulation for lambda = " << lambda << " -> max # packets sent of " << total_packets << " reached." << endl;
		cout << "\n** RESULTS **";
		double throughput = ((double)total_bytes_count) / sim_time;
		double average_delay = total_delay / total_packets;
		cout << endl << "lambda = " << lambda << " :" << endl << "Throughput = " << throughput << endl;
		cout << "Average packet delay = " << average_delay << endl << endl << endl;

		// Prepare for next lambda value
		servers.clear();
	}

	char temp[10];
	cout << endl << "Type anything and press ENTER to exit simulation ..." << endl;
	cin >> temp;

	return 0;
}