#include <iostream>
#include <fstream>
#include <cstring>
#include <climits>
#include <cctype>
using namespace std;

#define MAX_PORTS 15
#define MAX_NAME_LENGTH 30
#define MAX_DATE_LENGTH 11
#define MAX_TIME_LENGTH 6
#define MAX_COMPANY_LENGTH 15
#define DOCKING_SLOTS 2  // Fixed: All ports have 2 docking slots

// ---------------- STRUCTURES ----------------

// User preferences for route filtering
struct UserPreferences {
    bool hasCompanyFilter;
    char preferredCompany[MAX_COMPANY_LENGTH];
    bool hasAvoidPort;
    char avoidPort[MAX_NAME_LENGTH];
    bool hasMaxCostLimit;
    unsigned int maxCostLimit;
    bool hasMaxTimeLimit;
    unsigned int maxTimeLimit;
    
    UserPreferences() {
        hasCompanyFilter = false;
        hasAvoidPort = false;
        hasMaxCostLimit = false;
        hasMaxTimeLimit = false;
        preferredCompany[0] = '\0';
        avoidPort[0] = '\0';
        maxCostLimit = 0;
        maxTimeLimit = 0;
    }
    
    bool hasAnyFilter() const {
        return hasCompanyFilter || hasAvoidPort || hasMaxCostLimit || hasMaxTimeLimit;
    }
};

// Ship/Vessel information for queue
struct Ship {
    char shipName[MAX_NAME_LENGTH];
    char arrivalTime[MAX_TIME_LENGTH];
    char arrivalDate[MAX_DATE_LENGTH];
    unsigned short arrivalMins;
    unsigned int serviceTimeNeeded;  // in minutes
    char originPort[MAX_NAME_LENGTH];
    char destinationPort[MAX_NAME_LENGTH];
    char company[MAX_COMPANY_LENGTH];
    unsigned int voyageCost;
};

// Queue Node for ship queue
struct QueueNode {
    Ship ship;
    QueueNode* next;
};

// Custom Queue class for ships waiting at port
class ShipQueue {
private:
    QueueNode* front;
    QueueNode* rear;
    int size;

public:
    ShipQueue() : front(NULL), rear(NULL), size(0) {}
    
    ~ShipQueue() {
        while (front) {
            QueueNode* temp = front;
            front = front->next;
            delete temp;
        }
    }
    
    void enqueue(const Ship& ship) {
        QueueNode* newNode = new (nothrow) QueueNode();
        if (!newNode) return;
        
        newNode->ship = ship;
        newNode->next = NULL;
        
        if (rear == NULL) {
            front = rear = newNode;
        } else {
            rear->next = newNode;
            rear = newNode;
        }
        size++;
    }
    
    bool dequeue(Ship& ship) {
        if (front == NULL) return false;
        
        QueueNode* temp = front;
        ship = front->ship;
        front = front->next;
        
        if (front == NULL) {
            rear = NULL;
        }
        
        delete temp;
        size--;
        return true;
    }
    
    bool peek(Ship& ship) const {
        if (front == NULL) return false;
        ship = front->ship;
        return true;
    }
    
    bool isEmpty() const {
        return front == NULL;
    }
    
    int getSize() const {
        return size;
    }
    
    void display() const {
        if (isEmpty()) {
            cout << "    Queue: Empty\n";
            return;
        }
        
        cout << "    Queue (" << size << " ships waiting):\n";
        QueueNode* curr = front;
        int pos = 1;
        while (curr && pos <= 3) {  // Show first 3 ships
            cout << "      " << pos << ". " << curr->ship.shipName 
                 << " [" << curr->ship.company << "] - ETA: " 
                 << curr->ship.arrivalTime << "\n";
            curr = curr->next;
            pos++;
        }
        if (size > 3) {
            cout << "      ... and " << (size - 3) << " more\n";
        }
    }
};

// Compact RouteNode
struct RouteNode {
    unsigned char destinationIndex;
    char voyageDate[MAX_DATE_LENGTH];
    char departureTime[MAX_TIME_LENGTH];
    char arrivalTime[MAX_TIME_LENGTH];
    unsigned short departureMins;
    unsigned short arrivalMins;
    unsigned int voyageCost;
    char shippingCompany[MAX_COMPANY_LENGTH];
    RouteNode* nextRoute;
};

struct Port {
    char portName[MAX_NAME_LENGTH];
    unsigned short dailyDockingCharge;
    RouteNode* routeListHead;
    RouteNode* routeListTail;
    
    // Queue management fields
    ShipQueue* waitingQueue;
    int occupiedSlots;
    unsigned short currentDockedShips[DOCKING_SLOTS];  // Remaining service time
};

// Priority Queue Node
struct PQNode {
    unsigned char portIndex;
    unsigned int cost;
    unsigned int heuristic;
    PQNode* next;
};

