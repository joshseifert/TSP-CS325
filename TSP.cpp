/*************************EXECUTION PARAMETERS***********************************/
/*The following #define can be commented out if you do not wish functionality.  */
#define TIMED_TEST  //Time limits on algorithm execution
/********************************************************************************/

#include <iostream>
#include <string>
#include <chrono>
#include <climits>
#include <cmath>
#include <fstream>
#include <vector>

#define LIMIT 300  //# of seconds algorithm can continue optimizing until termination.
#define NN_CHECK 35  //Only do a NN search of 'NN_CHECK' vertices before starting optimizing.

using std::cout;
using std::endl;
using std::string;
using std::ofstream;
using std::ifstream;
using std::vector;
using std::flush;

typedef std::chrono::high_resolution_clock hr_clock;

void processInput(int argc, char *argv[], int **&distanceTable, int &size);
bool initialPath(int **dist, int size, int *&best, hr_clock::time_point start);
void optimizePath(int **dist, int size, int *&best, hr_clock::time_point start);
void saveResult(char *argv[], int *best, int size);

int main(int argc, char *argv[])
{
   hr_clock::time_point start;
   start = hr_clock::now();  //start timer.
   bool timed_out = false;
      
   int** distanceTable = NULL;  //2D array to house distances
   int* bestSolution = NULL;  //Array to house "best" result.
   int size;  //Track # of 'cities' in problem set.
   
   processInput(argc, argv, distanceTable, size);  //Populates size and 2D table of distances.
   timed_out = initialPath(distanceTable, size, bestSolution, start);  //Calculates path.
   if (!timed_out) { optimizePath(distanceTable, size, bestSolution, start); } //Optimizes while time remains.
   saveResult(argv, bestSolution, size);  //Writes output to file.

	return 0;
}


/*Checks for and processes input file.  Returns via reference a table of the distances from each point (2D Matrix),
 *and the # of verticies in table (size)*/
void processInput(int argc, char *argv[], int **&distanceTable, int &size){
   if (argc != 2)
   {
      cout << "\n\nProgram requires input file as command line entry.  Usage:\n"
         << "path://<program> <testfile>\n\n";
      exit(1);
   }

   ifstream inputStream;
   inputStream.open(argv[1]);

   vector<vector<int> > allCities;

   if (inputStream.is_open())
   {
      int temp = 0, count = 0;
      vector<int> oneCity;
      
      cout << "\n\nReading data from file " << argv[1] << " ... " << flush;
      
      while (inputStream >> temp){
         count++;
         oneCity.push_back(temp);
         if (count == 3) {  //If oneCity is complete
            count = 0;
            allCities.push_back(oneCity);
            oneCity.clear();
         }
      }

      cout << "Done.\n";
      inputStream.close();
   }
   else
   {
      cout << "\n\nError: Unable to open input file\n\n";
      exit(1);
   }

   //Declares dynamic array for distances (currently deallocated at program term).
   cout << "Calculating distances between cities ... " << flush;
   size = (int)allCities.size();
   distanceTable = new int* [size];
   for (int i = 0; i < size; i++) { distanceTable[i] = new int [size]; }

   //Only calculates for half the table, mirrors over symmetrical diagonal.
   for (int i = 0; i < size; i++)
   {
      for (int j = i; j < size; j++)
      {
         distanceTable[i][j] = (int)round(sqrt(((allCities[i][1] - allCities[j][1])*
                                                (allCities[i][1] - allCities[j][1])) + 
                                               ((allCities[i][2] - allCities[j][2])*
                                                (allCities[i][2] - allCities[j][2]))));
         distanceTable[j][i] = distanceTable[i][j];
      }
   }
   cout << "Done.\n";
}


/*Checks for initial path options using a nearest neighbor approach.  Will return the best found
 *path via the 'best' parameter.  Will stop early if time limit reached, and return true if so.*/
