#include "tree.hpp"
#include <fstream>
#include <sstream>
#include <cmath>
#include <climits>

/*
———————————————————————————————————————————————————————————————————————————————————————————————————————————————————
GLOBALS
———————————————————————————————————————————————————————————————————————————————————————————————————————————————————
*/
float DISCOUNT_FACTOR = 1.0;
float TOLERANCE = 0.01;
bool MIN = false;
int ITERATIONS = 100;

string filename;
map<string, Node *> created;

/*
———————————————————————————————————————————————————————————————————————————————————————————————————————————————————
HELPERS
———————————————————————————————————————————————————————————————————————————————————————————————————————————————————
*/
string getNode(char &c, istringstream &iss)
{
    string node;
    while (c != '=' && c != ':' && c != '%')
    {
        node.push_back(c);
        iss >> skipws >> c;
    }
    return node;
}
string getOp(char &c, istringstream &iss)
{
    string op;
    op.push_back(c);
    iss >> skipws >> c;
    return op;
}
string getVal(char &c, istringstream &iss)
{
    string val;
    while (!iss.eof())
    {

        val.push_back(c);
        iss >> c;
    }
    return val;
}

bool isCreated(string c)
{
    return created.find(c) != created.end();
}

Node *createNode(string n, string op, string val)
{
    Node *node;
    bool wasCreated = false;
    if (isCreated(n))
        node = created.find(n)->second, wasCreated = true;
    else
        node = new Node(n), created.insert(make_pair(node->id, node));

    if (op == "=")
    {
        node->reward = stoi(val);
    }
    else if (op == ":")
    {
        int i = 0;
        while (i < val.size())
        {
            string s = "";
            if (val[i] == '[' || val[i] == ']' || val[i] == ' ')
            {
                i++;
                continue;
            }

            while (val[i] != ',' && val[i] != ']')
            {
                s.push_back(val[i]);
                i++;
            }

            Node *child;
            if (isCreated(s))
                child = created.find(s)->second, wasCreated = true;
            else
                child = new Node(s), created.insert(make_pair(child->id, child));

            node->inorder_edges.push_back(child);
            i++;
        }
    }
    else if (op == "%")
    {
        int i = 0;
        while (i < val.size())
        {
            string token = "";
            if (val[i] == '.')
            {
                token.push_back(val[i]);
                i++;
            }
            while (val[i] != '.')
            {
                token.push_back(val[i]);
                i++;
            }
            node->inorder_probs.push_back(stof(token));
            node->initprob = stof(token);
        }
    }

    return node;
}

void setType(Node *n)
{
    if (n->inorder_edges.size() == 1 && n->inorder_probs.size() == 0)
    {
        n->setType(chance);
        n->value = 0;
        for (int i = 0; i < n->inorder_edges.size(); i++)
        {
            n->addEdge(n->inorder_edges[i]);
            n->addProbability(n->inorder_edges[i]->id, 1.0);
        }
        return;
    }
    if (n->inorder_edges.size() > 0 && n->inorder_probs.size() == 0)
    {
        n->addProbability("", 1.0);
        n->initprob = 1.0;
        for (int i = 0; i < n->inorder_edges.size(); i++)
        {
            n->addEdge(n->inorder_edges[i]);
        }
        n->setType(decision);
        return;
    }
    if (n->inorder_edges.size() == 0)
    {
        n->setType(terminal);
        if (n->probabilities.size() > 0)
            cout << "Terminal node " << n->id << " contains probability entry." << endl, exit(1);
        return;
    }
    if (n->inorder_edges.size() == n->inorder_probs.size())
    {
        n->setType(chance);
        n->value = 0;
        for (int i = 0; i < n->inorder_edges.size(); i++)
        {
            n->addEdge(n->inorder_edges[i]);
            n->addProbability(n->inorder_edges[i]->id, n->inorder_probs[i]);
        }
        return;
    }
    if (n->inorder_edges.size() > 0 && n->inorder_probs.size() == 1)
    {
        n->setType(decision);
        for (int i = 0; i < n->inorder_edges.size(); i++)
        {
            n->addEdge(n->inorder_edges[i]);
        }
        n->addProbability("", n->inorder_probs[0]);
        return;
    }
}