class PriorityQueue {
private:
    PQNode* head;
    int size;

public:
    PriorityQueue() : head(NULL), size(0) {}
    
    ~PriorityQueue() {
        while (head) {
            PQNode* temp = head;
            head = head->next;
            delete temp;
        }
    }
    
    void push(unsigned char portIdx, unsigned int cost, unsigned int heuristic = 0) {
        PQNode* newNode = new (nothrow) PQNode();
        if (!newNode) return;
        
        newNode->portIndex = portIdx;
        newNode->cost = cost;
        newNode->heuristic = heuristic;
        newNode->next = NULL;
        size++;
        
        unsigned int priority = (heuristic > 0) ? heuristic : cost;
        
        if (!head || priority < (head->heuristic > 0 ? head->heuristic : head->cost)) {
            newNode->next = head;
            head = newNode;
            return;
        }
        
        PQNode* curr = head;
        while (curr->next) {
            unsigned int nextPriority = (curr->next->heuristic > 0) ? 
                                        curr->next->heuristic : curr->next->cost;
            if (priority < nextPriority) break;
            curr = curr->next;
        }
        newNode->next = curr->next;
        curr->next = newNode;
    }
    
    bool pop(unsigned char& portIdx, unsigned int& cost, unsigned int& heuristic) {
        if (!head) return false;
        
        PQNode* temp = head;
        portIdx = head->portIndex;
        cost = head->cost;
        heuristic = head->heuristic;
        head = head->next;
        delete temp;
        size--;
        return true;
    }
    
    bool isEmpty() const { return head == NULL; }
    int getSize() const { return size; }
};

// ---------------- HELPER FUNCTIONS ----------------

inline unsigned short timeToMinutes(const char* time) {
    if (!time || strlen(time) < 5) return 0;
    return ((time[0] - '0') * 10 + (time[1] - '0')) * 60 + 
           ((time[3] - '0') * 10 + (time[4] - '0'));
}

void minutesToTime(unsigned int minutes, char* timeStr) {
    minutes = minutes % 1440;  // Wrap around 24 hours
    int hours = minutes / 60;
    int mins = minutes % 60;
    sprintf(timeStr, "%02d:%02d", hours, mins);
}

inline bool isValidTimeFormat(const char* time) {
    return time && time[0] >= '0' && time[0] <= '2' && 
           time[1] >= '0' && time[1] <= '9' && time[2] == ':' &&
           time[3] >= '0' && time[3] <= '5' && 
           time[4] >= '0' && time[4] <= '9' && time[5] == '\0';
}

inline bool isValidDateFormat(const char* date) {
    if (!date) return false;
    
    int len = 0;
    while (date[len] && len < 11) len++;
    if (len != 10) return false;
    
    return isdigit(date[0]) && isdigit(date[1]) && date[2] == '/' &&
           isdigit(date[3]) && isdigit(date[4]) && date[5] == '/' &&
           isdigit(date[6]) && isdigit(date[7]) && 
           isdigit(date[8]) && isdigit(date[9]);
}

inline bool isValidConnection(unsigned short arrivalMins, unsigned short departureMins) {
    return departureMins >= arrivalMins || departureMins < arrivalMins;
}

inline int calculateLayoverHours(unsigned short arrivalMins, unsigned short departureMins) {
    if (departureMins < arrivalMins) {
        return (1440 - arrivalMins + departureMins) / 60;
    }
    return (departureMins - arrivalMins) / 60;
}

inline int dateToInt(const char* date) {
    return ((date[6]-'0')*10000000 + (date[7]-'0')*1000000 + (date[8]-'0')*100000 + (date[9]-'0')*10000 +
            (date[3]-'0')*1000 + (date[4]-'0')*100 + (date[0]-'0')*10 + (date[1]-'0'));
}

inline bool isSameDateOrLater(const char* date1, const char* date2) {
    return dateToInt(date1) >= dateToInt(date2);
}

inline bool isSameDate(const char* date1, const char* date2) {
    return dateToInt(date1) == dateToInt(date2);
}

void clearInputBuffer() {
    cin.clear();
    cin.ignore(INT_MAX, '\n');
}

// Calculate service time based on voyage cost (Option B)
unsigned int calculateServiceTime(unsigned int voyageCost) {
    // Service time in minutes = (cost / 10000) * 60 + 120 (base 2 hours)
    return (voyageCost / 10000) * 60 + 120;
}

// Compare ships by arrival date and time for sorting
int compareShipArrival(const Ship& s1, const Ship& s2) {
    int date1 = dateToInt(s1.arrivalDate);
    int date2 = dateToInt(s2.arrivalDate);
    
    if (date1 != date2) {
        return date1 - date2;  // Earlier date comes first
    }
    
    // Same date, compare by time
    return s1.arrivalMins - s2.arrivalMins;
}

