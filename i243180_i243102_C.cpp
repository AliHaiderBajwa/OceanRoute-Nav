// -------------------------------------------------
// -       Name:       Ermish    &  ALi Haider     -
// -       Roll no:    24i-3180  &  24i3102        -
// -       Section:    Se-C                        -
// -       Project:    OceanRoute Nav              -
// -------------------------------------------------

#include <iostream>
#include <string>
#include <fstream>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <iomanip>

using namespace std;
using namespace sf;

// ------------------------------------
//          HELPER FUNCTIONS          -
// ------------------------------------

// Splits a line into word using space as separator
int splitBySpaces(const string &line, string words[], int maxWords)
{
    string current = "";
    int count = 0;

    for (int i = 0; i < line.length(); i++)
    {
        char c = line[i];

        if (c == ' ')
        {
            if (current.length() > 0 && count < maxWords)
            {
                words[count++] = current;
                current = "";
            }
        }
        else
        {
            current += c;
        }
    }

    // Add last word if any
    if (current.length() > 0 && count < maxWords)
    {
        words[count++] = current;
    }

    return count; // number of words extracted
}

// Convert "08:30" to minutes (8*60 + 30 = 510)
int timeToMinutes(const string &time)
{
    if (time.length() < 5)
        return 0;

    int hours = 0;
    int minutes = 0;

    // Get hours: first two characters
    hours = (time[0] - '0') * 10 + (time[1] - '0');

    // Get minutes: characters after ':'
    minutes = (time[3] - '0') * 10 + (time[4] - '0');

    return hours * 60 + minutes;
}

// Calculate difference between two times
// Example: "08:00" to "14:00" = 360 minutes (6 hours)
int getTimeDifference(const string &startTime, const string &endTime)
{
    int start = timeToMinutes(startTime);
    int end = timeToMinutes(endTime);

    if (end >= start)
    {
        return end - start; // Same day
    }
    else
    {
        return (1440 - start) + end; // Next day (1440 = 24 hours)
    }
}

// Check if two dates are the same
bool isSameDate(const string &date1, const string &date2)
{
    return date1 == date2;
}

// Convert minutes back to hours for display
float minutesToHours(int minutes)
{
    return (float)minutes / 60.0f;
}

// -------------------------------
//          DATA STRUCTURES      -
// -------------------------------

struct Port;
struct Edge;

struct Edge
{
    Port *dest; // pointer to destination port
    string date;
    string depart_time;
    string arrive_time;
    int cost;
    string company;
    Edge *next; // next wali edge, since i decided it to be a linkedlist of edges

    Edge(Port *d, const string &dt, const string &dep, const string &arr, int c, const string &comp)
        : dest(d), date(dt), depart_time(dep), arrive_time(arr), cost(c), company(comp), next(nullptr) {}
};

// Port (image ke nodes)
struct Port
{
    string name;
    int charge;
    Edge *adj; // head of adjacency list
    int index;
    Port *next; // linkdlist of ports
    float cordx, cordy;
    Port(const string &n, int ch) : name(n), charge(ch), adj(nullptr), index(-1), next(nullptr)
    {
        cordx = 0;
        cordy = 0;
    }
};

//--------------------------------------------
//   Dynamic array for temporary route record
//--------------------------------------------
struct RouteRec
{
    string origin;
    string dest;
    string date;
    string depart;
    string arrive;
    int cost;
    string company;
};

struct RouteArray
{
    RouteRec **arr;
    int size;
    int capacity;

    RouteArray()
    {
        capacity = 8;
        size = 0;
        arr = new RouteRec *[capacity];
        for (int i = 0; i < capacity; ++i)
            arr[i] = nullptr;
    }

    ~RouteArray()
    {
        for (int i = 0; i < size; ++i)
        {
            if (arr[i])
                delete arr[i];
        }
        delete[] arr;
    }

    void push(RouteRec *r)
    {
        if (size >= capacity)
        {
            int newcap = capacity * 2;
            RouteRec **newarr = new RouteRec *[newcap];

            for (int i = 0; i < newcap; ++i)
                newarr[i] = nullptr;

            // copy
            for (int i = 0; i < size; ++i)
                newarr[i] = arr[i];

            delete[] arr;
            arr = newarr;
            capacity = newcap;
        }
        arr[size++] = r;
    }
};

//----------------------------------
//   linked queue for docking ships
//----------------------------------

struct QNode
{
    string shipId;
    QNode *next;
    QNode(const string &id) : shipId(id), next(nullptr) {}
};

struct LinkedQueue
{
    QNode *head;
    QNode *tail;

    LinkedQueue() : head(nullptr), tail(nullptr) {}

    bool isEmpty()
    {
        return head == nullptr;
    }

    void enqueue(const string &id)
    {
        QNode *n = new QNode(id);
        if (tail)
            tail->next = n;
        tail = n;
        if (!head)
            head = n;
    }

    string dequeue()
    {
        if (!head)
            return string();

        QNode *n = head;
        string id = n->shipId; // copy
        head = head->next;
        if (!head)
            tail = nullptr;

        delete n;
        return id;
    }

    void clear()
    {
        while (!isEmpty())
        {
            dequeue();
        }
    }
};

//---------------------------------------
//   linkdlist for multi-leg journeys
//---------------------------------------

struct LNode
{
    string portName;
    LNode *next;
    LNode(const string &p) : portName(p), next(nullptr) {}
};

struct SimpleLinkedList
{
    LNode *head;
    LNode *tail;

    SimpleLinkedList() : head(nullptr), tail(nullptr) {}

    void append(const string &port)
    {
        LNode *n = new LNode(port);
        if (!head)
        {
            head = tail = n;
        }
        else
        {
            tail->next = n;
            tail = n;
        }
    }

    void print()
    {
        cout << "Journey: ";
        LNode *p = head;
        while (p)
        {
            cout << p->portName;
            if (p->next)
                cout << " -> ";
            p = p->next;
        }
        cout << "\n";
    }
    void remove(const string &name)
    {
        if (!head)
            return;

        if (head->portName == name)
        {
            LNode *temp = head;
            head = head->next;
            if (!head)
                tail = nullptr;
            delete temp;
            return;
        }

        LNode *curr = head;
        while (curr->next)
        {
            if (curr->next->portName == name)
            {
                LNode *temp = curr->next;
                curr->next = temp->next;

                if (curr->next == nullptr)
                    tail = curr;

                delete temp;
                return;
            }
            curr = curr->next;
        }
    }
    void clear()
    {
        LNode *p = head;
        while (p)
        {
            LNode *nx = p->next;
            delete p;
            p = nx;
        }
        head = tail = nullptr;
    }
};

//-----------------------------------
//    PRIORITY QUEUE (MIN HEAP)     -
//-----------------------------------

struct HeapNode
{
    Port *port;
    int cost; // priority value

    HeapNode(Port *p, int c) : port(p), cost(c) {}
};

struct MinHeap
{
    HeapNode **data;
    int size;
    int capacity;

    MinHeap()
    {
        capacity = 50;
        size = 0;
        data = new HeapNode *[capacity];
    }

    ~MinHeap()
    {
        for (int i = 0; i < size; i++)
        {
            delete data[i];
        }
        delete[] data;
    }

    bool isEmpty()
    {
        return size == 0;
    }

    void swap(int i, int j)
    {
        HeapNode *temp = data[i];
        data[i] = data[j];
        data[j] = temp;
    }

    int parent(int i) { return (i - 1) / 2; }
    int left(int i) { return 2 * i + 1; }
    int right(int i) { return 2 * i + 2; }

    void insert(Port *port, int cost)
    {
        if (size >= capacity)
        {
            // Double capacity
            capacity *= 2;
            HeapNode **newData = new HeapNode *[capacity];
            for (int i = 0; i < size; i++)
            {
                newData[i] = data[i];
            }
            delete[] data;
            data = newData;
        }

        data[size] = new HeapNode(port, cost);

        // Bubble up
        int current = size;
        size++;

        while (current > 0 && data[parent(current)]->cost > data[current]->cost)
        {
            swap(current, parent(current));
            current = parent(current);
        }
    }

    HeapNode *extractMin()
    {
        if (size == 0)
            return nullptr;

        HeapNode *minNode = data[0];
        data[0] = data[size - 1];
        size--;

        // Bubble down
        int current = 0;
        while (true)
        {
            int smallest = current;
            int l = left(current);
            int r = right(current);

            if (l < size && data[l]->cost < data[smallest]->cost)
            {
                smallest = l;
            }
            if (r < size && data[r]->cost < data[smallest]->cost)
            {
                smallest = r;
            }

            if (smallest == current)
                break;

            swap(current, smallest);
            current = smallest;
        }

        return minNode;
    }
};

// ----------------------------------
//    PATH RESULT                   -
// ----------------------------------

struct PathInfo
{
    SimpleLinkedList *path; // List of port names
    int totalCost;
    int totalTime; // in minutes
    bool found;

    PathInfo()
    {
        path = new SimpleLinkedList();
        totalCost = 0;
        totalTime = 0;
        found = false;
    }

    ~PathInfo()
    {
        if (path)
        {
            path->clear();
            delete path;
        }
    }

    void printInfo()
    {
        if (!found)
        {
            cout << "No path found!\n";
            return;
        }

        cout << "=== PATH FOUND ===\n";
        cout << "Total Cost: $" << totalCost << "\n";
        cout << "Total Time: " << minutesToHours(totalTime) << " hours\n";
        cout << "Route: ";
        path->print();
    }
};

//-----------------------------------
//    ROUTE BOOKING STRUCTURES      -
//-----------------------------------

struct BookedRoute
{
    SimpleLinkedList *ports;     // List of ports in the route
    SimpleLinkedList *companies; // List of companies for each leg
    int totalCost;
    int totalTime;
    int connections; // Number of connections (0 = direct)
    BookedRoute *next;

    BookedRoute()
    {
        ports = new SimpleLinkedList();
        companies = new SimpleLinkedList();
        totalCost = 0;
        totalTime = 0;
        connections = 0;
        next = nullptr;
    }

    ~BookedRoute()
    {
        if (ports)
        {
            ports->clear();
            delete ports;
        }
        if (companies)
        {
            companies->clear();
            delete companies;
        }
    }

    void print()
    {
        cout << "  Route: ";
        LNode *p = ports->head;
        while (p)
        {
            cout << p->portName;
            if (p->next)
                cout << " -> ";
            p = p->next;
        }
        cout << "\n";
        cout << "  Total Cost: $" << totalCost << "\n";
        cout << "  Total Time: " << minutesToHours(totalTime) << " hours\n";
        cout << "  Connections: " << connections << "\n";
    }
};

struct BookingList
{
    BookedRoute *head;
    int count;

    BookingList() : head(nullptr), count(0) {}

    ~BookingList()
    {
        clear();
    }

    void add(BookedRoute *route)
    {
        route->next = head;
        head = route;
        count++;
    }

    void printAll()
    {
        if (count == 0)
        {
            cout << "No routes found!\n";
            return;
        }

        cout << "\n========================================\n";
        cout << "  FOUND " << count << " ROUTE(S)\n";
        cout << "========================================\n\n";

        BookedRoute *current = head;
        int num = 1;
        while (current)
        {
            cout << "Route Option " << num++ << ":\n";
            current->print();
            cout << "\n";
            current = current->next;
        }
    }

