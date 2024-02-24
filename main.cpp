// Project Identifier: 19034C8F3B1196BF8E0C6E1C0F973D2FD550B88F
#include <getopt.h>
#include <string>
#include <limits>
#include <algorithm>
#include <deque>
#include <queue>
#include <vector>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "P2random.h"
using namespace std;

// overload () operator
struct Options {
    bool median = false;
    bool verbose = false;
    size_t stats = numeric_limits<size_t>::max();
};

struct Tile {
    bool discovered = false;
    bool investigated = false;
    int rubble;
    int rubbleOrig;
    size_t row;
    size_t col;
    bool isTNT = false;
};

struct TileComparator {
    bool operator()(const Tile *a, const Tile *b) const {
        // Compare based on priority criteria:
        // 1. Smallest rubble value
        // 2. Column number
        // 3. Row number
        if (a->rubble != b->rubble) {
            return a->rubble > b->rubble; // Smallest rubble value has higher priority
        }
        if (a->col != b->col) {
            return a->col > b->col; // Smaller column number has higher priority
        }
        return a->row > b->row; // Smaller row number has higher priority
    }
};

void printHelp(char *argv[])
{
    cout << "Usage: " << argv[0] << " [-m resize|reserve|nosize] | -h\n";
    cout << "This program is to help you learn command-line processing,\n";
    cout << "reading data into a vector, the difference between resize and\n";
    cout << "reserve and how to properly read until end-of-file." << endl;
} // printHelp()


class Map {
    private:
        size_t tile = numeric_limits<size_t>::max();
        size_t size = numeric_limits<size_t>::max();
        pair<size_t, size_t> start = {numeric_limits<size_t>::max(), numeric_limits<size_t>::max()};
        char format = 'F';
        int seed = 0;
        int maxRubble = 0;
        size_t tnt = 0;
        Options opt;

    public:
        void readInput() {
            string junk;
            // read in mapFormat
            cin >> format;
            if (format != 'M' && format != 'R') {
                cerr << "Invalid input mode" << endl;
                exit(1);
            }
            // read in size
            cin >> junk >> size;
            cin >> junk >> start.first >> start.second;
            // error check for column
            if (start.first > size) {
                cerr << "Invalid starting row" << endl;
                exit(1);
            }
            // error check for row
            if (start.second > size) {
                cerr << "Invalid starting row" << endl;
                exit(1);
            }
            // Clear the existing content of map2D and then resize it
            map2D.clear();
            map2D.resize(size, vector<Tile>(size));
            // read into map2D
            if (format == 'M') { // map input
                for (size_t i = 0; i < size; i++) {
                    for (size_t j = 0; j < size; j++) {
                        Tile t;
                        cin >> t.rubble;
                        t.rubbleOrig = t.rubble;
                        t.row = i;
                        t.col = j;
                        map2D[i][j] = t;
                        if (map2D[i][j].rubble == -1) map2D[i][j].isTNT = true;
                    }
            }
            }
            else { // grid input 
                stringstream ss;
                uint32_t seed, maxRubble, tnt;
                cin >> junk >> seed >> junk >> maxRubble >> junk >> tnt;
                P2random::PR_init(ss, static_cast<uint32_t>(size), seed, maxRubble, tnt);
                // istream &inputStream = (format == 'M') ? cin : ss;
                
                for (size_t i = 0; i < size; ++i) {
                    for (size_t j = 0; j < size; j++) {
                        int rubble;
                        ss >> rubble;
                        Tile t;
                        t.rubble = rubble;
                        t.rubbleOrig = rubble;
                        t.row = i;
                        t.col = j;
                        map2D[i][j] = t;
                        if (map2D[i][j].rubble == -1) map2D[i][j].isTNT = true;
                    }
                }
            }
        }
        void printInput() {
            cout << "Size: " << size << endl;
            cout << "Start: " << start.first << " " << start.second << "\n";
            //cout << "FORMAT: " << format << '\n';
            if (format == 'M') { // map format == M
                for (size_t i = 0; i < size; ++i) {
                    for (size_t j = 0; j < size; j++) {
                        if (j == 0) {
                            cout << '\n';
                        }
                        cout << map2D[i][j].rubble << " ";
                        }
                    }
                }  

            else { // map format == R
                cout << "Seed: " << seed << '\n';
                cout << "Max_Rubble: " << maxRubble << '\n';
                cout << "TNT: " << tnt << '\n';
            }
            cout << endl << endl;
        }
        // getter for format
        char getFormat() {
            return format;
        }
        // getter for size
        size_t getSize() {
            return size;
        }
        // getter for map2D
        vector<vector<Tile>> getMap2D() {
            return map2D;
        }
        // getter for maxRubble
        int getMaxRubble() {
            return maxRubble;
        }
        // getter for seed
        int getSeed() {
            return seed;
        }
        // getter for tnt
        size_t getTNT() {
            return tnt;
        }
        pair<size_t, size_t> getStart() {
            return start;
        }

