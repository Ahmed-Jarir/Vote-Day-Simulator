/******************************************************************************/

/***********/
/* Imports */
/***********/

/* STD */
#include <iostream>

using namespace std;

/* Custom Imports */

#include "custom/sleep.cpp"
#include "custom/struct.cpp"
/* #include "custom/voter.cpp" */
#include "custom/station.cpp"

/* Other Imports */

#include <argparse/argparse.hpp>
#include <random>
#include <chrono>
#include <map>

/******************************************************************************/

/*************/
/* Constants */
/*************/

/* #define NORMAL   3 */
/* #define SPECIAL  2 */
/* #define MECHANIC 1 */

/******************************************************************************/

/****************/
/* Global State */
/****************/

map<int,custom::Station*> stations;

/******************************************************************************/

/*************************/
/* Function Declarations */
/*************************/

int get_least_crowded_station( int number_of_stations );
void* create_voters( void* args_ptr );
void* start_station( void* args_ptr );

/******************************************************************************/

/*********/
/* Logic */
/*********/

int main(int argc, char **argv) { 

    // Pareser initializer
    argparse::ArgumentParser program("voting simulator");

    // Adding flags to parse
    program
        .add_argument("-t")
        .required()
        .help("specify the total time for the simulation")
        .scan<'i', int>();
        
    program
        .add_argument("-p")
        .required()
        .help("specify the probability of a voter arriving")
        .scan<'f', float>();

    program
        .add_argument("-c")
        .help("specify the number of polling stations")
        .default_value(1)
        .scan<'i', int>();

    program
        .add_argument("-n")
        .help("The nth second to start")
        .default_value(0)
        .scan<'i', int>();


    // Parse arguments 
    try {
      program.parse_args(argc, argv);
    }
    catch (const std::runtime_error& err) {
      cerr << err.what() << endl;
      cerr << program;
      return 1;
    }

    // Parser Constants
    auto time               = program.get<int>("-t");
    auto nth_second         = program.get<int>("-n");
    auto probability        = program.get<float>("-p");
    auto number_of_stations = program.get<int>("-c");

    // Constant Verifiers
    if (time <= 0){
        cout << "The total time for the simulation (-t) should be more than 0" << endl;
        return EXIT_FAILURE;
    }
    if (probability < 0 || probability > 1){
        cout << "The probability (-p) should be more than or equal to 0 and less than or equal to 1" << endl;
        return EXIT_FAILURE;
    }
    if (number_of_stations <= 0){
        cout << "The number of stations (-c) should be more than 0" << endl;
        return EXIT_FAILURE;
    }


    for (int i = 0; i < number_of_stations; i++) {

        custom::Station* station = new custom::Station();
        pair<int,custom::Station*> st =  { i+1, station };
        stations.insert(st); 
    }


    // Thread variables
    pthread_t creation_thread;
    vector<pthread_t> station_threads(number_of_stations);

    // Set values of the struct
    custom::voter_args_struct args = { probability, time, number_of_stations };

    // create the thread that creates voters
    pthread_create( &creation_thread, NULL, create_voters, (void*) &args );

    vector<custom::station_args_struct*> station_args_ptrs(number_of_stations);
    for (int i = 0; i < number_of_stations; i++) {
        custom::station_args_struct* station_args = new custom::station_args_struct{ time, i+1, nth_second };
        station_args_ptrs[i] = station_args;

        pthread_create( &station_threads[i], NULL, start_station, (void*) station_args);
    }

    // Join Threads
    pthread_join(creation_thread, NULL);
    for (int i = 0; i < number_of_stations; i++) {
        pthread_join(station_threads[i], NULL);
    }

    return EXIT_SUCCESS;
}

/******************************************************************************/

/********************/
/* Helper Functions */
/********************/

int get_least_crowded_station( int number_of_stations ) {

    int stat_number = 1;
    int total_waiting = stations[1]->get_total_waiting();

    for (int i = 0; i < number_of_stations; i++ ){

        int current = stations[i+1]->get_total_waiting();

        if ( current < total_waiting ){
            total_waiting = current;
            stat_number = i+1;
        }
    }

    return stat_number;
}

