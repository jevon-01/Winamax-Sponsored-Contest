#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include <chrono> //To test run time

using namespace std;

pair<int, int> operator+(const pair<int, int>& a, const pair<int, int>& b) {
    return pair<int, int>(a.first + b.first, a.second + b.second);
}

pair<int, int> operator-(const pair<int, int>& a, const pair<int, int>& b) {
    return pair<int, int>(a.first - b.first, a.second - b.second);
}

pair<int, int>& operator+=(pair<int, int>& a, const pair<int, int>& b) {
    a.first += b.first;
    a.second += b.second;
    return a;
}

pair<int, int> operator*(const pair<int, int>& a, const int& mul) {
    return pair<int, int>(a.first * mul, a.second * mul);
}

bool operator==(const pair<int, int>& a, const pair<int, int>& b) {
    return a.first == b.first && a.second == b.second;
}

bool operator!=(const pair<int, int>& a, const pair<int, int>& b) {
    return a.first != b.first || a.second != b.second;
}

ostream& operator<<(ostream& os, const pair<int, int>& a) {
    return os << a.first << " " << a.second;
}

struct PairHash {
    template<typename T, typename U>
    std::size_t operator()(const std::pair<T, U>& p) const {
        return std::hash<T>()(p.first) ^ std::hash<U>()(p.second);
    }
};

struct PairEqual {
    template<typename T, typename U>
    bool operator()(const std::pair<T, U>& lhs, const std::pair<T, U>& rhs) const {
        return lhs.first == rhs.first && lhs.second == rhs.second;
    }
};

static vector<pair<int, int>> directions = { {-1, 0}, {0, -1}, {0, 1}, {1, 0} };

static unordered_map<pair<int, int>, string, PairHash, PairEqual> dirToChar = { {{-1, 0}, "^"}, {{ 0, -1}, "<"}, {{0, 1}, ">"}, {{1, 0}, "v"} };

bool isOutOfBound(vector<vector<string>>& grid, pair<int, int>& pos) {
    return (pos.first < 0 || pos.first > grid.size() - 1) || (pos.second < 0 || pos.second > grid[0].size() - 1);
}

void findAllPath(vector<vector<string>>& grid, const pair<int, int>& pos, int dist, vector<pair<int, int>>& listMoves, vector<vector<pair<int, int>>>& listBallMoves, const vector<pair<pair<int, int>, int>>& listBalls) {
    pair<int, int> nextPos;
    bool validMove;
    if (dist <= 0) {
        if (grid[pos.first][pos.second] != "H")
            listMoves.pop_back();
        return;
    }
    auto size = listMoves.size();
    for (const auto& dir : directions) {
        nextPos = pos + (dir * dist);
        validMove = true;

        if (!isOutOfBound(grid, nextPos)) {
            for (auto oldMove : listMoves) {
                if (oldMove == nextPos) {
                    validMove = false;
                    break;
                }
            }
            for (auto otherBall : listBalls) {
                if (nextPos == otherBall.first) {
                    validMove = false;
                    break;
                }
            }
            if (validMove) {
                for (int x = 1; x <= dist; x++) {
                    auto vPos = pos + (dir * x);
                    if ((grid[vPos.first][vPos.second] == "H") && vPos != nextPos) {
                        validMove = false;
                        break;
                    }
                }
            }
            if (validMove && (grid[nextPos.first][nextPos.second] == "." || grid[nextPos.first][nextPos.second] == "H")) {
                for (int j = 1; j <= dist; j++)
                    listMoves.push_back(pos + (dir * j));
                if (grid[nextPos.first][nextPos.second] == "H") {
                    listBallMoves.push_back(listMoves);
                    if (listMoves.size() > dist)
                        listMoves.erase(listMoves.end() - dist, listMoves.end());
                }
                else
                    findAllPath(grid, nextPos, (dist - 1), listMoves, listBallMoves, listBalls);
            }
        }
        if (size < listMoves.size())
            listMoves.erase(listMoves.begin() + size, listMoves.end());
    }
}


void printListBallMoves(const vector<vector<pair<int, int>>>& listBallMoves) {
    int count = 0;
    for (auto& v : listBallMoves) {
        cout << "move #" << ++count << ": \n";
        for (const auto& move : v)
            cout << move << endl;
    }
}

