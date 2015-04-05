/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013-2015                                 */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     lcdAutonDemo_2_1.c                                           */
/*    Author:     James Pearman                                                */
/*    Created:    5 April 2015                                                 */
/*                                                                             */
/*    Revisions:                                                               */
/*                V1.00  5 April 2015 - Initial release                        */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    The author is supplying this software for use with the VEX cortex        */
/*    control system. This file can be freely distributed and teams are        */
/*    authorized to freely use this program , however, it is requested that    */
/*    improvements or additions be shared with the Vex community via the vex   */
/*    forum.  Please acknowledge the work of the authors when appropriate.     */
/*    Thanks.                                                                  */
/*                                                                             */
/*    Licensed under the Apache License, Version 2.0 (the "License");          */
/*    you may not use this file except in compliance with the License.         */
/*    You may obtain a copy of the License at                                  */
/*                                                                             */
/*      http://www.apache.org/licenses/LICENSE-2.0                             */
/*                                                                             */
/*    Unless required by applicable law or agreed to in writing, software      */
/*    distributed under the License is distributed on an "AS IS" BASIS,        */
/*    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. */
/*    See the License for the specific language governing permissions and      */
/*    limitations under the License.                                           */
/*                                                                             */
/*    The author can be contacted on the vex forums as jpearman                */
/*    or electronic mail using jbpearman_at_mac_dot_com                        */
/*    Mentor for team 8888 RoboLancers, Pasadena CA.                           */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------*/
/*  LCD autonomous demo 2 using user parameter save                            */
/*-----------------------------------------------------------------------------*/

//Competition Control and Duration Settings
#pragma competitionControl(Competition)

#include "Vex_Competition_Includes.c"   //Main competition background code...do not modify!

// Include the lcd button get utility function
#include "getlcdbuttons.c"

// Include the flash library
#include <FlashLib.h>

// global hold the auton selection
static int MyAutonomous = 0;

// Macro that detects whether comp switch is connected
// used to make testing easier
#define vexNoFieldControl (~nVexRCReceiveState & vrCompetitionSwitch )

/*-----------------------------------------------------------------------------*/
/*  Display autonomous selection                                               */
/*-----------------------------------------------------------------------------*/

// max number of auton choices
#define MAX_CHOICE  3

void
LcdAutonomousSet( int value, bool select = false )
{
    // Cleat the lcd
    clearLCDLine(0);
    clearLCDLine(1);

    // Display the selection arrows
    displayLCDString(1,  0, l_arr_str);
    displayLCDString(1, 13, r_arr_str);

    // Save autonomous mode for later if selected
    if(select)
        MyAutonomous = value;

    // If this choice is selected then display ACTIVE
    if( MyAutonomous == value )
        displayLCDString(1, 5, "ACTIVE");
    else
        displayLCDString(1, 5, "select");

    // Show the autonomous names
    switch(value) {
        case    0:
            displayLCDString(0, 0, "auton 0");
            break;
        case    1:
            displayLCDString(0, 0, "auton 1");
            break;
        case    2:
            displayLCDString(0, 0, "auton 2");
            break;
        case    3:
            displayLCDString(0, 0, "auton 3");
            break;
        default:
            displayLCDString(0, 0, "Unknown");
            break;
        }
}

/*-----------------------------------------------------------------------------*/
/*  Rotate through a number of choices and use center button to select         */
/*-----------------------------------------------------------------------------*/

void
LcdAutonomousSelection()
{
    TControllerButtons  button;
    int  choice = 0;

    // pointer to sdaved parameters
    flash_user  *u;

    // Turn on backlight
    bLCDBacklight = true;

    // Read the user parameters
    u = FlashUserRead();
    // parameters may have never been used - check for 255 in
    // the first slot
    if( u->data[0] > 3 )
        u->data[0] = 0;

    // display and select default choice
    LcdAutonomousSet(u->data[0], true);

    // If competition switch is not connected then we will stay in this loop
    // if robot is disabled we will also stay in this loop
    while( vexNoFieldControl || bIfiRobotDisabled )
        {
        // this function blocks until button is pressed
        button = getLcdButtons();

        // Display and select the autonomous routine
        if( ( button == kButtonLeft ) || ( button == kButtonRight ) ) {
            // previous choice
            if( button == kButtonLeft )
                if( --choice < 0 ) choice = MAX_CHOICE;
            // next choice
            if( button == kButtonRight )
                if( ++choice > MAX_CHOICE ) choice = 0;
            LcdAutonomousSet(choice);
            }

        // Select this choice
        if( button == kButtonCenter )
            {
            LcdAutonomousSet(choice, true );

            // Save the new choice
            u->data[0] = choice;
            if( !FlashUserWrite( u ) )
                writeDebugStreamLine("Flash write error");
            }

        // Don't hog the cpu !
        wait1Msec(10);
        }
}

void pre_auton()
{
    bStopTasksBetweenModes = true;

    LcdAutonomousSelection();
}

task autonomous()
{
    LcdAutonomousSet(MyAutonomous);
    clearLCDLine(1);
    displayLCDString(1, 0, "Running...");

    switch( MyAutonomous ) {
        case    0:
            // run auton code
            break;

        case    1:
            // run some other auton code
            break;

        default:
            break;
        }
}

task usercontrol()
{
    while (true) {
        wait1Msec(10);
        }
}
