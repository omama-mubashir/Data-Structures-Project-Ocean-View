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
#define MAX_SAVED_JOURNEYS 10

// ---------------- JOURNEY LEG STRUCTURES (MULTI-LEG ROUTE) ----------------

struct JourneyLeg {
    char fromPort[MAX_NAME_LENGTH];
    char toPort[MAX_NAME_LENGTH];
    char voyageDate[MAX_DATE_LENGTH];
    char departureTime[MAX_TIME_LENGTH];
    char arrivalTime[MAX_TIME_LENGTH];
    unsigned int voyageCost;
    char shippingCompany[MAX_COMPANY_LENGTH];
    unsigned short departureMins;
    unsigned short arrivalMins;
    unsigned int layoverHours;
    unsigned int queueWaitHours;
    unsigned int dockingCharge;
    JourneyLeg* nextLeg;
    
    JourneyLeg() {
        fromPort[0] = '\0';
        toPort[0] = '\0';
        voyageDate[0] = '\0';
        departureTime[0] = '\0';
        arrivalTime[0] = '\0';
        voyageCost = 0;
        shippingCompany[0] = '\0';
        departureMins = 0;
        arrivalMins = 0;
        layoverHours = 0;
        queueWaitHours = 0;
        dockingCharge = 0;
        nextLeg = NULL;
    }
};

class Journey {
private:
    JourneyLeg* head;
    JourneyLeg* tail;
    int legCount;
    unsigned int totalCost;
    unsigned int totalTime;
    char journeyName[50];
    
public:
    Journey() : head(NULL), tail(NULL), legCount(0), totalCost(0), totalTime(0) {
        journeyName[0] = '\0';
    }
    
    ~Journey() {
        clear();
    }
    
    void setName(const char* name) {
        strcpy(journeyName, name);
    }
    
    const char* getName() const {
        return journeyName;
    }
    
    void addLeg(const char* from, const char* to, const char* date,
                const char* depTime, const char* arrTime,
                unsigned int cost, const char* company,
                unsigned short depMins, unsigned short arrMins,
                unsigned int layover = 0, unsigned int queueWait = 0,
                unsigned int docking = 0) {
        
        JourneyLeg* newLeg = new (nothrow) JourneyLeg();
        if (!newLeg) return;
        
        strcpy(newLeg->fromPort, from);
        strcpy(newLeg->toPort, to);
        strcpy(newLeg->voyageDate, date);
        strcpy(newLeg->departureTime, depTime);
        strcpy(newLeg->arrivalTime, arrTime);
        newLeg->voyageCost = cost;
        strcpy(newLeg->shippingCompany, company);
        newLeg->departureMins = depMins;
        newLeg->arrivalMins = arrMins;
        newLeg->layoverHours = layover;
        newLeg->queueWaitHours = queueWait;
        newLeg->dockingCharge = docking;
        
        int voyageTime;
        if (arrMins < depMins) {
            voyageTime = (1440 - depMins + arrMins) / 60;
        } else {
            voyageTime = (arrMins - depMins) / 60;
        }
        
        totalCost += cost + docking;
        totalTime += voyageTime + layover + queueWait;
        
        if (!head) {
            head = tail = newLeg;
        } else {
            tail->nextLeg = newLeg;
            tail = newLeg;
        }
        legCount++;
    }
    
