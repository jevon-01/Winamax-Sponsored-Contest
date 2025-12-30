#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>
#include <unordered_map>
#include <unordered_set>

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


int main()
{
   int width, height;
   cin >> width >> height; cin.ignore();
   vector<vector<string>> grid(height, vector<string>(width));
   vector<pair<pair<int, int>, int>> listBalls;
   vector<vector<pair<int, int>>> listAllMoves;
   unordered_map<pair<int, int>, unordered_set<pair<int, int>, PairHash, PairEqual>, PairHash, PairEqual> endingHole;
   for (int i = 0; i < height; i++) {
       string row;
       cin >> row; cin.ignore();
       for (int j = 0; j < row.size(); j++) {
           string elem = row.substr(j, 1);
           grid[i][j] = elem;
           if (elem == "H") {
               unordered_set<pair<int, int>, PairHash, PairEqual> balls;
               endingHole.insert({ pair<int, int>{i, j}, balls });
           }
           else if (elem != "." && elem != "H" && elem != "X") {
               try {
                   listBalls.push_back({ pair<int, int>{i, j} , stoi(elem) });
               }
               catch (const invalid_argument& e) {}
           }
       }
   }
   vector<vector<vector<pair<int, int>>>> listReducedBallMoves;
   unordered_set<pair<int, int>, PairHash, PairEqual> confirmedPos;
   for (auto ball : listBalls) {
       pair<int, int> init = ball.first;
       vector<pair<int, int>> listMoves = { init };
       vector<vector<pair<int, int>>> listBallMoves;
       for (const auto& v : listBallMoves)
           endingHole[v.back()].insert(init);
       findAllPath(grid, init, ball.second, listMoves, listBallMoves, listBalls);
       listReducedBallMoves.push_back(listBallMoves);
   }
   isRejected(listReducedBallMoves, confirmedPos, 0, endingHole, listBalls, true);
   unordered_set<pair<int, int>, PairHash, PairEqual> visitedNodes;
   bool foundSln = false;
   enumPossiblePaths(grid, listBalls, listReducedBallMoves, listAllMoves, foundSln, visitedNodes, endingHole);
}
