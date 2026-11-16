🌐 Network Failure Recovery Simulator

A Data Structures and Algorithms-based network simulation system built using C, Graph Algorithms, and HTTP communication. The project simulates router networks, detects connectivity failures, and determines alternative communication paths when a router or network connection becomes unavailable.

⸻

📌 Overview

In a real-world computer network, routers are interconnected to facilitate communication between different devices and networks. However, when a router fails or a communication link becomes defective, certain paths may become unavailable, potentially disrupting network connectivity.

The Network Failure Recovery Simulator addresses this scenario through a simulated router network.

It allows users to represent an existing or hypothetical router network, simulate router and connection failures, and analyze how the network should be reconnected to maintain communication between routers.

Using graph-based algorithms and fundamental data structures, the system identifies alternative paths and demonstrates network recovery strategies.

Important: This is a simulation-based project, not a physical network management or router configuration tool. It does not automatically detect, configure, or repair real-world routers.

⸻

⚠️ Prerequisite: Router Network

A router network must be represented in the system before performing network failure simulations.

Users must provide or create a network consisting of:

* Multiple routers (network nodes).
* Connections between routers (graph edges).
* Connection weights representing communication cost or distance.

The simulator uses this network representation as its input.

Once the network is established, users can simulate scenarios such as:

* A router becoming unavailable.
* A connection or communication path becoming defective.
* A previously available route becoming inaccessible.
* Communication between two routers being disrupted.

The system then analyzes the remaining network and determines possible alternative communication paths.

The purpose is to simulate how routers should remain connected or be reconnected when a router or communication link fails.

⸻

🎯 Objectives

* Understand real-world router connectivity through graph representation.
* Apply fundamental Data Structures and Algorithms to network problems.
* Simulate router and connection failures.
* Identify alternative communication paths.
* Demonstrate shortest-path and graph traversal algorithms.
* Maintain network event history.
* Explore basic network recovery strategies.

⸻

✨ Key Features

1. Router Network Representation

* Represent routers as vertices in a graph.
* Represent communication links as weighted edges.
* Maintain network connectivity using an adjacency matrix.

2. Router Management

* Add and remove routers.
* Maintain router information.
* Update the network structure.

3. Connection Management

* Establish connections between routers.
* Assign weights to communication links.
* Remove defective or unavailable connections.

4. Failure Simulation

* Simulate individual router failures.
* Simulate connection failures.
* Analyze the effect of failures on network connectivity.

5. Network Recovery Analysis

* Identify alternative paths between routers.
* Calculate shortest paths using Dijkstra’s algorithm.
* Explore network traversal using BFS.
* Determine whether communication remains possible after a failure.

6. Event History

* Maintain a record of network operations.
* Track failure and recovery-related events.
* Support event-history traversal.

7. Undo Operations

* Use stack-based functionality to support undo operations where implemented.

8. Persistent Storage

* Store router information.
* Store network connections.
* Maintain event records using text files.

⸻

🧠 Data Structures & Algorithms Used

Data Structure / Algorithm	Application
Graph (Adjacency Matrix)	Represents the router network
BFS	Explores network connectivity
Dijkstra’s Algorithm	Calculates shortest communication paths
Stack	Supports undo operations
Queue	Supports BFS traversal
Linked List	Maintains event history
Binary Search Tree	Organizes and retrieves stored information
Hash Table	Supports efficient lookup operations
File Handling	Provides persistent storage

⸻

🏗️ Technology Stack

Backend

* C
* Mongoose HTTP Server
* HTTP
* JSON

Frontend (Planned / In Development)

* C++
* Qt 6
* Qt Widgets
* Qt Network

Storage

* Text-file-based persistence

⸻

📂 Project Structure

NetworkRecovery/
│
├── Backend/
│   ├── Makefile
│   │
│   ├── data/
│   │   ├── connections.txt
│   │   ├── events.txt
│   │   └── routers.txt
│   │
│   ├── include/
│   │   ├── bst.h
│   │   ├── graph.h
│   │   ├── hashtable.h
│   │   ├── history.h
│   │   ├── mongoose.h
│   │   ├── queue.h
│   │   ├── router.h
│   │   ├── server.h
│   │   ├── stack.h
│   │   └── storage.h
│   │
│   └── src/
│       ├── bst.c
│       ├── graph.c
│       ├── hashtable.c
│       ├── history.c
│       ├── main.c
│       ├── mongoose.c
│       ├── queue.c
│       ├── router.c
│       ├── server.c
│       ├── stack.c
│       └── storage.c
│
├── Frontend/                 # Qt frontend (in development)
│
└── README.md

⸻

⚙️ Installation & Setup

Prerequisites

* GCC or Clang compiler.
* GNU Make.
* Git.

1. Clone the Repository

git clone https://github.com/USERNAME/REPOSITORY.git
cd REPOSITORY

2. Navigate to Backend

cd Backend

3. Compile the Backend

make

4. Run the Server

./network_server

The backend server will start listening on the port configured in the server implementation.

Refer to the backend configuration for the actual host and port.

⸻

🔄 How It Works

The general workflow of the simulator is:

       Router Network Input
                |
                v
       Graph Construction
                |
                v
       Network Connectivity
                |
                v
        Failure Simulation
          /           \
         /             \
  Router Failure   Link Failure
         \             /
          \           /
                |
                v
       Network Re-evaluation
                |
                v
       BFS / Dijkstra Analysis
                |
                v
       Alternative Path Detection
                |
                v
        Recovery Visualization

The backend processes network operations and algorithmic computations, while the frontend is intended to provide an interactive interface for network visualization and simulation.

⸻

🎓 Academic Relevance

This project demonstrates the practical application of Data Structures and Algorithms in computer networking scenarios.

Instead of implementing graph algorithms as isolated programs, it applies them to a simulated network infrastructure problem.

It demonstrates how:

* Graphs represent interconnected systems.
* BFS explores network connectivity.
* Dijkstra identifies minimum-cost paths.
* Stacks support reversible operations.
* Queues facilitate traversal.
* Linked lists maintain event history.
* Hash tables and BSTs support efficient data organization.

⸻

🚧 Future Enhancements

* Interactive network topology visualization.
* Real-time simulation of router and connection failures.
* Visual representation of alternative recovery paths.
* Network health monitoring dashboard.
* Detailed recovery analytics.
* Enhanced event tracking.
* More advanced network recovery strategies.

⸻

⚠️ Scope & Limitations

* This application operates on a simulated representation of a router network.
* It does not establish physical connections between actual routers.
* It does not monitor real network traffic.
* It does not automatically detect hardware failures.
* Recovery paths are calculated using the network information provided to the simulator.
* The effectiveness of a recovery path depends on the remaining available connections and their configured weights.

⸻

👨‍💻 Project Information

Project Name: Network Failure Recovery Simulator

Domain: Data Structures & Algorithms / Computer Networks

Backend Language: C

Communication: HTTP + JSON

Frontend: Qt 6 / C++

Storage: File-based persistence

⸻

Built to demonstrate how fundamental data structures and graph algorithms can be applied to network failure analysis and recovery simulation.