bool isRejected(vector<vector<vector<pair<int, int>>>>& listReducedBallMoves, unordered_set<pair<int, int>, PairHash, PairEqual>& visitedNodes, const int ballIndex, unordered_map<pair<int, int>, unordered_set<pair<int, int>, PairHash, PairEqual>, PairHash, PairEqual>& endingHole, const vector<pair<pair<int, int>, int>>& listBalls, bool simplify = false) {
    auto listReducedCopy = listReducedBallMoves;
    auto visitedCopy = visitedNodes;
    auto endingHoleCopy = endingHole;
    int initialcount = listReducedCopy.size();
    int count = 0;
    bool setHoleChanged = false;
    while (count < initialcount || setHoleChanged) {
        setHoleChanged = false;
        initialcount = listReducedCopy.size();
        for (int i = ballIndex; i < listReducedCopy.size(); i++) {
            int initSize = listReducedCopy[i].size();
            auto& listBallMoves = listReducedCopy[i];
            bool uniqueHole = true;
            auto ballPos = listBallMoves[0].front();
            auto firstHolePos = listBallMoves[0].back();
            for (int i = 1; i < listBallMoves.size(); i++) {
                if (firstHolePos != listBallMoves[i].back()) {
                    uniqueHole = false;
                    break;
                }
            }
            if (uniqueHole) {
                endingHoleCopy[firstHolePos] = { {listBallMoves[0].front()} };
                for (auto& it : endingHoleCopy)
                    if (it.first != firstHolePos) {
                        it.second.erase(ballPos);
                    }
            }

            if (listBallMoves.size() == 1) {
                const auto& v = listBallMoves[0];
                for (const auto& move : v) {
                    visitedCopy.insert(move);
                }
                endingHoleCopy[firstHolePos] = { {listBallMoves[0].front()} };
            }
            if (listBallMoves.size() > 1) {
                vector<pair<int, int>> ballHolesErase;
                auto it = remove_if(listBallMoves.begin(), listBallMoves.end(), [&visitedCopy, &endingHoleCopy, &ballHolesErase](const vector<pair<int, int>>& moves) {
                    bool notValid = any_of(moves.begin(), moves.end(),
                    [&visitedCopy, &ballHolesErase](const pair<int, int>& move) {
                            return visitedCopy.find(move) != visitedCopy.end(); });
                bool cond = (endingHoleCopy[moves.back()].size() == 1) ? moves.front() != *endingHoleCopy[moves.back()].begin() : false;
                if ((notValid || cond) && !moves.empty())
                    ballHolesErase.push_back(moves.back());
                return notValid || cond;
                    });
                if (it != listBallMoves.end()) {
                    listBallMoves.erase(it, listBallMoves.end());
                    if (listBallMoves.size() == 0)
                        return true;
                }
                unordered_set<pair<int, int>, PairHash, PairEqual> currentHoles;
                for (const auto& v : listBallMoves)
                    currentHoles.insert(v.back());
                if (ballHolesErase.size()) {
                    for (const auto& hole : ballHolesErase)
                        if (currentHoles.find(hole) != currentHoles.end())
                            endingHole.at(hole).erase(ballPos);
                    setHoleChanged = true;
                }
            }
        }
        count = listReducedCopy.size();
    }
    if (simplify) {
        listReducedBallMoves = listReducedCopy;
        visitedNodes = visitedCopy;
        endingHole = endingHoleCopy;
    }
    return false;
}

void convertMovesToChar(vector<vector<string>>& grid, const vector<vector<pair<int, int>>>& listAllMoves, const int height, const int width) {
    for (auto& b : listAllMoves)
        for (int i = 0; i < b.size() - 1; i++) {
            auto dir = b[i + 1] - b[i];
            grid[b[i].first][b[i].second] = dirToChar[dir];
        }
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            if (grid[i][j] == "X" || grid[i][j] == "H")
                grid[i][j] = ".";
}

void printGrid(const vector<vector<string>>& grid, const int height, const int width) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++)
            cout << grid[i][j];
        cout << "\n";
    }

}

void printPathSolution(vector<vector<string>>& grid, vector<vector<pair<int, int>>>& listAllMoves) {
    int height = grid.size();
    int width = grid[0].size();
    convertMovesToChar(grid, listAllMoves, height, width);
    printGrid(grid, height, width);
    return;
}