bool initialPath(int **dist, int size, int *&best, hr_clock::time_point start){
   bool *visited = new bool [size];  //To track which vertices we've already visited in a given path.
   bool timed_out = false;  //If time limit is reached.
   int next, min;  //To track our likely next vertex to add.
   int jump;  //Will hold how many vertices to skip between trail starting locations.
   best = new int[size+1];  //Stores absolute best path found to date.  (Returns by reference)
   best[0] = INT_MAX;  //Initialized to infinity.  (Returns by reference)
   int *temp = new int [size+1];  //To track recent attempt to better tour.

   cout << "Defining Initial Path ... " << flush;

   #ifdef TIMED_TEST
      hr_clock::time_point finish;  //Start and finish times.
      std::chrono::duration<long double> elapsed;  //Variable to hold diff in start & finish.
   #endif

   //Defines how many vertices should be evaluated, for small city sets all will be checked.
   //For larger city sets (which take longer per vertex) only every 'NN_CHECK' will be checked.
   if (size < 250) { jump = 1; }
   else { jump = size / NN_CHECK; }
   //Currently checks every 'jump'th vertex for nearest neighbor cycle.
   for (int i = 0; i < size && !timed_out; i += jump){
      //Resets tracking variables for each new starting vertex.
      for (int j = 0; j < size; j++) { visited[j] = 0; }
      visited[i] = 1;
      temp[0] = 0;
      temp[1] = i;

      //Add vertices until all have been visited.
      for (int j = 2; j < size + 1 && !timed_out; j++){
         next = -1;
         min = INT_MAX;
         //Find the nearest neighbor
         for (int k = 0; k < size; k++){
            if (!visited[k] && dist[temp[j-1]][k] < min) {
               next = k;
               min = dist[temp[j-1]][k];
            }
         }
         //Add nearest neighbor
         temp[j] = next;
         temp[0] += min;
         visited[next] = 1;
         
         //Checks if 'LIMIT' seconds have passed and exits if so, best holds ideal so far.
         #ifdef TIMED_TEST
            finish = hr_clock::now();
            elapsed = (std::chrono::duration_cast<std::chrono::microseconds> (finish - start));
            if (elapsed.count() > LIMIT) { 
               cout << "Time Limit Reached ("  << LIMIT << " seconds).\n";
               timed_out = true; }
         #endif
      }
      
      if (!timed_out) {
         //Add distance from end of path back home.
         temp[0] += dist[next][i];

         //If tour found is best tour so far, save it.
         if (temp[0] < best[0]){
            for (int j = 0; j < size + 1; j++) { best[j] = temp[j]; }
         }
      }
   }

   if (!timed_out) { cout << "Done.\n"; }

   delete [] visited;
   delete [] temp;

   return timed_out;
}

/*Writes final result to file for evaluation.*/
void saveResult(char *argv[], int *best, int size){
   string fileName = argv[1];
   
   cout << "\nProgram has evaluated " << fileName << " for pseudo optimal TSP Solution\n";   
   
   fileName.append(".tour");
   ofstream outputStream;
   outputStream.open(fileName);

   //Prints total distance, followed by list of vertices in order.
   if (outputStream.is_open()){
      for (int i = 0; i < size + 1; i++){ outputStream << best[i] << endl; }
      outputStream.close();
   }
   else{
      cout << "Unable to write to " << fileName << endl << endl;
   }

   cout << "Results written to " << fileName << ", review file for results.\n\n";
}

/*If time allows will run a 2-opt on the best NN path found.  Will continue to run until
 *either no optimizations are found, or time runs out*/
void optimizePath(int **dist, int size, int *&best, hr_clock::time_point start){
   int improve = 0, check = 0;
   int swapi, swapj, spin;
   int *temp = NULL;
   bool imp_found = true;
   
   #ifdef TIMED_TEST
      hr_clock::time_point finish;  //Start and finish times.
      std::chrono::duration<long double> elapsed;  //Variable to hold diff in start & finish.
   #endif
   
   cout << "Optimizing Locally ... " << flush;
   
   while (imp_found){
      imp_found = false;  //Start with assumption no improvement found.
      improve = 0; //Reset best improvement possible.
      for (int i = 0; i < (size - 2); i++) {  //For every vertex in path (allowing for later vertex to swap with)
         for (int j = i + 2; j < size - 1; j++) {  //Check every downstream vertex.
            //Looks for crossed paths.  (i.e. A-D-C-B-E should be A-B-C-D-E...)
            check = ((dist[best[i + 1]][best[i + 2]] + dist[best[j + 1]][best[j + 2]]) -
                     (dist[best[i + 1]][best[j + 1]] + dist[best[i + 2]][best[j + 2]]));
            if (check > improve) {  //Capture best possible swap with index i.
               improve = check;
               swapi = i+1;
               swapj = j+1;
               imp_found = true;  //And tell while to run again.
            }     
         }
      }

      //If a positive swap is found make swap.
      if (imp_found) {
         spin = swapj - swapi;  //Determine length of subarray (ie A- 'D-C-B' - E)
         temp = new int[spin];
         for (int i = 0; i < spin; i++){  //Save subarray in reverse order (B-C-D)
            temp[i] = best[swapj];
            swapj--;
         }
         for (int i = 0; i < spin; i++){  //Reinsert reversed subarray into best. (A- 'B-C-D' -E
            best[swapi + 1] = temp[i];
            swapi++;
         }
         best[0] -= improve;  //Reduce overall distance by amount swap improves path.
         delete[] temp;
      }
      
      #ifdef TIMED_TEST
         finish = hr_clock::now();
         elapsed = (std::chrono::duration_cast<std::chrono::microseconds> (finish - start));
         if (elapsed.count() > LIMIT) { 
            cout << "Time Limit Reached (" << LIMIT << " seconds).\n";
            return; }
      #endif
   }
   
   cout << "Done.\n\n";
   
   #ifdef TIMED_TEST
     finish = hr_clock::now();
     elapsed = (std::chrono::duration_cast<std::chrono::microseconds> (finish - start));
     cout << "Calculations completed in " << elapsed.count() << " seconds.\n";
   #endif
}


