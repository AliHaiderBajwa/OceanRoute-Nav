<div align="center">

# 🌊 OceanRoute Nav
### Maritime Navigation & Logistics Optimizer

**Data Structures Project · FAST-NUCES Islamabad · Spring 2026**

![C++](https://img.shields.io/badge/C++-17-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![SFML](https://img.shields.io/badge/SFML-2.x-8CC445?style=for-the-badge&logo=sfml&logoColor=white)
![Status](https://img.shields.io/badge/Status-Complete-brightgreen?style=for-the-badge)
![License](https://img.shields.io/badge/License-Academic-blue?style=for-the-badge)

*An intelligent maritime route visualization system that maps and optimizes cargo ship paths between international ports — built entirely with custom data structures, no STL allowed.*

</div>

---

## 👥 Team

| Name | Roll Number | Section |
|---|---|---|
| Ali Haider Bajwa | 24i-3102 | SE-C |
| Ermish | 24i-3180 | SE-C |

**Course:** Data Structures &nbsp;|&nbsp; **FAST-NUCES, Islamabad**

---

## 📌 Overview

**OceanRoute Nav** is a fully interactive maritime navigation system rendered on a real world map using **SFML**. It ingests two data files — `Routes.txt` (sea routes) and `ports.txt` (port coordinates) — builds a live **adjacency-list graph**, and lets users find optimal shipping paths, manage multi-leg voyages, filter by preferences, and visualize docking queues.

Every data structure in this project — graphs, heaps, queues, linked lists — is **implemented from scratch** with no STL containers whatsoever.

---

## ✨ Features

### 🗺 Interactive World Map
- All ports rendered as clickable nodes on a geographic world map
- Hover over any port to see its name and available routes
- Routes drawn as colour-coded lines (width/colour = cost or duration)
- Dotted lines for connecting/indirect routes

### 🔍 Shortest & Cheapest Route Finder
- **Dijkstra's Algorithm** with a custom **Min-Heap (Priority Queue)** for efficient node relaxation
- Computes both the **cheapest path** (minimise cost) and **fastest path** (minimise voyage time) simultaneously
- Optimal path highlighted in real-time on the map; ports light up as the algorithm explores them
- Handles connecting routes respecting **actual departure/arrival times** — a connection is only valid if the next ship departs after the previous one arrives

### 📦 Ship Route Booking
- Select origin and destination port by clicking on the map
- System finds all direct **and** connecting routes on your chosen date
- Displays full voyage details: shipping company, cost, departure/arrival times, layover duration
- Booked routes stored in a persistent **BookingList** (custom linked list)

### 🔗 Multi-Leg Route Builder
- Build custom multi-stop journeys interactively (e.g. Karachi → Dubai → Athens → Rotterdam)
- Each voyage leg stored as a node in a **custom Singly Linked List**
- Add or remove ports mid-journey; the structure updates dynamically
- Visualized as a sequential arrow chain on the map

### 🏗 Docking & Layover Queue
- Each port maintains a **FIFO Linked Queue** of ships waiting to dock
- First-come-first-serve processing with timing constraints
- Port icons animate ships entering and leaving the docking area

### 🎛 Custom Ship Preferences / Filtering
- Filter routes by **preferred shipping company** (Maersk, MSC, Evergreen, COSCO, ZIM, etc.)
- **Avoid specific ports** — excluded ports fade out on the map
- Set a **maximum voyage time** cap; routes exceeding it are hidden
- Subgraph generated dynamically — irrelevant ports greyed out, matching routes highlighted
- Reset all filters with one keypress

---

## 🏗 Data Structures Used

| Structure | Custom Implementation | Used For |
|---|---|---|
| **Adjacency-List Graph** | `struct Graph` + `struct Port` + `struct Edge` | Core route network (ports = vertices, routes = edges) |
| **Min-Heap (Priority Queue)** | `struct MinHeap` + `struct HeapNode` | Dijkstra's algorithm — O(E log V) route finding |
| **Singly Linked List** | `struct SimpleLinkedList` + `struct LNode` | Multi-leg journey builder |
| **Linked Queue (FIFO)** | `struct LinkedQueue` + `struct QNode` | Port docking management |
| **Dynamic Array** | `struct RouteArray` | Parsed route records before graph build |
| **Booking List** | `struct BookingList` + `struct BookedRoute` | Storing confirmed user bookings |

---

## 📐 Architecture

```
main()
 ├── parsePortsFile()            → reads ports.txt → inserts vertices into Graph
 ├── buildGraphFromRouteArray()  → reads Routes.txt → inserts edges into Graph
 ├── setportscoordinates()       → maps port names to screen coordinates
 │
 └── SFML Event Loop
      ├── drawports()        → renders port nodes, handles hover & click
      ├── drawroutes()       → renders edges (normal / filtered / optimal / multi-leg)
      │    └── Dijkstra's    → MinHeap-based, outputs PathInfo (cheapest + fastest)
      ├── drawBookingState() → shows route options, accepts booking
      ├── drawMultiLegState() → linked-list journey builder
      ├── drawFilter()       → UserPreferences menu (company / avoid / max-time)
      └── drawLegend()       → on-screen colour/symbol key
```

---

## 📂 Repository Structure

```
oceanroute-nav/
├── i243180_i243102_C.cpp   # Full source — C++ implementation
├── Routes.txt              # Sea route dataset (origin, dest, date, times, cost, company)
├── ports.txt               # Port list for graph vertex pre-seeding
├── worldmap.png            # Background world map (SFML texture)
├── button.png              # UI button sprite
├── font.ttf                # UI font
└── README.md
```

---

## ⚙️ Build & Run

### Prerequisites
- A C++17 compiler (g++ / MSVC)
- [SFML 2.x](https://www.sfml-dev.org/download.php) installed and linked

### Compile (g++)
```bash
g++ i243180_i243102_C.cpp -o oceanroute -lsfml-graphics -lsfml-window -lsfml-system -std=c++17
```

### Run
```bash
./oceanroute
```

> Make sure `worldmap.png`, `button.png`, `font.ttf`, `Routes.txt`, and `ports.txt` are all in the **same directory** as the executable.

---

## 🗃 Data File Format

### `Routes.txt`
Each line represents one available sea route:
```
<Origin> <Destination> <DD/MM/YYYY> <Depart HH:MM> <Arrive HH:MM> <Cost USD> <Company>
```
```
Karachi   Dubai    09/12/2024  08:00  18:00  3200  MaerskLine
Dubai     Athens   10/12/2024  09:00  22:00  7800  MaerskLine
```
> Arrival before departure implies an overnight voyage (next-day arrival handled automatically).

### `ports.txt`
One port name per line — pre-seeds graph vertices before route edges are added.

---

## 🕹 Controls

| Input | Action |
|---|---|
| **Left Click** on port | Select as source / destination |
| **Hover** over port / route | Show tooltip with details |
| `1` | Normal map view |
| `2` | Route booking mode |
| `3` | Multi-leg journey builder |
| `4` | Filter / preferences menu |
| `1–4` in filter menu | Set company / avoid port / max time / reset |
| `ESC` | Return to previous state |

---

## 📄 License

Developed as an academic submission for the Data Structures course at FAST-NUCES Islamabad. Free to reference for educational purposes.