// Simple bubble sort for ships (custom implementation, no STL)
void sortShipsByArrival(Ship* ships, int count) {
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (compareShipArrival(ships[j], ships[j + 1]) > 0) {
                // Swap ships
                Ship temp = ships[j];
                ships[j] = ships[j + 1];
                ships[j + 1] = temp;
            }
        }
    }
}

// ---------------- GRAPH CLASS ----------------
class Graph {
private:
    Port ports[MAX_PORTS];
    unsigned char totalPorts;
    
    unsigned int calculateHeuristic(unsigned char fromPort, unsigned char toPort) const {
        if (fromPort == toPort) return 0;
        
        RouteNode* route = ports[fromPort].routeListHead;
        unsigned int minDirectCost = UINT_MAX;
        
        while (route) {
            if (route->destinationIndex == toPort) {
                if (route->voyageCost < minDirectCost) {
                    minDirectCost = route->voyageCost;
                }
            }
            route = route->nextRoute;
        }
        
        if (minDirectCost != UINT_MAX) {
            return minDirectCost;
        }
        
        return 5000;
    }
    
    bool matchesPreferences(const RouteNode* route, unsigned char portIndex, 
                           const UserPreferences& prefs) const {
        if (!route) return false;
        
        if (prefs.hasCompanyFilter) {
            if (strcmp(route->shippingCompany, prefs.preferredCompany) != 0) {
                return false;
            }
        }
        
        if (prefs.hasAvoidPort) {
            if (strcmp(ports[route->destinationIndex].portName, prefs.avoidPort) == 0) {
                return false;
            }
        }
        
        if (prefs.hasMaxCostLimit) {
            if (route->voyageCost > prefs.maxCostLimit) {
                return false;
            }
        }
        
        return true;
    }
    
    unsigned int calculateVoyageTime(const RouteNode* route) const {
        if (!route) return 0;
        
        int depMins = route->departureMins;
        int arrMins = route->arrivalMins;
        
        int totalMinutes;
        if (arrMins < depMins) {
            totalMinutes = (1440 - depMins) + arrMins;
        } else {
            totalMinutes = arrMins - depMins;
        }
        
        return totalMinutes / 60;
    }
    
    bool hasValidDirectRoute(int srcIdx, int destIdx, const char* date, 
                            const UserPreferences* prefs) const {
        if (!isValidPortIndex(srcIdx) || !isValidPortIndex(destIdx)) return false;
        
        RouteNode* route = ports[srcIdx].routeListHead;
        
        while (route) {
            if (route->destinationIndex == destIdx) {
                if (isSameDateOrLater(route->voyageDate, date)) {
                    if (prefs) {
                        if (!matchesPreferences(route, destIdx, *prefs)) {
                            route = route->nextRoute;
                            continue;
                        }
                    }
                    return true;
                }
            }
            route = route->nextRoute;
        }
        
        return false;
    }
    
    // Calculate queue wait time at a port for a given arrival time
    unsigned int calculateQueueWaitTime(int portIdx, const char* arrivalDate, 
                                       unsigned short arrivalMins) const {
        if (!isValidPortIndex(portIdx)) return 0;
        
        // Get current queue state
        int queueSize = ports[portIdx].waitingQueue->getSize();
        int availableSlots = DOCKING_SLOTS - ports[portIdx].occupiedSlots;
        
        // If slots available and no queue, no wait time
        if (availableSlots > 0 && queueSize == 0) {
            return 0;
        }
        
        // Calculate total wait time based on ships ahead in queue
        unsigned int totalWaitMinutes = 0;
        
        // If all slots are occupied, need to wait for current ships to finish
        if (availableSlots <= 0) {
            // Find minimum remaining service time of docked ships
            unsigned int minServiceTime = UINT_MAX;
            for (int i = 0; i < DOCKING_SLOTS; i++) {
                if (ports[portIdx].currentDockedShips[i] > 0 && 
                    ports[portIdx].currentDockedShips[i] < minServiceTime) {
                    minServiceTime = ports[portIdx].currentDockedShips[i];
                }
            }
            
            if (minServiceTime != UINT_MAX) {
                totalWaitMinutes += minServiceTime;
            }
        }
        
        // Add wait time for ships in queue
        // Need to estimate service time for each ship in queue
        // Since we don't have direct access to queue internals, use estimation
        if (queueSize > 0) {
            // Each position in queue adds average service time
            // Position = (queueSize / DOCKING_SLOTS) gives number of "rounds"
            int myPosition = queueSize + 1;  // Position if we join queue now
            int roundsToWait = (myPosition - 1) / DOCKING_SLOTS;
            
            if (roundsToWait > 0) {
                // Average service time per ship is ~3 hours = 180 minutes
                // This is a reasonable estimate based on typical cargo operations
                totalWaitMinutes += roundsToWait * 180;
            }
        }
        
        return totalWaitMinutes;
    }
    
