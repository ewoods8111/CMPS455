// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"

//-----------------------GLOBAL VARIABLES-------------------------------
char *shouts[5] = {"Ham Sandwich", "Turkey Sandwich", "Tuna Sandwich", 
					"Chicken Sandwich", "Roast Beef Sandwich"};
int numShouts;
int numPhils;
int numPhilsEntered = 0;
int numPhilsSat = 0;
int numMeals;
bool *chopsticks;
typedef int semaphore;
semaphore mutex = 1;
semaphore *semChops;

int P;
int S;
int M;
bool **postOffice;


//----------------------------------------------------------------------
// InputType
//		A simple enumeration type that is used to hold and say what type
//		of input the user has given.
enum InputType {INT, DEC, CHAR, NEGDEC, NEGINT, UNEXPECTED};

//----------------------------------------------------------------------
// Task1
//		Implementation of the task 1 problem.
//		Analyzes and validates input, telling the user which category
//		his or her input falls under.
//		Creates a thread that forks on the ValidateInput function.
//----------------------------------------------------------------------
void Task1();

//----------------------------------------------------------------------
// ValidateInput
//		Create variables to hold user input and inputType
//		Call the EvaluateInput function to figure out what type of input
//		the user has given.
//
//		"threadNum" is simply to identify the thread which called this,
//		and may or may not be used.
//----------------------------------------------------------------------
void ValidateInput(int threadNum);

//----------------------------------------------------------------------
// IsLetter
//		Checks the passed character against every number character, '-',
//		and '.' to see if it is a letter or keyboard character that
//		would prevent it from being a number.
//----------------------------------------------------------------------
bool IsChar(char a);

//----------------------------------------------------------------------
// EvaluateInput
//		Determines what type of input the user has given.
//		Iterates through the length of input with the following steps:
//			Check if char IsLetter.
//				Return CHAR if so
//			Check if char is '-'
//				If it's the first character, mark possible neg
//				Else, return CHAR.
//			Check if char is '.'
//				If it's the second character and it's not possible neg,
//				mark possible decimal.
//				If it's the third character and it's possible neg,
//				makr possible decimal.
//				Else, return CHAR.
//			If it's gotten here, it must be a number, so just continue.
//
//			
//----------------------------------------------------------------------
InputType EvaluateInput(char * input);

//----------------------------------------------------------------------
// Task2
//		Implementation of Task 2 problem.
//		Prompts the user for a number of threads and a number of shouts
//		per thread.
//		Creates that many threads and forks them to the Shout funtion.
//----------------------------------------------------------------------
void Task2();

//----------------------------------------------------------------------
// Shout
//		Loop a given number of times, each time shouting a random
//		word from the shouts array.
//----------------------------------------------------------------------
void Shout(int threadNum);

//----------------------------------------------------------------------
// SimpleThread
// 		Loop 5 times, yielding the CPU to another ready thread 
//		each iteration.
//
//		"which" is simply a number identifying the thread, for debugging
//		purposes.
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Task3
//		Implementation of Task 3 problem.
//		Prompts the user for a number of philosophers and a number of meals.
//		Creates an array of chopsticks to use; one per philosopher

//		Creates a thread for each philosopher and forks them to the Dine function.
//		
//----------------------------------------------------------------------
void task34Input(int taskNum);

void Dine1(int threadNum);
void Dine2(int threadNum);


void task56Input(int taskNum);
void Post1(int threadNum);
void readMail(int personNum);
void composeMail(int personNum);
int mailResend(int receiver, int mailPosition);
void Post2(int threadNum);

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
		printf("*** threads %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}


//----------------------------------------------------------------------
// ThreadTest
// 		Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
    DEBUG('t', "Entering ThreadTest");
	
	if (EvaluateInput(taskToDo) != INT) {
		printf("***Error, improper input or no -A command found\n");
		currentThread->Finish();
	}

	//Decide which task
	if (atoi(taskToDo) == 1)
		Task1();
	else if (atoi(taskToDo) == 2)
    	Task2();
	else if (atoi(taskToDo) == 3)
	task34Input(3);
	else if (atoi(taskToDo) == 4)
	task34Input(4);
	else if (atoi(taskToDo) == 5)
		task56Input(5);
	else if (atoi(taskToDo) == 6)
		task56Input(6);
    else {
    	printf("***Error, improper input or no -A command found\n");
    	currentThread->Finish();
    }
}

