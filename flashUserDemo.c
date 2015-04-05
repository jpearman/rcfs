/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2015                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     flashUserDemo.c                                              */
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

#include <FlashLib.h>

void
WaitForKey()
{
    while( nLCDButtons == 0 )
        wait1Msec(5);
    while( nLCDButtons != 0 )
        wait1Msec(5);
}

task main()
{
    char  str[32];

    // pointer to user parameters
    flash_user  *u;

    bLCDBacklight = true;

    while(1)
        {
        // use LCD as trigger to start code
        displayLCDString(0, 0, "Hit button to   ");
        displayLCDString(1, 0, "continue...     ");
        WaitForKey();

        // Read the user parameters
        u = FlashUserRead();

        displayLCDString(0, 0, "read done       ");
        sprintf(str, "param 0 is %d   ", u->data[0]);
        displayLCDString(1, 0, str);

        // wait a while
        wait1Msec(4000);

        displayLCDString(0, 0, "Hit button to   ");
        displayLCDString(1, 0, "continue...     ");
        WaitForKey();

        // increase that parameter by 1
        if( u->data[0] < 255 )
            u->data[0] = u->data[0] + 1;
        else
            u->data[0] = 1;

        // now write parameters
        if( FlashUserWrite( u ) )
            displayLCDString(0, 0, "write ok        ");
        else
            displayLCDString(0, 0, "write error     ");
        clearLCDLine(1);

        // wait then repeat
        wait1Msec(2000);
        }
}
