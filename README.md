# Ramseys_bane
Our project seeks to find the minimum number of guests that must be invited to an event so that at least 10 will know each other **or** at least 10 will not know each other. The solutions to this problem are known as Ramsey numbers. This project seeks to improve the known bounds on Ramsey(10,10), which is currently known as **798–23556**. From this, we know that the number of people that must be invited to an event so that at least 10 will know each other **or** at least 10 will not know each other is somewhere in that range.

More information on Ramsey Numbers can be found [here.](https://en.wikipedia.org/wiki/Ramsey%27s_theorem)

# Methods of Execution
In order to determine whether a number **m** of chosen nodes is not a Ramsey number for **R(10,10)**, we have to find a counter example which shows that with **m** nodes, it is not possible to form a clique which meets the requirements as described above. We use matrices of **m x m** to demonstrate these relationships and run those through `clique_count.c` to determine whether any cliques are present. If a chosen change to the relationship matrix produces a smaller clique count than a previous matrix held, that change will be considered a "good" change.

# Project Writeup
The full project writeup can be read [here](ramseys-bane.pdf)
