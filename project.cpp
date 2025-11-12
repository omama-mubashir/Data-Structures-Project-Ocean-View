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

// ---------------- OPTIMIZED STRUCTURES ----------------

// Compact RouteNode - minimal memory footprint
struct RouteNode {
    unsigned char destinationIndex;  // 0-14 fits in 1 byte instead of 4
    char voyageDate[MAX_DATE_LENGTH];
    char departureTime[MAX_TIME_LENGTH];
    char arrivalTime[MAX_TIME_LENGTH];
    unsigned short departureMins;    // Pre-computed for O(1) comparisons
    unsigned short arrivalMins;      // Pre-computed for O(1) comparisons
    unsigned int voyageCost;
    char shippingCompany[MAX_COMPANY_LENGTH];
    RouteNode* nextRoute;
};

struct Port {
    char portName[MAX_NAME_LENGTH];
    unsigned short dailyDockingCharge;
    RouteNode* routeListHead;
    RouteNode* routeListTail;  // O(1) insertion at end
};

// Priority Queue Node for efficient Dijkstra
struct PQNode {
    unsigned char portIndex;
    unsigned int cost;
    PQNode* next;
};

// Simple Priority Queue (Min-Heap behavior via sorted linked list)
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
    
    // O(n) insertion but keeps list sorted, better than array for dynamic size
    void push(unsigned char portIdx, unsigned int cost) {
        PQNode* newNode = new (nothrow) PQNode();
        if (!newNode) return;
        
        newNode->portIndex = portIdx;
        newNode->cost = cost;
        newNode->next = NULL;
        size++;
        
        // Insert in sorted position
        if (!head || cost < head->cost) {
            newNode->next = head;
            head = newNode;
            return;
        }
        
        PQNode* curr = head;
        while (curr->next && curr->next->cost < cost) {
            curr = curr->next;
        }
        newNode->next = curr->next;
        curr->next = newNode;
    }
    
    // O(1) extraction
    bool pop(unsigned char& portIdx, unsigned int& cost) {
        if (!head) return false;
        
        PQNode* temp = head;
        portIdx = head->portIndex;
        cost = head->cost;
        head = head->next;
        delete temp;
        size--;
        return true;
    }
    
    bool isEmpty() const { return head == NULL; }
    int getSize() const { return size; }
};

// ---------------- OPTIMIZED HELPER FUNCTIONS ----------------

// Inline for performance on frequently called functions
inline unsigned short timeToMinutes(const char* time) {
    if (!time || strlen(time) < 5) return 0;
    return ((time[0] - '0') * 10 + (time[1] - '0')) * 60 + 
           ((time[3] - '0') * 10 + (time[4] - '0'));
}

// Fast validation without repeated strlen calls
inline bool isValidTimeFormat(const char* time) {
    return time && time[0] >= '0' && time[0] <= '2' && 
           time[1] >= '0' && time[1] <= '9' && time[2] == ':' &&
           time[3] >= '0' && time[3] <= '5' && 
           time[4] >= '0' && time[4] <= '9' && time[5] == '\0';
}

inline bool isValidDateFormat(const char* date) {
    if (!date) return false;
    
    // Quick length check
    int len = 0;
    while (date[len] && len < 11) len++;
    if (len != 10) return false;
    
    // Format: DD/MM/YYYY
    return isdigit(date[0]) && isdigit(date[1]) && date[2] == '/' &&
           isdigit(date[3]) && isdigit(date[4]) && date[5] == '/' &&
           isdigit(date[6]) && isdigit(date[7]) && 
           isdigit(date[8]) && isdigit(date[9]);
}

// O(1) time comparison using pre-computed minutes
inline bool isValidConnection(unsigned short arrivalMins, unsigned short departureMins) {
    return departureMins >= arrivalMins || departureMins < arrivalMins; // Next day is always valid
}

inline int calculateLayoverHours(unsigned short arrivalMins, unsigned short departureMins) {
    if (departureMins < arrivalMins) {
        return (1440 - arrivalMins + departureMins) / 60; // Next day
    }
    return (departureMins - arrivalMins) / 60;
}

// Optimized date comparison - parse once, compare as integers
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

// ---------------- OPTIMIZED GRAPH CLASS ----------------
class Graph {
private:
    Port ports[MAX_PORTS];
    unsigned char totalPorts;

public:
    Graph() : totalPorts(0) {}

    // O(n) but n is small (15), could use hash for larger datasets
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

    // Optimized file loading - single pass, minimal string operations
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
            
