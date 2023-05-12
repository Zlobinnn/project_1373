#ifndef ASTAR_H
#define ASTAR_H
#include <iostream>
#include <queue>
#include <vector>
#include <stack>
#include<algorithm>
using namespace std;

typedef struct Node
{
    int x, y;
    int g;                   // cost from the starting point to the current point
    int h;                   // estimated cost of the best path from the current node to the target node
    int f;                   // estimated value
    Node* father;
    Node(int x, int y)
    {
        this->x = x;
        this->y = y;
        this->g = 0;
        this->h = 0;
        this->f = 0;
        this->father = NULL;
    }
    Node(int x, int y, Node* father)
    {
        this->x = x;
        this->y = y;
        this->g = 0;
        this->h = 0;
        this->f = 0;
        this->father = father;
    }
}Node;
class Astar{
public:
    Astar();
    ~Astar();
    void search(Node* startPos, Node* endPos);

    void checkPoit(int x, int y, Node* father, int g);

    void NextStep(Node* currentPoint);

    int isContains(vector<Node*>* Nodelist, int x, int y);

    void countGHF(Node* sNode, Node* eNode, int g);

    static bool compare(Node* n1, Node* n2);

    bool unWalk(int x, int y);

    void printPath(Node* current);

    void printMap();

    vector<Node*> openList;
    vector<Node*> closeList;
    Node *startPos;
    Node *endPos;

    static const int WeightW = 10;  // up/down/left/right
    static const int WeightWH = 14; // diagonal
    static const int row = 800;
    static const int col = 600;
};
#endif // ASTAR_H