void enumPossiblePaths(vector<vector<string>>& grid, const vector<pair<pair<int, int>, int>>& listBalls, vector<vector<vector<pair<int, int>>>>& listReducedBallMoves, vector<vector<pair<int, int>>>& listAllMoves, bool& foundSln, unordered_set<pair<int, int>, PairHash, PairEqual>& visitedNodes, unordered_map<pair<int, int>, unordered_set<pair<int, int>, PairHash, PairEqual>, PairHash, PairEqual>& endingHole, int ballIndex = 0) {

    if (ballIndex + 1 < listReducedBallMoves.size())
        if (isRejected(listReducedBallMoves, visitedNodes, ballIndex, endingHole, listBalls))
            return;

    if (listBalls.size() == listAllMoves.size()) {
        printPathSolution(grid, listAllMoves);
        foundSln = true;
        return;
    }

    for (int i = listAllMoves.size(); i < listBalls.size(); i++) {
        for (const auto& move : listReducedBallMoves[i]) {
            bool valid = true;
            for (const auto& m2 : move) {
                if (visitedNodes.find(m2) != visitedNodes.end()) {
                    valid = false;
                    break;
                }
            }
            if (valid) {
                for (const auto& m : move)
                    visitedNodes.insert(m);
                listAllMoves.push_back(move);
                enumPossiblePaths(grid, listBalls, listReducedBallMoves, listAllMoves, foundSln, visitedNodes, endingHole, i + 1);
                if (foundSln)
                    return;
                for (const auto& m : move)
                    visitedNodes.erase(m);
                listAllMoves.pop_back();
            }

        }
    }
}

void test() {
    //5 5 4..XX .H.H. ...H. .2..2 .....
    //6 6 3..H.2 .2..H. ..H..H  .X.2.X ...... 3..H..
    //Test 6 : 8 8 .......4 ....HH.2 ..5..... H....22X .3XH.HXX ..X3.H.X ..XH.... H2X.H..3
    //Test 9, 15, 19, 20 failed
    //Test 9 : 40 8 .XXX.5XX4H5............4H..3XXH.2.HX3... XX4.X..X......3.....HH.2X.....5.....4XX. X4..X3.X......H...5.....XXXXXXX2.HX2..H. X..XXXXX.....H3.H.X..22X3XXH.X2X...2HHXH .X.X.H.X........X3XH.HXX.XXXXX.H..HX..2. X.HX.X.X....HH....X3.H.X.....H..XXXX3... X..X.H.X.43......XXH....HXX3..H.X2.HX2.. .XHXXXXX..H3H...H2X.H..3X2..HXX3H.2XXXXH

    cout << "Enter width, height" << endl;
    cout << "Enter grid rows seperated" << endl;

    /*cout << "Test 6: \n"
         << "6 6 3..H.2 .2..H. ..H..H  .X.2.X ...... 3..H.." << endl;*/
    //cout << "8 8 .......4 ....HH.2 ..5..... H....22X .3XH.HXX ..X3.H.X ..XH.... H2X.H..3" << endl;
    
    cout << "\nTest 9:\n"
        << "40 8 .XXX.5XX4H5............4H..3XXH.2.HX3... "
        << "XX4.X..X......3.....HH.2X.....5.....4XX. X4..X3.X......H...5.....XXXXXXX2.HX2..H. "
        << "X..XXXXX.....H3.H.X..22X3XXH.X2X...2HHXH .X.X.H.X........X3XH.HXX.XXXXX.H..HX..2. "
        << "X.HX.X.X....HH....X3.H.X.....H..XXXX3... X..X.H.X.43......XXH....HXX3..H.X2.HX2.. .XHXXXXX..H3H...H2X.H..3X2..HXX3H.2XXXXH" << endl;
    
    cout << "\nTest 15:\n" // test 15 works but not fast enough
         << "10 10 .......... .......... H.H...3..H .H........ .X.244.2.X ....44...X ....3...H. ..X....H.H H.X....... ..X......." << endl;
    // .......... 
    // .......... 
    // H.H...3..H 
    // .H........ 
    // .X.244.2.X 
    // ....44...X 
    // ....3...H. 
    // ..X....H.H 
    // H.X....... 
    // ..X......."

    cout << "\nTest 19:\n"
        << "40 10 5.....X..3...H................HX.....4XH "
        << "......X....XXXXX..............XX..2..HXX "
        << "......4H........X..4H.H...3..H....4..... "
        << ".HH.........H5XX.....H................5. "
        << "X............XXXX....X.244.2.X..H.5..... "
        << "X.H..........XXXX.......44...X.........5 "
        << "..............XX4.......3...H.........3. "
        << "...3......3..X........X....H.H.......... "
        << ".......HH.....XXXXX.H.X.......XX....H.XX "
        << "3........5....H.H.....X.......HX......XH" << endl;

    cout << "\nTest 20:\n"
        << "50 10 ...X............XX.....H.........H............H... "
        << ".XXXXX........XXXXX..XXXXX.....XXXXX........XXXXX. "
        << "...XXXX.XXXXXXXXX.........X..4......X..44..X...... "
        << "..H5XX........XX5H....H5XX......H5XX........XX5X.. "
        << "...XXXX......XXXX......XXXX......XXXX......XXXXXXX "
        << "...XXXX......XXXX......XXXX......XXXX......XXXX..X "
        << "....XX4......4XX........XX4.......XX4......4XX...X "
        << "3..X............X..33..X....X.3..X............X..X "
        << "....XXXXX..XXXXX........XXXXX.....XXXXX..XXXXX.... "
        << "....H.H......H.H........H.H.......H.H......H.H...." << endl;

    cout << "Input: ";
}

