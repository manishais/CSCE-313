#include "RoundRobin.h"

/*
This is a constructor for RoundRobin Scheduler, you should use the extractProcessInfo function first
to load process information to process_info and then sort process by arrival time;

Also initialize time_quantum
*/
RoundRobin::RoundRobin(string file, int time_quantum) : time_quantum(time_quantum) 
{
	extractProcessInfo(file); 		// you should use the extractProcessInfo function first to load process information to process_info
	set_time_quantum(time_quantum);		// Also initialize time_quantum
}

// Schedule tasks based on RoundRobin Rule
// the jobs are put in the order the arrived
// Make sure you print out the information like we put in the document
void RoundRobin::schedule_tasks() 
{
	int system_time = 0;			// current time
	queue<shared_ptr<Process>> processList = queue<shared_ptr<Process>>();				// queue stores the processes that are ready to run in the round robin scheduler

	while(!processList.empty()|| !processVec.empty())			// at least one needs to have elements
	{
		shared_ptr <Process> process;			// share pointers are used to share ownership of a dynamically allocated object

		if(!processList.empty())
		{
			process = processList.front();		// assigns first process
		}

		else
		{
			process = processVec.front();		// schedule first process in this queue
		}

		if(process->get_arrival_time() > system_time)			// no process is currently available to run
		{
			print(system_time, -1, false);				// int, int, isComplete
			system_time = system_time + 1;
			continue;
		}

		if(process->get_cpu_burst_time() == 0)				// process has completed execution
		{
			process->Run(process->get_remaining_time());			// simulate running process with remaining time
			print(system_time, process->getPid(), process->is_Completed());

			if(!processList.empty())			// if not empty, pop
			{
				processList.pop();
			}

			if(!processVec.empty() && process->getPid() == (processVec.front())->getPid())
			{
				processVec.pop();		
			}

			continue;
		}

		int increment_time = 0;

		if(process->get_remaining_time() > get_time_quantum())
		{
			process->Run(get_time_quantum());			// run process for the time quantum, no process can run longer than this time
			increment_time = get_time_quantum();
		}

		else
		{
			increment_time = process->get_remaining_time();
			process->Run(process->get_remaining_time());	
		}

		if(!processList.empty())
			processList.pop();			// remove first process from list

		if(!processVec.empty() && process->getPid() == (processVec.front())->getPid())
		{
			processVec.pop();			// if both processes have same pid, then transition occurs
		}

		//for(int i = increment_time ; i > 0 ; i++)							// gets stuck in loop for some reason 
		while(increment_time > 0)
		{
			print(system_time, process->getPid(), false);

			increment_time = increment_time - 1;
			system_time = system_time + 1;
		}

		while(!processVec.empty() && (processVec.front()) -> get_arrival_time() <= system_time)         // checks if processes are ready to be scheduled
		{
			processList.push(processVec.front());		// moved from vec to list => processes have been scheduled	
			processVec.pop();
		}

		if(process->is_Completed())
			print(system_time, process->getPid(), true);

		else
			processList.push(process);

	}
}


/*************************** 
ALL FUNCTIONS UNDER THIS LINE ARE COMPLETED FOR YOU
You can modify them if you'd like, though :)
***************************/


// Default constructor
RoundRobin::RoundRobin() {
	time_quantum = 0;
}

// Time quantum setter
void RoundRobin::set_time_quantum(int quantum) {
	this->time_quantum = quantum;
}

// Time quantum getter
int RoundRobin::get_time_quantum() {
	return time_quantum;
}

// Print function for outputting system time as part of the schedule tasks function
void RoundRobin::print(int system_time, int pid, bool isComplete){
	string s_pid = pid == -1 ? "NOP" : to_string(pid);
	cout << "System Time [" << system_time << "].........Process[PID=" << s_pid << "] ";
	if (isComplete)
		cout << "finished its job!" << endl;
	else
		cout << "is Running" << endl;
}

// Read a process file to extract process information
// All content goes to proces_info vector
void RoundRobin::extractProcessInfo(string file){
	// open file
	ifstream processFile (file);
	if (!processFile.is_open()) {
		perror("could not open file");
		exit(1);
	}

	// read contents and populate process_info vector
	string curr_line, temp_num;
	int curr_pid, curr_arrival_time, curr_burst_time;
	while (getline(processFile, curr_line)) {
		// use string stream to seperate by comma
		stringstream ss(curr_line);
		getline(ss, temp_num, ',');
		curr_pid = stoi(temp_num);
		getline(ss, temp_num, ',');
		curr_arrival_time = stoi(temp_num);
		getline(ss, temp_num, ',');
		curr_burst_time = stoi(temp_num);
		shared_ptr<Process> myshared_ptr(new Process(curr_pid, curr_arrival_time, curr_burst_time));

		processVec.push(myshared_ptr);
	}

	// close file
	processFile.close();
}