    void clear()
    {
        BookedRoute *current = head;
        while (current)
        {
            BookedRoute *next = current->next;
            delete current;
            current = next;
        }
        head = nullptr;
        count = 0;
    }
};

//-----------------------------------
//    USER PREFERENCES STRUCTURE    -
//-----------------------------------

struct UserPreferences
{
    string preferredCompany;
    SimpleLinkedList *avoidedPorts;
    int maxVoyageTime; // in minutes
    bool isActive;

    UserPreferences()
    {
        preferredCompany = "";
        avoidedPorts = new SimpleLinkedList();
        maxVoyageTime = 999999; // No limit by default
        isActive = false;
    }

    ~UserPreferences()
    {
        if (avoidedPorts)
        {
            avoidedPorts->clear();
            delete avoidedPorts;
        }
    }

    void setPreferredCompany(const string &company)
    {
        preferredCompany = company;
        isActive = true;
    }

    void addAvoidedPort(const string &portName)
    {
        avoidedPorts->append(portName);
        isActive = true;
    }

    void removeAvoidedPort(const string &portName)
    {
        avoidedPorts->remove(portName);
    }

    void setMaxVoyageTime(int minutes)
    {
        maxVoyageTime = minutes;
        isActive = true;
    }

    bool shouldAvoidPort(const string &portName)
    {
        if (!avoidedPorts || !avoidedPorts->head)
            return false;

        LNode *curr = avoidedPorts->head;
        while (curr)
        {
            if (curr->portName == portName)
                return true;
            curr = curr->next;
        }
        return false;
    }

    void reset()
    {
        preferredCompany = "";
        avoidedPorts->clear();
        maxVoyageTime = 999999;
        isActive = false;
    }

    void print()
    {
        cout << "\n=== USER PREFERENCES ===\n";
        if (!isActive)
        {
            cout << "No preferences set.\n";
            return;
        }

        if (preferredCompany.length() > 0)
        {
            cout << "Preferred Company: " << preferredCompany << "\n";
        }

        if (avoidedPorts->head)
        {
            cout << "Avoided Ports: ";
            LNode *n = avoidedPorts->head;
            while (n)
            {
                cout << n->portName;
                if (n->next)
                    cout << ", ";
                n = n->next;
            }
            cout << "\n";
        }

        if (maxVoyageTime < 999999)
        {
            cout << "Max Voyage Time: " << minutesToHours(maxVoyageTime) << " hours\n";
        }
    }
};

// -------------------------------------------
// Graph container (linked list of Port nodes)
// -------------------------------------------

struct Graph
{
    Port *ports;
    int nodeCount;

    Graph() : ports(nullptr), nodeCount(0) {}

    // find port by name
    Port *findPort(const string &name)
    {
        Port *p = ports;
        while (p)
        {
            if (p->name == name)
                return p;
            p = p->next;
        }
        return nullptr;
    }

    // find or create port
    Port *findOrCreatePort(const string &name, int chargeIfNew = 0, bool setChargeIfExists = false)
    {
        Port *p = findPort(name);
        if (p)
        {
            if (setChargeIfExists)
            {
                p->charge = chargeIfNew;
            }
            return p;
        }

        Port *np = new Port(name, chargeIfNew);
        np->next = ports;
        ports = np;
        nodeCount++;
        return np;
    }

    // origin->dest new edge
    void insertEdge(const string &originName, const string &destName,
                    const string &date, const string &depart, const string &arrive,
                    int cost, const string &company)
    {
        Port *o = findPort(originName);
        Port *d = findPort(destName);

        if (!o)
            o = findOrCreatePort(originName, 0, false);
        if (!d)
            d = findOrCreatePort(destName, 0, false);

        Edge *e = new Edge(d, date, depart, arrive, cost, company);
        e->next = o->adj;
        o->adj = e;
    }

    // count edges
    int countEdges()
    {
        int total = 0;
        Port *p = ports;
        while (p)
        {
            Edge *e = p->adj;
            while (e)
            {
                total++;
                e = e->next;
            }
            p = p->next;
        }
        return total;
    }

    // print summary
    void printSummary()
    {
        cout << "GRAPH SUMMARY\n";
        cout << "Nodes (ports): " << nodeCount << "\n";
        cout << "Edges (routes): " << countEdges() << "\n\n";

        Port *p = ports;
        while (p)
        {
            cout << "Port: " << p->name << " (charge=" << p->charge << ")\n";
            Edge *e = p->adj;
            if (!e)
                cout << "  -> No outgoing routes\n";
            else
            {
                while (e)
                {
                    cout << "  -> " << e->dest->name
                         << " | date=" << e->date
                         << " depart=" << e->depart_time
                         << " arrive=" << e->arrive_time
                         << " cost=" << e->cost
                         << " comp=" << e->company << "\n";
                    e = e->next;
                }
            }
            cout << "\n";
            p = p->next;
        }
    }

    void assignIndices()
    {
        int idx = 0;
        Port *p = ports;
        while (p)
        {
            p->index = idx++;
            p = p->next;
        }
    }

    // Find cheapest path using Dijkstra's algorithm
    PathInfo *findCheapestPath(const string &startName, const string &endName)
    {
        PathInfo *result = new PathInfo();

        Port *start = findPort(startName);
        Port *end = findPort(endName);

        if (!start || !end)
        {
            cout << "Start or end port not found!\n";
            result->found = false;
            return result;
        }

        // Make sure ports have indices
        assignIndices();

        // Create arrays for algorithm
        const int BIG_NUMBER = 999999;
        int *distance = new int[nodeCount];
        Port **previous = new Port *[nodeCount];
        bool *visited = new bool[nodeCount];

        // Initialize
        for (int i = 0; i < nodeCount; i++)
        {
            distance[i] = BIG_NUMBER;
            previous[i] = nullptr;
            visited[i] = false;
        }

        distance[start->index] = 0;

        // Priority queue
        MinHeap pq;
        pq.insert(start, 0);

        cout << "\n[Dijkstra] Starting from " << startName << "...\n";

        while (!pq.isEmpty())
        {
            HeapNode *node = pq.extractMin();
            Port *current = node->port;
            int currentCost = node->cost;
            delete node;

            // Skip if already visited
            if (visited[current->index])
                continue;

            visited[current->index] = true;

            cout << "[Dijkstra] Visiting: " << current->name
                 << " (cost so far: $" << currentCost << ")\n";

            // Found destination?
            if (current == end)
            {
                cout << "[Dijkstra] Reached destination!\n";
                break;
            }

            // Check all neighbors
            Edge *edge = current->adj;
            while (edge)
            {
                Port *neighbor = edge->dest;

                if (!visited[neighbor->index])
                {
                    int newCost = distance[current->index] + edge->cost;

                    if (newCost < distance[neighbor->index])
                    {
                        distance[neighbor->index] = newCost;
                        previous[neighbor->index] = current;
                        pq.insert(neighbor, newCost);
                    }
                }

                edge = edge->next;
            }
        }

        // Check if path exists
        if (distance[end->index] == BIG_NUMBER)
        {
            cout << "[Dijkstra] No path exists!\n";
            result->found = false;
            delete[] distance;
            delete[] previous;
            delete[] visited;
            return result;
        }

        // Build path backwards
        result->found = true;
        result->totalCost = distance[end->index];

        // Create temporary list (backwards)
        Port *current = end;
        SimpleLinkedList tempPath;

        while (current != nullptr)
        {
            tempPath.append(current->name);
            current = previous[current->index];
        }

        // Reverse into result
        LNode *node = tempPath.head;
        int pathLength = 0;

        // Count nodes
        LNode *counter = tempPath.head;
        while (counter)
        {
            pathLength++;
            counter = counter->next;
        }

        // Store in array to reverse
        string *pathArray = new string[pathLength];
        node = tempPath.head;
        for (int i = 0; i < pathLength; i++)
        {
            pathArray[i] = node->portName;
            node = node->next;
        }

        // Add in reverse order
        for (int i = pathLength - 1; i >= 0; i--)
        {
            result->path->append(pathArray[i]);
        }

        // Cleanup
        delete[] pathArray;
        delete[] distance;
        delete[] previous;
        delete[] visited;

        return result;
    }

    // Find shortest TIME path using Dijkstra's algorithm
    PathInfo *findShortestTimePath(const string &startName, const string &endName)
    {
        PathInfo *result = new PathInfo();

        Port *start = findPort(startName);
        Port *end = findPort(endName);

        if (!start || !end)
        {
            cout << "Start or end port not found!\n";
            result->found = false;
            return result;
        }

        assignIndices();

        const int BIG_NUMBER = 999999;
        int *distance = new int[nodeCount];
        Port **previous = new Port *[nodeCount];
        bool *visited = new bool[nodeCount];

        for (int i = 0; i < nodeCount; i++)
        {
            distance[i] = BIG_NUMBER;
            previous[i] = nullptr;
            visited[i] = false;
        }

        distance[start->index] = 0;

        MinHeap pq;
        pq.insert(start, 0);

        cout << "\n[Shortest Time] Starting from " << startName << "...\n";

        while (!pq.isEmpty())
        {
            HeapNode *node = pq.extractMin();
            Port *current = node->port;
            int currentTime = node->cost;
            delete node;

            if (visited[current->index])
                continue;

            visited[current->index] = true;

            cout << "[Shortest Time] Visiting: " << current->name
                 << " (time so far: " << minutesToHours(currentTime) << " hrs)\n";

            if (current == end)
            {
                cout << "[Shortest Time] Reached destination!\n";
                break;
            }

            Edge *edge = current->adj;
            while (edge)
            {
                Port *neighbor = edge->dest;

                if (!visited[neighbor->index])
                {
                    // Calculate travel time for this edge
                    int travelTime = getTimeDifference(edge->depart_time, edge->arrive_time);
                    int newTime = distance[current->index] + travelTime;

                    if (newTime < distance[neighbor->index])
                    {
                        distance[neighbor->index] = newTime;
                        previous[neighbor->index] = current;
                        pq.insert(neighbor, newTime);
                    }
                }

                edge = edge->next;
            }
        }

        if (distance[end->index] == BIG_NUMBER)
        {
            cout << "[Shortest Time] No path exists!\n";
            result->found = false;
            delete[] distance;
            delete[] previous;
            delete[] visited;
            return result;
        }

        result->found = true;
        result->totalTime = distance[end->index];

        // Build path (same logic as cheapest path)
        Port *current = end;
        SimpleLinkedList tempPath;

        while (current != nullptr)
        {
            tempPath.append(current->name);
            current = previous[current->index];
        }

        LNode *node = tempPath.head;
        int pathLength = 0;
        LNode *counter = tempPath.head;
        while (counter)
        {
            pathLength++;
            counter = counter->next;
        }

        string *pathArray = new string[pathLength];
        node = tempPath.head;
        for (int i = 0; i < pathLength; i++)
        {
            pathArray[i] = node->portName;
            node = node->next;
        }

        for (int i = pathLength - 1; i >= 0; i--)
        {
            result->path->append(pathArray[i]);
        }

        delete[] pathArray;
        delete[] distance;
        delete[] previous;
        delete[] visited;

        return result;
    }

    //===================================================
    //    PREFERENCE-BASED FILTERING FUNCTIONS (NEW)    -
    //===================================================

