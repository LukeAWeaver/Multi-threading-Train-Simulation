# Train Simulator
Train Simulator is a C++ application that simulates the traffic between train stations over the course of a day. This allows us to visualize potential traffic trains may experience on a bidirectional track. After the simulation is complete the results are printed!

# How does it work?
The program is given N amount of trains and M amount of stations in a text file; followed by each trains path for the day. All trains move at the same speed, the distance is identical between stations, and it takes a train T seconds to arrive to a train station. N number of threads fire off and each worker thread represents a train on their path. To avoid two or more trains utilizing the same track at the same time I utilized barriers, Mutex, and semaphores.

# Who is it for?
Train conductors and train enthusiast. 

# Where can I get it?
Right here! (may need to zoom in to see I/O examples)

![Infographic](https://github.com/LukeAWeaver/Multi-threading-Train-Simulation/blob/master/output.png)