//-------------------------------------------------------------
//--------------------TASK 1-----------------------------------
//-------------------------------------------------------------
void
Task1()
{
	Thread *t = new Thread("Task 1 Thread");
	//Execute the new thread with the ValidateInput()
	t->Fork(ValidateInput, 1);
}

//-------------------------------------------------------------
//--------------------VALIDATE INPUT---------------------------
//-------------------------------------------------------------
void
ValidateInput(int threadNum)
{
	char * buffer = new char[256];
	InputType iType;
	
	//Inform the user how to enter input.
	printf("***Guidelines For Entering Information - \n");
	printf("***Enter any type of information to determine its type.\n");
	printf("***All leading and trailing whitespaces will be taken out.\n");
	printf("***Enter '/' to terminate the loop.\n");
	
	while (buffer[0] != '/') {
	//Get input and evaluate it
	printf("***Enter information ('/' to stop): ");
	fgets(buffer, 256, stdin);
	
	//Strip off leading whitespace to nip that in the bud.
	while (buffer[0] == ' ')
		buffer++;
	

	iType = EvaluateInput(buffer);
	
	
	
	printf("***In thread %d.  %s is of type: ", threadNum, buffer);
	switch(iType) {
		case INT:
			printf("INTEGER");
			break;
		case DEC:
			printf("DECIMAL");
			break;
		case CHAR:
			printf("CHARACTER");
			break;
		case NEGDEC:
			printf("NEGATIVE DECIMAL");
			break;
		case NEGINT:
			printf("NEGATIVE INTEGER");
			break;
		case UNEXPECTED:
			printf("YOU DIDN'T ENTER INPUT");
			break;
		default:
			printf("What did you do");
			break;
	}
	printf("\n\n");
	
	}
	
	currentThread->Finish();

}

//-------------------------------------------------------------
//-------------------EVALUATE INPUT----------------------------
//-------------------------------------------------------------
InputType EvaluateInput(char * input) {
	//Bool types to check possibilities
	bool maybeDecimal = false;
	bool maybeNegative = false;
	
	
	//Strip off trailing newLines or spaces, if any.
	while (input[strlen(input) - 1]  == '\n' || input[strlen(input) - 1] == ' ') {
		input[strlen(input) - 1] = '\0';
	}
	
	
	//First handle if there was just no input
	//Since we've just trimmed it, the size would be 0
	if (strlen(input) == 0)
		return UNEXPECTED;
	
	//Begin evaluating and iterating through the input
	for (int i = 0; (int)i < (int)strlen(input); i++) {
		//Check if the current char is a character
		//If at any time we find a character, it's a string
		if (IsChar(input[i]))
			return CHAR;
		//Check if current char is a - sign
		//If at the first position, it might be negative.
		//If at anywhere else, it just makes it a string
		else if (input[i] == '-') {
			if (i == 0 && strlen(input) > 1)
				maybeNegative = true;
			else
				return CHAR;
		}
		//Check if current char is a .
		//If it already may be a decimal, then we've already hit
		// a ., thus there are 2 and we know it's a character
		//If the . is at either the first or last position, we
		// know it is a character
		//If the . is in the second position, but we know it may
		// be negative ("-.xxxx"), we know it is a character.
		//Else, we can assume the . is in the middle somewhere,
		// and can set maybeDecimal to true.
		else if (input[i] == '.') {
			//Already hit a .
			if (maybeDecimal)
				return CHAR;
			//. is at first or last position
			else if (i == 0 || i == (int)strlen(input) - 1)
				return CHAR;
			//"-.xxxxx"
			else if (i == 1 && maybeNegative)
				return CHAR;
			//It's assumed in the middle and first occurrence
			else
				maybeDecimal = true;
		}
		
		//If we've reached this part of the code, we are sure this
		//character is a number.  Thus we can continue.
	}
	//If we've reached this point, we have eliminated all chance
	//of it being a char.  So we must figure out what kind of number.
	if (maybeNegative && maybeDecimal)
		return NEGDEC;
	else if (maybeNegative)
		return NEGINT;
	else if (maybeDecimal)
		return DEC;
	else
		return INT;
		
	//If we've gotten to this point, then something weird is up.
	return UNEXPECTED;
}

//-------------------------------------------------------------
//--------------------IS CHAR----------------------------------
//-------------------------------------------------------------
bool IsChar(char a) {
	//Check a against every single number, and - and .
	if (a != '0' && a != '1' && a != '2' && a != '3' &&
		a != '4' && a != '5' && a != '6' && a != '7' &&
		a != '8' && a != '9' && a != '-' && a != '.')
		return true;
	else
		return false;

}