    // Check if an edge matches user preferences
    bool edgeMatchesPreferences(Edge *edge, Port *fromPort, UserPreferences &prefs)
    {
        if (!prefs.isActive)
            return true; // No preferences = all routes valid

        // 1. Check preferred company
        if (prefs.preferredCompany.length() > 0)
        {
            if (edge->company != prefs.preferredCompany)
            {
                return false;
            }
        }

        // 2. Check avoided ports
        if (prefs.shouldAvoidPort(fromPort->name) || prefs.shouldAvoidPort(edge->dest->name))
        {
            return false;
        }

        // 3. Check max voyage time
        int voyageTime = getTimeDifference(edge->depart_time, edge->arrive_time);
        if (voyageTime > prefs.maxVoyageTime)
        {
            return false;
        }

        return true;
    }

    // Dijkstra with preferences applied (COST-BASED)
    PathInfo *findCheapestPathWithPreferences(const string &startName, const string &endName, UserPreferences &prefs)
    {
        PathInfo *result = new PathInfo();

        Port *start = findPort(startName);
        Port *end = findPort(endName);

        if (!start || !end)
        {
            cout << "[Filtered Search] Start or end port not found!\n";
            result->found = false;
            return result;
        }

        assignIndices();

        const int BIG_NUMBER = 999999;
        int *distance = new int[nodeCount];
        Port **previous = new Port *[nodeCount];
        bool *visited = new bool[nodeCount];

        for (int i = 0; i < nodeCount; i++)
        {
            distance[i] = BIG_NUMBER;
            previous[i] = nullptr;
            visited[i] = false;
        }

        distance[start->index] = 0;

        MinHeap pq;
        pq.insert(start, 0);

        cout << "\n[Filtered Cost Search] Starting from " << startName << " with preferences...\n";
        prefs.print();

        while (!pq.isEmpty())
        {
            HeapNode *node = pq.extractMin();
            Port *current = node->port;
            int currentCost = node->cost;
            delete node;

            if (visited[current->index])
                continue;

            visited[current->index] = true;

            cout << "[Filtered Cost Search] Visiting: " << current->name
                 << " (cost: $" << currentCost << ")\n";

            if (current == end)
            {
                cout << "[Filtered Cost Search] Reached destination!\n";
                break;
            }

            // Check all neighbors with preference filtering
            Edge *edge = current->adj;
            while (edge)
            {
                Port *neighbor = edge->dest;

                // Apply preference filter
                if (!visited[neighbor->index] && edgeMatchesPreferences(edge, current, prefs))
                {
                    int newCost = distance[current->index] + edge->cost;

                    if (newCost < distance[neighbor->index])
                    {
                        distance[neighbor->index] = newCost;
                        previous[neighbor->index] = current;
                        pq.insert(neighbor, newCost);
                    }
                }

                edge = edge->next;
            }
        }

        if (distance[end->index] == BIG_NUMBER)
        {
            cout << "[Filtered Cost Search] No path exists matching preferences!\n";
            result->found = false;
            delete[] distance;
            delete[] previous;
            delete[] visited;
            return result;
        }

        result->found = true;
        result->totalCost = distance[end->index];

        // Build path
        Port *current = end;
        SimpleLinkedList tempPath;

        while (current != nullptr)
        {
            tempPath.append(current->name);
            current = previous[current->index];
        }

        LNode *node = tempPath.head;
        int pathLength = 0;
        LNode *counter = tempPath.head;
        while (counter)
        {
            pathLength++;
            counter = counter->next;
        }

        string *pathArray = new string[pathLength];
        node = tempPath.head;
        for (int i = 0; i < pathLength; i++)
        {
            pathArray[i] = node->portName;
            node = node->next;
        }

        for (int i = pathLength - 1; i >= 0; i--)
        {
            result->path->append(pathArray[i]);
        }

        delete[] pathArray;
        delete[] distance;
        delete[] previous;
        delete[] visited;

        return result;
    }

    // Find shortest TIME path with preferences
    PathInfo *findShortestTimePathWithPreferences(const string &startName, const string &endName, UserPreferences &prefs)
    {
        PathInfo *result = new PathInfo();

        Port *start = findPort(startName);
        Port *end = findPort(endName);

        if (!start || !end)
        {
            cout << "[Filtered Time Search] Start or end port not found!\n";
            result->found = false;
            return result;
        }

        assignIndices();

        const int BIG_NUMBER = 999999;
        int *distance = new int[nodeCount];
        Port **previous = new Port *[nodeCount];
        bool *visited = new bool[nodeCount];

        for (int i = 0; i < nodeCount; i++)
        {
            distance[i] = BIG_NUMBER;
            previous[i] = nullptr;
            visited[i] = false;
        }

        distance[start->index] = 0;

        MinHeap pq;
        pq.insert(start, 0);

        cout << "\n[Filtered Time Search] Starting from " << startName << "...\n";
        prefs.print();

        while (!pq.isEmpty())
        {
            HeapNode *node = pq.extractMin();
            Port *current = node->port;
            int currentTime = node->cost;
            delete node;

            if (visited[current->index])
                continue;

            visited[current->index] = true;

            cout << "[Filtered Time Search] Visiting: " << current->name
                 << " (time: " << minutesToHours(currentTime) << " hrs)\n";

            if (current == end)
            {
                cout << "[Filtered Time Search] Reached destination!\n";
                break;
            }

            Edge *edge = current->adj;
            while (edge)
            {
                Port *neighbor = edge->dest;

                // Apply preference filter
                if (!visited[neighbor->index] && edgeMatchesPreferences(edge, current, prefs))
                {
                    int travelTime = getTimeDifference(edge->depart_time, edge->arrive_time);
                    int newTime = distance[current->index] + travelTime;

                    if (newTime < distance[neighbor->index])
                    {
                        distance[neighbor->index] = newTime;
                        previous[neighbor->index] = current;
                        pq.insert(neighbor, newTime);
                    }
                }

                edge = edge->next;
            }
        }

        if (distance[end->index] == BIG_NUMBER)
        {
            cout << "[Filtered Time Search] No path exists!\n";
            result->found = false;
            delete[] distance;
            delete[] previous;
            delete[] visited;
            return result;
        }

        result->found = true;
        result->totalTime = distance[end->index];

        // Build path
        Port *current = end;
        SimpleLinkedList tempPath;

        while (current != nullptr)
        {
            tempPath.append(current->name);
            current = previous[current->index];
        }

        LNode *node = tempPath.head;
        int pathLength = 0;
        LNode *counter = tempPath.head;
        while (counter)
        {
            pathLength++;
            counter = counter->next;
        }

        string *pathArray = new string[pathLength];
        node = tempPath.head;
        for (int i = 0; i < pathLength; i++)
        {
            pathArray[i] = node->portName;
            node = node->next;
        }

        for (int i = pathLength - 1; i >= 0; i--)
        {
            result->path->append(pathArray[i]);
        }

        delete[] pathArray;
        delete[] distance;
        delete[] previous;
        delete[] visited;

        return result;
    }

    // Get count of routes matching preferences
    int countFilteredRoutes(UserPreferences &prefs)
    {
        int count = 0;
        Port *p = ports;

        while (p)
        {
            Edge *e = p->adj;
            while (e)
            {
                if (edgeMatchesPreferences(e, p, prefs))
                {
                    count++;
                }
                e = e->next;
            }
            p = p->next;
        }

        return count;
    }

    // Print all routes that match preferences
    void printFilteredRoutes(UserPreferences &prefs)
    {
        cout << "\n========================================\n";
        cout << "  FILTERED ROUTES\n";
        cout << "========================================\n\n";

        if (!prefs.isActive)
        {
            cout << "No filters active.  Showing all routes.\n";
            printSummary();
            return;
        }

        prefs.print();
        cout << "\n";

        int count = 0;
        Port *p = ports;

        while (p)
        {
            Edge *e = p->adj;
            bool portPrinted = false;

            while (e)
            {
                if (edgeMatchesPreferences(e, p, prefs))
                {
                    if (!portPrinted)
                    {
                        cout << "From " << p->name << ":\n";
                        portPrinted = true;
                    }

                    cout << "  -> " << e->dest->name
                         << " | Company: " << e->company
                         << " | Cost: $" << e->cost
                         << " | Time: " << getTimeDifference(e->depart_time, e->arrive_time) << " min"
                         << "\n";
                    count++;
                }
                e = e->next;
            }

            if (portPrinted)
                cout << "\n";
            p = p->next;
        }

        cout << "Total filtered routes: " << count << "\n";
        cout << "========================================\n\n";
    }

    //-----------------------------------
    //    ROUTE BOOKING FUNCTIONS       -
    //-----------------------------------

    // Find all DIRECT routes between two ports
    BookingList *findDirectRoutes(const string &originName, const string &destName)
    {
        BookingList *results = new BookingList();

        Port *origin = findPort(originName);
        if (!origin)
        {
            cout << "[Booking] Origin port not found: " << originName << "\n";
            return results;
        }

        cout << "[Booking] Searching for direct routes from " << originName << " to " << destName << "...\n";

        // Check all edges from origin
        Edge *edge = origin->adj;
        while (edge)
        {
            if (edge->dest->name == destName)
            {
                // Found a direct route!
                BookedRoute *route = new BookedRoute();
                route->ports->append(originName);
                route->ports->append(destName);
                route->companies->append(edge->company);
                route->totalCost = edge->cost;
                route->totalTime = getTimeDifference(edge->depart_time, edge->arrive_time);
                route->connections = 0;

                results->add(route);
                cout << "[Booking] Found direct route via " << edge->company << "\n";
            }
            edge = edge->next;
        }

        if (results->count == 0)
        {
            cout << "[Booking] No direct routes found.\n";
        }

        return results;
    }

    // Find routes with ONE connection
    BookingList *findOneConnectionRoutes(const string &originName, const string &destName)
    {
        BookingList *results = new BookingList();

        Port *origin = findPort(originName);
        Port *dest = findPort(destName);

        if (!origin || !dest)
        {
            cout << "[Booking] Origin or destination not found!\n";
            return results;
        }

        cout << "[Booking] Searching for routes with 1 connection...\n";

        // Try each port as intermediate
        Edge *firstLeg = origin->adj;
        while (firstLeg)
        {
            Port *intermediate = firstLeg->dest;

            // Skip if intermediate is the destination
            if (intermediate->name == destName)
            {
                firstLeg = firstLeg->next;
                continue;
            }

            // Check routes from intermediate to destination
            Edge *secondLeg = intermediate->adj;
            while (secondLeg)
            {
                if (secondLeg->dest->name == destName)
                {
                    // Check layover feasibility
                    int arrivalTime = timeToMinutes(firstLeg->arrive_time);
                    int departureTime = timeToMinutes(secondLeg->depart_time);

                    int layover = 0;
                    if (departureTime >= arrivalTime)
                    {
                        layover = departureTime - arrivalTime;
                    }
                    else
                    {
                        // Next day
                        layover = (1440 - arrivalTime) + departureTime;
                    }

                    // Minimum 2 hours for cargo transfer
                    const int MIN_LAYOVER = 120;
                    // Maximum 24 hours wait
                    const int MAX_LAYOVER = 1440;

                    if (layover >= MIN_LAYOVER && layover <= MAX_LAYOVER)
                    {
                        BookedRoute *route = new BookedRoute();
                        route->ports->append(originName);
                        route->ports->append(intermediate->name);
                        route->ports->append(destName);
                        route->companies->append(firstLeg->company);
                        route->companies->append(secondLeg->company);

                        route->totalCost = firstLeg->cost + secondLeg->cost;

                        int leg1Time = getTimeDifference(firstLeg->depart_time, firstLeg->arrive_time);
                        int leg2Time = getTimeDifference(secondLeg->depart_time, secondLeg->arrive_time);
                        route->totalTime = leg1Time + layover + leg2Time;
                        route->connections = 1;

                        results->add(route);
                        cout << "[Booking] Found route via " << intermediate->name
                             << " (layover: " << minutesToHours(layover) << " hrs)\n";
                    }
                }
                secondLeg = secondLeg->next;
            }

            firstLeg = firstLeg->next;
        }

        if (results->count == 0)
        {
            cout << "[Booking] No connecting routes found.\n";
        }

        return results;
    }