            // Direct copy without extra validation overhead
            strcpy(ports[totalPorts].portName, name);
            ports[totalPorts].dailyDockingCharge = (unsigned short)charge;
            ports[totalPorts].routeListHead = NULL;
            ports[totalPorts].routeListTail = NULL;
            totalPorts++;
        }

        file.close();
        cout << "Loaded " << (int)totalPorts << " ports.\n";
    }

    // Optimized route loading - O(1) tail insertion
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
            node->departureMins = timeToMinutes(depTime);  // Pre-compute
            node->arrivalMins = timeToMinutes(arrTime);    // Pre-compute
            node->voyageCost = (unsigned int)cost;
            strcpy(node->shippingCompany, company);
            node->nextRoute = NULL;

            // O(1) tail insertion
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
            cout << ports[i].portName << " ($" << ports[i].dailyDockingCharge << "/day)\n";
            
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

    // Optimized Dijkstra with Priority Queue - O(E log V) instead of O(V²)
    void findCheapestRoute(int srcIdx, int destIdx, const char* preferredDate) {
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

        // Compact arrays using smaller types
        unsigned int minCost[MAX_PORTS];
        bool visited[MAX_PORTS] = {false};
        char prevPort[MAX_PORTS];
        RouteNode* usedRoute[MAX_PORTS] = {NULL};

        // Initialize
        for (unsigned char i = 0; i < totalPorts; i++) {
            minCost[i] = UINT_MAX;
            prevPort[i] = -1;
        }
        minCost[srcIdx] = 0;

        // Priority Queue for efficient minimum extraction
        PriorityQueue pq;
        pq.push(srcIdx, 0);

        while (!pq.isEmpty()) {
            unsigned char currPort;
            unsigned int currCost;
            
            if (!pq.pop(currPort, currCost)) break;
            
            if (visited[currPort]) continue;
            visited[currPort] = true;

            // Early termination - found destination
            if (currPort == destIdx) break;

            // Explore neighbors
            RouteNode* route = ports[currPort].routeListHead;
            while (route) {
                unsigned char nextPort = route->destinationIndex;
                
                // Date filter
                if (isSameDateOrLater(route->voyageDate, preferredDate)) {
                    // Time validation using pre-computed minutes - O(1)
                    bool timeValid = true;
                    if (prevPort[currPort] != -1 && usedRoute[currPort]) {
                        timeValid = isValidConnection(
                            usedRoute[currPort]->arrivalMins,
                            route->departureMins
                        );
                    }
                    
                    if (timeValid && !visited[nextPort]) {
                        unsigned int newCost = minCost[currPort] + route->voyageCost;
                        
                        if (newCost < minCost[nextPort]) {
                            minCost[nextPort] = newCost;
                            prevPort[nextPort] = currPort;
                            usedRoute[nextPort] = route;
                            pq.push(nextPort, newCost);
                        }
                    }
                }
                route = route->nextRoute;
            }
        }

        // Results
        if (minCost[destIdx] == UINT_MAX) {
            cout << "\n❌ No route from " << ports[srcIdx].portName 
                 << " to " << ports[destIdx].portName << "\n";
            return;
        }

        // Reconstruct path - optimized with single pass
        unsigned char path[MAX_PORTS];
        RouteNode* routes[MAX_PORTS];
        unsigned char len = 0;
        
        for (char curr = destIdx; curr != -1 && len < MAX_PORTS; curr = prevPort[curr]) {
            path[len] = curr;
            routes[len] = usedRoute[curr];
            len++;
        }

        // Display results
        cout << "\n========== CHEAPEST ROUTE ==========\n";
        cout << "From: " << ports[srcIdx].portName << "\n";
        cout << "To: " << ports[destIdx].portName << "\n";
        cout << "Date: " << preferredDate << "\n\n";

        unsigned int totalDocking = 0;
        
        for (int i = len - 1; i >= 0; i--) {
            cout << ports[path[i]].portName;
            totalDocking += ports[path[i]].dailyDockingCharge;
            
            if (i > 0 && routes[i-1]) {
                RouteNode* r = routes[i-1];
                cout << "\n  ↓ [" << r->shippingCompany << "] "
                     << r->departureTime << " (" << r->voyageDate << ") "
                     << "→ " << r->arrivalTime << " | $" << r->voyageCost;
                
                if (i > 1 && routes[i-2]) {
                    int layover = calculateLayoverHours(r->arrivalMins, routes[i-2]->departureMins);
                    if (layover > 0) {
                        cout << "\n  ⏱ Layover: " << layover << "h";
                        if (layover > 12) cout << " (Extended)";
                    }
                }
                cout << "\n\n";
            }
        }

        cout << "====================================\n";
        cout << "Voyage Cost: $" << minCost[destIdx] << "\n";
        cout << "Port Charges: $" << totalDocking << "\n";
        cout << "TOTAL: $" << (minCost[destIdx] + totalDocking) << "\n";
        cout << "====================================\n\n";
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
            cout << "3. Exit\n\n";
            cout << "Choice (1-3): ";
            
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
                    
                    cout << "\nPorts: ";
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
                        findCheapestRoute(si, di, date);
                    }
                    clearInputBuffer();
                    break;
                }
                
                case 3:
                    cout << "\nThank you! Safe travels! 🚢\n";
                    break;
                    
                default:
                    cout << "❌ Invalid choice!\n";
            }
        } while (choice != 3);
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
    }
};

// ---------------- MAIN ----------------
int main() {
    Graph network;
    network.run();
    return 0;
}