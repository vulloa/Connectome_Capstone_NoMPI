#include <iostream>
#include <vector>
#include <fstream>
#include <ctime>
#include "synapse.h"
#include "mpi.h"

/*
 * when (neuron A, neuron B, weight)
 * when weight is positive, A is on then B is on
 * when weight is negative, A is off when B is on
 *
 */

using namespace std;

// global variables

//  file names for linux implementation
//string connectome_file = "connectome.csv";
//string synaptic_file = "postsynaptic.csv";
string connectome_file = "edgelist.csv";
string synaptic_file = "synaptic.csv";

//  direct file paths for debugging
//string connectome_file = "/Users/vanessaulloa/ClionProjects/connectome/connectome.csv";
//string synaptic_file = "/Users/vanessaulloa/ClionProjects/connectome/postsynaptic.csv";
//string connectome_file = "/Users/vanessaulloa/ClionProjects/connectome/edgelist.csv";
//string synaptic_file = "/Users/vanessaulloa/ClionProjects/connectome/synaptic.csv";

/*
 * threshold - determines when neuron fires
 * counter - for display
 */
int threshold = 15;
int neuronFireCount,muscleFireCount = 0;
clock_t t1,t2,t3;
ofstream outputfile;

//  vectors to hold neuron data
vector<synapse> connectome_vector;
vector<synapse> postsynaptic_vector;

//   function prototypes
void read_connectome(vector<synapse> &);
void read_postsynaptic(vector<synapse> &);
void dendriteAccumulate(vector<synapse> &, vector<synapse> &,synapse);
void fireNeuron(vector<synapse> &, vector<synapse> &,synapse);
void runconnectome(vector<synapse> &, vector<synapse> &,synapse);
void testFiles(vector<synapse> &, vector<synapse> &);

///