    void displayJourney() const {
        if (!head) {
            cout << "\n📭 Journey is empty!\n";
            return;
        }
        
        cout << "\n╔════════════════════════════════════════════╗\n";
        cout << "║         MULTI-LEG JOURNEY DETAILS         ║\n";
        cout << "╚════════════════════════════════════════════╝\n";
        
        if (journeyName[0] != '\0') {
            cout << "Journey: " << journeyName << "\n";
        }
        cout << "\n";
        
        JourneyLeg* curr = head;
        int legNum = 1;
        
        cout << "🏁 " << curr->fromPort << " (START)\n";
        
        while (curr) {
            cout << "  |\n";
            cout << "  | Leg " << legNum << ": " << curr->fromPort 
                 << " → " << curr->toPort << "\n";
            cout << "  | 📅 " << curr->voyageDate << " | ⏰ " 
                 << curr->departureTime << " → " << curr->arrivalTime << "\n";
            cout << "  | 🚢 " << curr->shippingCompany 
                 << " | 💰 $" << curr->voyageCost;
            
            if (curr->dockingCharge > 0) {
                cout << " + $" << curr->dockingCharge << " docking";
            }
            cout << "\n";
            
            if (curr->layoverHours > 0 || curr->queueWaitHours > 0) {
                cout << "  | ⏱️  ";
                if (curr->layoverHours > 0) {
                    cout << "Layover: " << curr->layoverHours << "h";
                }
                if (curr->queueWaitHours > 0) {
                    if (curr->layoverHours > 0) cout << " + ";
                    cout << "Queue: " << curr->queueWaitHours << "h";
                }
                cout << "\n";
            }
            
            cout << "  ↓\n";
            cout << "📍 " << curr->toPort;
            
            if (curr->nextLeg) {
                cout << "\n";
            } else {
                cout << " (DESTINATION)\n";
            }
            
            curr = curr->nextLeg;
            legNum++;
        }
        
        cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        cout << "📊 JOURNEY SUMMARY:\n";
        cout << "   • Total Legs: " << legCount << "\n";
        cout << "   • Total Cost: $" << totalCost << "\n";
        cout << "   • Total Time: " << totalTime << " hours\n";
        cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";
    }
    
    void displayChain() const {
        if (!head) {
            cout << "Empty";
            return;
        }
        
        JourneyLeg* curr = head;
        cout << curr->fromPort;
        
        while (curr) {
            cout << " → " << curr->toPort;
            curr = curr->nextLeg;
        }
    }
    
    void displaySummary() const {
        if (!head) return;
        
        if (journeyName[0] != '\0') {
            cout << journeyName << ": ";
        }
        displayChain();
        cout << " | " << legCount << " legs | $" << totalCost 
             << " | " << totalTime << "h\n";
    }
    
    void clear() {
        while (head) {
            JourneyLeg* temp = head;
            head = head->nextLeg;
            delete temp;
        }
        tail = NULL;
        legCount = 0;
        totalCost = 0;
        totalTime = 0;
    }
    
    int getLegCount() const { return legCount; }
    unsigned int getTotalCost() const { return totalCost; }
    unsigned int getTotalTime() const { return totalTime; }
    bool isEmpty() const { return head == NULL; }
};

// ---------------- DOCKING QUEUE STRUCTURES (FIXED WITH CAPACITY) ----------------

struct QueueNode {
    char shipName[MAX_COMPANY_LENGTH];
    unsigned short arrivalMinutes;
    unsigned short serviceMinutes;
    unsigned short actualDockTime;
    unsigned short departureMinutes;
    QueueNode* next;
    
    QueueNode() : next(NULL) {
        shipName[0] = '\0';
        arrivalMinutes = 0;
        serviceMinutes = 0;
        actualDockTime = 0;
        departureMinutes = 0;
    }
};

class DockingQueue {
private:
    QueueNode* front;
    QueueNode* rear;
    int queueSize;
    int availableSlots;  // FIX Q1: Port capacity
    
public:
    // FIX Q1: Add capacity parameter (default 2 slots per port)
    DockingQueue(int slots = 2) : front(NULL), rear(NULL), queueSize(0), availableSlots(slots) {}
    
    ~DockingQueue() {
        while (front) {
            QueueNode* temp = front;
            front = front->next;
            delete temp;
        }
    }
    