        ~Map() {
            // Destructor to deallocate memory for map2D
            map2D.clear();
        }
        Map() : 
            tile(numeric_limits<size_t>::max()), 
            size(numeric_limits<size_t>::max()), 
            start({numeric_limits<size_t>::max(), numeric_limits<size_t>::max()})
        {}

        Map(const Map &other) :
            tile(other.tile),
            size(other.size),
            start({other.start.first, other.start.second})
        {}
        vector<vector<Tile>> map2D;
};

// solver
class Mining {
    private:
        Options opt;
        priority_queue<Tile*, vector<Tile*>, TileComparator> pq;
        size_t numCleared = 0;
        int amountCleared = 0;
        vector<Tile> clearedTiles;
        Map m;
        priority_queue<int, vector<int>, less<int>> minHeap;
        priority_queue<int, vector<int>, greater<int>> maxHeap;

    public:
        void solve() {
            m.readInput();
            size_t size = m.getSize();
            Tile* c = &(m.map2D[m.getStart().first][m.getStart().second]);
            c->discovered = true;
            pq.push(c);
            if (((size == 1) || (c->row == size - 1) || (c->col == size - 1) || (c->row == 0) || (c->col == 0)) && !c->isTNT && c->rubble != 0) {
                numCleared++;
                amountCleared += c->rubbleOrig;
                if (opt.median) {
                    push(c->rubble);
                    printMedian(c);
                }
            }

            while (!pq.empty() && (c->row < size - 1) && (c->col < size - 1) && (c->row > 0) && (c->col > 0)) {
                c = pq.top();
                verbose(c);
                pq.pop();
                if (c->investigated) continue;

                if (c->isTNT && c->rubble != 0) {
                    expload(c); 
                    continue; 

                } else {
                    amountCleared += c->rubble;
                    c->rubble = 0;
                }
                c->rubble = 0;

                c->investigated = true;

                if (c->row > 0) {
                    Tile *t = &(m.map2D[c->row-1][c->col]); // top
                    if (!t->discovered) {
                        if (t->isTNT) {
                            expload(t);
                            t->investigated = true;
                            continue;
                        }
                        else pq.push(t);
                        t->discovered = true;
                    }
                }
                if (c->row < m.getSize() - 1) {
                    Tile *b = &(m.map2D[c->row+1][c->col]);  // bottom
                    if (!b->discovered) {
                        if (b->isTNT) {
                            expload(b);
                            b->investigated = true;
                            continue;
                        }
                        else pq.push(b);
                        b->discovered = true;
                    }
                }
                if (c->col > 0) {
                    Tile *l = &(m.map2D[c->row][c->col-1]);  // left
                    if (!l->discovered) {
                        if (l->isTNT) {
                            expload(l);
                            l->investigated = true;
                            continue;
                        }
                        else pq.push(l);
                        l->discovered = true;
                    }
                }
                if (c->col < m.getSize() - 1) {
                    Tile *r = &(m.map2D[c->row][c->col+1]); // right
                    if (!r->discovered) {
                        if (r->isTNT) {
                            expload(r);
                            r->investigated = true;
                            continue;
                        }
                        else pq.push(r);
                        r->discovered = true;
                    }
                }
            }
            printSummary();
            }
        void expload(Tile *c) {
            priority_queue<Tile*, vector<Tile*>, TileComparator> clearedPQ;

            clearedPQ.push(c);
            while (!clearedPQ.empty()) {
                if (clearedPQ.top()->rubbleOrig == 0) clearedPQ.pop();
                c = clearedPQ.top();
                clearedPQ.pop();
                c->rubble = 0;
                c->discovered = true;
                pq.push(c);
                if (c->investigated) continue;
                if (c->isTNT) {
                    if (opt.verbose) cout << "TNT explosion at [" << c->row << "," << c->col << "]!\n";
                    clearedTiles.push_back(*c);
                    // printMedian(c);
                }
                else {
                    amountCleared += c->rubbleOrig;
                    numCleared++;
                    clearedTiles.push_back(*c);
                    if (opt.verbose) cout << "Cleared by TNT: " << c->rubbleOrig << " at " << "[" << c->row << "," << c->col << "]\n";
                    printMedian(c);
                    continue;
                }

                if (c->row > 0) {
                Tile *t = &(m.map2D[c->row-1][c->col]); // top
                    if (!t->discovered) {
                        t->discovered = true;
                        clearedPQ.push(t);
                    }
                }
                if (c->row < m.getSize() - 1) {
                    Tile *b = &(m.map2D[c->row+1][c->col]);  //  bottom
                    if (!b->discovered) {
                        b->discovered = true;
                        clearedPQ.push(b);
                    }
                }
                if (c->col > 0) {
                    Tile *l = &(m.map2D[c->row][c->col-1]);  // left
                    if (!l->discovered) {
                        l->discovered = true;
                        clearedPQ.push(l);
                    }
                }
                if (c->col < m.getSize() - 1) {
                    Tile *r = &(m.map2D[c->row][c->col+1]); // right
                    if (!r->discovered) {
                        r->discovered = true;
                        clearedPQ.push(r);
                    }
                }

            }   
        }

