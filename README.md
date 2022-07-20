# Generic Markov Process Solver
- Using value iteration and greedy policy computation
- Based on Chapter 17 of AIMA Third Edition

# Running
- run 'make'
- to run, do './mdp {filename}'
- Flags:
    - df: Discount Factor
    - tol: Tolerance
    - iter: Iterations
    - min: Min/Max value boolean

- See example input files for schema:

Node={value}
Node : [Neighbor1, Neighbor2, Neighbor3]
Node % {probabilites}

Nodes with probabilities for each state are chance nodes, nodes with single probabilities are decision nodes.

# Comments

The value of an individual state is computed using the Bellman equation for a Markov property:

v(s) = r(s) + df * P * v

See AIMA Third Edition.