void parse(string filename)
{
    int index = 0;
    ifstream in(filename);
    if (!in.is_open())
    {
        cout << "File could not be opened or command format is incorrect. Check command and try again." << endl;
        exit(1);
    }
    string line, node, op, val;
    char c;
    while (getline(in, line))
    {
        istringstream iss(line);
        iss >> skipws >> c;
        if (iss.eof())
            continue;
        if (c == '#')
            continue;

        node = getNode(c, iss);
        op = getOp(c, iss);
        val = getVal(c, iss);

        Node *n = createNode(node, op, val);
    }
}

void assignInitValues()
{
    for (pair<string, Node *> p : created)
    {
        if (p.second->type == decision)
        {
            p.second->value = 0;
            // arbitrary policy
            float prob = p.second->probabilities.find("")->second;
            float remaining = 1.0 - p.second->probabilities.find("")->second;
            float todistrib = remaining / (p.second->edges.size() - 1);
            p.second->probabilities.erase("");

            bool first = true;
            for (pair<string, Node *> e : p.second->edges)
            {
                if (first)
                {
                    p.second->probabilities.insert(make_pair(e.first, prob));
                    p.second->policy = make_pair(p.second->id, p.second->edges.find(e.first)->second->id);
                    first = false;
                    continue;
                }
                p.second->addProbability(p.second->edges.find(e.first)->second->id, todistrib);
            }
        }
    }
}

map<string, string> extractPolicy(map<string, Node *> &V)
{
    map<string, string> policy;
    for (pair<string, Node *> p : V)
    {
        if (p.second->type == decision)
            policy.insert(p.second->policy);
    }
    return policy;
}

void printResults()
{
    for (auto n : created)
    {
        if (n.second->type == decision)
            cout << n.second->policy.first << " -> " << n.second->policy.second << endl;
    }
    for (auto n : created)
    {
        cout << n.second->id << "=";
        printf("%.3f", n.second->value);
        cout << " ";
    }
    cout << endl;
}

bool isFloat(string s)
{
    istringstream iss(s);
    float f;
    iss >> noskipws >> f;
    return iss.eof() && !iss.fail();
}

bool isInt(const string &s)
{
    string::const_iterator it = s.begin();
    while (it != s.end() && isdigit(*it))
        ++it;
    return !s.empty() && it == s.end();
}

bool checkTolerance(map<string, Node *> &V)
{
    for (pair<string, Node *> n : V)
    {
        float change = n.second->newvalue - n.second->value;
        if (abs(change) > TOLERANCE)
        {
            return false;
        }
    }
    return true;
}

void updateValues(map<string, Node *> &V)
{
    for (pair<string, Node *> n : V)
    {
        n.second->value = n.second->newvalue;
    }
}

void updateProbabilities(map<string, Node *> &V)
{
    for (pair<string, Node *> n : V)
    {
        if (n.second->type == decision)
        {
            float todistrib = (1 - n.second->initprob) / (n.second->probabilities.size() - 1);

            for (pair<string, float> p : n.second->probabilities)
            {
                if (p.first == n.second->policy.second)
                    n.second->probabilities.find(p.first)->second = n.second->initprob;
                else
                    n.second->probabilities.find(p.first)->second = todistrib;
            }
        }
    }
}

bool policiesEqual(map<string, string> &pi, map<string, string> &newpi)
{
    map<string, string>::iterator piIT = pi.begin();
    map<string, string>::iterator newpiIT;
    for (newpiIT = newpi.begin(); newpiIT != newpi.end(); newpiIT++)
    {
        if (newpiIT->second != piIT->second)
        {
            return false;
        }
        piIT++;
    }
    return true;
}

void allZeros(map<string, Node *> &V)
{
    for (pair<string, Node *> n : V)
    {
        n.second->value = 0;
        n.second->newvalue = 0;
    }
}

