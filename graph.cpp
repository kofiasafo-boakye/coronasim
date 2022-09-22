#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <random>

using namespace std;

// This function takes a probability and returns true or false.
bool simulate_prob(float prob){

  prob = prob*1000;

  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_int_distribution<std::mt19937::result_type> random_number(1,1000); // distribution in range [1, 1000]

  if ( random_number(rng) <= prob)
    return true;

  return false;
}


class Person{
	public:
		string name;
		int age;
		float spread_prob;
		float disease_prob;
		float recovery_prob;
		int number_of_days;
		bool infection_status; 
        vector<Person*> victims;
	
		Person(string name, int age, float spread_prob): name(name), age(age), spread_prob(spread_prob), number_of_days(0), infection_status(false){}

		bool equals(Person person){
			return (this->name == person.name && this->age == person.age && this->spread_prob == person.spread_prob);
		}

        // This method prints the list of people that a given person can infect in the population.
        void print_people_list(){
            cout << name;
            if(victims.size() !=0){
                cout << "  |  ";
                for(int i = 0; i < victims.size() - 1; i++){
                    cout << victims[i]->name << ",";
                }
                cout << victims[victims.size() - 1]->name << ".";
            }
            cout << endl;
        }
};

// The graph class is stores the population as nodes and contains the methods for the various simulations and control policy.
class Graph{
	public:
		int largest_age = 1;
        int num_cases_recorded = 0;  
        int num_sick = 0; 
        int recoveries_recorded = 0;
		vector<Person> adjlist;

		// This method adds a new node into the graph. 
		void add_node(Person person){
			adjlist.push_back(person);
			if(person.age > largest_age){largest_age = person.age;}
		}

		void add_edge(string person1_name, string person2_name){
			int index1;
            int index2;
			for(int i = 0; i < adjlist.size(); i++){
				if(adjlist[i].name == person1_name){
					index1 = i;
                    break;
				}
			}

            for(int i = 0; i < adjlist.size(); i++){
				if(adjlist[i].name == person2_name){
					index2 = i;
                    break;
				}
			}
			adjlist[index1].victims.push_back(&adjlist[index2]);
		}
	
        // This methods prints the persons in the graph, alongside the people they can infect.
		void printPeople(){
			for(int i = 0; i < adjlist.size(); i++){
				adjlist[i].print_people_list();
			}
		}

        
        void set_disease_prob(){
            for(int i = 0; i < adjlist.size(); i++){
                adjlist[i].disease_prob = adjlist[i].age / (float) largest_age;
                adjlist[i].recovery_prob = 1 - adjlist[i].disease_prob;
            }
        } 

        // This methods infects the first person after choosing them randomly
        bool infect_patient_zero(){
            srand(time(NULL));
            int i = rand() % adjlist.size();

            if(adjlist[i].victims.size() > 0){
               adjlist[i].infection_status = true;
               num_cases_recorded++; 
               return true;
            }
            return false;
            
        }

        // This method goes through and lets infected people infect other people, using their spread probability.
        void infection_simulation(){
            for(int i = 0; i < adjlist.size(); i++){
                if(adjlist[i].infection_status && (adjlist[i].number_of_days <= 7)){  // The check of whether number of days is <= 7 confirms that the person is not dead.
                    // Only carry this loop for infected people and not dead people. As the rest cannot infect others. 
                    Person* temp = &adjlist[i];
                    for(int j = 0; j < temp->victims.size(); j++){
                        // Important to remember that the victims array is an array of Person pointers.
                        if(!(temp->victims[j]->infection_status)){
                            if(simulate_prob(temp->spread_prob)){
                                temp->victims[j]->infection_status = true;
                                num_cases_recorded++;
                            }
                        }
                    }
                }
            }
        }

        // This function goes through and simulates the phase in which infected people get sick
        // For the persons who are already sick and not dead, the number of days sick increases.
        void get_sick_simulation(){
            for(int i = 0; i < adjlist.size(); i++){
                if(adjlist[i].infection_status && (adjlist[i].number_of_days == 0)){
                    if(simulate_prob(adjlist[i].disease_prob)){
                        adjlist[i].number_of_days = 1;
                        num_sick++;
                    }
                }
                // Check if the person is already sick and increase the number of days sick
                else if((adjlist[i].number_of_days > 0) && (adjlist[i].number_of_days <= 7)){
                    adjlist[i].number_of_days++;
                }
            }
        }