void solution() {
    /*cout << "\nTest 6 solution\n\n"
        << "v<<<<<<<\n"
        << "v>>>..<<\n"
        << "v^>>>>>v\n"
        << ".^.v<<vv\n"
        << ".^..>.vv\n"
        << "v<<<^.<v\n"
        << "v...^<<<\n"
        << ".>>^.<<<" << endl;*/
   
    cout << "\n\nTest 9 solution \n\n"
        << "v<<<<<..v.>>>>>vv<<<<<<<.<<<...<>>..>>>v\n"
        << "v.>>>>v.v^<<<<<vv>>>..<<.v<<<<<^v<<<<..v\n"
        << "vvv<<<v.v>>>>>.vv^>>>>>v.v.....^v..>>v.<\n"
        << "vvv...v.v^>>>.vv.^.v<<vvvv..<.v.v^<<....\n"
        << "vvv.>.v.v^^>>vvv.^..>.vvvv..^.v.>>..^<<^\n"
        << "vv..^.v.v^^^..vvv<<<^.<vv>>>^.<^....>>>^\n"
        << "v>>>^.<.v^^^^<<vv...^<<<...>>>.^.>>..>>v\n"
        << ">>......>>.^.<<<.>>^.<<<.>>>...^.<<.....";

    cout << "\n\nTest 15 solution \n\n";
    cout << ".v<<<>>>v.\n" 
        << ".v..^^..v.\n" 
        << ".v.<^^v.>.\n" 
        << "^..^^^v...\n" 
        << "^..^^^vv..\n" 
        << "^<<<<vvv..\n" 
        << ".v<<<vv>..\n" 
        << ".v...v>.>.\n" 
        << ".<...v..^.\n"
        << ".....>>>^.\n" << endl;

    cout << "\n\nTest 20 solution \n\n";
    cout << "......................>.........>..............<..\n"
        << "......................^.........^..............^..\n"
        << "......................^<<<<<<<..^<<<<<<<>>>>>>>^..\n"
        << "...>>>>>v..v<<<<<......>>>>>>>>>.>>>>>v..v<<<<<...\n"
        << ">>^.....v..v.....^<<>>^...............v..v........\n"
        << "^.......v..v.......^^.................v..v........\n"
        << "^.v<<<<.v..v.>>>>v.^^.v<<<<.....v<<<<.v..v.>>>>v..\n"
        << "^.v..v<<<..>>>v..v.^^.v....v<<<.v..v<<<..>>>v..v..\n"
        << "..v..v........v..v....v....v....v..v........v..v..\n"
        << "..>>.>........<.<<....>>...<....>>.>........<.<<..\n" << endl;
}