    // FIX Q1: Respect capacity when calculating docking time
    unsigned short calculateDockingTime(unsigned short arrivalTime) {
        // If fewer ships than slots, dock immediately
        if (queueSize < availableSlots) {
            return arrivalTime;
        }
        
        // Port is full, find when earliest slot becomes available
        unsigned short earliestDeparture = USHRT_MAX;
        QueueNode* curr = front;
        int count = 0;
        
        // Check first N ships (where N = availableSlots)
        while (curr && count < availableSlots) {
            if (curr->departureMinutes < earliestDeparture) {
                earliestDeparture = curr->departureMinutes;
            }
            curr = curr->next;
            count++;
        }
        
        // Ship docks when slot frees up OR when it arrives (whichever is later)
        if (arrivalTime >= earliestDeparture) {
            return arrivalTime;
        } else {
            return earliestDeparture;
        }
    }
    
    int calculateWaitTime(unsigned short arrivalTime) {
        unsigned short actualDockTime = calculateDockingTime(arrivalTime);
        
        if (actualDockTime < arrivalTime) {
            return (1440 - arrivalTime + actualDockTime);
        }
        
        return actualDockTime - arrivalTime;
    }
    
    void enqueue(const char* shipName, unsigned short arrival, unsigned short service) {
        QueueNode* newShip = new (nothrow) QueueNode();
        if (!newShip) return;
        
        strcpy(newShip->shipName, shipName);
        newShip->arrivalMinutes = arrival;
        newShip->serviceMinutes = service;
        newShip->actualDockTime = calculateDockingTime(arrival);
        newShip->departureMinutes = newShip->actualDockTime + service;
        
        if (!rear) {
            front = rear = newShip;
        } else {
            rear->next = newShip;
            rear = newShip;
        }
        queueSize++;
    }
    
    void dequeue() {
        if (!front) return;
        
        QueueNode* temp = front;
        front = front->next;
        
        if (!front) {
            rear = NULL;
        }
        
        delete temp;
        queueSize--;
    }
    
    void clearQueue() {
        while (front) {
            dequeue();
        }
    }
    
    bool isEmpty() const { return queueSize == 0; }
    int getSize() const { return queueSize; }
    int getAvailableSlots() const { return availableSlots; }
};

// ---------------- USER PREFERENCES ----------------

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

// ---------------- ROUTE NODE ----------------

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

// ---------------- PORT ----------------

struct Port {
    char portName[MAX_NAME_LENGTH];
    unsigned short dailyDockingCharge;
    RouteNode* routeListHead;
    RouteNode* routeListTail;
    DockingQueue* dockQueue;
    
    Port() {
        portName[0] = '\0';
        dailyDockingCharge = 0;
        routeListHead = NULL;
        routeListTail = NULL;
        dockQueue = new DockingQueue(2);  // FIX Q1: 2 docking slots per port
    }
    
    ~Port() {
        delete dockQueue;
    }
};

// ---------------- PRIORITY QUEUE ----------------

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
    const int MIN_TRANSFER_MINUTES = 60;
    
    if (departureMins >= arrivalMins) {
        return (departureMins - arrivalMins) >= MIN_TRANSFER_MINUTES;
    }
    return (1440 - arrivalMins + departureMins) >= MIN_TRANSFER_MINUTES;
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

void clearInputBuffer() {
    cin.clear();
    cin.ignore(INT_MAX, '\n');
}