        // This function determines if a person that is sick will get well. 
        void recovery_simulation(){
            for(int i = 0; i < adjlist.size(); i++){
                // Checking if the person is sick and alive.
                if((adjlist[i].number_of_days > 0) && (adjlist[i].number_of_days <= 7)){
                    if(simulate_prob(adjlist[i].recovery_prob)){
                        recoveries_recorded++;
                        adjlist[i].infection_status = false;
                        adjlist[i].number_of_days = 0; // Number of days is set to zero and the infection-status to false since the person can be infected again.
                    }
                }
            }
        }

        // This function carries out the simulation. Please before carrying out the simulation, always ensure that a random person has been infected.
        void simulate_spread(){

            while (num_cases_recorded == 0){
                infect_patient_zero();
            }
            infection_simulation();
            get_sick_simulation();
            recovery_simulation();
        }

        // This function resets the parameters of the population to enable a new simulation.
        void reset(){
            for(int i = 0; i < adjlist.size(); i++){
               adjlist[i].infection_status = false;
               adjlist[i].number_of_days = 0;
           }
           num_cases_recorded = 0;
           num_sick = 0;
           recoveries_recorded = 0;
        }

       void control_policy(){
           reset();
           for(int i = 0; i < adjlist.size(); i++){
               for(int j = 0; j < adjlist[i].victims.size(); i++){
                   if( adjlist[i].victims[j]->age / (float)largest_age > 0.35){
                       adjlist[i].spread_prob = adjlist[i].spread_prob / 2.0;
                   }
                   break;
                   
               }
           }
           simulate_spread();
       }

        // This function gets the current number of infected people
        int get_num_infected(){
            int num = 0;
            for(int i = 0; i < adjlist.size(); i++){
                if(adjlist[i].infection_status && (adjlist[i].number_of_days == 0)){
                    num++;
                }
            }
            return num;
        }

        // This function gets the current number of people that are infected and sick
        int get_num_sick(){
            int num = 0;
            for(int i = 0; i < adjlist.size(); i++){
                if((adjlist[i].number_of_days > 0) && (adjlist[i].number_of_days <= 7)){
                    num++;
                }
            }
            return num;
        }

        // This function gets the number of dead people
        int get_num_dead(){
            int num = 0;
            for(int i = 0; i < adjlist.size(); i++){
                if(adjlist[i].number_of_days > 7){
                    num++;
                }
            }
            return num;
        }

        // This function prints the summary statistics of the population in a user-friendly manner.
        void printStatistics(){
            int current_infections = get_num_infected();
            int current_sick = get_num_sick();
            int num_dead = get_num_dead();

            
            string labels[] = {"Total number of cases recorded", "Total sick cases recorded", "Current Number Infected but not Sick", "Current Number Sick",
                               "Current Healthy Persons", "Total number of Recoveries", "Number of Deaths"};
            
            int numbers[] = {num_cases_recorded, num_sick, get_num_infected(), get_num_sick(), ((int) adjlist.size()-get_num_dead()-get_num_sick()-get_num_infected()) , recoveries_recorded, get_num_dead()};
            float percentages[] = {0, 0, 0, 0, 100, 0, 0};
            if(num_cases_recorded != 0){
                                 percentages[0] = ((float)num_cases_recorded / adjlist.size()) * 100;
                                 percentages[1] = ((float)num_sick / num_cases_recorded) * 100;
                                 percentages[2] = ((float) get_num_infected() / adjlist.size()) * 100;
                                 percentages[3] = ((float) get_num_sick() / adjlist.size()) * 100;
                                 percentages[4] = ((float) (adjlist.size() - get_num_dead() - get_num_sick() - get_num_infected()) / adjlist.size()) * 100;
                                 percentages[5] = ((float)recoveries_recorded / num_cases_recorded) * 100;
                                 percentages[6] = ((float)get_num_dead() / num_cases_recorded) * 100;
                                };

            cout << "  \n*******************************Statistics for the spread of Corona Virus in the population******************************* \n" << endl;
            cout << "                                   Total number of Persons: " << adjlist.size() << "                                             " << endl;
            for(int i = 0; i < 7; i++){
                const char* tempString = labels[i].c_str();
                printf("%40s %10d %10s %6.4f %%", tempString, numbers[i], " ", percentages[i]);
                switch (i)
                {
                case 0:
                    printf(" of the Population.");
                    break;
                case 1:
                    printf(" of Total Number of Cases Recorded.");
                    break;
                case 2:
                    printf(" of the Population.");
                    break;
                case 3:
                    printf(" of the Population.");
                    break;
                case 4:
                    printf(" of the Population.");
                    break;
                case 5:
                    printf(" of Total Number of Cases Recorded.");
                    break;
                case 6:
                    printf(" of Total Number of Cases Recorded.");
                    break;
                default:
                    break;
                }
                cout << endl;
            }
            cout << "\n" << endl;

        }

};


  