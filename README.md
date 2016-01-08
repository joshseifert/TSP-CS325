# TSP-CS325
Travelling Salesperson Algorithm

This program was written as part of a group project for a class on algorithms, to optimize the NP-Complete Travelling Salesperson problem. Data was given in text files with the following format, all fields integers:

[City Number] [X Coordinate] [Y Coordinate]

We were given the freedom to choose any language and algorithm(s) we wanted, with a class-wide competition to see who could produce the best possible results within a 5-minute computation time.
This particular implementation uses a greedy nearest-neighbor algorithm, along with 2-opt algorithm, to optimize the route. We chose to write in C++, to optimize speed. Of ~20 groups in the class, our program produced the best results in 4 of 7 provided test data sets. It outputs the results (as city numbers) in a new file - the same name as the input file, with ".tour" appended, per instructor specifications.