int main(int argc, char** argv) {

    /*
        connectome_vector:
            full C Elegans Connectome
        postsynaptic_vector: 
            maintains accumulated values for each neuron and muscle.
    */

    //  variable for user input
    //  MPI variables
    MPI_Init(&argc, &argv);

    string neuron;
    int world_size,world_rank,name_len,token = 0;
    int source,dest,offset,chunksize = 0;
    char processor_name[MPI_MAX_PROCESSOR_NAME];

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Get_processor_name(processor_name, &name_len);

    /***** FOR MASTER NODE *****/

    if(world_rank == 0) {

        /***** FILL VECTORS *****/

        read_connectome(connectome_vector);
        read_postsynaptic(postsynaptic_vector);
        //testFiles(connectome_vector,postsynaptic_vector);

        /***** END FILL VECTORS *****/

        /***** BEGINNING OUTPUT *****/

        cout << "\n----------\n";
        cout << "The World Size is: " << world_size << endl;
        cout << "Initial World Rank is: " << world_rank << endl;
        cout << "connectome_vector size: " << connectome_vector.size() << endl;
        cout << "postsynaptic_vector size: " << postsynaptic_vector.size() << endl;
        cout << "----------\n";

        outputfile << "\n----------\n";
        outputfile << "The World Size is: " << world_size << endl;
        outputfile << "Initial World Rank is: " << world_rank << endl;
        outputfile << "connectome_vector size: " << connectome_vector.size() << endl;
        outputfile << "postsynaptic_vector size: " << postsynaptic_vector.size() << endl;
        outputfile << "----------\n";

        cout << "\n----------\n";
        cout << "Processor: " << processor_name << ", world_rank: " << world_rank << endl;
        cout << "----------\n";

        outputfile << "\n----------\n";
        outputfile << "Processor: " << processor_name << ", world_rank: " << world_rank << endl;
        outputfile << "----------\n";

        /***** END BEGINNING OUTPUT *****/

        /***** START USER INPUT *****/

        cout << "Please enter a neuron: ";
        cin >> neuron;

        /***** END USER INPUT *****/

        /***** OPEN FILE TO STORE SELECTED OUTPUT *****/

        //  get local time to append to file name for storage in output folder
        time_t t = time(NULL);
        char* charTime = ctime(&t);
        tm* localTime = localtime(&t);

        int Day    = localTime->tm_mday;
        int Month  = localTime->tm_mon + 1;
        int Year   = localTime->tm_year + 1900;
        int Hour   = localTime->tm_hour;
        int Min    = localTime->tm_min;
        int Sec    = localTime->tm_sec;

        string outputDate = to_string(Day) + to_string(Month) + to_string(Year) + "_" + to_string(Hour) + to_string(Min) + to_string(Sec);

        //outputfile.open("/Users/vanessaulloa/ClionProjects/connectome/output.txt");
        //outputfile.open("/Users/vanessaulloa/ClionProjects/connectome/output/"+ neuron + "_" + outputDate  + ".dat");
        //outputfile.open("output.txt");
        outputfile.open("output/" + neuron + "_" + outputDate  + ".dat");

        /***** END FILE DECLARATION *****/

        /***** ADDITIONAL VARIABLES *****/

        chunksize = connectome_vector.size() / world_size;
        t1 = clock();

        /***** SEND TO OTHER NODES *****/

        offset = chunksize;
        for(dest = 1; dest < world_size; dest++) {

            MPI_Send(&offset, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
            //MPI_Send(&connectome_vector[offset],chunksize,MPI_FLOAT,dest,tag2,MPI_COMM_WORLD);
            cout << "Sent [" << chunksize << "] elements to task [" << dest << "] offset = [" << offset << "]" << endl;

            offset += chunksize;
        }

        /***** END SEND TO OTHER NODES *****/

        /***** MASTER NODE WORK START *****/

        //  for master to do work
        offset = 0;

        cout << "offset [" << world_rank << "]: " << offset << endl;
        cout << "chunksize [" << world_rank << "]: " << chunksize << endl;
        outputfile << "offset [" << world_rank << "]: " << offset << endl;
        outputfile << "chunksize [" << world_rank << "]: " << chunksize << endl;

        for (int i = offset; i < (offset + chunksize); i++) {

            if (connectome_vector[i].get_neuronA() == neuron) {

                cout << "\n----------\n" << endl;
                cout << "Running Connectome with neuron: " << connectome_vector[i].get_neuronA() << ", on " << processor_name << endl;
                cout << "----------\n" << endl;

                outputfile << "\n----------\n" << endl;
                outputfile << "Running Connectome with neuron: " << connectome_vector[i].get_neuronA() << ", on " << processor_name << endl;
                outputfile << "----------\n" << endl;

                //  pass neuron from user input to runconnectome function
                runconnectome(connectome_vector, postsynaptic_vector, connectome_vector[i]);

            }
        }

        /***** END OF PROGRAM DATA *****/

        t2 = clock();
        //t3 = ((float)t2 - (float)t1);

        cout << "\n----------\n";
        cout << "Processor: " << processor_name << endl;
        cout << "Total Neurons Fired: " << neuronFireCount << endl;
        cout << "Total Muscles Fired: " << muscleFireCount << endl;
        //cout << "Start Time: " << 0 << endl;
        //cout << "End Time: " << (double)(t2/CLOCKS_PER_SEC) << endl;
        cout << "Total Run Time: " << (double)(t2/CLOCKS_PER_SEC) << " seconds" << endl;
        cout << "----------\n";

        outputfile << "\n----------\n";
        outputfile << "Processor: " << processor_name << endl;
        outputfile << "Total Neurons Fired: " << neuronFireCount << endl;
        outputfile << "Total Muscles Fired: " << muscleFireCount << endl;
        //outputfile << "Start Time: " << 0 << endl;
        //outputfile << "End Time: " << (double)(t2/CLOCKS_PER_SEC) << endl;
        outputfile << "Total Run Time: " << (double)(t2/CLOCKS_PER_SEC) << " seconds" << endl;
        outputfile << "----------\n";

        /***** END OF PROGRAM DATA END *****/

        /*
        //  receive from other nodes
        for(int i = 1; i < world_size; i++) {

            source = i;
            MPI_Recv(&offset, 1, MPI_INT, source, tag1, MPI_COMM_WORLD, &status);
            //MPI_Recv(&connectome_vector[offset],chunksize, MPI_FLOAT, source, tag2, MPI_COMM_WORLD, &status);

        }
         */

    }

    /***** MASTER NODE WORK END *****/

/***** END MASTER NODE *****/

/***** NON MASTER NODES *****/

    if(world_rank > 0) {

        /***** RECEIVE DATA FROM MASTER NODE *****/

        source = 0;
        MPI_Recv(&offset,1,MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        //MPI_Recv(&connectome_vector[offset],chunksize,MPI_FLOAT, source, tag2, MPI_COMM_WORLD, &status);

        cout << "offset [" << world_rank << "]: " << offset << endl;
        cout << "chunksize [" << world_rank << "]: " << chunksize << endl;
        outputfile << "offset [" << world_rank << "]: " << offset << endl;
        outputfile << "chunksize [" << world_rank << "]: " << chunksize << endl;

        /***** END RECEIVE DATA FROM MASTER NODE *****/

        /***** NON MASTER NODE WORK START *****/
        cout << "\n----------\n";
        cout << "Processor: " << processor_name << ", world_rank: " << world_rank << endl;
        cout << "----------\n";

        outputfile << "\n----------\n";
        outputfile << "Processor: " << processor_name << ", world_rank: " << world_rank << endl;
        outputfile << "----------\n";

        for (int i = offset; i < (offset + chunksize); i++) {

            if (connectome_vector[i].get_neuronA() == neuron) {

                cout << "\n----------" << endl;
                cout << "Running Connectome with neuron: " << connectome_vector[i].get_neuronA() << ", on " << processor_name << endl;
                cout << "----------\n" << endl;

                outputfile << "\n----------" << endl;
                outputfile << "Running Connectome with neuron: " << connectome_vector[i].get_neuronA() << ", on " << processor_name << endl;
                outputfile << "----------\n" << endl;

                //  pass neuron from user input to runconnectome function
                runconnectome(connectome_vector, postsynaptic_vector, connectome_vector[i]);

            }
        }

        /***** NON MASTER NODE WORK END *****/

        /***** END OF PROGRAM DATA *****/

        //t1 = 0;
        t2 = clock();
        //t3 = ((float)t2 - (float)t1);

        cout << "\n----------\n";
        cout << "Processor: " << processor_name << endl;
        cout << "Total Neurons Fired: " << neuronFireCount << endl;
        cout << "Total Muscles Fired: " << muscleFireCount << endl;
        //cout << "Start Time: " << 0 << endl;
        //cout << "End Time: " << (double)(t2/CLOCKS_PER_SEC) << endl;
        cout << "Total Run Time: " << (double)(t2/CLOCKS_PER_SEC) << " seconds" << endl;
        cout << "----------\n";

        outputfile << "\n----------\n";
        outputfile << "Processor: " << processor_name << endl;
        outputfile << "Total Neurons Fired: " << neuronFireCount << endl;
        outputfile << "Total Muscles Fired: " << muscleFireCount << endl;
        //outputfile << "Start Time: " << 0 << endl;
        //outputfile << "End Time: " << (double)(t2/CLOCKS_PER_SEC) << endl;
        outputfile << "Total Run Time: " << (double)(t3/CLOCKS_PER_SEC) << " seconds" << endl;
        outputfile << "----------\n";

        /***** END OF PROGRAM DATA END *****/

        /*
        //  send back to master task
        dest = 0;
        MPI_Send(&offset, 1, MPI_INT, dest, tag1, MPI_COMM_WORLD);
        //MPI_Send(&connectome_vector[offset], chunksize, MPI_FLOAT, 0, tag2, MPI_COMM_WORLD);

        //  MPI_Reduce statement needed????
        */

    }

/***** END NON MASTER NODES *****/

    //  close the filestream
    outputfile.close();

    //  close MPI
    MPI_Finalize();

    return 0;

}// end main

/*      FUNCTIONS       */

/*
 * testFiles()
 * tests that the vectors were filled with data from .csv
 * by displayed some of the values
 *
 */

void testFiles(vector<synapse> &connectome_vector, vector<synapse> &postsynaptic_vector)  {

    ///
    /*      testing for file reads and pushed values onto vectors   */
    ///

    //   to test that values were stored in the vector
    //   by checking the size and the first value connectome elements
    cout << "connectome vector size: " << connectome_vector.size() << endl;
    for(int i = 0; i < connectome_vector.size(); i++) {
        cout << "\t" << i << " : " << connectome_vector[i].get_neuronA() << endl;
        cout << "\t" << i << " : " << connectome_vector[i].get_neuronB() << endl;
        cout << "\t" << i << " : " << connectome_vector[i].get_weight() << endl;
        cout << endl;
    }



    //  to test the values stored in the vector
    //  by checking the size and values of the postsynaptic elements
    cout << "postsynaptic vector size: " << postsynaptic_vector.size() << endl;
    for(int i = 0; i < postsynaptic_vector.size(); i++) {
        cout << "\t" << i << " : " << postsynaptic_vector[i].get_neuronA() << endl;
        cout << "\t" << i << " : " << postsynaptic_vector[i].get_weight() << endl;
        cout << endl;
    }

    ///
    /*  end test    */
    ///

}

/*
 * read_connectome()
 *
 */
void read_connectome(vector<synapse> &connectome_vector) {

    // create file reading object and open the file
    /*  NEED WAY TO DYNAMICALLY REFERENCE file  */
    ifstream file;
    file.open(connectome_file);

    // substring variables
    string line;
    size_t pos1, pos2,length;
    string neuronA,neuronB,weight,temp;

    //  continue while file is open

    if(!file.is_open()) {

        cout << " Connectome.csv file could not be opened. " << endl;
        exit(0);

    }   else    {

        while (getline(file, line)) {

            /*
             * int pos1 = first position of ',' in value
             * string neuronA = from 0 to pos1 in value
             * string temp = pos1 to length
             * int pos2 = fiirst position of ',' in temp
             * string neuronB = from pos1 to pos2 in value
             * string weight = from pos2 to string_length in value
             *
             */

            length = line.length();
            pos1 = line.find(',') + 1;

            neuronA = line.substr(0,pos1 - 1);
            temp = line.substr(pos1,length);
            pos2 = temp.find(',');

            neuronB = temp.substr(0,pos2);
            weight = temp.substr(pos2 + 1,length);
            int w = atoi(weight.c_str());

            //  push a new synapse object onto the vector
            connectome_vector.push_back(synapse(neuronA,neuronB,w));

        }   //end while loop

        cout << "\nFile " << connectome_file << " successfully read." << endl;

    }// end if statement

    //  close the file
    file.close();

}// end function

/*
 * read_postsynaptic()
 *
 */
void read_postsynaptic(vector<synapse> &postsynaptic_vector)    {

    // create file reading object and open the file
    /*  NEED WAY TO DYNAMICALLY REFERENCE file  */
    ifstream file;
    file.open(synaptic_file);

    // substring variables
    string line,neuronA;
    size_t pos1;

    if(!file.is_open()) {

        cout << "file could not be opened. " << endl;
        exit(0);

    }   else    {

        while(getline(file,line)) {

            pos1 = line.find(',') + 1;
            neuronA = line.substr(0,pos1 - 1);

            //cout << "neuronA: " << neuronA << endl;

            //  push a new synapse object onto the vector
            postsynaptic_vector.push_back(synapse(neuronA, 0));

        }// end while

        cout << "\nFile " << synaptic_file << " successfully read." << endl;

    }//  end if statement

    file.close();

}// end function

/*
 * dendriteAccumulate()
 *
 */
void dendriteAccumulate(vector<synapse> &connectome_vector, vector<synapse> &postsynaptic_vector, synapse a)  {

    int x,y;

    for(x = 0 ; x < connectome_vector.size() ; x++) {

        if(connectome_vector[x].get_neuronA() == a.get_neuronA())    {

            //cout << "\nconnectome_vector[" << x << "].get_neuronA() == a.get_neuronA() --> " << connectome_vector[x].get_neuronA() << " = " << a.get_neuronA() << " " << (connectome_vector[x].get_neuronA() == a.get_neuronA()) << endl;
            //outputfile << "\nconnectome_vector[" << x << "].get_neuronA() == a.get_neuronA() --> " << connectome_vector[x].get_neuronA() << " = " << a.get_neuronA() << " " << (connectome_vector[x].get_neuronA() == a.get_neuronA()) << endl;

            for(y = 0; y < postsynaptic_vector.size() ; y++)    {

                if(postsynaptic_vector[y].get_neuronA() == connectome_vector[x].get_neuronB())   {
                    /*
                    cout << "\tpostsynaptic_vector[" << y << "].get_neuronA() == connectome_vector[" << x << "].get_neuronB() --> " << postsynaptic_vector[y].get_neuronA() << " = " << connectome_vector[x].get_neuronB() << " " << (postsynaptic_vector[y].get_neuronA() == connectome_vector[x].get_neuronB()) << endl;
                    cout << endl;
                    outputfile << "\tpostsynaptic_vector[" << y << "].get_neuronA() == connectome_vector[" << x << "].get_neuronB() --> " << postsynaptic_vector[y].get_neuronA() << " = " << connectome_vector[x].get_neuronB() << " " << (postsynaptic_vector[y].get_neuronA() == connectome_vector[x].get_neuronB()) << endl;
                    outputfile << endl;
                    */
                    //  POSTSYNAPTIC VECTOR is altered here.
                    postsynaptic_vector[y].set_weight(connectome_vector[x].get_weight());
                    /*
                    cout << "\t\tpostsynaptic vector altered at: ";
                    cout << " " << postsynaptic_vector[y].get_neuronA() << ", " << (postsynaptic_vector[y].get_weight() - connectome_vector[x].get_weight()) << "+" << connectome_vector[x].get_weight() << endl;
                    outputfile << "\t\tpostsynaptic vector altered at: ";
                    outputfile << " " << postsynaptic_vector[y].get_neuronA() << ", " << (postsynaptic_vector[y].get_weight() - connectome_vector[x].get_weight()) << "+" << connectome_vector[x].get_weight() << endl;
                    */
                }// end if

            }// end for

        }// end if

    }// end for

}// end dendriteAccumulate()

/*
 * fireNeuron()
 * when the threshold has been exceeded, fire the neurite
 * 1st we accumulate the postsynaptic weights
 * then we check everywhere the accumulator is > threshold
 *
 */
void fireNeuron(vector<synapse> &connectome_vector, vector<synapse> &postsynaptic_vector,synapse a)   {

    int y;
    dendriteAccumulate(connectome_vector,postsynaptic_vector,a);

    for(y = 0 ; y < postsynaptic_vector.size() ; y++ )   {

        //cout << "postsynaptic_vector[" << y << "]: " << postsynaptic_vector[y].get_neuronA() << " , " << postsynaptic_vector[y].get_weight() << endl;
        //outputfile << "postsynaptic_vector[" << y << "]: " << postsynaptic_vector[y].get_neuronA() << " , " << postsynaptic_vector[y].get_weight() << endl;

        if(abs(postsynaptic_vector[y].get_weight()) > threshold)  {

            //if(postsynaptic_vector[y].get_neuronA().substr(0,2) == "MV" || postsynaptic_vector[y].get_neuronA().substr(0,2) == "MD") {
            if(postsynaptic_vector[y].get_neuronA() == "PLMR" || postsynaptic_vector[y].get_neuronA() == "PLML") {

                muscleFireCount++;
                cout << "Fire Muscle " + postsynaptic_vector[y].get_neuronA() << postsynaptic_vector[y].get_weight() << endl;
                outputfile << "Fire Muscle " + postsynaptic_vector[y].get_neuronA() << postsynaptic_vector[y].get_weight() << endl;
                postsynaptic_vector[y].reset_weight();

                //cout << "\t\tpostsynaptic_vector[" << y << "]: reset " << endl;
                //outputfile << "\t\tpostsynaptic_vector[" << y << "]: reset " << endl;

                //cout << "After Fire Muscle: " << postsynaptic_vector[y].get_weight() << endl;

            } else {

                neuronFireCount++;
                cout << "Fire Neuron " + postsynaptic_vector[y].get_neuronA() << endl;
                outputfile << "Fire Neuron " + postsynaptic_vector[y].get_neuronA() << endl;
                dendriteAccumulate(connectome_vector,postsynaptic_vector,postsynaptic_vector[y]);
                postsynaptic_vector[y].reset_weight();

                //cout << "\t\tpostsynaptic_vector[" << y << "]: reset " << endl;
                //outputfile << "\t\tpostsynaptic_vector[" << y << "]: reset " << endl;

                //cout << "After Fire Neuron: " << postsynaptic_vector[y].get_weight() << endl;

            }// end if/else

        }// end if

    }// end for

}// end fireNeuron()

/*
 * runconnectome()
 *
 *
 */
void runconnectome(vector<synapse> &connectome_vector, vector<synapse> &postsynaptic_vector, synapse a)   {

    int y;

    dendriteAccumulate(connectome_vector,postsynaptic_vector,a);

    for(y = 0 ; y < postsynaptic_vector.size() ; y++)    {

        //cout << "y: " << y << " " << postsynaptic_vector[y].get_neuronA()  << " , " << postsynaptic_vector[y].get_weight() << endl;

        if(postsynaptic_vector[y].get_weight() > threshold)  {

            //cout << "y: " << y << " " << postsynaptic_vector[y].get_neuronA()  << " , " << postsynaptic_vector[y].get_weight() << endl;

            fireNeuron(connectome_vector,postsynaptic_vector,postsynaptic_vector[y]);
            postsynaptic_vector[y].reset_weight();

        }// end if

    }// end for

}// end runconnectome()