        void printSummary() {
            cout << "Cleared " << numCleared << " tiles containing " << amountCleared << " rubble and escaped.\n";
            if (opt.stats != numeric_limits<size_t>::max()) printStats();
        }
        void verbose(Tile *c) {
            if (!c->isTNT && c->rubble != 0) {
                numCleared++;
                clearedTiles.push_back(*c);
                if (opt.verbose) cout << "Cleared: " << pq.top()->rubble << " at [" << c->row << "," << c->col << "]\n";
                printMedian(c);
            }
        }
        void printMedian(Tile *c) {
            if (!opt.median || c->isTNT) return;
            push(c->rubbleOrig);
            if (opt.median) cout << "Median difficulty of clearing rubble is: " << fixed << setprecision(2) << getMedian() << "\n";
        }
        void printStats() {

            size_t size = clearedTiles.size();
            // First tiles cleared
            cout << "First tiles cleared:\n";
            size_t N = min(size, static_cast<size_t>(opt.stats));
            for (size_t i = 0; i < N; ++i) {
                printTileInfo(clearedTiles[i]);
            }

            // Last tiles cleared
            cout << "Last tiles cleared:\n";
            N = min(size, opt.stats);
            for (auto it = clearedTiles.rbegin(); it != clearedTiles.rbegin() + static_cast<long>(N); it++) {
                printTileInfo(*it);
            }

            // Easiest tiles cleared
            cout << "Easiest tiles cleared:\n";
            vector<Tile> sortedByRubble(clearedTiles);
            sort(sortedByRubble.begin(), sortedByRubble.end(), [](Tile a, Tile b) {
                if (a.rubbleOrig == b.rubbleOrig) {
                    if (a.col == b.col) return a.row < b.row;
                    return a.col < b.col;
                }
                return a.rubbleOrig < b.rubbleOrig;
            });
            N = min(size, static_cast<size_t>(opt.stats));
            for (size_t i = 0; i < N; ++i) {
                printTileInfo(sortedByRubble[i]);
            }  

            // Hardest tiles cleared
            cout << "Hardest tiles cleared:\n";
            N = min(size, static_cast<size_t>(opt.stats));
            for (auto it = sortedByRubble.rbegin(); it != sortedByRubble.rbegin() + static_cast<long>(N); it++) {
                printTileInfo(*it);
            }
            // printTileInfo(sortedByRubble[size - N - 1]);
        }
        void printTileInfo(const Tile t) {
            if (t.isTNT) {
                cout << "TNT at [" << t.row << "," << t.col << "]\n";
            }
            else cout << t.rubbleOrig << " at [" << t.row << "," << t.col << "]\n";
        }
        void push(int rubble) {
            if (maxHeap.empty()) {
                maxHeap.push(rubble);
                return;
            }
            if (rubble >= maxHeap.top()) {
                maxHeap.push(rubble);
                // cout << "PUSHED: " << rubble << " to maxHeap\n";
            }
            else {
                minHeap.push(rubble); //otherwise it is a lower number
                // cout << "PUSHED: " << rubble << " to minHeap\n";

            }
            // Keep both heaps balanced
            if (maxHeap.size() - minHeap.size() == 2) {
                minHeap.push(maxHeap.top());
                maxHeap.pop();
            } else if (minHeap.size() - maxHeap.size() == 2) {
                maxHeap.push(minHeap.top());
                minHeap.pop();
            }
        }
        double getMedian() const {
            if (maxHeap.empty()) return 0;
            if (minHeap.size() == maxHeap.size()) {
                return (minHeap.top() + maxHeap.top()) / 2.0;
            } else if (minHeap.size() > maxHeap.size()) {
                return minHeap.top();
            } else {
                return maxHeap.top();
            }
            return 0;
        }