    // Simulate ship arrival and queue management
    void simulateShipArrival(int portIdx, const Ship& ship) {
        if (!isValidPortIndex(portIdx)) return;
        
        if (ports[portIdx].occupiedSlots < DOCKING_SLOTS) {
            // Slot available, dock immediately
            ports[portIdx].occupiedSlots++;
            ports[portIdx].currentDockedShips[ports[portIdx].occupiedSlots - 1] = 
                ship.serviceTimeNeeded;
        } else {
            // All slots occupied, add to queue
            ports[portIdx].waitingQueue->enqueue(ship);
        }
    }
    
    // Process port queues (simulate time passing)
    void processPortQueues(int portIdx, unsigned int timeElapsed) {
        if (!isValidPortIndex(portIdx)) return;
        
        // Reduce service time for docked ships
        for (int i = 0; i < DOCKING_SLOTS; i++) {
            if (ports[portIdx].currentDockedShips[i] > 0) {
                if (ports[portIdx].currentDockedShips[i] >= timeElapsed) {
                    ports[portIdx].currentDockedShips[i] -= timeElapsed;
                } else {
                    ports[portIdx].currentDockedShips[i] = 0;
                }
                
                // If ship finished, free the slot and dock next from queue
                if (ports[portIdx].currentDockedShips[i] == 0) {
                    ports[portIdx].occupiedSlots--;
                    
                    Ship nextShip;
                    if (ports[portIdx].waitingQueue->dequeue(nextShip)) {
                        ports[portIdx].occupiedSlots++;
                        ports[portIdx].currentDockedShips[i] = nextShip.serviceTimeNeeded;
                    }
                }
            }
        }
    }

public:
    Graph() : totalPorts(0) {}

    int getPortIndex(const char* portName) const {
        if (!portName) return -1;
        for (unsigned char i = 0; i < totalPorts; i++) {
            if (strcmp(ports[i].portName, portName) == 0)
                return i;
        }
        return -1;
    }

    inline bool isValidPortIndex(int index) const {
        return (index >= 0 && index < totalPorts);
    }

    void loadPortCharges() {
        ifstream file("PortCharges.txt");
        if (!file.is_open()) {
            cout << "Error: Could not open PortCharges.txt\n";
            return;
        }

        char name[MAX_NAME_LENGTH];
        int charge;
        
        while (file >> name >> charge && totalPorts < MAX_PORTS) {
            if (charge < 0) continue;
            
            strcpy(ports[totalPorts].portName, name);
            ports[totalPorts].dailyDockingCharge = (unsigned short)charge;
            ports[totalPorts].routeListHead = NULL;
            ports[totalPorts].routeListTail = NULL;
            ports[totalPorts].waitingQueue = new ShipQueue();
            ports[totalPorts].occupiedSlots = 0;
            for (int i = 0; i < DOCKING_SLOTS; i++) {
                ports[totalPorts].currentDockedShips[i] = 0;
            }
            totalPorts++;
        }

        file.close();
        cout << "Loaded " << (int)totalPorts << " ports.\n";
    }

