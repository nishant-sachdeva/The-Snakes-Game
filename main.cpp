#include <bits/stdc++.h>
#include "helper.cpp"
#include <unistd.h>
#include <cstdlib>
#include <sys/select.h>
#include <termios.h>
#include <stropts.h>
#include <sys/ioctl.h>

// this is for the random number seeds
#include <ctime>

#include<iostream>
#include<string>
#include<fstream>

// these further libs are to enable locking mechanism for the game, 
#include <chrono>             // std::chrono::seconds
#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable, std::cv_status

// for getting sytem dimensions
#include <sys/ioctl.h>

using namespace std;

#define max_snake_length 100

const int KEY_ARROW_UP = 'w';
const int KEY_ARROW_DOWN = 's';
const int KEY_ARROW_LEFT = 'a';
const int KEY_ARROW_RIGHT = 'd';
const int KEY_EXIT = (const int)('q') ;


bool gameover;

int width = 30;
int height = 30;
// for now we are having to keep them equal, but the aim is to expand to multi-dimensional game

int x, y, fruitx, fruity, score;
int jump = 1;
int sleep_duration = 1;
int snake_speed = 300;

int row = 1;
int col = 1;
string cursor_at_top_left = "\033[" + to_string(row) + ";" + to_string(col) + "H" ; 

char game_input_character;
int speed_increment = 5;
std::condition_variable cv;

// here we add variables for the tail

int tailx[max_snake_length], taily[max_snake_length];
int ntail = 1; // length of tail

// these are the basic variables that we'll need in the game, 
enum direction
{
	STOP = 0,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

direction dir;

// this is for our locking mechanism to detect inputs



void exit_function()
{
	// here we write the loop for exit 
	system("clear");
	for (int i = 0; i <= width; ++i)
		cout << "#" ;
	cout << endl;

	string end = "The game has ended" ;
	// here we'll deal with the game loop,
	for(int j = 0 ; j < width ; j++)
	{
		if(j == 0)
			cout << "#";

		if(j == width/2)
		{
			cout << end << endl;
			j += (int)end.size();
		}

	}
	cout << "#" << endl;

	for (int i = 0; i <= width; ++i)
		cout << "#" ;
	cout << endl;

	exit(0);
}


void setup ()
{
	placeCursorAtTop();
	cout << endl;

	// now init all variables
	gameover = false;
	dir = LEFT;
	x = width/2;  // half the width
	y = height/2; // hald the height 
	 
	fruitx = rand()%width;
	fruity = rand()%height;
	score = 0;

}

void draw()
{
	placeCursorAtTop(); // imported from helper.cpp
	
	cout << "Score : " << score << "  Speed : " << 1000 - snake_speed << endl;

	for (int i = 0; i <= width; ++i)
		cout << "#" ;
	cout << endl;


	// here we'll deal with the game loop,
	for (int i = 0; i < height; ++i)
	{
		for(int j = 0 ; j < width ; j++)
		{
			if(j == 0)
				cout << "#";

		    if( i == y and j == x)
			{
				// means we are at the snake head!!
				cout << "O" ;
			}
			else if(j == fruitx and i == fruity)
			{
				cout << "F" ;
			}
			else
			{
				bool print = false;
				for (int k = 0; k < ntail; ++k)
				{
					if( i == taily[k] and j ==  tailx[k])
					{
						cout << "o";
						print = true;
					}
				}
				if (!print)
				{
					cout << " " ;
				}

			}
		}
		cout << "#" << endl;
	}

	for (int i = 0; i <= width; ++i)
		cout << "#" ;
	cout << endl;

}

void input(char ch1)
{
	// here we shall deal with the key presses
	// cout << "Game input Detected : " << ch1 << endl;
	switch(ch1)
	{
		case 'w':
			dir = UP;
			break;
		case 's':
			dir = DOWN;
			break;
		case 'a':
			dir = LEFT;
			break;
		case 'd':
			dir = RIGHT;
			break;
		case 'q':
			gameover = true;
			break;
	}
}

void logic()
{
	// move all the tail coordinates back by one step
	int prevx = tailx[0];
	int prevy = taily[0];

	for(int i = 1; i < ntail ; i++)
	{
		int tempx = tailx[i];
		int tempy = taily[i];

		tailx[i] = prevx;
		taily[i] = prevy;

		prevx = tempx;
		prevy = tempy;
	}

	// this should have moved our tail one step back

	if(dir == LEFT)
		x -= jump;
	else if(dir == RIGHT)
		x += jump;
	else if(dir == UP)
		y -= jump;
	else if(dir == DOWN)
		y += jump;

	// now we have put in the coordinate

	tailx[0] = x;
	taily[0] = y;

	// check for self hits

	for(int i = 1 ; i < ntail ; i++)
		if(x == tailx[i] and y == taily[i])
		{
			gameover = true;
			exit_function();
		}


	if(x < 0 or x > width or y < 0 or y > height)
	{
		gameover = true;
		exit_function();
	}

	if(x == fruitx and y == fruity)
	{
		// we have eaten the fruit , so we'll increase the tail length
		ntail++;

		score += 10;
		snake_speed -= speed_increment;
		srand((unsigned)time(0)); 

		fruitx = (rand())%width;
		fruity = (rand())%height;

		placeCursorAtTop();

		if(score == 100 or ntail == max_snake_length)
			exit_function();
	}

	write_file_logs(x, y, fruitx, fruity);
}



void read_value()
{
	cin >> game_input_character;

	cv.notify_one();
}


void logic_and_input()
{
	// here what I am looking for is that
	// we will keep executing the game loop until we detect a collision,
	// and if we do we will change direction and move on. 
	// This will almost be a game loop in it's own right


	// so here is the idea,
	// while input does not come, keep running things as they are, 
	// as soon as input comes, do things as you are supposed to do, 
	
	std::thread th (read_value);
	std::mutex mtx;
	std::unique_lock<std::mutex> lck(mtx);

	while (cv.wait_for(lck,std::chrono::milliseconds(snake_speed))==std::cv_status::timeout){
		// insert code for execution normally 
		logic(); 
		draw();
		// this code carries on the game normally as if no input has come,
	}
	// insert code for execution after input has come, namely changing direction or exiting
	input(game_input_character);
	// since we got the input in the game_input_character, we send this to the function
	// that is reponsible for changind directions


	th.join();

	return; // since we got our input, we are going set up further drawing, 
}



int main(int argc, char const *argv[])
{
	// srand(time(NULL));
	// this is for seeding random numbers so that I get a new position for my object every time

	cout << "GAME STARTS In " << endl;
	for (int i = 0; i < 5; ++i)
	{
		cout << "\r" << 5-1 ; 

		sleep(1);
	}
	cout << endl;
	system("clear");
	system("echo new game starting >> logs.txt");
	system("Color 0A");
	struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    // width = w.ws_col - 10;
    // height = w.ws_row - 10;
	setup();
	while(!gameover)
	{
		draw();
		logic_and_input();
	}
	exit_function();
	return 0;
}