    // Main booking function
    BookingList *bookRoute(const string &originName, const string &destName)
    {
        cout << "\n========================================\n";
        cout << "  BOOKING ROUTE: " << originName << " -> " << destName << "\n";
        cout << "========================================\n\n";

        // Try direct routes first
        BookingList *directRoutes = findDirectRoutes(originName, destName);

        if (directRoutes->count > 0)
        {
            return directRoutes;
        }

        // If no direct routes, try one connection
        cout << "\n[Booking] Trying connecting routes...\n";
        delete directRoutes;

        BookingList *connectingRoutes = findOneConnectionRoutes(originName, destName);

        if (connectingRoutes->count > 0)
        {
            return connectingRoutes;
        }

        // If still nothing, use Dijkstra
        cout << "\n[Booking] No simple routes found. Using pathfinding...\n";
        PathInfo *dijkstraPath = findCheapestPath(originName, destName);

        if (dijkstraPath->found)
        {
            BookedRoute *route = new BookedRoute();
            LNode *node = dijkstraPath->path->head;
            while (node)
            {
                route->ports->append(node->portName);
                node = node->next;
            }
            route->totalCost = dijkstraPath->totalCost;
            route->totalTime = dijkstraPath->totalTime;

            // Count connections
            LNode *counter = route->ports->head;
            int portCount = 0;
            while (counter)
            {
                portCount++;
                counter = counter->next;
            }
            route->connections = portCount - 2; // ports minus origin and dest

            connectingRoutes->add(route);
        }

        delete dijkstraPath;
        return connectingRoutes;
    }

    // clear all edges and ports
    void clearAll()
    {
        // deletin edges
        Port *p = ports;
        while (p)
        {
            Edge *e = p->adj;
            while (e)
            {
                Edge *nx = e->next;
                delete e;
                e = nx;
            }
            p->adj = nullptr;
            p = p->next;
        }
        // delete ports
        p = ports;
        while (p)
        {
            Port *nx = p->next;
            delete p;
            p = nx;
        }
        ports = nullptr;
        nodeCount = 0;
    }
};

void parsePortsFile(const string &filename, Graph &g)
{
    ifstream fin(filename.c_str());
    if (!fin.is_open())
    {
        cout << "Could not open " << filename << ".\n";
        return;
    }
    string line;
    while (getline(fin, line))
    {
        // find first non-space and last non-space
        int L = 0, R = (int)line.size() - 1;

        while (L <= R && (line[L] == ' ' || line[L] == '\t'))
        {
            L++;
        }
        while (R >= L && (line[R] == ' ' || line[R] == '\t'))
        {
            R--;
        }

        if (L > R)
            continue; // empty

        string sub = line.substr(L, R - L + 1);

        string toks[3];
        int n = splitBySpaces(sub, toks, 3);
        if (n >= 1)
        {
            string pname = toks[0];
            int charge = 0;
            if (n >= 2)
            {
                string num = toks[1];
                charge = 0;

                int sign = 1;
                int i = 0;

                // Check for a minus sign
                if (num.length() > 0 && num[0] == '-')
                {
                    sign = -1;
                    i = 1;
                }

                // Convert each character manually
                for (; i < num.length(); i++)
                {
                    char c = num[i];

                    if (c >= '0' && c <= '9')
                    {
                        int digit = c - '0';
                        charge = charge * 10 + digit;
                    }
                    else
                    {
                        // If any non-digit found tou charge be 0
                        charge = 0;
                        break;
                    }
                }

                charge = charge * sign;
            }

            g.findOrCreatePort(pname, charge, true);
        }
    }
    fin.close();
}

RouteArray *parseRoutesFile(const string &filename)
{
    ifstream fin(filename.c_str());
    if (!fin.is_open())
    {
        cout << "Could not open " << filename << ".\n";
        return nullptr;
    }

    RouteArray *arr = new RouteArray();
    string line;
    while (getline(fin, line))
    {
        // basic trim
        int L = 0, R = (int)line.size() - 1;

        while (L <= R && (line[L] == ' ' || line[L] == '\t'))
        {
            L++;
        }
        while (R >= L && (line[R] == ' ' || line[R] == '\t'))
        {
            R--;
        }

        if (L > R)
            continue;
        string sub = line.substr(L, R - L + 1);

        string toks[10];
        int n = splitBySpaces(sub, toks, 10);
        if (n < 6)
            continue; // masla hai kyunke we expect 7 tokens

        RouteRec *r = new RouteRec();
        r->origin = toks[0];
        r->dest = toks[1];
        r->date = (n >= 3) ? toks[2] : string();
        r->depart = (n >= 4) ? toks[3] : string();
        r->arrive = (n >= 5) ? toks[4] : string();
        r->cost = 0;

        if (n >= 6)
        {
            string num = toks[5];
            r->cost = 0;

            int sign = 1;
            int i = 0;

            // Handle negative sign
            if (num.length() > 0 && num[0] == '-')
            {
                sign = -1;
                i = 1;
            }

            // conversion to int from string
            for (; i < num.length(); i++)
            {
                char c = num[i];

                if (c >= '0' && c <= '9')
                {
                    int digit = c - '0';
                    r->cost = r->cost * 10 + digit;
                }
                else
                {
                    // validation, if some issue tou 0 set krdo and move on, MOVE ON ALI!!!
                    r->cost = 0;
                    break;
                }
            }

            r->cost = r->cost * sign;
        }

        r->company = (n >= 7) ? toks[6] : string("Unknown");

        arr->push(r);
    }
    fin.close();
    return arr;
}

//-------------------------------------
//   Build graph from route array     -
//-------------------------------------

void buildGraphFromRouteArray(RouteArray *arr, Graph &g)
{
    if (!arr)
        return;

    // Ensure all ports exist
    for (int i = 0; i < arr->size; ++i)
    {
        RouteRec *r = arr->arr[i];
        g.findOrCreatePort(r->origin, 0, false);
        g.findOrCreatePort(r->dest, 0, false);
    }

    // Insert edges
    for (int i = 0; i < arr->size; ++i)
    {
        RouteRec *r = arr->arr[i];
        g.insertEdge(r->origin, r->dest, r->date, r->depart, r->arrive, r->cost, r->company);
    }
}

//-----------------------------------
//    PATH VALIDITTY CHECK          -
//-----------------------------------
bool isOnPath(SimpleLinkedList *pathList, string from, string to)
{
    if (pathList == nullptr || pathList->head == nullptr)
        return false;

    LNode *current = pathList->head;
    while (current->next != nullptr)
    {
        if (current->portName == from && current->next->portName == to)
        {
            return true;
        }
        current = current->next;
    }
    return false;
}

// ------------------------------------
//          FILTER GLOBALS            -
// ------------------------------------
int filterSubState = 0;
string selectedCompanyFilter = ""; // Stores which company user clicked
int maxTimeFilter = 600;           // Default 10 hours

string companiesList[] = {"MaerskLine", "MSC", "ZIM", "COSCO", "Evergreen", "CMA_CGM", "HapagLloyd", "PIL", "YangMing", "ONE"};
SimpleLinkedList *avoidedPortsList = new SimpleLinkedList();
bool filtersApplied = false; // ENTER = applies filters

bool isAvoided(SimpleLinkedList *list, string name)
{
    if (!list || !list->head)
        return false;
    LNode *curr = list->head;
    while (curr)
    {
        if (curr->portName == name)
            return true;
        curr = curr->next;
    }
    return false;
}

//-----------------------------------
//    setting points for ports      -
//-----------------------------------