//-------------------------------------------------------------
//--------------------TASK 2-----------------------------------
//-------------------------------------------------------------
void Task2() {
	char * buffer = new char[256];
	int numThreads = 0;
	
	//Collect Input
	printf("***Enter desired number of threads: ");
	fgets(buffer, 256, stdin);
	
	if (EvaluateInput(buffer) != INT) {
		printf("***Input must be in integer format.\n\n");
		currentThread->Finish();
	}
	
	
	//Convert Number of Threads
	numThreads = atoi(buffer);
	
	//Collect Input
	printf("***Enter desired number of shouts per thread: ");
	fgets(buffer, 256, stdin);
	
	if (EvaluateInput(buffer) != INT) {
		printf("***Input must be in integer format.\n\n");
		currentThread->Finish();
	} 
	
	
	//Convert Number of Shouts
	numShouts = atoi(buffer);
	
	//Set up our loop to fork numThreads
	Thread * t;
	for (int i = 0; i < numThreads; i++) {
		t = new Thread("Task 2 Thread");
		t->Fork(Shout, i);
	}
	
	
	currentThread->Finish();
}

void Shout(int threadNum) {
	int shout;
	int waitUntil;
	int waitCounter;
	for (int i = 0; i < numShouts; i++) {
		shout = Random() % 5; //Random b/w 0-4
		waitUntil = (Random() % 4) + 2; //Random b/w 2-5
		waitCounter = 0;
		
		printf("***Thread %d shouting: %s \n\n", threadNum, shouts[shout]);
	
		//Busy waiting loop
		//Keep yielding until the waitCounter hits the waitUntil
		while (waitCounter < waitUntil) {
			currentThread->Yield();
			waitCounter++;
		}
		
	}
	
	currentThread->Finish();
}

//-------------------------------------------------------------
//--------------------Input for Tasks 5 & 6--------------------
//-------------------------------------------------------------
void task34Input(int taskNum){
	char * buffer = new char[256];
	
	//Collect Input
	printf("***Enter desired number of philosophers: ");
	fgets(buffer, 256, stdin);
	
	if (EvaluateInput(buffer) != INT) {
		printf("***Input must be in integer format.\n\n");
		currentThread->Finish();
	}
	
	
	//Convert Number of Philosophers
	numPhils = atoi(buffer);
	
	//Collect Input
	printf("***Enter desired number of meals to eat: ");
	fgets(buffer, 256, stdin);
	
	if (EvaluateInput(buffer) != INT) {
		printf("***Input must be in integer format.\n\n");
		currentThread->Finish();
	} 
	
	
	//Convert Number of Meals
	numMeals = atoi(buffer);

	printf("\n\n");
	
	//Set up our loop to fork numPhils
	if(taskNum == 3){
		//Create an array of chopsticks with one per philosopher
		chopsticks = new bool[numPhils];
		Thread * t;
		for (int i = 0; i < numPhils; i++) {
			t = new Thread("Task 3 Thread");
			t->Fork(Dine1, i);
		}
	}
	else
	{	
		//Create an array of chopsticks with one per philosopher
		semChops = new semaphore[numPhils];
		Thread * t;
		for (int i = 0; i < numPhils; i++) {
			t = new Thread("Task 4 Thread");
			t->Fork(Dine2, i);
		}
	
	}
	
	
	currentThread->Finish();
}


