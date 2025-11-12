#include <iostream>
#include <fstream>
#include <cstring>
#include <climits>
using namespace std;

#define MAX_PORTS 15

// ---------------- STRUCTURES ----------------
struct RouteNode {
    int destIndex;
    char date[20];
    char depTime[10];
    char arrTime[10];
    int distance;
    char company[30];
    RouteNode* next;
};

struct Port {
    char name[30];
    int dailyCharge;
    RouteNode* head;
};

// ---------------- CLASS DEFINITION ----------------
class Graph {
private:
    Port ports[MAX_PORTS];
    int portCount;

public:
    Graph() {
        portCount = 0;
    }

    // Find port index by name
    int getPortIndex(const char* portName) {
        for (int i = 0; i < portCount; i++) {
            if (strcmp(ports[i].name, portName) == 0)
                return i;
        }
        return -1;
    }

    // Load ports and their daily charges
    void loadPortCharges() {
        ifstream file("portcharges.txt");
        if (!file) {
            cout << "Error opening portcharges.txt" << endl;
            return;
        }

        while (file >> ports[portCount].name >> ports[portCount].dailyCharge) {
            ports[portCount].head = NULL;
            portCount++;
        }

        file.close();
    }

    // Load routes (only those involving the 15 known ports)
    void loadRoutes() {
        ifstream file("routes.txt");
        if (!file) {
            cout << "Error opening routes.txt" << endl;
            return;
        }

        char origin[30], destination[30], date[20], dep[10], arr[10], company[30];
        int distance;

        while (file >> origin >> destination >> date >> dep >> arr >> distance >> company) {
            int from = getPortIndex(origin);
            int to = getPortIndex(destination);

            if (from == -1 || to == -1)
                continue; // skip unknown ports

            RouteNode* newNode = new RouteNode();
            newNode->destIndex = to;
            strcpy(newNode->date, date);
            strcpy(newNode->depTime, dep);
            strcpy(newNode->arrTime, arr);
            newNode->distance = distance;
            strcpy(newNode->company, company);
            newNode->next = NULL;

            if (ports[from].head == NULL)
                ports[from].head = newNode;
            else {
                RouteNode* temp = ports[from].head;
                while (temp->next != NULL)
                    temp = temp->next;
                temp->next = newNode;
            }
        }

        file.close();
    }

    // Display all ports and their routes
    void showGraph() {
        cout << "\n--- PORT GRAPH (15 Ports) ---\n";
        for (int i = 0; i < portCount; i++) {
            cout << ports[i].name << " (" << ports[i].dailyCharge << "$/day) → ";
            RouteNode* temp = ports[i].head;
            while (temp != NULL) {
                cout << ports[temp->destIndex].name
                     << " (" << temp->distance << " km, " << temp->company << ")  ";
                temp = temp->next;
            }
            cout << endl;
        }
    }

    // Dijkstra’s shortest path algorithm
    void dijkstra(int src, int dest) {
        int dist[MAX_PORTS];
        bool visited[MAX_PORTS];
        int prev[MAX_PORTS];

        for (int i = 0; i < portCount; i++) {
            dist[i] = INT_MAX;
            visited[i] = false;
            prev[i] = -1;
        }

        dist[src] = 0;

        for (int count = 0; count < portCount - 1; count++) {
            int u = -1, minDist = INT_MAX;
            for (int i = 0; i < portCount; i++) {
                if (!visited[i] && dist[i] < minDist) {
                    minDist = dist[i];
                    u = i;
                }
            }

            if (u == -1) break;
            visited[u] = true;

            RouteNode* temp = ports[u].head;
            while (temp != NULL) {
                int v = temp->destIndex;
                int newDist = dist[u] + temp->distance;
                if (!visited[v] && newDist < dist[v]) {
                    dist[v] = newDist;
                    prev[v] = u;
                }
                temp = temp->next;
            }
        }

        if (dist[dest] == INT_MAX) {
            cout << "\nNo route from " << ports[src].name << " to " << ports[dest].name << endl;
            return;
        }

        // Path reconstruction
        int path[MAX_PORTS];
        int len = 0;
        for (int cur = dest; cur != -1; cur = prev[cur])
            path[len++] = cur;

        int totalCharge = 0;
        for (int i = len - 1; i >= 0; i--)
            totalCharge += ports[path[i]].dailyCharge;

        cout << "\nShortest route from " << ports[src].name
             << " to " << ports[dest].name << ":\n";
        for (int i = len - 1; i >= 0; i--) {
            cout << ports[path[i]].name;
            if (i > 0) cout << " → ";
        }

        cout << "\nTotal distance: " << dist[dest] << " km";
        cout << "\nTotal port charges: $" << totalCharge << endl;
    }

    // Run the full system
    void run() {
        loadPortCharges();
        loadRoutes();
        showGraph();

        char source[30], destination[30];
        cout << "\nEnter Source Port: ";
        cin >> source;
        cout << "Enter Destination Port: ";
        cin >> destination;

        int srcIndex = getPortIndex(source);
        int destIndex = getPortIndex(destination);

        if (srcIndex == -1 || destIndex == -1) {
            cout << "Invalid port name entered!" << endl;
            return;
        }

        dijkstra(srcIndex, destIndex);
    }
};

// ---------------- MAIN ----------------
int main() {
    Graph g;
    g.run();
    return 0;
}