    void loadRoutes() {
        ifstream file("Routes.txt");
        if (!file.is_open()) {
            cout << "Error: Could not open Routes.txt\n";
            return;
        }

        char origin[MAX_NAME_LENGTH], dest[MAX_NAME_LENGTH];
        char date[MAX_DATE_LENGTH], depTime[MAX_TIME_LENGTH], arrTime[MAX_TIME_LENGTH];
        char company[MAX_COMPANY_LENGTH];
        int cost, loaded = 0;
        
        // Temporary array to store all ships before sorting
        const int MAX_SHIPS = 500;  // Allocate space for ships
        Ship* allShips = new Ship[MAX_SHIPS];
        int shipCount = 0;

        while (file >> origin >> dest >> date >> depTime >> arrTime >> cost >> company) {
            int fromIdx = getPortIndex(origin);
            int toIdx = getPortIndex(dest);

            if (fromIdx == -1 || toIdx == -1 || cost < 0) continue;

            RouteNode* node = new (nothrow) RouteNode();
            if (!node) break;

            node->destinationIndex = (unsigned char)toIdx;
            strcpy(node->voyageDate, date);
            strcpy(node->departureTime, depTime);
            strcpy(node->arrivalTime, arrTime);
            node->departureMins = timeToMinutes(depTime);
            node->arrivalMins = timeToMinutes(arrTime);
            node->voyageCost = (unsigned int)cost;
            strcpy(node->shippingCompany, company);
            node->nextRoute = NULL;

            if (!ports[fromIdx].routeListHead) {
                ports[fromIdx].routeListHead = node;
                ports[fromIdx].routeListTail = node;
            } else {
                ports[fromIdx].routeListTail->nextRoute = node;
                ports[fromIdx].routeListTail = node;
            }
            
            // Create ship for queue simulation (store in array first)
            if (shipCount < MAX_SHIPS) {
                Ship& ship = allShips[shipCount];
                sprintf(ship.shipName, "%s_%s_%d", company, date, shipCount);
                strcpy(ship.arrivalTime, arrTime);
                strcpy(ship.arrivalDate, date);
                ship.arrivalMins = timeToMinutes(arrTime);
                ship.serviceTimeNeeded = calculateServiceTime(cost);
                strcpy(ship.originPort, origin);
                strcpy(ship.destinationPort, dest);
                strcpy(ship.company, company);
                ship.voyageCost = cost;
                
                shipCount++;
            }
            
            loaded++;
        }

        file.close();
        
        // CRITICAL FIX: Sort all ships by arrival date and time
        cout << "Sorting " << shipCount << " ships by arrival time...\n";
        sortShipsByArrival(allShips, shipCount);
        
        // Now simulate ship arrivals in chronological order
        cout << "Simulating port arrivals in chronological order...\n";
        for (int i = 0; i < shipCount; i++) {
            int destIdx = getPortIndex(allShips[i].destinationPort);
            if (destIdx != -1) {
                simulateShipArrival(destIdx, allShips[i]);
            }
        }
        
        // Clean up
        delete[] allShips;
        
        cout << "Loaded " << loaded << " routes.\n";
        cout << "Port queues initialized with " << shipCount << " ships (chronologically sorted).\n\n";
    }

    void displayGraph() const {
        if (totalPorts == 0) {
            cout << "\nNo ports loaded!\n";
            return;
        }
        
        cout << "\n========== PORT NETWORK ==========\n\n";
        for (unsigned char i = 0; i < totalPorts; i++) {
            cout << ports[i].portName << " ($" << ports[i].dailyDockingCharge << "/day)\n";
            cout << "  Docking: " << ports[i].occupiedSlots << "/" << DOCKING_SLOTS << " slots occupied\n";
            ports[i].waitingQueue->display();
            
            RouteNode* route = ports[i].routeListHead;
            if (!route) {
                cout << "  No outgoing routes\n";
            } else {
                cout << "  Outgoing routes:\n";
                while (route) {
                    cout << "    → " << ports[route->destinationIndex].portName
                         << " | " << route->voyageDate
                         << " | " << route->departureTime << "-" << route->arrivalTime
                         << " | $" << route->voyageCost
                         << " | " << route->shippingCompany << "\n";
                    route = route->nextRoute;
                }
            }
            cout << "\n";
        }
    }