void* create_voters( void* args_ptr )
{
    // variables from struct
    float   probability;
    int     sim_time;
    int     number_of_stations;

    auto args = *(custom::voter_args_struct *) args_ptr;
    probability         = args.probability;
    sim_time            = args.sim_time;
    number_of_stations  = args.number_of_stations;

    // Time per voter 
    const auto t = 1;

    // Ticket Counter
    int ticket_no = 1;

    // Simulation timer
    auto current_time = chrono::system_clock::to_time_t(chrono::system_clock::now());
    auto end_sim_time =  static_cast<int>(current_time) + sim_time;

    while(current_time < end_sim_time){

        // Random value gen for probability
        random_device rd;
        mt19937 gen(rd());
        uniform_real_distribution<> dis(0, 1);

        double random_number = dis(gen);

        /* auto min = std::min_element(next_ticket.begin(), next_ticket.end()); */
        /* auto least_crowded_station =  std::distance(next_ticket.begin(), min); */

        int least_crowded_station = get_least_crowded_station(number_of_stations);
        auto& station = stations[least_crowded_station];


        pthread_mutex_lock (station->get_mutex());

        if ( random_number <= probability ) {
            station->add_normal(ticket_no);
        }
        else { 
            station->add_special(ticket_no);
        }

        pthread_mutex_unlock (station->get_mutex());

        ticket_no++;

        // Thread sleeps for t secs
        custom::pthread_sleep(t);
        current_time = chrono::system_clock::to_time_t(chrono::system_clock::now());
    }


    /* for (auto it = stations.begin(); it != stations.end(); it++) { */

    /*     cout << "Station Number:" << it->first << endl; */

    /*     auto station = it->second; */

    /*     pthread_mutex_lock (station->get_mutex()); */
    /*     auto normal  = station->get_normal(); */
    /*     auto special = station->get_special(); */

    /*     cout << "Ordinary Voters:" << endl; */
    /*     while (!normal.empty()) { */
    /*         cout << " " << normal.front(); */
    /*         normal.pop(); */
    /*     } */

    /*     cout << endl << "Special Voters:" << endl; */
    /*     while (!special.empty()) { */
    /*         cout << " " << special.front(); */
    /*         special.pop(); */
    /*     } */
    /*     cout << endl; */
    /*     pthread_mutex_unlock (station->get_mutex()); */

    /* } */

    return EXIT_SUCCESS;
}

string vote(){
    // Candidates
    const map<string, pair<float,float>> Candidates = {
        {"Mary", {0.0, 0.40}},
        {"John", {0.40, 0.55}},
        {"Anna", {0.55, 1.0}}
    };
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(0, 1);

    double random_number = dis(gen);
    for (auto it = Candidates.begin(); it != Candidates.end(); it++) {
        if(random_number >= it->second.first && random_number <= it->second.second){
            return it->first;
        }
    }
    return NULL;
}

void* start_station( void* args_ptr ) {
    // variables from struct
    int sim_time;
    int station_number;
    int n;

    auto args      = *(custom::station_args_struct *) args_ptr;
    sim_time       = args.sim_time;
    station_number = args.station_number;
    n              = args.nth_second;
    /* cout << "starting station" << station_number << endl; */


    // Simulation timer
    auto t = 1;
    auto station = stations[ station_number ];
    auto current_time = chrono::system_clock::to_time_t(chrono::system_clock::now());
    auto starting_time = chrono::system_clock::to_time_t(chrono::system_clock::now());
    auto end_sim_time =  static_cast<int>(starting_time) + sim_time;
    int voter;

    while(current_time < end_sim_time) {
        
        if(station->normal_waiting() <= 0 && station->special_waiting() <= 0) {
            current_time = chrono::system_clock::to_time_t(chrono::system_clock::now());
            continue;
        }

        pthread_mutex_lock (station->get_mutex());

        if(station->normal_waiting() <= 0 || (station->normal_waiting() < 5 && station->special_waiting() <= 0)) {
            voter = station->pop_special();
        }

        else if(station->special_waiting() <= 0) {
            voter = station->pop_normal();
        }

        else {
            station->get_normal().front() > station->get_special().front() ? station->pop_special() : station->pop_normal();
        }

        pthread_mutex_unlock (station->get_mutex());
        string vote_res = vote();

        station->increment_vote( vote_res );

        if (current_time - starting_time >= n) {
            auto total_votes = station->get_total_votes();
            cout << "Station : " << station_number << endl;
            cout << "Mary: " << total_votes["Mary"] << " John: "  <<  total_votes["John"] << " Anna: " << total_votes["Anna"] <<  endl;
        }


        custom::pthread_sleep(2 * t);
        current_time = chrono::system_clock::to_time_t(chrono::system_clock::now());
    }

    return 0;
}

/******************************************************************************/