int main()
{
    // "X" water go over it, "." nothing, "H" hole, int first shot len then --1 until 0 or in Hole
    // ">" right, "<" left, "V" down, "^" up
    
    int width;
    int height;
    test(); // To enter inputs
    cin >> width >> height; cin.ignore();

    //Initialise the grid
    vector<vector<string>> grid(height, vector<string>(width));
    vector<pair<pair<int, int>, int>> listBalls;
    vector<vector<pair<int, int>>> listAllMoves;
    unordered_map<pair<int, int>, unordered_set<pair<int, int>, PairHash, PairEqual>, PairHash, PairEqual> endingHole;

    using namespace std::chrono; //Test run time
    auto start = high_resolution_clock::now(); 


    for (int i = 0; i < height; i++) {
        string row;
        cin >> row; cin.ignore();
        //Fill the grid with the inputs
        for (int j = 0; j < row.size(); j++) {
            string elem = row.substr(j, 1);
            grid[i][j] = elem;
            if (elem == "H") {
                unordered_set<pair<int, int>, PairHash, PairEqual> balls;
                endingHole[{i, j}] = balls;
            }
            else if (elem != "." && elem != "H" && elem != "X") {
                try {
                    listBalls.push_back({ pair<int, int>{i, j} , stoi(elem) });
                }
                catch (const invalid_argument& e) {}
            }
        }      
    }
    
    //Test listBalls
    /*cout << "listBalls size: " << listBalls.size() << endl;
    for (auto it : listBalls)
        cout << it.first << endl;*/

    //Check test input
    // cout << width << " " << height << endl;
    // for (auto& v : grid) {
    //     for (const auto& move : v) {
    //         cout << move;
    //     }
    //     cout << endl;
    // }


    auto start4 = high_resolution_clock::now(); //Test run time

    vector<vector<vector<pair<int, int>>>> listReducedBallMoves;
    unordered_set<pair<int, int>, PairHash, PairEqual> confirmedPos;
    int count = 1;
    int counter = 0; //for test
    for (auto ball : listBalls) {
        pair<int, int> init = ball.first;
        vector<pair<int, int>> listMoves = { init };
        vector<vector<pair<int, int>>> listBallMoves;
        findAllPath(grid, init, ball.second, listMoves, listBallMoves, listBalls);

        counter += listBallMoves.size(); // for test
        for (const auto& v : listBallMoves)
            endingHole[v.back()].insert(init);

        listReducedBallMoves.push_back(listBallMoves);
        /*cout << "Test ListBallMoves reduced " << count++ << ": " << init << endl;
        printListBallMoves(listBallMoves);*/
    }
    auto stop4 = high_resolution_clock::now(); //Test run time
    auto duration4 = duration_cast<milliseconds>(stop4 - start4);
    std::cout << "Time taken by findAllPath for all balls: " << duration4.count() << " milliseconds" << std::endl;

    auto start2 = high_resolution_clock::now(); //Test run time
    //simplifyList(listReducedBallMoves, confirmedPos, endingHole, listBalls);
    isRejected(listReducedBallMoves, confirmedPos, 0, endingHole, listBalls, true);
    auto stop2 = high_resolution_clock::now();
    auto duration2 = duration_cast<milliseconds>(stop2 - start);
    std::cout << "Time taken by simplifyList: " << duration2.count() << " milliseconds" << std::endl;
    
    int counter2 = 0;
    for (const auto& v : listReducedBallMoves)
        for (const auto& m : v)
            counter2++;
    cout << "\nlistReducedBallMoves size = " << listReducedBallMoves.size() << "\nList size difference: " << " " << counter2 << " < " << counter << endl << endl;

    //test simplify

    //vector<vector<vector<pair<int, int>>>> listReducedBallMoves2 = { 
    //  {{pair<int,int>{0,0}, {0, 1}, {0, 2}}}, 
    //  {{pair<int,int>{1, 1}, {1, 2}, {0, 2}}, {pair<int,int>{1, 1}, {1, 2}, {1, 3}}, {pair<int,int>{1, 1}, {1, 2}, {2, 4}} },
    //  {{pair<int,int>{2, 2}, {0, 1}, {0, 2}}, {pair<int,int>{2, 2}, {2, 3}, {2, 4}}, {pair<int,int>{2, 2}, {5, 4}, {5, 5}} },
    //  {{pair<int,int>{4, 4}, {1, 2}, {5, 5}}, {pair<int,int>{4, 4}, {5, 6}, {5, 5}}, {pair<int,int>{4, 4}, {7, 7}, {5, 5}} }
    //};
    //int initSize = 0;
    //for (const auto& v : listReducedBallMoves2)
    //    for (const auto& m : v)
    //        initSize++;
    //unordered_set<pair<int, int>, PairHash, PairEqual> confirmedPos2;
    //unordered_map<pair<int, int>, pair<int, int>, PairHash, PairEqual> ballConfirmedHole;
    //unordered_map<pair<int, int>, unordered_set<pair<int, int>, PairHash, PairEqual>, PairHash, PairEqual> endingHole2 = {
    //    {pair<int, int>{0, 2}, {{0, 0}, {1, 1}}},
    //    {pair<int, int>{1, 3}, {{1, 1}}}, 
    //    {pair<int, int>{2, 4}, {{1, 1}, {2, 2}}},
    //    {pair<int, int>{5, 5}, {{2, 2}, {4, 4}}}
    //};
    //vector<pair<pair<int, int>, int>> listBalls2 = { {{0, 0}, 2}, {{1, 1}, 2}, {{2, 2}, 2} , {{4, 4}, 2} };
    //int counter3 = 0;// counted manually for test: 
    //simplifyList(listReducedBallMoves2, confirmedPos2, endingHole2, listBalls2, ballConfirmedHole);
    //for (const auto& v : listReducedBallMoves2)
    //    for (const auto& m : v)
    //        counter3++;
    ////solution should be:
    //// {0,0}, {0, 1}, {0, 2} -- {1, 1}, {1, 2}, {1, 3} -- {{2, 2}, {2, 3}, {2, 4}}, {{2, 2}, {5, 4}, {2, 4}} -- {{4, 4}, {5, 6}, {5, 5}}, {{4, 4}, {7, 7}, {5, 5}}
    ////listReducedBallMoves2 size should be = 5
    //cout << "\nlistReducedBallMoves size = " << listReducedBallMoves2.size() << "\nList size difference: final (5) = " << " " << counter3 << " < " << initSize << endl << endl;
    //for (const auto& v : listReducedBallMoves2)
    //    printListBallMoves(v);
    //cout << "\n\nconfirmedPos2:\n";
    //for (auto it : confirmedPos2)
    //    cout << it << endl;
    //cout << "\n\nendingHole2:\n";
    //for (auto it : endingHole2) {
    //    cout << "hole: " << it.first << endl;
    //    for (auto it2 : it.second)
    //        cout << "\tball: " << it2 << endl;
    //}
           
    ////Final test
    //cout << endl << "Final test" << endl;
    /*if (listReducedBallMoves.size() == listBalls.size()) {
        vector<vector<pair<int, int>>> listAllMoves2;
        for (const auto& v : listReducedBallMoves)
            for (const auto& m : v)
                listAllMoves2.push_back(m);
        cout << listAllMoves2.size();

        cout << " simplify grid test start ============================" << endl;
        printPathSolution(grid, listAllMoves2);
        cout << "simplify grid test end ============================" << endl;
    }*/
   
    //check input
    // cout << width << " " << height << endl;
    // for (auto row : grid) {
    //     for (auto col : row) 
    //         cout << col;
    //     cout << endl;
    // }
    //check answer
    //cout << ".v<<<>>>v.\n" << ".v..^^..v.\n"  << ".v.<^^v.>.\n" << "^..^^^v...\n" << "^..^^^vv..\n" << "^<<<<vvv..\n" << ".v<<<vv>..\n" << ".v...v>.>.\n" << ".<...v..^.\n" << ".....>>>^.\n" << endl;

    unordered_set<pair<int, int>, PairHash, PairEqual> visitedNodes;
    bool foundSln = false;
    //cout << "reduced list size before enumPOssiblePaths: " << listReducedBallMoves.size() << endl;
    auto start3 = high_resolution_clock::now();
    enumPossiblePaths(grid, listBalls, listReducedBallMoves, listAllMoves, foundSln, visitedNodes, endingHole);
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start3);
    std::cout << "Time taken by enumPossiblePaths: " << duration.count() << " milliseconds" << std::endl;

    solution();
    //cout << "\n\nlistAllmoves.size() = " << listAllMoves.size() << endl;
}