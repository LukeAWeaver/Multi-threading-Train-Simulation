#include <iostream>       // std::cout
#include <thread>         // std::thread//:wq to save
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include "Barrier.h"
#include <mutex>
#include <condition_variable>
#include <cassert>


using namespace std;

Barrier b;
mutex coutMutex;
mutex cs;
condition_variable cv;
vector<vector<string>> movesDuringTimeStep;
volatile int activeTrains;
volatile int syncInt;
int nTrains;
volatile unsigned int activeThreads;
vector<int> finishTime;
class getInfo
{
	string textFile;
	string line;
	vector<string> args; //contains all arguments
	vector<string> trainPath; //temp, contains a train path
	vector<vector<string>> trainPaths; //contains train paths

public:
	getInfo(string file)
	{
		textFile = file;
	}
	vector<vector<string>> readStuff()
	{
		return trainPaths;
	}
	int getNumberOfTrains()
	{
		return nTrains;
	}
	void parseFile()
	{
		ifstream myfile(textFile);
		if (myfile.is_open())
		{
			int input = -1;
			int lineCounter = -2;
			while (getline(myfile, line))
			{
				string tempArg = "";
				string tempTrainStation = "";
				int tempTrainStationint = -1;
				for (unsigned int characterCounter = 0; characterCounter < line.length(); characterCounter++) {
					if (line[characterCounter] != ' ') {
						tempArg = tempArg + line[characterCounter];
						tempTrainStation = tempTrainStation + line[characterCounter];
					}
					if (line[characterCounter] == ' ' || characterCounter == line.length() - 1) {
						input++;
						args.resize(input);
						args.insert(args.begin(), tempArg);
						//cout << "argument detected: " << args[0] << "\n";
						tempArg = "";
					}
					if ((line[characterCounter] == ' ' || characterCounter == line.length() - 1) && lineCounter >= -1) {
						tempTrainStationint++;
						trainPath.resize(tempTrainStationint);
						trainPath.insert(trainPath.begin(), tempTrainStation);
						tempTrainStation = "";
					}
				}
				lineCounter++;
				if (lineCounter >= 0)
				{
					reverse(trainPath.begin(), trainPath.end());

					trainPaths.resize(lineCounter);
					trainPaths.insert(trainPaths.begin(), trainPath);
					//cout << "Train paths: " << trainPaths[0][0] << "\n"; //access like a 2d array
				}
			}
			reverse(args.begin(), args.end());
			reverse(trainPaths.begin(), trainPaths.end());
			nTrains = stoi(args[0]);
			myfile.close();
			//cout << "total number of arguments:  " << args.size() << '\n';
			//cout << "number of trains: " << nTrains <<"\n";
			//cout << "Number of Train paths: " << trainPaths.size() << "\n";
		}
		else cout << "Unable to open file";
	}
};

vector<string> setTrainNames(int nTrains)
{
	vector<string> returnVec;
	returnVec.resize(nTrains);
	char trainName = 64;
	for (int i = 0; i < nTrains; i++)
	{
		trainName++;
		returnVec[i] = trainName;
	}
	return returnVec;
}

void workerThread(vector<string> trainNames, vector<vector<string>> trainPath, int identifier)
{
	//variables only accessable by the current thread
	int timeStep = 0;
	unsigned int trackProgress = 0;
	bool trainCannotDepart;
	vector<string> moveAttempt(2);
	//shared variables
	activeThreads = activeTrains;
	//wait for all thread to initialize
	while (trackProgress < trainPath[identifier].size() - 2)
	{
		moveAttempt.clear();
		movesDuringTimeStep.clear();
		activeThreads = activeTrains;
		b.barrier(activeTrains); //syncing
		activeThreads = activeTrains;
		movesDuringTimeStep.resize(activeThreads); //resizes for amount of moves to store
		moveAttempt.insert(moveAttempt.begin(), trainPath[identifier][trackProgress + 1]);
		moveAttempt.insert(moveAttempt.begin(), trainPath[identifier][trackProgress + 2]);
		b.barrier(activeThreads); //synced
		trainCannotDepart = false;
		cs.lock();
		for (unsigned int i = 0; (i < activeThreads); i++) //try to store move or check if can depart 
		{
			if (movesDuringTimeStep[i].empty()) //if index is empty, store vector and break
			{
				movesDuringTimeStep.insert(movesDuringTimeStep.begin(), moveAttempt);
				break;
			}
			else if ((moveAttempt[0] == movesDuringTimeStep[i][0] && moveAttempt[1] == movesDuringTimeStep[i][1]) || (moveAttempt[1] == movesDuringTimeStep[i][0] && moveAttempt[0] == movesDuringTimeStep[i][1])) //track in use
			{
				trainCannotDepart = true;
				break;
			}
		}
		coutMutex.lock();
		if (trainCannotDepart)
		{
			cout << "At time step: " << timeStep << " train " << trainNames[identifier] << " must stay at station " << moveAttempt[1] << "\n";
		}
		else
		{
			cout << "At time step: " << timeStep << " train " << trainNames[identifier]
				<< " is going from station " << moveAttempt[1] << " to station " << moveAttempt[0] << "\n";
			trackProgress++;
		}
		cs.unlock();
		coutMutex.unlock(); 
		if (trackProgress == trainPath[identifier].size() - 2)
		{
			activeTrains--;
			finishTime[identifier] = timeStep;
		}
		timeStep++;
		b.barrier(activeThreads); // wait for all threads to arrive, let program know amount of threads for next loop

	}
}

int main(int argc, char** argv) {
	getInfo getInfoObject(argv[1]);
	getInfoObject.parseFile();
	nTrains = getInfoObject.getNumberOfTrains();
	vector<string> trainNames = setTrainNames(nTrains); //holds train names
	vector<vector<string>> trainPaths = getInfoObject.readStuff(); //hold train paths
	thread** t = new thread*[nTrains];
	activeTrains = nTrains;
	finishTime.resize(nTrains);
	syncInt = 0;

	cout << "Starting simulation...\n";
	for (int threadNum = 0; threadNum < nTrains; threadNum++)
	{
		t[threadNum] = new thread(workerThread, trainNames, trainPaths, threadNum);
	}
	for (int threadNum = 0; threadNum < nTrains; threadNum++)
	{
		t[threadNum]->join();
		delete t[threadNum];
	}
	cout << "Simulation complete. \n\n";
	for (int identifier = 0; identifier < nTrains; identifier++)
	{
		cout << "Train " << trainNames[identifier] << " completed its route at time step " << finishTime[identifier] << "\n";
	}
	//delete[] t;
	return 0;
}