    void findCheapestRoute(int srcIdx, int destIdx, const char* preferredDate, 
                          bool useAStar = false, const UserPreferences* prefs = NULL) {
        if (!isValidPortIndex(srcIdx) || !isValidPortIndex(destIdx)) {
            cout << "\n❌ Invalid port indices!\n";
            return;
        }
        
        if (!isValidDateFormat(preferredDate)) {
            cout << "\n❌ Invalid date format!\n";
            return;
        }
        
        if (srcIdx == destIdx) {
            cout << "\n❌ Source and destination are the same!\n";
            return;
        }
        
        if (prefs && prefs->hasAvoidPort) {
            if (strcmp(ports[srcIdx].portName, prefs->avoidPort) == 0 ||
                strcmp(ports[destIdx].portName, prefs->avoidPort) == 0) {
                cout << "\n❌ Cannot avoid source or destination port!\n";
                return;
            }
        }

        const char* algoName = useAStar ? "A* ALGORITHM" : "DIJKSTRA'S ALGORITHM";
        
        unsigned int minCost[MAX_PORTS];
        unsigned int totalTime[MAX_PORTS];
        unsigned int queueWaitTime[MAX_PORTS];
        bool visited[MAX_PORTS] = {false};
        char prevPort[MAX_PORTS];
        RouteNode* usedRoute[MAX_PORTS] = {NULL};
        unsigned int nodesExplored = 0;
        unsigned int routesFiltered = 0;

        for (unsigned char i = 0; i < totalPorts; i++) {
            minCost[i] = UINT_MAX;
            totalTime[i] = 0;
            queueWaitTime[i] = 0;
            prevPort[i] = -1;
        }
        minCost[srcIdx] = 0;

        PriorityQueue pq;
        
        if (useAStar) {
            unsigned int h = calculateHeuristic(srcIdx, destIdx);
            pq.push(srcIdx, 0, h);
        } else {
            pq.push(srcIdx, 0, 0);
        }

        while (!pq.isEmpty()) {
            unsigned char currPort;
            unsigned int currCost, currHeuristic;
            
            if (!pq.pop(currPort, currCost, currHeuristic)) break;
            
            if (visited[currPort]) continue;
            visited[currPort] = true;
            nodesExplored++;

            if (currPort == destIdx) break;

            RouteNode* route = ports[currPort].routeListHead;
            while (route) {
                unsigned char nextPort = route->destinationIndex;
                
                bool passesFilter = true;
                if (prefs) {
                    passesFilter = matchesPreferences(route, nextPort, *prefs);
                    if (!passesFilter) {
                        routesFiltered++;
                    }
                }
                
                if (passesFilter && isSameDateOrLater(route->voyageDate, preferredDate)) {
                    bool timeValid = true;
                    if (prevPort[currPort] != -1 && usedRoute[currPort]) {
                        timeValid = isValidConnection(
                            usedRoute[currPort]->arrivalMins,
                            route->departureMins
                        );
                    }
                    
                    if (timeValid && !visited[nextPort]) {
                        // Calculate queue wait time at next port (Option A - affects cost)
                        unsigned int queueWait = calculateQueueWaitTime(
                            nextPort, route->voyageDate, route->arrivalMins);
                        unsigned int serviceTime = calculateServiceTime(route->voyageCost);
                        
                        // Add queue wait time to cost calculation
                        unsigned int queueCostPenalty = (queueWait / 60) * 
                                                       ports[nextPort].dailyDockingCharge / 24;
                        unsigned int newCost = minCost[currPort] + route->voyageCost + queueCostPenalty;
                        
                        unsigned int voyageTime = calculateVoyageTime(route);
                        unsigned int newTotalTime = totalTime[currPort] + voyageTime + 
                                                   (queueWait / 60) + (serviceTime / 60);
                        
                        bool withinTimeLimit = true;
                        if (prefs && prefs->hasMaxTimeLimit) {
                            withinTimeLimit = (newTotalTime <= prefs->maxTimeLimit);
                        }
                        
                        if (withinTimeLimit && newCost < minCost[nextPort]) {
                            minCost[nextPort] = newCost;
                            totalTime[nextPort] = newTotalTime;
                            queueWaitTime[nextPort] = queueWaitTime[currPort] + (queueWait / 60);
                            prevPort[nextPort] = currPort;
                            usedRoute[nextPort] = route;
                            
                            if (useAStar) {
                                unsigned int h = calculateHeuristic(nextPort, destIdx);
                                pq.push(nextPort, newCost, newCost + h);
                            } else {
                                pq.push(nextPort, newCost, 0);
                            }
                        }
                    }
                }
                route = route->nextRoute;
            }
        }

        if (minCost[destIdx] == UINT_MAX) {
            cout << "\n❌ No route found from " << ports[srcIdx].portName 
                 << " to " << ports[destIdx].portName;
            if (prefs && prefs->hasAnyFilter()) {
                cout << " matching your preferences";
            }
            cout << "\n";
            if (routesFiltered > 0) {
                cout << "(" << routesFiltered << " routes filtered out by preferences)\n";
            }
            return;
        }

        unsigned char path[MAX_PORTS];
        RouteNode* routes[MAX_PORTS];
        unsigned char len = 0;
        
        for (char curr = destIdx; curr != -1 && len < MAX_PORTS; curr = prevPort[curr]) {
            path[len] = curr;
            routes[len] = usedRoute[curr];
            len++;
        }

        cout << "\n========== " << algoName << " ==========\n";
        cout << "From: " << ports[srcIdx].portName << "\n";
        cout << "To: " << ports[destIdx].portName << "\n";
        cout << "Date: " << preferredDate << "\n";
        
        if (prefs && prefs->hasAnyFilter()) {
            if (prefs->hasCompanyFilter) {
                cout << "Company Filter: " << prefs->preferredCompany << "\n";
            }
            if (prefs->hasAvoidPort) {
                cout << "Avoiding Port: " << prefs->avoidPort << "\n";
            }
            if (prefs->hasMaxCostLimit) {
                cout << "Max Cost Limit: $" << prefs->maxCostLimit << "\n";
            }
            if (prefs->hasMaxTimeLimit) {
                cout << "Max Time Limit: " << prefs->maxTimeLimit << " hours\n";
            }
        }
        
        cout << "Nodes Explored: " << nodesExplored << "/" << (int)totalPorts;
        if (routesFiltered > 0) {
            cout << " (" << routesFiltered << " routes filtered)";
        }
        cout << "\n\n";

        unsigned int totalDocking = 0;
        
        for (int i = len - 1; i >= 0; i--) {
            cout << ports[path[i]].portName;
            totalDocking += ports[path[i]].dailyDockingCharge;
            
            // Show queue status at intermediate ports (Option B - only if queue exists)
            if (i > 0 && i < len - 1) {  // Not source or destination
                int queueSize = ports[path[i]].waitingQueue->getSize();
                int occupied = ports[path[i]].occupiedSlots;
                
                if (queueSize > 0 || occupied > 0) {
                    cout << "\n  Port Status:";
                    cout << "\n    Docking: " << occupied << "/" << DOCKING_SLOTS << " slots occupied";
                    if (queueSize > 0) {
                        cout << "\n    Queue: " << queueSize << " ships waiting";
                        unsigned int waitTime = calculateQueueWaitTime(path[i], 
                            routes[i-1]->voyageDate, routes[i-1]->arrivalMins);
                        if (waitTime > 0) {
                            cout << "\n    Estimated wait: " << (waitTime / 60) << " hours";
                        }
                    }
                }
            }
            
            if (i > 0 && routes[i-1]) {
                RouteNode* r = routes[i-1];
                unsigned int serviceTime = calculateServiceTime(r->voyageCost);
                
                cout << "\n  ↓ [" << r->shippingCompany << "] "
                     << r->departureTime << " (" << r->voyageDate << ") "
                     << "→ " << r->arrivalTime << " | $" << r->voyageCost;
                cout << "\n    Service time: " << (serviceTime / 60) << " hours";
                
                if (i > 1 && routes[i-2]) {
                    int layover = calculateLayoverHours(r->arrivalMins, routes[i-2]->departureMins);
                    if (layover > 0) {
                        cout << "\n    Layover: " << layover << "h";
                        if (layover > 12) cout << " (Extended)";
                    }
                }
                cout << "\n";
            }
        }

        cout << "\n====================================\n";
        cout << "Voyage Cost: $" << minCost[destIdx] << "\n";
        cout << "Total Time: " << totalTime[destIdx] << " hours\n";
        if (queueWaitTime[destIdx] > 0) {
            cout << "Queue Wait Time: " << queueWaitTime[destIdx] << " hours\n";
        }
        cout << "Port Charges: $" << totalDocking << "\n";
        cout << "TOTAL: $" << (minCost[destIdx] + totalDocking) << "\n";
        cout << "====================================\n\n";
    }

