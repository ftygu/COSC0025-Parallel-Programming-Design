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
*/

#pragma once

#include <dirt/timer.h>
#include <string>
#include <atomic>
#include <thread>

/// Helper object to display and update a progress bar in the terminal.
/**
    Example usage:

    \code
        {
            Progress p1("Solving", 10);
            for (int i = 0; i < 10; ++i, ++p1)
            {
                // do something
            }
        } // end progress p1
    \endcode
*/
class Progress
{
public:
    Progress(const std::string & title, int64_t totalWork);
    ~Progress();

    void step(int64_t steps = 1) {m_workDone += steps;}
    void done();

    int progress() const
    {
        return int(m_workDone);
    }

    Progress & operator++ ()                {step(); return *this; }
    Progress & operator+=(int64_t steps)    {step(steps); return *this; }

private:

    void printBar();
    void resetPlusses();

    std::string m_title;
    const int64_t m_totalWork;
    std::atomic<int64_t> m_workDone;
    const Timer m_progressTimer;
    std::atomic<bool> m_exitThread;
    std::thread m_updateThread;
};