// ---------------- GRAPH CLASS ----------------
class Graph {
private:
    Port ports[MAX_PORTS];
    unsigned char totalPorts;
    Journey* savedJourneys[MAX_SAVED_JOURNEYS];
    int journeyCount;
    
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

public:
    Graph() : totalPorts(0), journeyCount(0) {
        for (int i = 0; i < MAX_SAVED_JOURNEYS; i++) {
            savedJourneys[i] = NULL;
        }
    }

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
            loaded++;
        }

        file.close();
        cout << "Loaded " << loaded << " routes.\n\n";
    }

    void displayGraph() const {
        if (totalPorts == 0) {
            cout << "\nNo ports loaded!\n";
            return;
        }
        
        cout << "\n========== PORT NETWORK ==========\n\n";
        for (unsigned char i = 0; i < totalPorts; i++) {
            cout << ports[i].portName << " ($" << ports[i].dailyDockingCharge << "/day)";
            
            // FIX Q4: Show ship count for each port
            if (ports[i].dockQueue->getSize() > 0) {
                cout << " [" << ports[i].dockQueue->getSize() << " ships in queue]";
            }
            cout << "\n";
            
            RouteNode* route = ports[i].routeListHead;
            if (!route) {
                cout << "  No routes\n";
            } else {
                while (route) {
                    cout << "  → " << ports[route->destinationIndex].portName
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

    Journey* findCheapestRoute(int srcIdx, int destIdx, const char* preferredDate, 
                          bool useAStar = false, const UserPreferences* prefs = NULL,
                          bool saveJourney = false) {
        if (!isValidPortIndex(srcIdx) || !isValidPortIndex(destIdx)) {
            cout << "\n❌ Invalid port indices!\n";
            return NULL;
        }
        
        if (!isValidDateFormat(preferredDate)) {
            cout << "\n❌ Invalid date format!\n";
            return NULL;
        }
        
        if (srcIdx == destIdx) {
            cout << "\n❌ Source and destination are the same!\n";
            return NULL;
        }
        
        if (prefs && prefs->hasAvoidPort) {
            if (strcmp(ports[srcIdx].portName, prefs->avoidPort) == 0 ||
                strcmp(ports[destIdx].portName, prefs->avoidPort) == 0) {
                cout << "\n❌ Cannot avoid source or destination port!\n";
                return NULL;
            }
        }

        for (unsigned char i = 0; i < totalPorts; i++) {
            ports[i].dockQueue->clearQueue();
        }

        const char* algoName = useAStar ? "A* ALGORITHM" : "DIJKSTRA'S ALGORITHM";
        
        unsigned int minCost[MAX_PORTS];
        unsigned int totalTime[MAX_PORTS];
        unsigned int queueWaitTime[MAX_PORTS] = {0};
        unsigned int layoverTime[MAX_PORTS] = {0};
        unsigned int dockingCharges[MAX_PORTS] = {0};
        bool visited[MAX_PORTS] = {false};
        char prevPort[MAX_PORTS];
        RouteNode* usedRoute[MAX_PORTS] = {NULL};
        unsigned int nodesExplored = 0;
        unsigned int routesFiltered = 0;

        for (unsigned char i = 0; i < totalPorts; i++) {
            minCost[i] = UINT_MAX;
            totalTime[i] = 0;
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
                    unsigned int additionalWait = 0;
                    unsigned int layoverHrs = 0;
                    unsigned int dockCharge = 0;
                    
                    if (prevPort[currPort] != -1 && usedRoute[currPort]) {
                        timeValid = isValidConnection(
                            usedRoute[currPort]->arrivalMins,
                            route->departureMins
                        );
                        
                        if (timeValid) {
                            int queueWait = ports[currPort].dockQueue->calculateWaitTime(
                                usedRoute[currPort]->arrivalMins
                            );
                            additionalWait = queueWait / 60;
                            
                            layoverHrs = calculateLayoverHours(
                                usedRoute[currPort]->arrivalMins,
                                route->departureMins
                            );
                            
                            int totalLayover = layoverHrs + additionalWait;
                            
                            if (totalLayover > 12) {
                                int days = (totalLayover / 24) + 1;
                                dockCharge = ports[currPort].dailyDockingCharge * days;
                            }
                        }
                    }
                    
                    if (timeValid && !visited[nextPort]) {
                        unsigned int newCost = minCost[currPort] + route->voyageCost + dockCharge;
                        unsigned int voyageTime = calculateVoyageTime(route);
                        unsigned int newTotalTime = totalTime[currPort] + voyageTime + additionalWait;
                        
                        bool withinTimeLimit = true;
                        if (prefs && prefs->hasMaxTimeLimit) {
                            withinTimeLimit = (newTotalTime <= prefs->maxTimeLimit);
                        }
                        
                        if (withinTimeLimit && newCost < minCost[nextPort]) {
                            minCost[nextPort] = newCost;
                            totalTime[nextPort] = newTotalTime;
                            queueWaitTime[nextPort] = queueWaitTime[currPort] + additionalWait;
                            layoverTime[nextPort] = layoverHrs;
                            dockingCharges[nextPort] = dockCharge;
                            prevPort[nextPort] = currPort;
                            usedRoute[nextPort] = route;
                            
                            if (prevPort[currPort] != -1) {
                                // FIX Q3: Keep service time fixed at 120 minutes (2 hours)
                                // Reason: We don't have cargo/capacity data in Routes.txt
                                ports[currPort].dockQueue->enqueue(
                                    usedRoute[currPort]->shippingCompany,
                                    usedRoute[currPort]->arrivalMins,
                                    120  // Fixed service time
                                );
                            }
                            
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
            return NULL;
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

        // BUILD MULTI-LEG JOURNEY LINKED LIST
        Journey* newJourney = new Journey();
        char journeyName[50];
        sprintf(journeyName, "%s to %s", ports[srcIdx].portName, ports[destIdx].portName);
        newJourney->setName(journeyName);
        
        unsigned int totalDocking = 0;
        
        for (int i = len - 1; i >= 0; i--) {
            cout << ports[path[i]].portName;
            
            // FIX Q4: Show queue count if ships waiting
            if (i < len - 1 && ports[path[i]].dockQueue->getSize() > 0) {
                cout << " [" << ports[path[i]].dockQueue->getSize() << " ships]";
            }
            
            if (i > 0 && routes[i-1]) {
                RouteNode* r = routes[i-1];
                cout << "\n  ↓ [" << r->shippingCompany << "] "
                     << r->departureTime << " (" << r->voyageDate << ") "
                     << "→ " << r->arrivalTime << " | $" << r->voyageCost;
                
                unsigned int layover = 0;
                unsigned int queueWait = 0;
                unsigned int dockCharge = 0;
                
                if (i > 1 && routes[i-2]) {
                    layover = calculateLayoverHours(r->arrivalMins, routes[i-2]->departureMins);
                    queueWait = ports[path[i]].dockQueue->calculateWaitTime(r->arrivalMins) / 60;
                    int totalWait = layover + queueWait;
                    
                    if (layover > 0 || queueWait > 0) {
                        cout << "\n  ⏱ Layover: " << layover << "h";
                        if (queueWait > 0) {
                            cout << " + Queue Wait: " << queueWait << "h";
                        }
                        cout << " (Total: " << totalWait << "h)";
                        
                        if (totalWait > 12) {
                            int days = (totalWait / 24) + 1;
                            dockCharge = ports[path[i]].dailyDockingCharge * days;
                            totalDocking += dockCharge;
                            cout << " → Docking charge: $" << dockCharge << " (" << days << " days)";
                        }
                    }
                    
                    // Add ship to queue for simulation
                    ports[path[i]].dockQueue->enqueue(
                        r->shippingCompany,
                        r->arrivalMins,
                        120  // FIX Q3: Fixed service time
                    );
                }
                
                // ADD LEG TO JOURNEY LINKED LIST
                newJourney->addLeg(
                    ports[path[i]].portName,
                    ports[path[i-1]].portName,
                    r->voyageDate,
                    r->departureTime,
                    r->arrivalTime,
                    r->voyageCost,
                    r->shippingCompany,
                    r->departureMins,
                    r->arrivalMins,
                    layover,
                    queueWait,
                    dockCharge
                );
                
                cout << "\n\n";
            }
        }

        cout << "====================================\n";
        cout << "Voyage Cost: $" << minCost[destIdx] - totalDocking << "\n";
        if (queueWaitTime[destIdx] > 0) {
            cout << "Queue Wait Time: " << queueWaitTime[destIdx] << " hours\n";
        }
        cout << "Total Time: " << totalTime[destIdx] << " hours\n";
        if (totalDocking > 0) {
            cout << "Port Docking Charges: $" << totalDocking << "\n";
        }
        cout << "TOTAL COST: $" << minCost[destIdx] << "\n";
        cout << "====================================\n\n";
        
        // SAVE JOURNEY IF REQUESTED
        if (saveJourney && journeyCount < MAX_SAVED_JOURNEYS) {
            savedJourneys[journeyCount] = newJourney;
            journeyCount++;
            cout << "✅ Journey saved! (Total saved: " << journeyCount << ")\n\n";
            return newJourney;
        }
        
        return newJourney;
    }

    void viewSavedJourneys() {
        if (journeyCount == 0) {
            cout << "\n📭 No saved journeys yet!\n";
            return;
        }
        
        cout << "\n╔════════════════════════════════════════════╗\n";
        cout << "║           SAVED JOURNEYS                  ║\n";
        cout << "╚════════════════════════════════════════════╝\n\n";
        
        for (int i = 0; i < journeyCount; i++) {
            cout << (i + 1) << ". ";
            savedJourneys[i]->displaySummary();
        }
        cout << "\n";
    }
    
    void viewJourneyDetails() {
        if (journeyCount == 0) {
            cout << "\n📭 No saved journeys yet!\n";
            return;
        }
        
        viewSavedJourneys();
        
        cout << "Enter journey number to view details (0 to cancel): ";
        int choice;
        cin >> choice;
        clearInputBuffer();
        
        if (choice < 1 || choice > journeyCount) {
            cout << "❌ Invalid choice!\n";
            return;
        }
        
        savedJourneys[choice - 1]->displayJourney();
    }
    
    void compareJourneys() {
        if (journeyCount < 2) {
            cout << "\n⚠️ Need at least 2 saved journeys to compare!\n";
            return;
        }
        
        viewSavedJourneys();
        
        cout << "\nEnter first journey number: ";
        int j1;
        cin >> j1;
        cout << "Enter second journey number: ";
        int j2;
        cin >> j2;
        clearInputBuffer();
        
        if (j1 < 1 || j1 > journeyCount || j2 < 1 || j2 > journeyCount) {
            cout << "❌ Invalid choice!\n";
            return;
        }
        
        cout << "\n╔════════════════════════════════════════════╗\n";
        cout << "║         JOURNEY COMPARISON                ║\n";
        cout << "╚════════════════════════════════════════════╝\n\n";
        
        Journey* journey1 = savedJourneys[j1 - 1];
        Journey* journey2 = savedJourneys[j2 - 1];
        
        cout << "Journey 1: ";
        journey1->displayChain();
        cout << "\n  • Legs: " << journey1->getLegCount() << "\n";
        cout << "  • Cost: $" << journey1->getTotalCost() << "\n";
        cout << "  • Time: " << journey1->getTotalTime() << " hours\n\n";
        
        cout << "Journey 2: ";
        journey2->displayChain();
        cout << "\n  • Legs: " << journey2->getLegCount() << "\n";
        cout << "  • Cost: $" << journey2->getTotalCost() << "\n";
        cout << "  • Time: " << journey2->getTotalTime() << " hours\n\n";
        
        cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        
        if (journey1->getTotalCost() < journey2->getTotalCost()) {
            cout << "💰 Journey 1 is cheaper by $" 
                 << (journey2->getTotalCost() - journey1->getTotalCost()) << "\n";
        } else if (journey2->getTotalCost() < journey1->getTotalCost()) {
            cout << "💰 Journey 2 is cheaper by $" 
                 << (journey1->getTotalCost() - journey2->getTotalCost()) << "\n";
        } else {
            cout << "💰 Both journeys cost the same\n";
        }
        
        if (journey1->getTotalTime() < journey2->getTotalTime()) {
            cout << "⏱️  Journey 1 is faster by " 
                 << (journey2->getTotalTime() - journey1->getTotalTime()) << " hours\n";
        } else if (journey2->getTotalTime() < journey1->getTotalTime()) {
            cout << "⏱️  Journey 2 is faster by " 
                 << (journey1->getTotalTime() - journey2->getTotalTime()) << " hours\n";
        } else {
            cout << "⏱️  Both journeys take the same time\n";
        }
        
        cout << "\n";
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
            cout << "2. Find Cheapest Route\n";
            cout << "3. Find Route with Preferences\n";
            cout << "4. View Saved Journeys\n";
            cout << "5. View Journey Details\n";
            cout << "6. Compare Journeys\n";
            cout << "7. Exit\n\n";
            cout << "Choice (1-7): ";
            
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
                    
                case 2: {
                    char src[MAX_NAME_LENGTH], dst[MAX_NAME_LENGTH], date[MAX_DATE_LENGTH];
                    char saveChoice;
                    
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
                    clearInputBuffer();

                    int si = getPortIndex(src);
                    int di = getPortIndex(dst);

                    if (si == -1) {
                        cout << "❌ Source port '" << src << "' not found!\n";
                    } else if (di == -1) {
                        cout << "❌ Destination port '" << dst << "' not found!\n";
                    } else {
                        bool hasDirectRoute = hasValidDirectRoute(si, di, date, NULL);
                        
                        cout << "Save this journey? (y/n): ";
                        cin >> saveChoice;
                        clearInputBuffer();
                        bool shouldSave = (saveChoice == 'y' || saveChoice == 'Y');
                        
                        if (hasDirectRoute) {
                            cout << "\n🎯 Direct route detected - Using A* algorithm\n";
                            Journey* j = findCheapestRoute(si, di, date, true, NULL, shouldSave);
                            if (!shouldSave && j) delete j;
                        } else {
                            cout << "\n🔍 Multi-hop route needed - Using Dijkstra's algorithm\n";
                            Journey* j = findCheapestRoute(si, di, date, false, NULL, shouldSave);
                            if (!shouldSave && j) delete j;
                        }
                    }
                    break;
                }
                
                case 3: {
                    char src[MAX_NAME_LENGTH], dst[MAX_NAME_LENGTH], date[MAX_DATE_LENGTH];
                    char saveChoice;
                    
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
                        
                        cout << "\nSave this journey? (y/n): ";
                        cin >> saveChoice;
                        clearInputBuffer();
                        bool shouldSave = (saveChoice == 'y' || saveChoice == 'Y');
                        
                        bool hasDirectRoute = hasValidDirectRoute(si, di, date, &prefs);
                        
                        if (hasDirectRoute && !prefs.hasAnyFilter()) {
                            cout << "\n🎯 Direct route detected - Using A* algorithm\n";
                            Journey* j = findCheapestRoute(si, di, date, true, &prefs, shouldSave);
                            if (!shouldSave && j) delete j;
                        } else {
                            cout << "\n🔍 Multi-hop/Filtered route - Using Dijkstra's algorithm\n";
                            Journey* j = findCheapestRoute(si, di, date, false, &prefs, shouldSave);
                            if (!shouldSave && j) delete j;
                        }
                    }
                    break;
                }
                
                case 4:
                    viewSavedJourneys();
                    break;
                
                case 5:
                    viewJourneyDetails();
                    break;
                
                case 6:
                    compareJourneys();
                    break;
                
                case 7:
                    cout << "\nThank you! Safe travels! 🚢\n";
                    break;
                    
                default:
                    cout << "❌ Invalid choice!\n";
            }
        } while (choice != 7);
    }

    ~Graph() {
        for (unsigned char i = 0; i < totalPorts; i++) {
            RouteNode* curr = ports[i].routeListHead;
            while (curr) {
                RouteNode* temp = curr;
                curr = curr->nextRoute;
                delete temp;
            }
        }
        
        for (int i = 0; i < journeyCount; i++) {
            delete savedJourneys[i];
        }
    }
};

// ---------------- MAIN ----------------
int main() {
    Graph network;
    network.run();
    return 0;
}