//-------------------------------------------------------------
//--------------------DINE1 for Task3--------------------------
//-------------------------------------------------------------
void Dine1(int which)
{

	int waitUntil;
	int waitCounter;
	
	//Current Philosopher enters the room
	numPhilsEntered++;
	printf("***Philosopher %i has entered the room. \n", which);
	while(numPhilsEntered < numPhils)
		currentThread->Yield();

	//Current Philosopher sits at the table
	numPhilsSat++;
	printf("***Philosopher %i has sat down \n", which);
	while(numPhilsSat < numPhils)
		currentThread->Yield();
	
	while(numMeals > 0){
	//If available, current Philosopher picks up both chopsticks
	if(chopsticks[which] == false)
	{
		if(chopsticks[(which + 1) % numPhils] == false)
		{
			chopsticks[which] = true;
			
			if(chopsticks[(which + 1) % numPhils] == true)
				chopsticks[which] = false;
			else{	
				chopsticks[(which + 1) % numPhils] = true;
			
				printf("***Philosopher %i picks up chopstick[%i].\n", which, which);
				printf("***Philosopher %i picks up chopstick[%i].\n", which, (which + 1) % numPhils);
				printf("***Philosopher %i has started eating \n", which);
				
					//Eating for 2-5 cycles
	waitUntil = (Random() % 4) + 2; //Random b/w 2-5
	waitCounter = 0;
		
	//Busy waiting loop
	//Keep yielding until the waitCounter hits the waitUntil
	while (waitCounter < waitUntil) 
	{
		currentThread->Yield();
		waitCounter++;
	}
			}
		}
	}
	else
	{
		while((chopsticks[which] == true) || (chopsticks[(which + 1) % numPhils] == true))
			currentThread->Yield();
	}
	
	//Current Philosopher starts eating
	//printf("***Philosopher %i has started eating \n", which);
	
	if((chopsticks[which] == true) || (chopsticks[(which + 1) % numPhils] == true)){
	//Current Philosopher puts down both chopsticks
	chopsticks[which] = false;
	chopsticks[(which + 1) % numPhils] = false;
			
	printf("***Philosopher %i puts down chopstick[%i].\n", which, which);
	printf("***Philosopher %i puts down chopstick[%i].\n", which, (which + 1) % numPhils);
	
	//Current Philosopher starts thinking
	printf("***Philosopher %i has started thinking \n", which);
	
	numMeals--;
	printf("***%i meals left \n", numMeals);
	//Eating for 2-5 cycles
	waitUntil = (Random() % 4) + 2; //Random b/w 2-5
	waitCounter = 0;
		
	//Busy waiting loop
	//Keep yielding until the waitCounter hits the waitUntil
	while (waitCounter < waitUntil) 
	{
		currentThread->Yield();
		waitCounter++;
	}
	}
	
	}

}


//-------------------------------------------------------------
//--------------------DINE2 for Task4--------------------------
//-------------------------------------------------------------
void Dine2(int which)
{

	int waitUntil;
	int waitCounter;
	
	//Current Philosopher enters the room
	numPhilsEntered++;
	printf("***Philosopher %i has entered the room. \n", which);
	while(numPhilsEntered < numPhils)
		currentThread->Yield();

	//Current Philosopher sits at the table
	numPhilsSat++;
	printf("***Philosopher %i has sat down \n", which);
	while(numPhilsSat < numPhils)
		currentThread->Yield();
	/*
	//If available, current Philosopher picks up both chopsticks
	if(chopsticks[which] == false)
	{
		if(chopsticks[(which + 1) % numPhils] == false)
		{
			chopsticks[which] = true;
			
			if(chopsticks[(which + 1) % numPhils] == true)
				chopsticks[which] = false;
			else		
				chopsticks[(which + 1) % numPhils] = true;
			
			printf("***Philosopher %i picks up chopstick[%i].\n", which, which);
			printf("***Philosopher %i picks up chopstick[%i].\n", which, (which + 1) % numPhils);
		}
	}
	else
	{
		while((chopsticks[which] == true) && (chopsticks[(which + 1) % numPhils] == true))
			currentThread->Yield();
	}
	
	//Current Philosopher starts eating
	printf("***Philosopher %i has started eating \n", which);
	
	
	//Eating for 2-5 cycles
	waitUntil = (Random() % 4) + 2; //Random b/w 2-5
	waitCounter = 0;
		
	//Busy waiting loop
	//Keep yielding until the waitCounter hits the waitUntil
	while (waitCounter < waitUntil) 
	{
		currentThread->Yield();
		waitCounter++;
	}
	
	//Current Philosopher puts down both chopsticks
	chopsticks[which] = false;
	chopsticks[(which + 1) % numPhils] = false;
			
	printf("***Philosopher %i puts down chopstick[%i].\n", which, which);
	printf("***Philosopher %i puts down chopstick[%i].\n", which, (which + 1) % numPhils);
	
	//Current Philosopher starts thinking
	printf("***Philosopher %i has started thinking \n", which);
	
	//Eating for 2-5 cycles
	waitUntil = (Random() % 4) + 2; //Random b/w 2-5
	waitCounter = 0;
		
	//Busy waiting loop
	//Keep yielding until the waitCounter hits the waitUntil
	while (waitCounter < waitUntil) 
	{
		currentThread->Yield();
		waitCounter++;
	}
	
	numMeals--;*/

}


