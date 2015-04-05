/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                 2012-2015                                   */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     flash_user.c                                                 */
/*    Author:     James Pearman                                                */
/*    Created:    21 Aug 2012                                                  */
/*                                                                             */
/*    Revisions:                                                               */
/*                V1.00     4 Apr 2015 - Initial public release                */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    The author is supplying this software for use with the VEX cortex        */
/*    control system. this is free software; you can redistribute it           */
/*    and/or modify it under the terms of the GNU General Public License       */
/*    as published by the Free Software Foundation; either version 3 of        */
/*    the License, or (at your option) any later version.                      */
/*                                                                             */
/*    This software is distributed in the hope that it will be useful,         */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of           */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            */
/*    GNU General Public License for more details.                             */
/*                                                                             */
/*    You should have received a copy of the GNU General Public License        */
/*    along with this program.  If not, see <http://www.gnu.org/licenses/>.    */
/*                                                                             */
/*    The author can be contacted on the vex forums as jpearman                */
/*    or electronic mail using jbpearman_at_mac_dot_com                        */
/*    Mentor for team 8888 RoboLancers, Pasadena CA.                           */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Description:                                                             */
/*                                                                             */
/*    Read and write user parameters to NV storage on the cortex               */
/*    ROBOTC version                                                           */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */

/*-----------------------------------------------------------------------------*/
/** @file    flash_user.c
  * @brief   Save a small number of user settings on the cortex
*//*---------------------------------------------------------------------------*/

// page 190 at present
#define FLASH_USER_PAGE_ADDR    0x0805F000
#define FLASH_USER_INDEX_SIZE   64
#define FLASH_USER_MAX_WRITE    32
#define FLASH_USER_PAGE_SIZE    512


// Number of user parameter words
// Do not change !!
#define FLASH_USER_SIZE         8

// Structure to hold user parameters
typedef struct _flash_user {
    // storage for the NV data
    unsigned char data[FLASH_USER_SIZE * sizeof(uint32_t)];

    // useful debug data
             int  offset;
    void          *addr;
    } flash_user;

// local storage for user parameters
static  flash_user  params;

// ROBOTC version 3.XX has issues with pointer calculations
// so we declare a variable that holds the page address
static  long        __FLASH_USER_PAGE_ADDR = FLASH_USER_PAGE_ADDR;

/*-----------------------------------------------------------------------------*/
/** @brief      Send the contents of the user params page to the debug stream  */
/*-----------------------------------------------------------------------------*/

void
FlashUserDebug()
{
    unsigned char   *p;
    int i,j;

    p = (unsigned char *)__FLASH_USER_PAGE_ADDR;

    for(j=0;j<128;j++)
        {
        writeDebugStream("%08X: ", (uint32_t)p);

        for(i=0;i<16;i++)
            writeDebugStream("%02X ", *p++);

        writeDebugStreamLine("");

        // allow debugger to empty buffer
        wait1Msec(25);
        }
}

/*-----------------------------------------------------------------------------*/
/** @brief      Get the offset into the user parameter block                   */
/** @returns    The offset (0 to 55) or -1 indicating no parameters            */
/*-----------------------------------------------------------------------------*/

int
FlashUserOffsetGet()
{
    uint16_t     *p;
    int     offset = -1;
    int     i;
    uint16_t    su, sl;

    // the first 64 words (256 bytes) are used as an index
    // calculate user param offset
    // look for non zero half word
    p = (uint16_t *)FLASH_USER_PAGE_ADDR;

    for(i=0;i<FLASH_USER_INDEX_SIZE;i++)
        {
        // avoids comparing with 0xFFFFFFFF which does not work
        su = *p++;
        sl = *p++;
        if((su != 0xFFFF)||(sl != 0xFFFF))
            offset++;
         else
            break;
        }

    return(offset);
}

/*-----------------------------------------------------------------------------*/
/** @brief     Read the user parameters                                        */
/** @returns   a pointer to the user parameters                                */
/*-----------------------------------------------------------------------------*/

