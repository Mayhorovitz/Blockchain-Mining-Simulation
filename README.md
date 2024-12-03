
# Blockchain Mining Simulation

## Overview
This project implements a **containerized blockchain simulation** using **Docker** and **C/C++**. It demonstrates:
- **Multithreaded synchronization**.
- **Inter-process communication (IPC)** within a simulated blockchain network.
- A setup consisting of a **server** and multiple **miners**.

---

## Features

### Dockerized Components:
- The **server** and **miners** run as separate containers, managed using Docker.
- Images are hosted on Docker Hub under [`royhz/mta_blockchain`](https://hub.docker.com/repository/docker/royhz/mta_blockchain).

### Configuration Management:
- A `config.txt` file is auto-generated using the **Makefile**.
- The server reads the mining difficulty from this configuration file.

### Automated Execution:
The provided **Makefile** automates the setup:
1. **Pulls** the required Docker images.
2. **Runs** one server container and three miner containers.

### Blockchain Simulation:
- Miners communicate with the server via **named pipes**.
- The server:
  - Manages miner subscriptions.
  - Validates new blocks.
  - Broadcasts approved blocks to all miners.

---

## Prerequisites

Ensure you have **Docker** installed and properly configured on your system.

---

## How to Run

Run the following command to start the simulation:

```bash
make
```

This will:
- **Generate** the necessary configuration file (`config.txt`).
- **Pull** the required images from `royhz/mta_blockchain`.
- **Launch** the server and miners in separate containers.

---

## Technologies Used

- **C/C++**: Core implementation of the blockchain simulation.
- **Docker**: For containerizing and managing the server and miner processes.
- **Makefile**: Automates setup and execution.
