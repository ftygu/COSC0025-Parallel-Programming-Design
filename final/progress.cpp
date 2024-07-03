/*
    This file is part of Dirt, the Dartmouth introductory ray tracer.

    Copyright (c) 2017-2019 by Wojciech Jarosz

    Dirt is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 3
    as published by the Free Software Foundation.

    Dirt is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.

    A portion of the code is taken from the wget project. The
    original license follows below.
*/

#include <dirt/progress.h>
#include <dirt/timer.h>

#include <vector>
#include <iostream>


#if defined(_WIN32)
#include <windows.h>

#ifdef NOMINMAX
    #undef NOMINMAX
#endif

#ifndef snprintf
#define snprintf _snprintf
#endif

#else

#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#endif // !_WIN32

#include <signal.h>

using std::endl;
using std::string;
using std::vector;
using std::flush;

namespace
{

// Assumed screen width if we can't find the real value.
#define DEFAULT_SCREEN_WIDTH 80

// A flag that, when set, means SIGWINCH was received.
std::atomic<bool> receivedSIGWINCH(false);
std::atomic<bool> monitoringSIGWINCH(false);


// Determine the width of the terminal we're running on.
int terminalWidth()
{
#if defined(_WIN32)
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    if (h == INVALID_HANDLE_VALUE || h == NULL)
    {
        fprintf(stderr, "GetStdHandle() call failed");
        return DEFAULT_SCREEN_WIDTH;
    }
    CONSOLE_SCREEN_BUFFER_INFO bufferInfo = { 0 };
    GetConsoleScreenBufferInfo(h, &bufferInfo);
    return bufferInfo.dwSize.X;
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) < 0)
    {
        // ENOTTY is fine and expected, e.g. if output is being piped to a file.
        if (errno != ENOTTY)
        {
            static bool warned = false;
            if (!warned)
            {
                warned = true;
                error("Error in ioctl() in terminalWidth(): %d", errno);
            }
        }
        return DEFAULT_SCREEN_WIDTH;
    }
    return w.ws_col;
#endif
}

#ifdef SIGWINCH
void progressHandleSIGWINCH(int sig)
{
    receivedSIGWINCH = true;
    signal(SIGWINCH, progressHandleSIGWINCH);
}
#endif


} // namespace


Progress::Progress(const string & title, int64_t totalWork) :
    m_title(title), m_totalWork(std::max((int64_t)1, totalWork)), m_workDone(0)
{
    m_exitThread = false;

#ifdef SIGWINCH
    if (!monitoringSIGWINCH.exchange(true))
        signal(SIGWINCH, progressHandleSIGWINCH);
#endif

    m_updateThread = std::thread([this]()
        {
            printBar();
        });
}


Progress::~Progress()
{
    done();
    m_exitThread = true;
    m_updateThread.join();

    // print the bar one more time with total elapsed time
	printBar();
    printf("\n");
}

void Progress::done()
{
    m_workDone = m_totalWork;
}

void Progress::printBar()
{
    std::chrono::milliseconds sleepDuration(250);
    int iterCount = 0;
    int screenWidth;
    int currentTick;
    vector<char> buffer;
    string ticks;

    auto resetBar = [&screenWidth, &currentTick, &buffer, &ticks, this]()
        {
            screenWidth = terminalWidth();
            if (!screenWidth)
                screenWidth = DEFAULT_SCREEN_WIDTH;

            buffer = vector<char>(screenWidth+1, ' ');
            ticks = string(std::max(1, screenWidth - 30 - (int)m_title.size()), ' ');

            currentTick = 0;
        };

    resetBar();

    do
    {
        // If terminal width might have changed, update the length of the progress bar
        if (receivedSIGWINCH.exchange(false))
            resetBar();

        // print the different parts of the line, while keeping
        // track of number of printed characters i

        int i = snprintf(&buffer[0], buffer.size(),
                         "\r%s: ", m_title.c_str());

        // fill in the progress bar ticks
        float percentDone = std::min(std::max(float(m_workDone)/float(m_totalWork), 0.0f), 1.0f);
        int ticksNeeded = int(std::round(percentDone * (ticks.size()-1)));
        while (currentTick < ticksNeeded)
            ticks[currentTick++] = '#';
        ticks[currentTick] = (percentDone < 1.f) ? '>' : '#';
        i += snprintf(&buffer[i], buffer.size()-i,
                      "[%s]", ticks.c_str());

        // write the percentage done
        if (percentDone < 1.f)
            i += snprintf(&buffer[i], buffer.size()-i,
                          "  %2d%%", int(percentDone*100));
        else
            i += snprintf(&buffer[i], buffer.size()-i, " 100%%");

        // update elapsed time and estimated time to completion
        float elp = float(m_progressTimer.elapsed());
        float eta = elp/percentDone - elp;

        if (percentDone == 1.f)
        {
            // print elapsed time
            i += snprintf(&buffer[i], buffer.size()-i,
                          " (%s)", timeString(elp).c_str());
        }
        else
        {
            // print elapsed and total time
            i += snprintf(&buffer[i], buffer.size()-i,
                          " (%s/%s)", timeString(elp).c_str(),
                          timeString(std::max(0.f, elp + eta)).c_str());
        }


        buffer[i] = '\0';
	    fputs(&buffer[0], stdout);

		// fill the rest of the line with spaces
	    printf("%*s", max((int)buffer.size()-i, 0), "");
	    fflush(stdout);

	    if (!m_exitThread)
	    {
		    std::this_thread::sleep_for(sleepDuration);

		    // Periodically increase sleepDuration to reduce overhead of
		    // updates.
		    ++iterCount;
		    if (iterCount == 10)
			    // Up to 0.5s after ~2.5s elapsed
			    sleepDuration *= 2;
		    else if (iterCount == 70)
			    // Up to 1s after an additional ~30s have elapsed.
			    sleepDuration *= 2;
		    else if (iterCount == 520)
			    // After 15m, jump up to 5s intervals
			    sleepDuration *= 5;
	    }
    } while (!m_exitThread);
}
