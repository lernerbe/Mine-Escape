// Project Identifier: 19034C8F3B1196BF8E0C6E1C0F973D2FD550B88F
#include <getopt.h>
#include <string>
#include <limits>
#include <algorithm>
#include <deque>
#include <queue>
#include <vector>
#include <iostream>
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
        int seed;
        int maxRubble;
        size_t tnt = 0;
        Options opt;

    public:
        void readInput() {
            string junk;
            // read in mapFormat
            cin >> format;
            if (format != 'M' && format != 'V') {
                cerr << "Invalid input mode" << endl;
            }
            // read in size
            cin >> junk >> size;
            cin >> junk >> start.first >> start.second;
            // error check for column
            if (start.first > size) {
                cerr << "Invalid starting row" << endl;
            }
            // error check for row
            if (start.second > size) {
                cerr << "Invalid starting row" << endl;
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
                cin >> junk >> seed >> junk >> maxRubble >> junk >> tnt;
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

    public:
        void solve() {
            m.readInput();
            size_t size = m.getSize();
            Tile* c = &(m.map2D[m.getStart().first][m.getStart().second]);
            c->discovered = true;
            pq.push(c);

            while (!pq.empty() && (c->row < size) && (c->col < size) && (c->row > 0) && (c->col > 0)) {
                c = pq.top();
                verbose(c);
                pq.pop();
                if (c->investigated) continue;
                
                Tile copy = *c;
                clearedTiles.push_back(copy);

                if (!c->isTNT) amountCleared += c->rubble;
                else expload(c);
                c->rubble = 0;

                c->investigated = true;

                if (c->row > 0) {
                    Tile *l = &(m.map2D[c->row-1][c->col]); // left
                    if (!l->discovered) {
                        pq.push(l);
                    }
                }
                if (c->row < m.getSize() - 1) {
                    Tile *r = &(m.map2D[c->row+1][c->col]);  // right
                    if (!r->discovered) {
                        pq.push(r);
                    }
                }
                if (c->col > 0) {
                    Tile *t = &(m.map2D[c->row][c->col-1]);  // top
                    if (!t->discovered) {
                        pq.push(t);
                    }
                }
                if (c->col < m.getSize() - 1) {
                    Tile *b = &(m.map2D[c->row][c->col+1]); // bottom
                    if (!b->discovered) {
                        pq.push(b);
                    }
                }
            }
            printSummary();
            // m.printInput();
            }
            vector<Tile*> expload(Tile *c) {
                priority_queue<Tile*, vector<Tile*>, TileComparator> clearedPQ;
                vector<Tile*> exploaded;
                // numCleared--;

                clearedPQ.push(c);
                clearedTiles.pop_back();
                while (!clearedPQ.empty()) {
                    c = clearedPQ.top();
                    clearedPQ.pop();
                    int rubble = c->rubble;
                    c->rubble = 0;
                    pq.push(c);
                    c->discovered = true;
                    if (c->investigated) continue;
                    Tile copy = *c;
                    clearedTiles.push_back(copy);
                    if (c->isTNT && opt.verbose) {
                        cout << "TNT explosion at [" << c->row << "," << c->col << "]!\n";
                        
                    }
                    else {
                        amountCleared += rubble;
                        numCleared++;
                        if (!opt.verbose) continue;
                        cout << "Cleared by TNT: " << rubble << " at " << "[" << c->row << "," << c->col << "]\n";
                        continue;
                    }
                    exploaded.push_back(c);


                    if (c->row > 0) {
                    Tile *l = &(m.map2D[c->row-1][c->col]); // left
                        if (!l->discovered) {
                            clearedPQ.push(l);
                            l->discovered = true;
                        }
                    }
                    if (c->row < m.getSize() - 1) {
                        Tile *r = &(m.map2D[c->row+1][c->col]);  // right
                        if (!r->discovered) {
                            clearedPQ.push(r);
                            r->discovered = true;
                        }
                    }
                    if (c->col > 0) {
                        Tile *t = &(m.map2D[c->row][c->col-1]);  // top
                        if (!t->discovered) {
                            clearedPQ.push(t);
                            t->discovered = true;
                        }
                    }
                    if (c->col < m.getSize() - 1) {
                        Tile *b = &(m.map2D[c->row][c->col+1]); // bottom
                        if (!b->discovered) {
                            clearedPQ.push(b);
                            b->discovered = true;
                        }
                    }

                }   
                return exploaded;
            }


        void printSummary() {
                cout << "Cleared " << numCleared << " tiles containing " << amountCleared << " rubble and escaped.\n";
            if (opt.stats != numeric_limits<size_t>::max()) {
                printStats();
            }
        }
        void verbose(Tile *c) {
            if (opt.verbose && !c->isTNT && c->rubble != 0) {
                numCleared++;
                    cout << "Cleared: " << pq.top()->rubble
                    << " at [" << c->row << "," << c->col << "]\n";
                }
            }
        void printStats() {
            // First tiles cleared
            cout << "First tiles cleared:\n";
            size_t firstN = min(clearedTiles.size(), static_cast<size_t>(opt.stats));
            for (size_t i = 0; i < firstN; ++i) {
                printTileInfo(clearedTiles[i]);
            }

            // Last tiles cleared
            cout << "Last tiles cleared:\n";
            size_t lastN = min(clearedTiles.size(), static_cast<size_t>(opt.stats));
            for (size_t i = clearedTiles.size() - lastN; i < clearedTiles.size(); ++i) {
                printTileInfo(clearedTiles[i]);
            }

            // Easiest tiles cleared
            cout << "Easiest tiles cleared:\n";
            vector<Tile> sortedByRubble(clearedTiles);
            sort(sortedByRubble.begin(), sortedByRubble.end(), [](Tile a, Tile b) {
                return a.rubbleOrig < b.rubbleOrig;
            });
            size_t easiestN = min(clearedTiles.size(), static_cast<size_t>(opt.stats));
            for (size_t i = 0; i < easiestN; ++i) {
                printTileInfo(sortedByRubble[i]);
            }  

            // Hardest tiles cleared
            cout << "Hardest tiles cleared:\n";
            size_t hardestN = min(clearedTiles.size(), static_cast<size_t>(opt.stats)) - 1;
            for (size_t i = clearedTiles.size() - 1; i >= clearedTiles.size() - hardestN; --i) {
                printTileInfo(sortedByRubble[i]);
            }
        }
        void printTileInfo(const Tile t) {
            if (t.isTNT) {
                cout << "TNT at [" << t.row << "," << t.col << "]\n";
            }
            cout << t.rubbleOrig << " at [" << t.row << "," << t.col << "]\n";
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
            cerr << "Invalid input mode" << endl;
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