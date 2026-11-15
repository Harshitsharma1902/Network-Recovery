# Network Failure Recovery Simulator

An enterprise-grade Network Operations Center (NOC) dashboard and failure recovery simulation suite. The system combines a high-performance C backend running an embedded Mongoose HTTP server with a modern, dark-themed C++ Qt 6 desktop application for interactive graph topology visualization, chaos/failure simulation, Dijkstra shortest-path recovery, and BFS reachability verification.

---

## Architecture Overview

```
NetworkRecovery/
│
├── Backend/                       # High-Performance C Core & HTTP Service
│   ├── include/
│   │   ├── graph.h                # Adjacency Matrix topology representation & path algorithms
│   │   ├── router.h               # Router entity definitions
│   │   ├── stack.h                # LIFO Stack for rollback / undo operations
│   │   ├── queue.h                # FIFO Queue for BFS reachability traversal
│   │   ├── history.h              # Linked list for event auditing
│   │   ├── hashtable.h            # Fast key-value lookups
│   │   ├── bst.h                  # Binary Search Tree indexing
│   │   ├── storage.h              # Text-file persistence engine
│   │   ├── mongoose.h             # Embedded Mongoose HTTP server header
│   │   └── server.h               # API routing and server lifecycle
│   ├── src/
│   │   ├── main.c                 # Backend daemon entry point
│   │   ├── graph.c                # Dijkstra & BFS implementations
│   │   ├── router.c               # Router operations
│   │   ├── stack.c                # Stack push/pop operations
│   │   ├── queue.c                # Circular queue operations
│   │   ├── history.c              # Event linked list operations
│   │   ├── hashtable.c            # Hash table operations
│   │   ├── bst.c                  # BST operations
│   │   ├── storage.c              # connections.txt and events.txt loaders/savers
│   │   ├── mongoose.c             # Mongoose HTTP implementation
│   │   └── server.c               # REST API endpoints & JSON serialization
│   ├── data/
│   │   ├── connections.txt        # Persisted link topology and metric costs
│   │   ├── events.txt             # Audit log of network events
│   │   └── routers.txt            # Router configuration definitions
│   └── Makefile                   # Clang/GCC build script
│
├── Frontend/                      # C++ Qt 6 Desktop NOC Application
│   ├── CMakeLists.txt             # CMake project configuration
│   ├── main.cpp                   # Qt Application bootstrap
│   ├── MainWindow.h / .cpp        # Central layout controller & signal dispatcher
│   ├── api/
│   │   ├── ApiClient.h            # Centralized Qt Network manager (HTTP + JSON)
│   │   └── ApiClient.cpp          # Endpoints integration: /api/state, /api/link, /api/route, /api/undo
│   ├── models/
│   │   ├── Router.h               # Node model (ID, IP, status, incident degree)
│   │   ├── Connection.h           # Edge model (endpoints, weight metric, failed flag)
│   │   └── NetworkEvent.h         # Audit event category & timestamp model
│   ├── widgets/
│   │   ├── TopBar.h / .cpp        # NOC header, connection status pill, live health %, clock
│   │   ├── Sidebar.h / .cpp       # 9-item NOC navigation pane
│   │   ├── StatCard.h / .cpp      # Sleek metric telemetry card
│   │   ├── CustomChart.h / .cpp   # Custom QPainter-rendered Bar & Donut Gauge charts
│   │   └── NetworkCanvas.h / .cpp # Qt Graphics View interactive canvas with draggable nodes & glowing routes
│   ├── views/
│   │   ├── DashboardView.h / .cpp # Overview: 6 stat cards, mini topology preview, charts, live feed
│   │   ├── TopologyView.h / .cpp  # Interactive canvas with side inspection drawer & layout tools
│   │   ├── RouterView.h / .cpp    # Router table with status indicators & search filters
│   │   ├── ConnectionView.h / .cpp# Link table with inline Sever/Restore actions
│   │   ├── TopologyEditorView.h / .cpp # Dedicated Route Endpoints & Edges Editor
│   │   ├── FailureSimulationView.h / .cpp # Controlled link failure injection & C stack undo
│   │   ├── RecoveryView.h / .cpp  # Dijkstra shortest-route & BFS reachability visualizer
│   │   ├── HistoryView.h / .cpp   # Searchable audit event log table
│   │   ├── AnalyticsView.h / .cpp # Mesh redundancy, availability ratios, degree distributions
│   │   └── SettingsView.h / .cpp  # API endpoints, polling controls, & DSA architecture specs
│   └── resources/
│       ├── resources.qrc          # Qt resource manifest
│       └── styles/
│           └── dark_theme.qss     # NOC dark theme stylesheet
│
└── README.md
```

