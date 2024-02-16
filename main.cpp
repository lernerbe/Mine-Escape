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
    int stats = 0;
};

struct Tile {
    bool discovered = false;
    bool investigated = false;
    int rubble;
    size_t row;
    size_t col;
    bool isTNT = false;
};

struct TileComparator {
    bool operator()(const Tile& a, const Tile& b) const {
        // Compare based on priority criteria:
        // 1. Smallest rubble value
        // 2. Column number
        // 3. Row number
        if (a.rubble != b.rubble) {
            return a.rubble > b.rubble; // Smallest rubble value has higher priority
        }
        if (a.col != b.col) {
            return a.col > b.col; // Smaller column number has higher priority
        }
        return a.row > b.row; // Smaller row number has higher priority
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
        priority_queue<Tile, vector<Tile>, TileComparator> pq;
        size_t numCleared = 0;
        int amountCleared = 0;
        Map m;

    public:
        // solve
        void solve() {
            m.readInput();
            m.printInput();
            // Get necessary information from Map object
            // size_t currRow = m.getStart().first;
            // size_t currCol = m.getStart().second;
            size_t size = m.getSize();
            Tile c = m.map2D[m.getStart().first][m.getStart().second];
            pq.push(c);
            c.investigated = true;
            verbose(c.row, c.col, c);


            // if (m.map2D[currRow][currCol].isTNT) {
            //     expload(m.map2D[currRow][currCol], pq);
            // }
            // currRow = pq.top().row;
            // currCol = pq.top().col;
            // amountCleared += pq.top().rubble;
            // ++numCleared;
            while (!pq.empty() && (c.row < size) && (c.col < size) && (c.row > 0) && (c.col > 0)) {
                
                amountCleared += pq.top().rubble;
                c = pq.top();
                numCleared++;
                
                if (c.investigated) continue;
                
                expload(c); // TODO: Remove pq from signature

                Tile &l = m.map2D[c.row-1][c.col]; // left
                Tile &r = m.map2D[c.row+1][c.col]; // right
                Tile &t = m.map2D[c.row][c.col-1]; // top
                Tile &b = m.map2D[c.row][c.col+1]; // bottom
                t.discovered = true;

                // push neighbors to PQ
                if (l.discovered == false) { // left
                    pq.push(l);
                    l.discovered = true;
                }
                if (r.discovered == false) { // right
                    pq.push(r);
                    r.discovered = true;
                }
                if (t.discovered == false) { // top
                    pq.push(t);
                    t.discovered = true;
                }
                if (b.discovered == false) { // bottom
                    pq.push(b);
                    b.discovered = true;
                }
                // for testing
                // cout << pq.top().rubble << '\n';

                pq.pop();
                verbose(c.row, c.col, c);
    
            }
            printSummary();
            //printDiscoveredMatrix(m.map2D, size);
            m.printInput();
        }

        bool expload(Tile &s) {
            if (!s.isTNT) return false;
            priority_queue<Tile, vector<Tile>, TileComparator> pq;
            s.rubble = 0; // Set the rubble of the neighbor TNT tile to zero
            // Explode the neighbors of the neighbor recursively
            cout << "TNT explosion at [" << s.row << "," << s.col << "]\n";
            size_t size = m.getSize();
            // size_t nRow = c.row;
            // size_t nCol = c.col;
            size_t exploadedCount = 0;
            int exploaded = 1;
            Tile c = s;
            pq.push(c);
            int count = 0;
            while(!pq.empty()) {
                count++;
                if (c.investigated) continue;
                c.investigated = true;
                c = pq.top();
                // cout << pq.top().row << pq.top().col << " THERE\n";
                pq.pop();
                // cout << "investigating: " << "[" << c.row << "," << c.col << "]\n";
                // c.investigated = true;
                // TODO: this is not working
                for (int i = -1; i <= 1; ++i) {
                    for (int j = -1; j <= 1; ++j) {
                        if ((i != 0 && j != 0) || (i == 0 && j == 0)) continue;
                        int nRow = static_cast<int>(c.row) + i;
                        int nCol = static_cast<int>(c.col) + j;
                        if (nRow >= 0 && nRow < static_cast<int>(size)  && nCol >= 0 && nCol < static_cast<int>(size)) { // Check bounds
                            Tile &neighbor = m.map2D[static_cast<size_t>(nRow)][static_cast<size_t>(nCol)];
                            if (neighbor.investigated) continue;
                            if (!neighbor.isTNT) { // If neighbor is TNT and not already exploded
                                pq.push(neighbor); // Push the neighbor TNT tile to the priority queue
                                neighbor.discovered = true;
                                neighbor.rubble = 0; // Set the rubble of the neighbor TNT tile to zero
                                ++exploadedCount;
                            }
                            if (neighbor.isTNT && !neighbor.discovered && !neighbor.investigated) {
                                cout << "TNT explosion at [" << neighbor.row << "," << neighbor.col << "]\n";
                                pq.push(neighbor);
                                neighbor.discovered = true;
                                exploaded++;

                            }
                        }
                    }
                }
                // cout << "Exploaded Count: " << exploadedCount << endl;
                // cout << "Exploaded: " << exploaded << endl;
                if (exploaded == count) {
                    for (int i = 0; i < exploaded; i++) {
                        pq.pop();
                    }
                    for (size_t z = 0; z < exploadedCount - static_cast<size_t>(exploaded); z++) {
                        cout << "Cleared by TNT: " << pq.top().rubble << " at " << "[" << pq.top().row << "," << pq.top().col << "]\n";
                        numCleared++;
                        pq.pop(); 
                    }
                }
            } 
            
            return true;
        }

        void printSummary() {
            cout << "Cleared " << numCleared << " tiles containing " << amountCleared << " rubble and escaped\n";
        }
        void verbose(const size_t &currRow, const size_t &currCol, Tile &c) {
            if (opt.verbose && !c.isTNT) {
                    cout << "Cleared: " << pq.top().rubble 
                    << " at [" << currRow << "," << currCol << "]\n";
                }
            // else if (opt.verbose && c.isTNT) {
            //     cout << "TNT explosion at [" << currRow << "," << currCol << "]\n";
            // }
        }
        // void median() {
        //     if (opt.median) {
        //         cout << "Median difficulty of clearing rubble is: ";
        //         // this is a tough one lol
        //     }
        // }
        
        void printDiscoveredMatrix(vector<vector<Tile>> map2D, size_t size) {
            cout << "Discovered Matrix:" << endl;
            for (size_t i = 0; i < size; ++i) {
                for (size_t j = 0; j < size; ++j) {
                    cout << (map2D[i][j].isTNT ? "true" : "false") << " ";
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
                opt.stats = arg;
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