flash_user *
FlashUserRead()
{
    uint32_t    *p = (uint32_t *)FLASH_USER_PAGE_ADDR;
    uint32_t    *q = (uint32_t *)&params.data;
    uint16_t     i;

    params.offset = FlashUserOffsetGet();

    if(params.offset == (-1))
        {
        // no user parameters
        for(i=0;i<FLASH_USER_SIZE;i++)
            *q++ = 0xFFFFFFFF;

        // error
        params.addr =  (uint32_t *)0;
        }
    else
        {
        // Set address ptr
        p = (uint32_t *)(__FLASH_USER_PAGE_ADDR + ((FLASH_USER_INDEX_SIZE + (params.offset * FLASH_USER_SIZE)) * sizeof(uint32_t)));

        // save address
        params.addr = (uint32_t *)p;

        // Now read params stored at offset
        for(i=0;i<FLASH_USER_SIZE;i++)
            *q++ = *p++;
        }

    return( &params );
}

/*-----------------------------------------------------------------------------*/
/** @brief      write user parameters                                          */
/** @param[in]  u Pointer to user_param structure                              */
/** @returns    status or error code                                           */
/*-----------------------------------------------------------------------------*/

int
FlashUserWrite( flash_user *u )
{
    uint32_t     p = FLASH_USER_PAGE_ADDR;
    uint32_t    *q = (uint32_t *)u->data;
    uint16_t     i;

    // limit number of writes per run
    static  uint16_t flash_user_write_limit = 0;

    volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;

    int         ret = 1;

    // check write limit
    if( flash_user_write_limit >= FLASH_USER_MAX_WRITE )
        return(FLASH_ERROR_WRITE_LIMIT);

    // one more write
    flash_user_write_limit++;

    // Unlock the Flash Bank1 Program Erase controller
    FLASH_UnlockBank1();

    // Clear All pending flags
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

    // Get current param offset
    u->offset = FlashUserOffsetGet();

    // Next block
    u->offset++;

    // did we fill the page ?
    if( u->offset >= ((FLASH_USER_PAGE_SIZE - FLASH_USER_INDEX_SIZE) / FLASH_USER_SIZE) )
        {
#ifdef  FFDEBUG
        writeDebugStreamLine("erase, offset is %d\n", u->offset);
#endif
        // Do erase here
        FLASHStatus = FLASH_ErasePage(FLASH_USER_PAGE_ADDR);

        // check for error
        if( FLASHStatus != FLASH_COMPLETE )
            return(FLASH_ERROR_ERASE);

        // start over
        u->offset = 0;
        }

    // start of area to write params
    p = (uint32_t)(__FLASH_USER_PAGE_ADDR + ((FLASH_USER_INDEX_SIZE + (u->offset * FLASH_USER_SIZE)) * sizeof( uint32_t)));

    // Save addr for debug
    u->addr = (uint32_t *)p;

    // Write data
    for(i=0;i<FLASH_USER_SIZE;i++)
        {
        FLASHStatus = FLASH_ProgramWord( p, *q++ );

        p += 4;

        // check for error
        if( FLASHStatus != FLASH_COMPLETE )
            {
            ret = FLASH_ERROR_WRITE;
            break;
            }
        }


    // Update data at offset
    p = (uint32_t)(__FLASH_USER_PAGE_ADDR  + (u->offset * sizeof( uint32_t)));

    FLASHStatus = FLASH_ProgramWord( p, 0);

    // check for error
    if( FLASHStatus != FLASH_COMPLETE )
        ret = (FLASH_ERROR_WRITE);

    return( ret );
}

/*-----------------------------------------------------------------------------*/
/** @brief     Initialize the user parameter memory                            */
/** @Returns    status or error code                                           */
/*-----------------------------------------------------------------------------*/

int
FlashUserInit()
{
    static  int erase_done = 0;

    volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;

    if( !erase_done )
        {
        // only allow one init per run
        erase_done = 1;

        // Unlock the Flash Bank1 Program Erase controller
        FLASH_UnlockBank1();

        // Erase user parameters
        FLASHStatus = FLASH_ErasePage(FLASH_USER_PAGE_ADDR);

        // check for error
        if( FLASHStatus != FLASH_COMPLETE )
            return(FLASH_ERROR_ERASE);
        }
    else
        {
        return(FLASH_ERROR_ERASE_LIMIT);
        }

    return(1);
}