/*
———————————————————————————————————————————————————————————————————————————————————————————————————————————————————
ALGORITHM
———————————————————————————————————————————————————————————————————————————————————————————————————————————————————
*/
void ValueIteration(map<string, Node *> &V)
{
    allZeros(V);
    for (int i = 0; i < ITERATIONS; i++)
    {
        // compute V' w/ Bellman
        for (pair<string, Node *> n : V)
        {
            float reward = n.second->reward;
            float p = 0;
            for (pair<string, Node *> e : n.second->edges)
            {
                p += n.second->getProbabilityOfEdge(e.first) * n.second->getValueOfEdge(e.first);
            };
            n.second->newvalue = reward + DISCOUNT_FACTOR * p;
        }
        // if V' within tolerance of V, return
        if (checkTolerance(V))
        {
            return;
        }
        // V = V'
        updateValues(V);
    }
    // return once reached max iterations
    return;
}

void GreedyPolicyComputation(map<string, Node *> &V)
{
    for (pair<string, Node *> n : V)
    {
        if (n.second->type == decision)
        {
            if (!MIN)
            {
                string maxNode = "";
                float maxReward = INT_MIN;
                for (pair<string, Node *> e : n.second->edges)
                {
                    if (e.second->value > maxReward)
                    {
                        maxReward = e.second->value;
                        maxNode = e.second->id;
                    }
                }
                n.second->policy = make_pair(n.second->id, maxNode);
            }
            else if (MIN)
            {
                string minNode = "";
                float minReward = INT_MAX;
                for (pair<string, Node *> e : n.second->edges)
                {
                    if (e.second->value < minReward)
                    {
                        minReward = e.second->value;
                        minNode = e.second->id;
                    }
                }
                n.second->policy = make_pair(n.second->id, minNode);
            }
        }
    }
}

void solveMDP(map<string, Node *> V)
{
    map<string, string> pi = extractPolicy(V);
    while (true)
    {
        // V = ValueIteration
        ValueIteration(V);
        // compute new policy
        GreedyPolicyComputation(V);

        // update probabilities/policy
        updateProbabilities(V);

        map<string, string> newpi = extractPolicy(V);
        // check equality of policies
        if (policiesEqual(pi, newpi))
        {
            return;
        }
        pi = newpi;
    }
}

/*
———————————————————————————————————————————————————————————————————————————————————————————————————————————————————
MAIN
———————————————————————————————————————————————————————————————————————————————————————————————————————————————————
*/
int main(int argc, char *argv[])
{
    // parse args
    int i = 1;

    while (i < argc)
    {
        string arg = argv[i];
        if (arg == "-df")
        {
            if (i + 1 < argc)
            {
                i++;
                if (!isFloat(argv[i]))
                    cout << "-df requires a float argument." << endl, exit(0);
                DISCOUNT_FACTOR = atof(argv[i]);
            }
            else
            {
                cout << "-df requires a float argument." << endl;
                exit(0);
            }
        }
        else if (arg == "-tol")
        {
            if (i + 1 < argc)
            {
                i++;
                if (!isFloat(argv[i]))
                    cout << "-tol requires a float argument." << endl, exit(0);
                TOLERANCE = atof(argv[i]);
            }
            else
            {
                cout << "-tol requires a float argument." << endl;
                exit(0);
            }
        }
        else if (arg == "-iter")
        {
            if (i + 1 < argc)
            {
                i++;
                if (!isInt(argv[i]))
                    cout << "-iter requires an integer argument." << endl, exit(0);
                ITERATIONS = atoi(argv[i]);
            }
            else
            {
                cout << "-iter requires an integer argument." << endl;
                exit(0);
            }
        }
        else if (arg == "-min")
        {
            MIN = true;
        }
        else
        {
            filename = argv[i];
        }
        i++;
    }

    if (filename == "")
    {
        cout << "No file specified." << endl;
        exit(0);
    }

    parse(filename);

    // populate maps

    for (pair<string, Node *> n : created)
    {
        setType(n.second);
    }

    assignInitValues();

    solveMDP(created);

    printResults();
}