void setportscoordinates(Graph &g)
{
    Port *p = g.ports;
    float xresize = 1200.0f / 1100.0f; // scaling factor
    float yresize = 950.0f / 750.0f;

    while (p != NULL)
    {
        if (p->name == "Chittagong")
        {
            p->cordx = 767 * xresize;
            p->cordy = 362 * yresize;
        }
        else if (p->name == "London")
        {
            p->cordx = 518 * xresize;
            p->cordy = 261 * yresize;
        }
        else if (p->name == "Antwerp")
        {
            p->cordx = 541 * xresize;
            p->cordy = 263 * yresize;
        }
        else if (p->name == "LosAngeles")
        {
            p->cordx = 211 * xresize;
            p->cordy = 327 * yresize;
        }
        else if (p->name == "Vancouver")
        {
            p->cordx = 197 * xresize;
            p->cordy = 276 * yresize;
        }
        else if (p->name == "Copenhagen")
        {
            p->cordx = 557 * xresize;
            p->cordy = 240 * yresize;
        }
        else if (p->name == "Doha")
        {
            p->cordx = 653 * xresize;
            p->cordy = 357 * yresize;
        }
        else if (p->name == "AbuDhabi")
        {
            p->cordx = 665 * xresize;
            p->cordy = 357 * yresize;
        }
        else if (p->name == "NewYork")
        {
            p->cordx = 328 * xresize;
            p->cordy = 299 * yresize;
        }
        else if (p->name == "Alexandria")
        {
            p->cordx = 598 * xresize;
            p->cordy = 340 * yresize;
        }
        else if (p->name == "Shanghai")
        {
            p->cordx = 840 * xresize;
            p->cordy = 330 * yresize;
        }
        else if (p->name == "Busan")
        {
            p->cordx = 861 * xresize;
            p->cordy = 317 * yresize;
        }
        else if (p->name == "PortLouis")
        {
            p->cordx = 671 * xresize;
            p->cordy = 492 * yresize;
        }
        else if (p->name == "Stockholm")
        {
            p->cordx = 562 * xresize;
            p->cordy = 222 * yresize;
        }
        else if (p->name == "Singapore")
        {
            p->cordx = 795 * xresize;
            p->cordy = 421 * yresize;
        }
        else if (p->name == "CapeTown")
        {
            p->cordx = 570 * xresize;
            p->cordy = 529 * yresize;
        }
        else if (p->name == "Dublin")
        {
            p->cordx = 501 * xresize;
            p->cordy = 255 * yresize;
        }
        else if (p->name == "Athens")
        {
            p->cordx = 576 * xresize;
            p->cordy = 304 * yresize;
        }
        else if (p->name == "Tokyo")
        {
            p->cordx = 892 * xresize;
            p->cordy = 319 * yresize;
        }
        else if (p->name == "Jakarta")
        {
            p->cordx = 799 * xresize;
            p->cordy = 447 * yresize;
        }
        else if (p->name == "Istanbul")
        {
            p->cordx = 594 * xresize;
            p->cordy = 308 * yresize;
        }
        else if (p->name == "Manila")
        {
            p->cordx = 840 * xresize;
            p->cordy = 383 * yresize;
        }
        else if (p->name == "Mumbai")
        {
            p->cordx = 715 * xresize;
            p->cordy = 375 * yresize;
        }
        else if (p->name == "Montreal")
        {
            p->cordx = 333 * xresize;
            p->cordy = 272 * yresize;
        }
        else if (p->name == "Helsinki")
        {
            p->cordx = 586 * xresize;
            p->cordy = 222 * yresize;
        }
        else if (p->name == "Colombo")
        {
            p->cordx = 732 * xresize;
            p->cordy = 409 * yresize;
        }
        else if (p->name == "Melbourne")
        {
            p->cordx = 901 * xresize;
            p->cordy = 547 * yresize;
        }
        else if (p->name == "Sydney")
        {
            p->cordx = 919 * xresize;
            p->cordy = 530 * yresize;
        }
        else if (p->name == "Rotterdam")
        {
            p->cordx = 536 * xresize;
            p->cordy = 255 * yresize;
        }
        else if (p->name == "Hamburg")
        {
            p->cordx = 548 * xresize;
            p->cordy = 259 * yresize;
        }
        else if (p->name == "Lisbon")
        {
            p->cordx = 499 * xresize;
            p->cordy = 313 * yresize;
        }
        else if (p->name == "Osaka")
        {
            p->cordx = 877 * xresize;
            p->cordy = 326 * yresize;
        }
        else if (p->name == "Karachi")
        {
            p->cordx = 699 * xresize;
            p->cordy = 353 * yresize;
        }
        else if (p->name == "Genoa")
        {
            p->cordx = 541 * xresize;
            p->cordy = 287 * yresize;
        }
        else if (p->name == "Dubai")
        {
            p->cordx = 676 * xresize;
            p->cordy = 362 * yresize;
        }
        else if (p->name == "Oslo")
        {
            p->cordx = 545 * xresize;
            p->cordy = 227 * yresize;
        }
        else if (p->name == "Marseille")
        {
            p->cordx = 535 * xresize;
            p->cordy = 292 * yresize;
        }
        else if (p->name == "Durban")
        {
            p->cordx = 592 * xresize;
            p->cordy = 528 * yresize;
        }
        else if (p->name == "Jeddah")
        {
            p->cordx = 626 * xresize;
            p->cordy = 366 * yresize;
        }
        else if (p->name == "HongKong")
        {
            p->cordx = 820 * xresize;
            p->cordy = 358 * yresize;
        }
        p = p->next;
    }
}
void drawports(Graph &g, CircleShape &cport, RenderWindow &window, Vector2i &Currentmouse, string &consoleText, int &appState, string &source, string &dest, SimpleLinkedList *journey)
{
    Port *p = g.ports;
    while (p != NULL)
    {
        cport.setPosition(p->cordx, p->cordy);

        //-----------------------------------
        //              COLORS              -
        //-----------------------------------

        if (appState == 3) // Multi-Leg Mode
        {
            bool inJourney = false;
            if (journey)
            {
                LNode *n = journey->head;
                while (n)
                {
                    if (n->portName == p->name)
                    {
                        inJourney = true;
                        break;
                    }
                    n = n->next;
                }
            }
            if (inJourney)
                cport.setFillColor(Color::Yellow);
            else
                cport.setFillColor(Color(150, 150, 150));
        }
        else if (appState == 4 && filterSubState == 2)
        {
            // If port is in the avoided list -> YELLOW
            if (isAvoided(avoidedPortsList, p->name))
                cport.setFillColor(Color::Yellow);
            else
                cport.setFillColor(Color::Red);
        }
        else if (appState == 1 || appState == 2) // Booking Modes
        {
            if (p->name == source || p->name == dest)
                cport.setFillColor(Color::Blue);
            else
                cport.setFillColor(Color(150, 150, 150));
        }
        else
            cport.setFillColor(Color::Red); // Normal Mode

        //-----------------------------------
        //               MOUSE              -
        //-----------------------------------

        float mouseX = (float)Currentmouse.x;
        float mouseY = (float)Currentmouse.y;

        if (cport.getGlobalBounds().contains(mouseX, mouseY))
        {
            if (appState == 3)
            {
                if (Mouse::isButtonPressed(Mouse::Left))
                {
                    journey->append(p->name);
                    sleep(milliseconds(200));
                }
                else if (Mouse::isButtonPressed(Mouse::Right))
                {
                    journey->remove(p->name);
                    sleep(milliseconds(200));
                }
            }
            else if (appState == 1 && Mouse::isButtonPressed(Mouse::Left))
            {
                if (source == "")
                {
                    source = p->name;
                    sleep(milliseconds(200));
                }
                else if (dest == "")
                {
                    dest = p->name;
                    appState = 2;
                    sleep(milliseconds(200));
                }
            }
            else if ((appState == 0 || appState == 4) && Mouse::isButtonPressed(Mouse::Left))
            {
                consoleText = "PORT DETAILS\n\nName: " + p->name + "\nCharge: $" + to_string(p->charge);
            }
        }
        window.draw(cport);
        p = p->next;
    }
}
//-----------------------------------
//    setting Routes b/w ports      -
//-----------------------------------
float getDistance(float x1, float y1, float x2, float y2)
{
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}
void drawroutes(Graph &g, RenderWindow &window, Vector2i &Currentmouse, Font &font, string &consoleText, int appState, PathInfo *cheapP, PathInfo *fastP, SimpleLinkedList *journey, bool isMultiLegDone, UserPreferences &prefs)
{
    Port *p = g.ports;
    while (p != NULL)
    {
        Edge *route = p->adj;
        while (route != NULL)
        {
            float x1 = p->cordx;
            float y1 = p->cordy;
            float x2 = route->dest->cordx;
            float y2 = route->dest->cordy;

            Color lineColor = Color::White;
            bool shouldDraw = true;

            //-----------------------------------
            //          FILTER MODE             -
            //-----------------------------------

            if (appState == 4)
            {
                if (filtersApplied && prefs.isActive)
                {
                    if (g.edgeMatchesPreferences(route, p, prefs))
                        lineColor = Color::Cyan; // Active Route
                    else
                        lineColor = Color(40, 40, 40, 10);
                }
                else
                {
                    lineColor = Color(40, 40, 40, 10); // Very dim grey (Ghost routes)
                }
            }
            //-----------------------------------
            //             Booking Mode         -
            //-----------------------------------

            else if (appState == 1)
                shouldDraw = false;
            else if (appState == 2)
            {
                shouldDraw = false;
                if (cheapP && isOnPath(cheapP->path, p->name, route->dest->name))
                {
                    lineColor = Color::Green;
                    shouldDraw = true;
                }
                if (fastP && isOnPath(fastP->path, p->name, route->dest->name))
                {
                    lineColor = Color::Red;
                    shouldDraw = true;
                }
            }

            //-----------------------------------
            //          MULTILEG MODE           -
            //-----------------------------------

            else if (appState == 3)
            {
                shouldDraw = false;
                if (isMultiLegDone && journey && isOnPath(journey, p->name, route->dest->name))
                {
                    lineColor = Color::Yellow;
                    shouldDraw = true;
                }
            }
            else
            {
                if (route->cost < 15000 && route->cost > 0)
                    lineColor = Color(0, 255, 0);
                else if (route->cost < 30000)
                    lineColor = Color(255, 255, 0);
                else
                    lineColor = Color(255, 0, 0);
            }

            if (shouldDraw)
            {
                Vertex line[] = {Vertex(Vector2f(x1, y1), lineColor), Vertex(Vector2f(x2, y2), lineColor)};
                window.draw(line, 2, Lines);
            }
            route = route->next;
        }
        p = p->next;
    }
}

//--------------------------------------------------
//                 LAYOVER DOTTED LINES             -
//---------------------------------------------------
// dotted lines for layover
void drawDottedLine(RenderWindow &window, float x1, float y1, float x2, float y2, Color color)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    float steps = max(abs(dx), abs(dy)) / 15.0f; // Gap size

    float xIncrement = dx / steps;
    float yIncrement = dy / steps;

    float currentX = x1;
    float currentY = y1;

    CircleShape dot(3.0f);
    dot.setFillColor(color);

    for (int i = 0; i < steps; i++)
    {
        dot.setPosition(currentX, currentY);
        window.draw(dot);
        currentX += xIncrement;
        currentY += yIncrement;
    }
}

//---------------------------------------------------
//                  COST OF MULTILEG                -
//---------------------------------------------------
int calculateJourneyCost(Graph &g, SimpleLinkedList *journey)
{
    if (!journey || !journey->head)
        return 0;

    int total = 0;
    LNode *curr = journey->head;

    cout << "\n[Multi-Leg Calculation]\n";

    while (curr && curr->next)
    {
        string u = curr->portName;
        string v = curr->next->portName;
        bool found = false;

        // Find edge u -> v
        Port *p = g.findPort(u);
        if (p)
        {
            Edge *e = p->adj;
            while (e)
            {
                if (e->dest->name == v)
                {
                    total += e->cost;
                    cout << "  Leg Found: " << u << " -> " << v << " ($" << e->cost << ")\n";
                    found = true;
                    break;
                }
                e = e->next;
            }
        }

        if (!found)
        {
            cout << "  X NO DIRECT ROUTE: " << u << " -> " << v << " (Cost 0)\n";
        }

        curr = curr->next;
    }
    cout << "Total Journey Cost: " << total << "\n";
    return total;
}
// cost of a manual journey and validity of each leg
int validateMultiLegJourney(Graph &g, SimpleLinkedList *journey, bool &isValid)
{
    if (!journey || !journey->head)
        return 0;

    int totalCost = 0;
    isValid = true; // Assume valid initially
    LNode *curr = journey->head;

    while (curr && curr->next)
    {
        string u = curr->portName;
        string v = curr->next->portName;

        // Check if edge u -> v exists in Graph
        Port *p = g.findPort(u);
        bool legFound = false;
        if (p)
        {
            Edge *e = p->adj;
            while (e)
            {
                if (e->dest->name == v)
                {
                    totalCost += e->cost;
                    legFound = true;
                    break;
                }
                e = e->next;
            }
        }

        if (!legFound)
            isValid = false; // Found a broken link
        curr = curr->next;
    }
    return totalCost;
}

