#ifndef NODE_H
#define NODE_H

const double MAX = std::numeric_limits<double>::infinity();

struct Node{
    double linkCost[8];
    int state; //0-initial, 1-active, 2-expanded
    double totalCost;
    Node *prevNode; //predecessor
    int column, row; //position of the node in the image
    double maxDeriv;

    Node (){
        prevNode = NULL;
        totalCost = MAX;
        state = 0;
    }

    void graph(int &off_x, int &off_y, int idx){
            switch (idx)
            {
                case 0:
                    off_x = 1;
                    off_y = 0;
                    break;
                case 1:
                    off_x = 1;
                    off_y = -1;
                    break;
                case 2:
                    off_x = 0;
                    off_y = -1;
                    break;
                case 3:
                    off_x = -1;
                    off_y = -1;
                    break;
                case 4:
                    off_x = -1;
                    off_y = 0;
                    break;
                case 5:
                    off_x = -1;
                    off_y = 1;
                    break;
                case 6:
                    off_x = 0;
                    off_y = 1;
                    break;
                case 7:
                    off_x = 1;
                    off_y = 1;
                    break;
                default:
                    break;
            }
        }

};

struct greaterNode
{
    bool operator() (Node *a, Node *b)
    {
        return a->totalCost > b->totalCost;
    }
};
#endif // NODE_H