        void printDiscoveredMatrix(vector<vector<Tile>> map2D, size_t size) {
            cout << "Discovered Matrix:" << endl;
            for (size_t i = 0; i < size; ++i) {
                for (size_t j = 0; j < size; ++j) {
                    cout << std::boolalpha << map2D[i][j].isTNT << " ";
                }
                cout << endl << endl;
            }
        }
        Mining(Options o, Map ma) :
            opt(o),
            m(ma)
        {} 

};

void getMode(int argc, char *argv[], Options &opt)
{
    // These are used with getopt_long()
    opterr = false; // Let us handle all error output for command line options
    int choice;
    int index = 0;
    option long_options[] = {
        // TODO: Fill in two lines, for the "mode" ('m') and
        // the "help" ('h') options.
        {"help", no_argument, nullptr, 'h'},
        {"stats", required_argument, nullptr, 's'},
        {"median", no_argument, nullptr, 'm'},
        {"verbose", no_argument, nullptr, 'v'},
        
    }; // long_options[]

    // TODO: Fill in the double quotes, to match the mode and help options.
    while ((choice = getopt_long(argc, argv, "hs:mv", long_options, &index)) != -1)
    {
        switch (choice)
        {
        case 'h':
            printHelp(argv);
            exit(0);
        case 's':
            // Process stats option with optarg as the argument
            // e.g., processStats(optarg);
            {
                int arg{stoi(optarg)};
                opt.stats = static_cast<size_t>(arg);
                break;
            }
        case 'm':
            // Process median option
            // e.g., processMedian();
            opt.median = true;
            break;
        case 'v':
            // Process verbose option
            // e.g., processVerbose();
            opt.verbose = true;
            break;
        default:
            // Handle invalid options
            // e.g., handleInvalidOption();
            cerr << "Invalid command line arg" << endl;
            exit(1);

        }
    }
} // getMode()

// This function is already done.
int main(int argc, char *argv[])
{
    Options opt;
    getMode(argc, argv, opt);
    Map m;
    Mining mine(opt, m);
    //m.readMap();
    mine.solve();

    return 0;

} // main()
