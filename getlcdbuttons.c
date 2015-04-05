// Wrap code with definition so it's not included more than once
#ifndef  _GETLCDBUTTONS
#define  _GETLCDBUTTONS

// Some utility strings
#define LEFT_ARROW  247
#define RIGHT_ARROW 246
static  char l_arr_str[4] = { LEFT_ARROW,  LEFT_ARROW,  LEFT_ARROW,  0};
static  char r_arr_str[4] = { RIGHT_ARROW, RIGHT_ARROW, RIGHT_ARROW, 0};

/*-----------------------------------------------------------------------------*/
/*  This function is used to get the LCD hutton status but also acts as a      */
/*  "wait for button release" feature.                                         */
/*  Use it in place of nLcdButtons.                                            */
/*  The function blocks until a button is pressed.                             */
/*-----------------------------------------------------------------------------*/

// Little macro to keep code cleaner, masks both disable/ebable and auton/driver
#define vexCompetitionState (nVexRCReceiveState & (vrDisabled | vrAutonomousMode))

TControllerButtons
getLcdButtons()
{
    TVexReceiverState   competitionState = vexCompetitionState;
    TControllerButtons  buttons;

    // This function will block until either
    // 1. A button is pressd on the LCD
    //    If a button is pressed when the function starts then that button
    //    must be released before a new button is detected.
    // 2. Robot competition state changes

    // Wait for all buttons to be released
    while( nLCDButtons != kButtonNone ) {
        // check competition state, bail if it changes
        if( vexCompetitionState != competitionState )
            return( kButtonNone );
        wait1Msec(10);
        }

    // block until an LCD button is pressed
    do  {
        // we use a copy of the lcd buttons to avoid their state changing
        // between the test and returning the status
        buttons = nLCDButtons;

        // check competition state, bail if it changes
        if( vexCompetitionState != competitionState )
            return( kButtonNone );

        wait1Msec(10);
        } while( buttons == kButtonNone );

    return( buttons );
}

#endif  // _GETLCDBUTTONS