//-------------------------------------------------------------
//--------------------Input for Tasks 5 & 6--------------------
//-------------------------------------------------------------

void task56Input(int taskNum)
{
	//P= number of people
	//S= number of messages mailbox can hold
	//M= number of messages to be sent

	char * buffer = new char[256];
	
	//Collect Input
	printf("***Enter desired number of participating people: ");
	fgets(buffer, 256, stdin);
	
	if (EvaluateInput(buffer) != INT) {
		printf("***Input must be in integer format.\n\n");
		currentThread->Finish();
	}
	// Convert input to number of people
	P = atoi(buffer);
	
	//Collect Input
	printf("***Enter desired number of messages in a person's mailbox: ");
	fgets(buffer, 256, stdin);
	
	if (EvaluateInput(buffer) != INT) {
		printf("***Input must be in integer format.\n\n");
		currentThread->Finish();
	}
	// Convert input to number of messages mailbox can hold
	S = atoi(buffer);

	//Collect Input
	printf("***Enter desired number of messages to be sent: ");
	fgets(buffer, 256, stdin);
	
	if (EvaluateInput(buffer) != INT) {
		printf("***Input must be in integer format.\n\n");
		currentThread->Finish();
	}
	// Convert input to number messages to be sent
	M = atoi(buffer);

	//postOffice = new bool*[P][S];
	postOffice = new bool* [P];
	for (int i = 0; i < P; i++)
		postOffice[i] = new bool[S];

	printf("\n\n");
	
	//Set up our loop to fork Number of People in Simulation
	if(taskNum == 5){
		Thread * t;
		for (int i = 0; i < P; i++) {
			t = new Thread("Task 5 Thread");
			t->Fork(Post1, i);
		}
	}
	else
	{	
		Thread * t;
		for (int i = 0; i < P; i++) {
			t = new Thread("Task 6 Thread");
			t->Fork(Post2, i);
		}
	
	}
	
	
	currentThread->Finish();
}

//-------------------------------------------------------------
//--------------------POST1 for Task 5-------------------------
//-------------------------------------------------------------

void Post1(int which)
{
	int waitUntil;
	int waitCounter;
	printf("Person %i entered the Post Office.\n", which);
	readMail(which);
	composeMail(which);
	//int numMessagesRead = 0; // Current count of messages read by a person
	printf("Person %i left the Post Office.\n", which);
	waitUntil = (Random() % 4) + 2;
	waitCounter = 0;
	while (waitCounter < waitUntil)
	{
		currentThread->Yield();
		waitCounter++;
	}
	
}

void readMail(int personNum)
{
	
	//int mailPosition = S - 1;
	//while (postOffice[personNum][mailPosition] == false)
	//	mailPosition--;
	// true means there is a message in that position
	//if (postOffice[personNum][mailPosition] == true)
	//{
		// read the message by making the position false
	//	postOffice[personNum][mailPosition] = false;
	//	Yield();
		
	//if (mailPosition == 0)
	
	for (int i = 0; i < S; i++)
	{
		if (postOffice[personNum][i] == true)
		{
			printf("Person %d read message %i.\n", personNum, i);
			postOffice[personNum][i] = false;
			currentThread->Yield();
		}
		
	}
}

void composeMail(int personNum)
{
	bool hasMail = false;
	int waitUntil, waitCounter;
	int send = 1;
	int receiver = Random() % P; // Random recipient of the message
	// Find a new recipient if the Random recipient happens to be the sender
	while (receiver == personNum)
		receiver = Random() % P;
	int mailPosition = 0;
	while (postOffice[receiver][mailPosition] == true)
		mailPosition++;
	
	if (postOffice[receiver][mailPosition] == false)
	{
		// message has been sent to recipient
		postOffice[receiver][mailPosition] = true;
		M--;
	}
	else
	{// busy wait loop or retry idk which should go first
		// make retry function to resend 2 more times before deadlock prevention
		// BusyWaiting Loop
		//while (hasMail = true)
		//	currentThread->Yield();

		
		for (int i = 0; i < M; i++)
		{
			if (postOffice[receiver][i] == false)
			{
				postOffice[receiver][i] = true;
				M--;
			}
			while (send < 3)
				mailResend(receiver, mailPosition);
		
		}
	
	}
		
}

int mailResend(int receiver, int mailPosition)
{
	
}

