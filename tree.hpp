#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <map>

using namespace std;

enum Type
{
    chance,
    decision,
    terminal,
    none
};

class Node
{
public:
    string id;
    float reward = 0;
    float value = 0;
    float newvalue = 0;
    float initprob;
    Type type = none;
    vector<float> inorder_probs;
    vector<Node*> inorder_edges;
    map<string, float> probabilities;
    map<string, Node *> edges;
    pair<string, string> policy;

    Node(string i, float val = 0, Type t = none)
    {
        id = i;
        value = val;
        type = t;
    }

    ~Node() {}

    void print()
    {
        string printtype;
        switch (type)
        {
        case chance:
            printtype = "chance";
            break;
        case decision:
            printtype = "decision";
            break;
        case terminal:
            printtype = "terminal";
            break;
        case none:
            printtype = "none";
            break;
        }
        cout << "{ NODE: " << id << ", ";
        cout << "VAL: " << value << " REWARD: " << reward << " ";
        cout << "TYPE: " << printtype << " ";
        if (type != terminal)
        {
            cout << "PROB: [";
            for (pair<string, float> p : probabilities)
                cout << (p.first == "" ? "NULL" : p.first) << "=" << p.second << " ";

            cout << "] EDGES: [";
            for (pair<string, Node *> n : edges)
                cout << n.second->id << " ";

            cout << "INPROB: [";
            for (float p : inorder_probs)
                cout << p << " ";

            cout << "] INEDGES: [";
            for (Node * n : inorder_edges)
                cout << n->id << " ";
        }
        if (type == decision)
            cout << "] POLICY: " << policy.first << "->" << policy.second << " ";
        cout << "}" << endl;
    }

    float getProbabilityOfEdge(string e)
    {
        return probabilities.find(e)->second;
    }

    float getValueOfEdge(string e)
    {
        return edges.find(e)->second->value;
    }

    void addProbability(string s, float f)
    {
        probabilities.insert(make_pair(s, f));
    }
    void addEdge(Node *n)
    {
        edges.insert(make_pair(n->id, n));
    }

    void setValue(float v)
    {
        value = v;
    }

    void setType(Type t)
    {
        type = t;
    }

    bool isValid()
    {
        return type != none;
    }
};