// ---------------------------------------------------
//                  MODE 1: NORMAL                   -
// ---------------------------------------------------
void drawNormalState(Graph &g, RenderWindow &window, Vector2i &Currentmouse, string &consoleText)
{
    Port *p = g.ports;
    float mouseX = (float)Currentmouse.x;
    float mouseY = (float)Currentmouse.y;

    while (p != NULL)
    {
        Edge *route = p->adj;
        while (route != NULL)
        {
            float x1 = p->cordx;
            float y1 = p->cordy;
            float x2 = route->dest->cordx;
            float y2 = route->dest->cordy;

            // Color based on cost
            Color lineColor;
            if (route->cost < 15000 && route->cost > 0)
                lineColor = Color(0, 255, 0); // Green
            else if (route->cost < 30000)
                lineColor = Color(255, 255, 0); // Yellow
            else
                lineColor = Color(255, 0, 0); // Red

            Vertex line[] = {Vertex(Vector2f(x1, y1), lineColor), Vertex(Vector2f(x2, y2), lineColor)};
            window.draw(line, 2, Lines);

            // Interaction (Hover over line)
            float dist_Start_Mouse = getDistance(x1, y1, mouseX, mouseY);
            float dist_Mouse_End = getDistance(mouseX, mouseY, x2, y2);
            float dist_Start_End = getDistance(x1, y1, x2, y2);

            if ((dist_Start_Mouse + dist_Mouse_End) >= (dist_Start_End - 0.01f) &&
                (dist_Start_Mouse + dist_Mouse_End) <= (dist_Start_End + 0.01f) &&
                Mouse::isButtonPressed(Mouse::Left))
            {
                consoleText = "ROUTE DETAILS\n\nFrom: " + p->name + "\nTo: " + route->dest->name +
                              "\nCost: $" + to_string(route->cost) +
                              "\nTime: " + route->depart_time + " - " + route->arrive_time;

                Vertex highlight[] = {Vertex(Vector2f(x1, y1), Color::Cyan), Vertex(Vector2f(x2, y2), Color::Cyan)};
                window.draw(highlight, 2, Lines);
            }
            route = route->next;
        }
        p = p->next;
    }

    // 2. Draw Ports
    p = g.ports; // Reset pointer
    CircleShape cport(5.f);
    cport.setFillColor(Color::Red);

    while (p != NULL)
    {
        cport.setPosition(p->cordx, p->cordy);

        if (cport.getGlobalBounds().contains(mouseX, mouseY) && Mouse::isButtonPressed(Mouse::Left))
        {
            int cx = p->cordx;
            int cy = p->cordy;
            consoleText = "PORT DETAILS\n\nName: " + p->name + "\nCharge: $" + to_string(p->charge) + "\n" + "Xcoord: " + to_string(cx) + "\nYCoord: " + to_string(cy);
        }
        window.draw(cport);
        p = p->next;
    }
}

// ---------------------------------------------------
//                  MODE 2: BOOKING                   -
// ---------------------------------------------------
void drawBookingState(Graph &g, RenderWindow &window, Vector2i &Currentmouse, string &consoleText, int appState, string &source, string &dest, PathInfo *cheapP, PathInfo *fastP)
{
    // Draw Routes
    if (appState == 2)
    {
        Port *p = g.ports;
        while (p != NULL)
        {
            Edge *route = p->adj;
            while (route != NULL)
            {
                bool draw = false;
                Color col = Color::White;

                if (cheapP && isOnPath(cheapP->path, p->name, route->dest->name))
                {
                    col = Color::Green;
                    draw = true;
                }
                if (fastP && isOnPath(fastP->path, p->name, route->dest->name))
                {
                    col = Color::Red;
                    draw = true;
                }

                if (draw)
                {
                    //-----------------------------------
                    //               LAYOVER            -
                    //-----------------------------------
                    if (p->name != source)
                    {
                        drawDottedLine(window, p->cordx, p->cordy, route->dest->cordx, route->dest->cordy, col);
                    }
                    else
                    {
                        // Direct leg -> SOLID
                        Vertex line[] = {Vertex(Vector2f(p->cordx, p->cordy), col), Vertex(Vector2f(route->dest->cordx, route->dest->cordy), col)};
                        window.draw(line, 2, Lines);
                    }
                }
                route = route->next;
            }
            p = p->next;
        }
    }

    // Draw Ports
    Port *p = g.ports;
    CircleShape cport(5.f);
    float mouseX = (float)Currentmouse.x;
    float mouseY = (float)Currentmouse.y;

    while (p != NULL)
    {
        cport.setPosition(p->cordx, p->cordy);

        if (p->name == source)
            cport.setFillColor(Color::Blue);
        else if (p->name == dest)
            cport.setFillColor(Color::Cyan);
        else
            cport.setFillColor(Color(150, 150, 150));

        if (cport.getGlobalBounds().contains(mouseX, mouseY) && appState == 1 && Mouse::isButtonPressed(Mouse::Left))
        {
            if (source == "")
            {
                source = p->name;
                consoleText = "Origin: " + source + "\n\nStep 2:\nSelect Destination Port.";
                sleep(milliseconds(200));
            }
            else if (dest == "" && p->name != source)
            {
                dest = p->name;
                consoleText = "Origin: " + source + "\nDest: " + dest + "\n\nCalculating Route...";
                sleep(milliseconds(200));
            }
        }
        window.draw(cport);
        p = p->next;
    }
}
// ---------------------------------------------------
//                  MODE 3: MULTI-LEG                -
// ---------------------------------------------------
void drawMultiLegState(Graph &g, RenderWindow &window, Vector2i &Currentmouse, string &consoleText, SimpleLinkedList *journey, bool isFinished)
{
    // Draw Routes
    if (isFinished && journey)
    {
        LNode *curr = journey->head;
        while (curr && curr->next)
        {
            Port *p1 = g.findPort(curr->portName);
            Port *p2 = g.findPort(curr->next->portName);

            if (p1 && p2)
            {
                // Check if this specific leg exists
                bool legExists = false;
                Edge *e = p1->adj;
                while (e)
                {
                    if (e->dest->name == p2->name)
                    {
                        legExists = true;
                        break;
                    }
                    e = e->next;
                }

                Color col = legExists ? Color::Yellow : Color::Red;

                Vertex line[] = {
                    Vertex(Vector2f(p1->cordx, p1->cordy), col),
                    Vertex(Vector2f(p2->cordx, p2->cordy), col)};
                window.draw(line, 2, Lines);
            }
            curr = curr->next;
        }
    }

    // 2. Draw Ports (Selection Logic - largely same as before)
    Port *p = g.ports;
    float mouseX = (float)Currentmouse.x;
    float mouseY = (float)Currentmouse.y;

    while (p != NULL)
    {
        CircleShape cport(5.f);
        cport.setPosition(p->cordx - 5, p->cordy - 5);

        // Highlight selected ports
        bool inJourney = false;
        if (journey)
        {
            LNode *n = journey->head;
            while (n)
            {
                if (n->portName == p->name)
                    inJourney = true;
                n = n->next;
            }
        }

        // Visuals
        if (inJourney)
            cport.setFillColor(Color::Yellow);
        else
            cport.setFillColor(Color(100, 100, 100)); // Grey if not selected

        // Interaction (Add/Remove)
        if (cport.getGlobalBounds().contains(mouseX, mouseY))
        {
            if (Mouse::isButtonPressed(Mouse::Left))
            {
                journey->append(p->name);
                sleep(milliseconds(200)); // Prevent double clicks
            }
            else if (Mouse::isButtonPressed(Mouse::Right))
            {
                journey->remove(p->name);
                sleep(milliseconds(200));
            }
        }
        window.draw(cport);
        p = p->next;
    }
}

// ---------------------------------------------------
//                  MODE 4: FILTERS                  -
// ---------------------------------------------------
void drawFilter(RenderWindow &window, Font &font, int subState, string &consoleText)
{
    // Draw Sub-State 0: Main Filter Options
    if (subState == 0)
    {
        consoleText = "FILTER MENU:\n\n1. Filter by Company\n2. Avoid Specific Nodes\n3. Max Voyage Time\n\n4. RESET ALL FILTERS\n\n(Press 1-4 on Keyboard)";
    }
    // Draw Sub-State 1: Company Buttons
    else if (subState == 1)
    {
        consoleText = "SELECT COMPANY\n";

        int startX = 1210;
        int startY = 500;

        for (int i = 0; i < 10; i++)
        {
            RectangleShape btn(Vector2f(140, 30));
            btn.setPosition(startX + (i % 2) * 150, startY + (i / 2) * 40);

            // Yellow if selected (pending), Black if not
            if (companiesList[i] == selectedCompanyFilter)
                btn.setFillColor(Color::Yellow);
            else
                btn.setFillColor(Color(50, 50, 50));

            btn.setOutlineColor(Color::White);
            btn.setOutlineThickness(1);

            Text t;
            t.setFont(font);
            t.setString(companiesList[i]);
            t.setPosition(startX + (i % 2) * 150 + 5, startY + (i / 2) * 40 + 5);
            t.setCharacterSize(14);

            // Text color logic
            if (companiesList[i] == selectedCompanyFilter)
                t.setFillColor(Color::Black);
            else
                t.setFillColor(Color::White);

            window.draw(btn);
            window.draw(t);
        }
    }
    // Draw Sub-State 2: Avoid Nodes
    else if (subState == 2)
    {
        consoleText = "AVOID NODES:\n\nClick on ports in map\nto disable them.\n\nPorts to Avoid:\n";
        LNode *n = avoidedPortsList->head;
        while (n)
        {
            consoleText += "- " + n->portName + "\n";
            n = n->next;
        }
        consoleText += "\n[Press Enter to Apply]";
    }
    // Draw Sub-State 3: Max Time
    else if (subState == 3)
    {
        int hours = maxTimeFilter / 60;
        consoleText = "MAX VOYAGE TIME:\n\nUse Left/Right Arrows\nto adjust limit.\n\nCurrent Limit:\n" + to_string(hours) + " Hours";
        consoleText += "\n\nPress ENTER to Apply.";
    }
}

//-----------------------------------------------
//                  LEGEND                      -
//-----------------------------------------------
void drawLegend(RenderWindow &window, Font &font)
{
    // Create a background strip
    RectangleShape bg(Vector2f(1150, 90));
    bg.setPosition(25, 800);
    bg.setFillColor(Color(20, 20, 20, 150));
    bg.setOutlineColor(Color(100, 100, 100));
    bg.setOutlineThickness(1);
    window.draw(bg);

    // Setup Text
    Text t;
    t.setFont(font);
    t.setCharacterSize(14);
    t.setFillColor(Color::White);

    //-----------------------------------
    //          KEYBOARD SHORTCUTS      -
    //-----------------------------------

    t.setStyle(Text::Bold);
    t.setString("KEYBOARD CONTROLS");
    t.setPosition(40, 810);
    window.draw(t);

    t.setStyle(Text::Regular);
    t.setString("[M] -> Reset / Main Menu\n[S] -> Save Screenshot\n[Esc] -> Exit Application");
    t.setPosition(40, 825);
    window.draw(t);

    t.setString("[Arrows] -> Adjust Filters\n[Enter] -> Confirm Selection\n[1-3] -> Select Filter Type");
    t.setPosition(250, 825);
    window.draw(t);

    //-----------------------------------
    //             MOUSE DETAILS        -
    //-----------------------------------

    t.setStyle(Text::Bold);
    t.setString("MOUSE ACTIONS");
    t.setPosition(480, 810);
    window.draw(t);

    t.setStyle(Text::Regular);
    t.setString("[Left Click] -> Select Port / Add Node\n[Right Click] -> Remove Node (Multi-Leg)");
    t.setPosition(480, 825);
    window.draw(t);

    //-----------------------------------
    //          ROUTE COLORS            -
    //-----------------------------------

    t.setStyle(Text::Bold);
    t.setString("MAP LEGEND");
    t.setPosition(800, 805);
    window.draw(t);

    // Color Boxes
    RectangleShape box(Vector2f(12, 12));
    t.setStyle(Text::Regular);

    // Green
    box.setFillColor(Color::Green);
    box.setPosition(800, 825);
    window.draw(box);
    t.setString("Cheapest");
    t.setPosition(820, 823);
    window.draw(t);

    // Red
    box.setFillColor(Color::Red);
    box.setPosition(800, 845);
    window.draw(box);
    t.setString("Broken/Expensive/Fast");
    t.setPosition(820, 843);
    window.draw(t);

    // Yellow
    box.setFillColor(Color::Yellow);
    box.setPosition(980, 825);
    window.draw(box);
    t.setString("Selection / Valid Multi-Leg");
    t.setPosition(1000, 823);
    window.draw(t);

    // Cyan
    box.setFillColor(Color::Cyan);
    box.setPosition(980, 845);
    window.draw(box);
    t.setString("Active Route");
    t.setPosition(1000, 843);
    window.draw(t);
}