---

## Key Backend Data Structures & Algorithms (DSA)

| DSA Component | Implementation | Complexity | Purpose |
| :--- | :--- | :--- | :--- |
| **Adjacency Matrix** | `int weight[50][50]`, `int failed[50][50]` | $O(1)$ edge lookup | Graph topology and link weights |
| **Dijkstra's Algorithm** | Greedy shortest path with `dist[]`, `previous[]` | $O(V^2)$ | Calculates least-cost alternative recovery path |
| **Breadth-First Search (BFS)** | Array-based FIFO queue (`queue.h`) | $O(V + E)$ | Verifies network reachability and detects graph partitions |
| **LIFO Undo Stack** | Array stack (`stack.h`, size 100) | $O(1)$ push / pop | Reverts the most recent failure event |
| **Singly-Linked List** | Dynamic `EventNode` chain (`history.h`) | $O(1)$ insertion | Append-only event logging and persistence |
| **Persistent Storage** | File I/O (`storage.c`) | Sequential | Saves and restores network topology and event logs |

---

## API Endpoints (`Backend/src/server.c`)

| Method | Endpoint | Payload | Response | Description |
| :--- | :--- | :--- | :--- | :--- |
| `GET` | `/api/state` | None | `{"count": N, "edges": [...]}` | Current router count and all links with weights & failed states |
| `POST` | `/api/router/add` | `{}` | `{"success": true, "id": X, "count": N}` | Dynamically provisions a new router endpoint in the C Adjacency Matrix |
| `POST` | `/api/edge/save` | `{"u": 0, "v": 1, "weight": 5}` | `{"success": true, "u": 0, "v": 1, "weight": 5}` | Adds or updates metric weight of an edge in the matrix and persists to disk |
| `POST` | `/api/edge/delete`| `{"u": 0, "v": 1}` | `{"success": true}` | Deletes an edge link from the C Adjacency Matrix and updates connections.txt |
| `POST` | `/api/link/fail` | `{"u": 0, "v": 1}` | `{"success": true}` | Marks edge failed, pushes to undo stack, logs event |
| `POST` | `/api/link/recover`| `{"u": 0, "v": 1}` | `{"success": true}` | Restores severed link, logs event |
| `POST` | `/api/route` | `{"src": 0, "dest": 5}` | `{"reachable": true, "cost": X, "route": [...]}` | Runs BFS reachability + Dijkstra shortest path |
| `POST` | `/api/undo` | `{}` | `{"success": true}` | Pops last failure from stack and restores previous state |

---

## Build and Run Instructions

### 1. Prerequisites
- **Compiler**: Clang / AppleClang or GCC with C11 and C++17 support
- **Build System**: CMake 3.16+ and GNU Make
- **Qt Framework**: Qt 6 (Widgets, Network, Gui, Core)

### 2. Building and Starting the Backend
```bash
cd Backend
make
./network_server
```
*The backend server will run on `http://localhost:8080`.*

### 3. Building and Launching the Qt 6 Frontend
From the root directory:
```bash
mkdir -p Frontend/build
cd Frontend/build
cmake ..
cmake --build .
./NetworkRecoverySimulator
```

*Or open `Frontend/CMakeLists.txt` directly inside **Qt Creator** and click **Run**.*
