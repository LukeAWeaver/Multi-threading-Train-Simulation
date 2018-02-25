main: main.cpp 
	g++ main.cpp -o main -g -std=c++11 -Wall -lpthread
	
clean:
	rm main	