    UserPreferences getUserPreferences() {
        UserPreferences prefs;
        char choice;
        
        cout << "\n--- Set Your Preferences (Optional) ---\n";
        
        cout << "Filter by shipping company? (y/n): ";
        cin >> choice;
        clearInputBuffer();
        
        if (choice == 'y' || choice == 'Y') {
            cout << "Available companies: Evergreen, MSC, MaerskLine, COSCO, CMA_CGM,\n";
            cout << "                     HapagLloyd, ZIM, YangMing, PIL, ONE\n";
            cout << "Enter company name: ";
            cin >> prefs.preferredCompany;
            prefs.hasCompanyFilter = true;
            clearInputBuffer();
        }
        
        cout << "Avoid a specific port? (y/n): ";
        cin >> choice;
        clearInputBuffer();
        
        if (choice == 'y' || choice == 'Y') {
            cout << "Enter port to avoid: ";
            cin >> prefs.avoidPort;
            prefs.hasAvoidPort = true;
            clearInputBuffer();
        }
        
        cout << "Set maximum voyage cost limit? (y/n): ";
        cin >> choice;
        clearInputBuffer();
        
        if (choice == 'y' || choice == 'Y') {
            cout << "Enter max cost (USD): ";
            if (cin >> prefs.maxCostLimit) {
                prefs.hasMaxCostLimit = true;
            }
            clearInputBuffer();
        }
        
        cout << "Set maximum total voyage time limit? (y/n): ";
        cin >> choice;
        clearInputBuffer();
        
        if (choice == 'y' || choice == 'Y') {
            cout << "Enter max time (hours): ";
            if (cin >> prefs.maxTimeLimit) {
                prefs.hasMaxTimeLimit = true;
            }
            clearInputBuffer();
        }
        
        return prefs;
    }

    void displayPortQueueStatus() const {
        cout << "\n========== PORT QUEUE STATUS ==========\n\n";
        for (unsigned char i = 0; i < totalPorts; i++) {
            cout << ports[i].portName << ":\n";
            cout << "  Docking Capacity: " << DOCKING_SLOTS << " slots\n";
            cout << "  Currently Occupied: " << ports[i].occupiedSlots << " slots\n";
            
            if (ports[i].occupiedSlots > 0) {
                cout << "  Docked Ships Service Time:\n";
                for (int j = 0; j < DOCKING_SLOTS; j++) {
                    if (ports[i].currentDockedShips[j] > 0) {
                        cout << "    Slot " << (j+1) << ": " 
                             << (ports[i].currentDockedShips[j] / 60) << " hours remaining\n";
                    }
                }
            }
            
            ports[i].waitingQueue->display();
            cout << "\n";
        }
    }

