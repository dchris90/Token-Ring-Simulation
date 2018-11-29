#include "RandomGenerator.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <iostream>
using namespace std;

RandomGenerator::RandomGenerator()
{
	srand((unsigned)time(NULL));
}

RandomGenerator::~RandomGenerator()
{
}

// run a simulation for each lambda in the lambda array
void RandomGenerator::SetLambda(double lambda)
{
	this->lambda = lambda;
}

// set number of servers via user input
void RandomGenerator::SetServerCount(int server_count)
{
	this->server_count = server_count;
}

// for each packet, assign it a random server from (0, ... , N-1)
int RandomGenerator::GetRandomServer()
{
	return (int)((server_count - 1) * ((double)rand()) / RAND_MAX);
}

// for each packet, assign it a random arrival rate to a server
double RandomGenerator::GetRandomArrivalTime()
{
	double u = ((double)rand()) / (RAND_MAX + 1);
	return (-1.0 / lambda) * log(1 - u);
}

// for each packet, assign it a random size distributed in [64, 1518] bytes
int RandomGenerator::GetRandomPacketSize()
{
	return 64 + (int)((1518 - 64) * ((double)rand()) / RAND_MAX);
}