//--------------------------------------------------
//                  INT MAIN                        -
//---------------------------------------------------
int main()
{
    //---------------------------------------------------
    //                    MAIN LOGIC                    -
    //---------------------------------------------------
    Graph g;
    // parse ports
    const string portsFile = "Ports.txt";
    cout << "Parsing " << portsFile << " ...\n";
    parsePortsFile(portsFile, g);

    // parse routes
    const string routesFile = "Routes.txt";
    cout << "Parsing " << routesFile << " ...\n";
    RouteArray *routes = parseRoutesFile(routesFile);
    if (!routes)
    {
        cout << "No routes loaded. Exiting.\n";
        return 0;
    }
    cout << "Loaded " << routes->size << " route records (temporary storage).\n";

    // build graph
    cout << "Building graph from route records...\n";
    buildGraphFromRouteArray(routes, g);

    cout << "Node count (unique ports) = " << g.nodeCount << "\n";
    cout << "Edge count (route entries)  = " << g.countEdges() << "\n";

    cout << "\n";
    g.printSummary();

    // TESTING PATHFINDING
    cout << "\n\n";
    cout << "========================================\n";
    cout << "  TESTING PATHFINDING ALGORITHMS\n";
    cout << "========================================\n";

    // Test 1: Cheapest path
    cout << "\n--- TEST 1: Find CHEAPEST path ---\n";
    PathInfo *cheapPath = g.findCheapestPath("Karachi", "Singapore");
    cheapPath->printInfo();
    delete cheapPath;

    // Test 2: Shortest time path
    cout << "\n--- TEST 2: Find SHORTEST TIME path ---\n";
    PathInfo *fastPath = g.findShortestTimePath("Karachi", "Singapore");
    fastPath->printInfo();
    delete fastPath;

    // Test 3: Another example
    cout << "\n--- TEST 3: Mumbai to London (Cheapest) ---\n";
    PathInfo *test3 = g.findCheapestPath("Mumbai", "London");
    test3->printInfo();
    delete test3;

    cout << "\n========================================\n";
    cout << "  TESTS COMPLETE - Starting GUI...\n";
    cout << "========================================\n\n";

    cout << "\nTesting docking queue:\n";
    LinkedQueue dq;
    dq.enqueue("Ship-A");
    dq.enqueue("Ship-B");
    dq.enqueue("Ship-C");
    cout << "Dequeued: " << dq.dequeue() << "\n";
    dq.clear();

    cout << "\nTesting a multi-leg journey:\n";
    SimpleLinkedList journey;
    journey.append("HongKong");
    journey.append("Dubai");
    journey.append("Rotterdam");
    journey.print();
    journey.clear();
    setportscoordinates(g);
    //---------------------------------------------------
    //             SFML GRAPHICS WORKING                -
    //---------------------------------------------------

    RenderWindow window(VideoMode(1550, 750), "OceanRoute Nav", Style::Fullscreen);
    window.setFramerateLimit(60);
    Font font;
    font.loadFromFile("font.ttf");

    // Setting up worldmap
    RectangleShape backgrnd;
    backgrnd.setSize(Vector2f(1200, 950)); //+100 +200
    Texture map;
    map.loadFromFile("worldmap.png");
    backgrnd.setTexture(&map);

    // Setting up Menu on the right side of screen
    RectangleShape menurectangle;
    menurectangle.setSize(Vector2f(350, 750));
    menurectangle.setPosition(Vector2f(1200, 0));
    Texture menu;
    menurectangle.setFillColor(sf::Color(0, 0, 0));

    RectangleShape mheading;
    Text txt;
    txt.setFont(font);
    txt.setString("MENU");
    txt.setPosition(Vector2f(1300, 25));
    txt.setFillColor(Color::White);
    txt.setCharacterSize(50);

    RectangleShape console;
    Text txtc;
    txtc.setFont(font);
    txtc.setString("Console");
    txtc.setPosition(Vector2f(1280, 400));
    txtc.setFillColor(Color::White);
    txtc.setCharacterSize(50);

    // Other menu Buttons(counting starts form bottom)
    RectangleShape mb1; // menu button 1
    mb1.setSize(Vector2f(250, 50));
    Texture mb;
    mb.loadFromFile("button.png");
    mb1.setTexture(&mb);
    mb1.setPosition(Vector2f(1250, 100));
    Text txt1;
    txt1.setFont(font);
    txt1.setString("Book Route");
    txt1.setPosition(Vector2f(1320, 112));
    txt1.setFillColor(Color::White);
    txt1.setCharacterSize(20);

    RectangleShape mb2; // menu button 2
    mb2.setSize(Vector2f(250, 50));
    Texture mbb;
    mbb.loadFromFile("button.png");
    mb2.setTexture(&mbb);
    mb2.setPosition(Vector2f(1250, 150));
    Text txt2;
    txt2.setFont(font);
    txt2.setString("Save screen");
    txt2.setPosition(Vector2f(1315, 162));
    txt2.setFillColor(Color::White);
    txt2.setCharacterSize(20);

    RectangleShape mb3; // menu button 3
    mb3.setSize(Vector2f(250, 50));
    Texture mbbb;
    mbbb.loadFromFile("button.png");
    mb3.setTexture(&mbbb);
    mb3.setPosition(Vector2f(1250, 200));
    Text txt3;
    txt3.setFont(font);
    txt3.setString("Filter");
    txt3.setPosition(Vector2f(1350, 212));
    txt3.setFillColor(Color::White);
    txt3.setCharacterSize(20);

    RectangleShape mb4; // menu button 4
    mb4.setSize(Vector2f(250, 50));
    Texture mbbbb;
    mbbbb.loadFromFile("button.png");
    mb4.setTexture(&mbbbb);
    mb4.setPosition(Vector2f(1250, 250));
    Text txt4;
    txt4.setFont(font);
    txt4.setString("Multi-Leg");
    txt4.setPosition(Vector2f(1330, 262));
    txt4.setFillColor(Color::White);
    txt4.setCharacterSize(20);

    // Making buttons in menu
    RectangleShape exitbutton;
    exitbutton.setSize(Vector2f(250, 50));
    Texture exit;
    exit.loadFromFile("button.png");
    exitbutton.setTexture(&exit);
    exitbutton.setPosition(Vector2f(1250, 300));
    Text txt5;
    txt5.setFont(font);
    txt5.setString("Exit");
    txt5.setPosition(Vector2f(1355, 312));
    txt5.setFillColor(Color::White);
    txt5.setCharacterSize(20);

    // Coordinate fixing for the terminal
    Mouse mouse;
    Vector2i Currentmouse;

    // Making Circles for Ports
    CircleShape cport(5.f);
    cport.setFillColor(Color::Red);

    string consoleString = "Welcome to OceanRoute Nav.\nClick on a Port or Route\nto see details.";

    Text consoleDetails;
    consoleDetails.setFont(font);
    consoleDetails.setCharacterSize(18);
    consoleDetails.setFillColor(Color::Yellow);
    consoleDetails.setPosition(Vector2f(1220, 460));

    int appState = 0; // 0=Normal, 1=Booking Input, 2=Booking Result, 3=multileg 4=filters
    string sourcePort = "";
    string destPort = "";
    PathInfo *cheapestPathResult = nullptr;
    PathInfo *fastestPathResult = nullptr;
    bool pathCalculated = false;
    bool multiLegFinished = false;

    UserPreferences globalPrefs;

    // FOR MULTI-LEG
    SimpleLinkedList *customJourney = new SimpleLinkedList();

    while (window.isOpen())
    {
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        Currentmouse = Mouse::getPosition(window);
        float mouseX = (float)Currentmouse.x;
        float mouseY = (float)Currentmouse.y;
        //------------------------------------------
        //-         KEYBOARD KEYS OPERATIONS       -
        //------------------------------------------
        if (Keyboard::isKeyPressed(Keyboard::Escape))
            window.close();

        if (Keyboard::isKeyPressed(Keyboard::M))
        {
            appState = 0;
            pathCalculated = false;
            sourcePort = "";
            destPort = "";
            customJourney->clear();
            multiLegFinished = false;
            consoleString = "Normal Mode.\nClick ports/routes for details.";
        }

        if (appState == 3 && !multiLegFinished && (Keyboard::isKeyPressed(Keyboard::D) || Keyboard::isKeyPressed(Keyboard::Enter)))
        {
            multiLegFinished = true;

            bool isValid = true;
            int totalC = validateMultiLegJourney(g, customJourney, isValid);

            consoleString = "JOURNEY STATUS:\n\n";

            // Construct the path string
            LNode *n = customJourney->head;
            while (n)
            {
                consoleString += n->portName;
                if (n->next)
                    consoleString += " ->\n";
                n = n->next;
            }

            consoleString += "\n----------------\n";

            if (isValid)
            {
                consoleString += "Route Valid!\nTotal Cost: $" + to_string(totalC);
            }
            else
            {
                consoleString += "WARNING: Invalid Route!\nBroken links shown in RED.\n(Cost incomplete)";
            }

            consoleString += "\n\nPress 'M' to Reset.";

            sleep(milliseconds(200));
        }
        // FILTER MODE INTERACTIONS
        if (appState == 4)
        {
            // ---------------------------------------------
            //          MENU SELECTION (Main Menu)
            // ---------------------------------------------
            if (filterSubState == 0)
            {
                if (Keyboard::isKeyPressed(Keyboard::Num1))
                {
                    filterSubState = 1;
                    sleep(milliseconds(200));
                }
                if (Keyboard::isKeyPressed(Keyboard::Num2))
                {
                    filterSubState = 2;
                    sleep(milliseconds(200));
                }
                if (Keyboard::isKeyPressed(Keyboard::Num3))
                {
                    filterSubState = 3;
                    sleep(milliseconds(200));
                }
                if (Keyboard::isKeyPressed(Keyboard::Num4))
                {
                    // Backend Reset
                    globalPrefs.reset();

                    // Frontend Variables Reset
                    selectedCompanyFilter = "";
                    maxTimeFilter = 600; // Default
                    avoidedPortsList->clear();
                    filtersApplied = false;

                    consoleString = "ALL FILTERS RESET.\nMap cleared.\n\n(Select a new filter)";
                    sleep(milliseconds(200));
                }
            }
            // ------------------------------------------
            //          COMPANY FILTER                  -
            // ------------------------------------------
            else if (filterSubState == 1)
            {
                if (Mouse::isButtonPressed(Mouse::Left))
                {
                    int startX = 1210;
                    int startY = 500;
                    for (int i = 0; i < 10; i++)
                    {
                        FloatRect btnRect(startX + (i % 2) * 150, startY + (i / 2) * 40, 140, 30);
                        if (btnRect.contains(mouseX, mouseY))
                        {
                            selectedCompanyFilter = companiesList[i];
                            filtersApplied = false; // Pending apply
                            consoleString = "Selected: " + selectedCompanyFilter + "\nPress ENTER to Add.";
                            sleep(milliseconds(200));
                        }
                    }
                }
                if (Keyboard::isKeyPressed(Keyboard::Enter) && selectedCompanyFilter != "")
                {
                    globalPrefs.setPreferredCompany(selectedCompanyFilter); // Add to preferences
                    filtersApplied = true;
                    consoleString = "FILTER ADDED:\n" + selectedCompanyFilter + "\n\n(Press Backspace to\nadd more filters)";
                    sleep(milliseconds(200));
                }
            }
            // --------------------------------------------
            //           AVOID PORTS FILTER               -
            // --------------------------------------------
            else if (filterSubState == 2 && Mouse::isButtonPressed(Mouse::Left))
            {
                Port *p = g.ports;
                while (p)
                {
                    FloatRect portRect(p->cordx - 5, p->cordy - 5, 15, 15);
                    if (portRect.contains(mouseX, mouseY))
                    {
                        // Backend
                        if (!globalPrefs.shouldAvoidPort(p->name))
                            globalPrefs.addAvoidedPort(p->name);
                        else
                            globalPrefs.removeAvoidedPort(p->name);

                        // frontend
                        if (!isAvoided(avoidedPortsList, p->name))
                            avoidedPortsList->append(p->name);
                        else
                            avoidedPortsList->remove(p->name);

                        sleep(milliseconds(200));
                    }
                    p = p->next;
                }
            }
            // ---------------------------------------------
            //           MAX TIME FILTER
            // ---------------------------------------------
            else if (filterSubState == 3)
            {
                if (Keyboard::isKeyPressed(Keyboard::Right))
                {
                    maxTimeFilter += 60;
                    filtersApplied = false;
                    sleep(milliseconds(100));
                }
                if (Keyboard::isKeyPressed(Keyboard::Left) && maxTimeFilter > 60)
                {
                    maxTimeFilter -= 60;
                    filtersApplied = false;
                    sleep(milliseconds(100));
                }

                if (Keyboard::isKeyPressed(Keyboard::Enter))
                {
                    globalPrefs.setMaxVoyageTime(maxTimeFilter); // Add to preferences
                    filtersApplied = true;
                    consoleString = "FILTER ADDED:\nMax Time: " + to_string(maxTimeFilter / 60) + "h\n\n(Press Backspace to\nadd more filters)";
                    sleep(milliseconds(200));
                }
            }

            // Backspace for back to menu
            if (Keyboard::isKeyPressed(Keyboard::BackSpace))
            {
                filterSubState = 0;
                sleep(milliseconds(200));
            }

            // RESET
            if ((mb3.getGlobalBounds().contains(mouseX, mouseY) && Mouse::isButtonPressed(Mouse::Left)))
            {
                globalPrefs.reset();
                selectedCompanyFilter = "";
                maxTimeFilter = 600;
                avoidedPortsList->clear();
                filtersApplied = false;
                filterSubState = 0;
                consoleString = "FILTERS RESET.";
                sleep(milliseconds(200));
            }
        }
        //----------------------------------------
        //        BONUS: Screensot feature       -
        //----------------------------------------
        if (Keyboard::isKeyPressed(Keyboard::S) || (mb2.getGlobalBounds().contains(mouseX, mouseY) && Mouse::isButtonPressed(Mouse::Left)))
        {
            Texture texture;
            texture.create(window.getSize().x, window.getSize().y);

            texture.update(window);

            Image screenshot = texture.copyToImage();

            if (screenshot.saveToFile("screenshot.png"))
            {
                consoleString = "SCREENSHOT SAVED!\n\n(Saved in Project Folder)";
            }
            sleep(milliseconds(500));
        }

        //-------------------------------
        //           BUTTONS            -
        //-------------------------------
        // Book Route
        if (mb1.getGlobalBounds().contains(mouseX, mouseY) && Mouse::isButtonPressed(Mouse::Left))
        {
            mb1.setFillColor(Color::Red);
            appState = 1;
            sourcePort = "";
            destPort = "";
            pathCalculated = false;
            if (cheapestPathResult)
            {
                delete cheapestPathResult;
                cheapestPathResult = nullptr;
            }
            if (fastestPathResult)
            {
                delete fastestPathResult;
                fastestPathResult = nullptr;
            }
            consoleString = "BOOKING MODE\n\nStep 1:\nSelect Origin Port.";
            sleep(milliseconds(200));
        }
        // 4. Multi-Leg
        if (mb4.getGlobalBounds().contains(mouseX, mouseY) && Mouse::isButtonPressed(Mouse::Left))
        {
            mb4.setFillColor(Color::Red);
            sourcePort = "";
            destPort = "";
            appState = 3;
            multiLegFinished = false;
            customJourney->clear();
            consoleString = "MULTI-LEG BUILDER:\n[Left] Add\n[Right] Remove\n[D] Finish";
            sleep(milliseconds(200));
        }
        // filter
        if (mb3.getGlobalBounds().contains(mouseX, mouseY) && Mouse::isButtonPressed(Mouse::Left))
        {
            mb3.setFillColor(Color::Red);
            appState = 4;
            filterSubState = 0; // Reset to main menu of filter
            selectedCompanyFilter = "";
            avoidedPortsList->clear(); // Reset avoided list
            sleep(milliseconds(200));
        }
        //  Exit
        if (exitbutton.getGlobalBounds().contains(mouseX, mouseY))
        {
            exitbutton.setFillColor(Color::Red);
            if (Mouse::isButtonPressed(Mouse::Left))
                window.close();
        }
        else
        {
            exitbutton.setFillColor(sf::Color(255, 255, 255));
        }

        // Reset colors
        mb1.setFillColor(sf::Color(255, 255, 255));
        mb2.setFillColor(sf::Color(255, 255, 255));
        mb3.setFillColor(sf::Color(255, 255, 255));
        mb4.setFillColor(sf::Color(255, 255, 255));

        //----------------------------
        //          MAIN DRAWINGs    -
        //----------------------------
        window.clear();
        window.draw(backgrnd);
        window.draw(menurectangle);
        window.draw(mheading);
        window.draw(txt);
        window.draw(mb1);
        window.draw(txt1);
        window.draw(mb2);
        window.draw(txt2);
        window.draw(mb3);
        window.draw(txt3);
        window.draw(mb4);
        window.draw(txt4);
        window.draw(exitbutton);
        window.draw(txt5);
        window.draw(console);
        window.draw(txtc);
        consoleDetails.setString(consoleString);
        window.draw(consoleDetails);
        //---------------------------------------------
        // -         STATE BASED DRAWING              -
        //---------------------------------------------
        if (appState == 0)
        {
            drawNormalState(g, window, Currentmouse, consoleString);
        }
        // ---------------------------------------------------
        //        MODE 1 & 2: BOOKING (With Integration)     -
        // ---------------------------------------------------
        else if (appState == 1 || appState == 2)
        {
            if (appState == 1 && sourcePort != "" && destPort != "")
            {
                appState = 2; // Switch to Result Mode

                if (cheapestPathResult)
                {
                    delete cheapestPathResult;
                    cheapestPathResult = nullptr;
                }
                if (fastestPathResult)
                {
                    delete fastestPathResult;
                    fastestPathResult = nullptr;
                }

                if (globalPrefs.isActive)
                {
                    // FILTERED SEARCH
                    consoleString = "CALCULATING...\n(Filters Applied)";
                    cheapestPathResult = g.findCheapestPathWithPreferences(sourcePort, destPort, globalPrefs);
                    fastestPathResult = g.findShortestTimePathWithPreferences(sourcePort, destPort, globalPrefs);
                }
                else
                {
                    //  NORMAL SEARCH
                    consoleString = "CALCULATING...\n(Standard)";
                    cheapestPathResult = g.findCheapestPath(sourcePort, destPort);
                    fastestPathResult = g.findShortestTimePath(sourcePort, destPort);
                }

                if (!cheapestPathResult->found)
                {
                    consoleString = "RESULT:\nNo Route Found!\n\n(Try resetting filters\nor picking other ports)";
                }
                else
                {
                    // Build Strings for console output
                    string cheapRouteStr = "";
                    LNode *n = cheapestPathResult->path->head;
                    int i = 0;
                    while (n)
                    {
                        cheapRouteStr += n->portName;
                        if (n->next)
                            cheapRouteStr += " -> ";
                        n = n->next;
                        i++;
                        if (i % 3 == 0)
                            cheapRouteStr += "\n";
                    }

                    string fastRouteStr = "";
                    n = fastestPathResult->path->head;
                    i = 0;
                    while (n)
                    {
                        fastRouteStr += n->portName;
                        if (n->next)
                            fastRouteStr += " -> ";
                        n = n->next;
                        i++;
                        if (i % 3 == 0)
                            fastRouteStr += "\n";
                    }

                    consoleString = "Book: " + sourcePort + " -> " + destPort + "\n";

                    // Visualof filters
                    if (globalPrefs.isActive)
                        consoleString += "[FILTERED RESULTS]\n\n";
                    else
                        consoleString += "\n";

                    consoleString += "CHEAPEST (Green):\n$" + to_string(cheapestPathResult->totalCost) + "\n";
                    consoleString += cheapRouteStr + "\n\n";
                    consoleString += "FASTEST (Red):\n" + to_string((int)minutesToHours(fastestPathResult->totalTime)) + " Hours\n";
                    consoleString += fastRouteStr + "\n";
                }
                pathCalculated = true;
            }

            drawBookingState(g, window, Currentmouse, consoleString, appState, sourcePort, destPort, cheapestPathResult, fastestPathResult);
        }
        else if (appState == 3)
        {
            drawMultiLegState(g, window, Currentmouse, consoleString, customJourney, multiLegFinished);

            // Console update for interaction
            if (!multiLegFinished && (Mouse::isButtonPressed(Mouse::Left) || Mouse::isButtonPressed(Mouse::Right)))
            {
                consoleString = "MULTI-LEG BUILDER:\n[Left] Add\n[Right] Remove\n[D] Finish\n\nRoute:\n";
                LNode *n = customJourney->head;
                while (n)
                {
                    consoleString += n->portName;
                    if (n->next)
                        consoleString += " ->\n";
                    n = n->next;
                }
            }
        }
        else if (appState == 4)
        {
            //  Draw Buttons
            drawFilter(window, font, filterSubState, consoleString);

            //  Draw Lines (Cyan / Dimmed)
            drawroutes(g, window, Currentmouse, font, consoleString, appState, cheapestPathResult, fastestPathResult, customJourney, multiLegFinished, globalPrefs);

            //  Draw Ports (Dots Only)
            drawports(g, cport, window, Currentmouse, consoleString, appState, sourcePort, destPort, customJourney);
        }
        drawLegend(window, font);
        window.display();
    }
    return 0;
}
