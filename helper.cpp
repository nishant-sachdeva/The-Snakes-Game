
#include<bits/stdc++.h>
using namespace std;
#include <sys/select.h>
#include <termios.h>
#include <stropts.h>
#include <sys/ioctl.h>


#include<ctime>



void placeCursorAtTop()
{
	int row = 1;
	int col = 1;
	std::cout << "\033[" << row << ";" << col << "H" << endl;  
}


int has_keyboard_been_clicked()
{
    static const int STDIN = 0;
    static bool initialized = false;

    int bytesWaiting;
    ioctl(STDIN, FIONREAD, &bytesWaiting);
    return bytesWaiting;
}


string get_time()
{
	time_t rawtime;
	struct tm * timeinfo;
	char buffer[80];

	time (&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer,sizeof(buffer),"%d-%m-%Y %H:%M:%S",timeinfo);
	std::string str(buffer);

	return str;

}

void write_file_logs(int x, int y, int fruitx, int fruity)
{
	// now we have these variables and we will fill them into the file
	string time_stamp = get_time();
	// open the file and write the variables and the time;

	ofstream myfile;
	myfile.open ("logs.txt", std::ios_base::app);
	myfile << time_stamp << " " << x << " " << y << " " << fruitx << " " << fruity << std::endl;
	myfile.close();

	return;
	// we thus have written output to a log file that shall record all the actions takeen
}

