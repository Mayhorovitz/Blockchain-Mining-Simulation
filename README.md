Overview
This project implements a containerized blockchain simulation using Docker and C/C++. It demonstrates multithreaded synchronization and inter-process communication (IPC) within a simulated blockchain network, consisting of a server and multiple miners.

Features
Dockerized Components:

The server and miners run as separate containers, managed using Docker.
Images are hosted on Docker Hub under royhz/mta_blockchain.
Configuration Management:

A config.txt file is auto-generated using the Makefile and is used by the server to set the mining difficulty.
Automated Execution:

The provided Makefile automates the setup:
Pulls the required Docker images.
Runs one server container and three miner containers.
Blockchain Simulation:

Miners communicate with the server via named pipes.
The server manages miner subscriptions, validates new blocks, and broadcasts them to all miners.
How to Run
Clone the repository to your local machine.

Ensure you have Docker installed and configured.

Run the following command to start the simulation:

bash
Copy code
make
This will:

Generate the necessary configuration file (config.txt).
Pull the required images from royhz/mta_blockchain.
Launch the server and miners in separate containers.
Technologies Used
C/C++: Core implementation of the blockchain simulation.
Docker: For containerizing and managing the server and miner processes.
Makefile: Automates setup and execution.
