#pragma once

class RandomGenerator
{
public:
	RandomGenerator();
	~RandomGenerator();

	void SetLambda(double lambda);
	void SetServerCount(int server_count);
	int GetRandomServer();
	double GetRandomArrivalTime();
	int GetRandomPacketSize();

private:
	double lambda;
	int server_count;
};