    void run() {
        loadPortCharges();
        loadRoutes();
        
        if (totalPorts == 0) {
            cout << "❌ No ports loaded!\n";
            return;
        }

        int choice;
        do {
            cout << "\n╔════════════════════════════════╗\n";
            cout << "║   OCEANROUTE NAV - MENU       ║\n";
            cout << "╚════════════════════════════════╝\n";
            cout << "1. Display Network\n";
            cout << "2. Display Port Queue Status\n";
            cout << "3. Find Cheapest Route\n";
            cout << "4. Find Route with Preferences\n";
            cout << "5. Exit\n\n";
            cout << "Choice (1-5): ";
            
            if (!(cin >> choice)) {
                cout << "❌ Invalid input!\n";
                clearInputBuffer();
                continue;
            }
            clearInputBuffer();

            switch (choice) {
                case 1:
                    displayGraph();
                    break;
                
                case 2:
                    displayPortQueueStatus();
                    break;
                    
                case 3: {
                    char src[MAX_NAME_LENGTH], dst[MAX_NAME_LENGTH], date[MAX_DATE_LENGTH];
                    
                    cout << "\n--- Find Cheapest Route ---\n";
                    cout << "Ports: ";
                    for (unsigned char i = 0; i < totalPorts; i++) {
                        cout << ports[i].portName;
                        if (i < totalPorts - 1) cout << ", ";
                    }
                    cout << "\n\n";
                    
                    cout << "Source: ";
                    cin >> src;
                    cout << "Destination: ";
                    cin >> dst;
                    cout << "Date (DD/MM/YYYY): ";
                    cin >> date;

                    int si = getPortIndex(src);
                    int di = getPortIndex(dst);

                    if (si == -1) {
                        cout << "❌ Source port '" << src << "' not found!\n";
                    } else if (di == -1) {
                        cout << "❌ Destination port '" << dst << "' not found!\n";
                    } else {
                        bool hasDirectRoute = hasValidDirectRoute(si, di, date, NULL);
                        
                        if (hasDirectRoute) {
                            cout << "\n🎯 Direct route detected - Using A* algorithm\n";
                            findCheapestRoute(si, di, date, true, NULL);
                        } else {
                            cout << "\n🔍 Multi-hop route needed - Using Dijkstra's algorithm\n";
                            findCheapestRoute(si, di, date, false, NULL);
                        }
                    }
                    clearInputBuffer();
                    break;
                }
                
                case 4: {
                    char src[MAX_NAME_LENGTH], dst[MAX_NAME_LENGTH], date[MAX_DATE_LENGTH];
                    
                    cout << "\n--- Find Route with Custom Preferences ---\n";
                    cout << "Ports: ";
                    for (unsigned char i = 0; i < totalPorts; i++) {
                        cout << ports[i].portName;
                        if (i < totalPorts - 1) cout << ", ";
                    }
                    cout << "\n\n";
                    
                    cout << "Source: ";
                    cin >> src;
                    cout << "Destination: ";
                    cin >> dst;
                    cout << "Date (DD/MM/YYYY): ";
                    cin >> date;
                    clearInputBuffer();

                    int si = getPortIndex(src);
                    int di = getPortIndex(dst);

                    if (si == -1) {
                        cout << "❌ Source port '" << src << "' not found!\n";
                    } else if (di == -1) {
                        cout << "❌ Destination port '" << dst << "' not found!\n";
                    } else {
                        UserPreferences prefs = getUserPreferences();
                        
                        bool hasDirectRoute = hasValidDirectRoute(si, di, date, &prefs);
                        
                        if (hasDirectRoute && !prefs.hasAnyFilter()) {
                            cout << "\n🎯 Direct route detected - Using A* algorithm\n";
                            findCheapestRoute(si, di, date, true, &prefs);
                        } else {
                            cout << "\n🔍 Multi-hop/Filtered route - Using Dijkstra's algorithm\n";
                            findCheapestRoute(si, di, date, false, &prefs);
                        }
                    }
                    break;
                }
                
                case 5:
                    cout << "\nThank you! Safe travels! 🚢\n";
                    break;
                    
                default:
                    cout << "❌ Invalid choice!\n";
            }
        } while (choice != 5);
    }

    ~Graph() {
        for (unsigned char i = 0; i < totalPorts; i++) {
            RouteNode* curr = ports[i].routeListHead;
            while (curr) {
                RouteNode* temp = curr;
                curr = curr->nextRoute;
                delete temp;
            }
            delete ports[i].waitingQueue;
        }
    }
};

// ---------------- MAIN ----------------
int main() {
    Graph network;
    network.run();
    return 0;
}