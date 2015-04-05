/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                 2013-2014                                   */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     flash_rcfs.c                                                 */
/*    Author:     James Pearman                                                */
/*    Created:    23 Feb 2013                                                  */
/*                                                                             */
/*    Revisions:                                                               */
/*                V1.00     7 Jan 2014 - Initial release                       */
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

//#include <FirmwareVersion.h>

/*-----------------------------------------------------------------------------*/
/** @file    flash_rcfs.c
  * @brief   Access the ROBOTC file system
*//*---------------------------------------------------------------------------*/

/** @brief This will be the basename for all files not given a name            */
const char *RCFS_BASENAME = "debug";

/*-----------------------------------------------------------------------------*/
/** @brief   flash file header                                                 */
/*-----------------------------------------------------------------------------*/

typedef struct _flash_file {
    unsigned char name[16];                ///< file name
    unsigned char type;                    ///< file type, wav, exe etc.
    unsigned char time[4];                 ///< file creation time
    unsigned char unknown;                 ///< unknown flags V3.XX

    // V4 added a couple of bytes to the header
    unsigned char pad[2];                  ///< unknown flags V4.XX

    // not written to flash, we keep them here for convienience
    unsigned long addr;                    ///< address of file in flash
    unsigned char *data;                   ///< pointer to data for the file
             int  datalength;              ///< length of file
    } flash_file;

/** @cond    */
//#define FFDEBUG                  1
#if kRobotCVersionNumeric < 400
#define FLASH_FILE_HEADER_SIZE  22
#else
// fix, 16-Oct-2014, JP
// looks like V4.26 changed header size
#define FLASH_FILE_HEADER_SIZE  24
#endif

static  unsigned long  baseaddr = kStartOfFileSystem;

// Define maximum file size, can be overridden in user code
#ifndef MAX_FLASH_FILE_SIZE
#define MAX_FLASH_FILE_SIZE 8192
#endif

// ROBOTC changed the size of the header in V3.60 and on
#if kRobotCVersionNumeric < 359
#define VTOC_OFFSET     24
#else
#if kRobotCVersionNumeric < 400
#define VTOC_OFFSET     28
#else
// fix, 16-Oct-2014, JP
// V4.26 changed VTOC offset
#define VTOC_OFFSET     160
#endif
#endif

#define RCFS_SUCCESS    0
#define RCFS_ERROR      (-1)

/** @endcond */

/*-----------------------------------------------------------------------------*/
/** @brief     Dump contents of a header structure to the debug stream         */
/** @param[in] f pointer to a flash file header                                */
/*-----------------------------------------------------------------------------*/

void
RCFS_DebugFile( flash_file *f )
{
    char str[20];
    int  i;

    // some problem using sprintf here
    for(i=0;i<16;i++)
        {
        if( f->name[i] > 0x20 && f->name[i] < 0x7f )
            str[i] = f->name[i];
        else
            str[i] = ' ';
        }
    str[16] = 0;

    writeDebugStream(str);
    writeDebugStream(" Addr %08X", f->addr );
    writeDebugStream(" Data %08X", (unsigned long)f->data );
    writeDebugStream(" Size %5d", f->datalength );
    writeDebugStream(" Type %02X", f->type );
    writeDebugStream(" Time %02X%02X%02X%02X", f->time[0],f->time[1],f->time[2],f->time[3] );
//    writeDebugStream(" Flag %02X", f->unknown );
    writeDebugStreamLine("");
}

/*-----------------------------------------------------------------------------*/
/** @brief     Initialize a flash file header                                  */
/** @param[in] f pointer to a flash file header                                */
/*-----------------------------------------------------------------------------*/

static void
RCFS_FileInit( flash_file *f )
{
    int     i;

    // private vars
    f->addr = 0;
    f->data = NULL;
    f->datalength = 0;

    // Clear name
    for(i=0;i<16;i++)
        f->name[i] = 0;

    // Init metadata
    f->type   = ftData;
    f->time[0] = 0x38;
    f->time[1] = 0x64;
    f->time[2] = 0x09;
    f->time[3] = 0x00;
    f->unknown = 0;
    f->pad[0]  = 0;
    f->pad[1]  = 0;
}

/*-----------------------------------------------------------------------------*/
/** @brief     Write flash file                                                */
/** @param[in] f pointer to a flash file header                                */
/*-----------------------------------------------------------------------------*/
/** @details
 *  The flash file header should have been initialized with the file name,
 *  metadata and file address and length before this function is called.
 */

static void
RCFS_Write( flash_file *f )
{
    unsigned short *p;
    unsigned short *q;
    int   i;
    volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;

    // check for valid address
    if( f->addr < baseaddr )
        return;

    if( f->data == NULL )
        return;

    // Init pointers (some wierd bug in ROBOTC here)
    long tmp = f->addr;
    p = (unsigned short *)tmp;
    q = (unsigned short *)&(f->name[0]);

    // Write header
    for(i=0;i<(FLASH_FILE_HEADER_SIZE/2);i++)
        {
        // Write 16 bit data
        FLASHStatus = FLASH_ProgramHalfWord( (uint32_t)p++, *q++ );
        }

    // point at the file data, we will write as words
    q = (unsigned short *)f->data;

    // Write Data, datalength is now in bytes so divide datalength by 2
    for(i=0;i<(f->datalength/2);i++)
        {
        // Write 16 bit data
        FLASHStatus = FLASH_ProgramHalfWord( (uint32_t)p++, *q++ );

        // every 128 bytes abort time slice
        if( (i % 128) == 0 )
            abortTimeslice();
        }

    // Was it an odd number of bytes ?
    if( (f->datalength & 1) == 1 )
        {
        // pad with 0xFF and write the last byte
        unsigned short b = (*(unsigned char *)q) | 0xFF00;
        FLASHStatus = FLASH_ProgramHalfWord( (uint32_t)p++, b);
        }
}

/*-----------------------------------------------------------------------------*/
/** @brief     Read flash file Header                                          */
/** @param[in] f pointer to a flash file header                                */
/*-----------------------------------------------------------------------------*/

static void
RCFS_ReadHeader( flash_file *f )
{
    unsigned char *q;
    int  i;

    long tmp = f->addr;
    q = (unsigned char *)tmp;

    // Read name
    for(i=0;i<16;i++)
        f->name[i] = *q++;

    // Read ??
    f->type    = *q++;
    f->time[0] = *q++;
    f->time[1] = *q++;
    f->time[2] = *q++;
    f->time[3] = *q++;
    f->unknown = *q++;
    f->pad[0]  = 0;
    f->pad[1]  = 0;
}

/*-----------------------------------------------------------------------------*/
/** @brief     Read flash file table of contents and print in debug window     */
/*-----------------------------------------------------------------------------*/

void
RCFS_ReadVTOC()
{
    long *toc = (long *)(baseaddr + VTOC_OFFSET);
    long  addr;
    long  size;
    short slot;

    flash_file  f;

    // more than kMaxNumbofFlashFiles files we have an error
    for(slot=0;slot<kMaxNumbofFlashFiles;slot++)
        {
        // Read file address
        addr = *toc++;
        // read file size
        size = *toc++;

        // Good file ?
        if( addr == (-1) )
            return;

        f.addr       = baseaddr + addr;
        f.data       = (unsigned char *)(f.addr + FLASH_FILE_HEADER_SIZE);
        f.datalength = (size - FLASH_FILE_HEADER_SIZE);

        // Read header
        RCFS_ReadHeader( &f );
        // Display
        RCFS_DebugFile( &f );
        }
}

/*-----------------------------------------------------------------------------*/
/** @brief     Find the number of the next free slot in the file system        */
/*-----------------------------------------------------------------------------*/
static int
RCFS_FindLastSlot()
{
    long *toc = (long *)(baseaddr + VTOC_OFFSET);
    long  addr;
    short slot;

    // more than kMaxNumbofFlashFiles files we have an error
    for(slot=0;slot<kMaxNumbofFlashFiles;slot++)
        {
        // Read next file address
        addr = *toc;

        // End of table ?
        if( addr == (-1) )
            {
            return(slot);
            }
        else
            {
            // Valid file found
            toc+=2;
            }
        }

    return(RCFS_ERROR);
}

/*-----------------------------------------------------------------------------*/
/** @brief     Find the first file in the file system                          */
/** @param[in] f pointer to a flash file header                                */
/*-----------------------------------------------------------------------------*/
int
RCFS_FindFirstFile( flash_file *f )
{
    long *toc = (long *)(baseaddr + VTOC_OFFSET);
    long  addr;
    long  size;

    if( f == NULL )
        return(RCFS_ERROR);

    // Read file address
    addr = *toc++;
    // read file size
    size = *toc++;

    // Good file ?
    if( addr == (-1) )
        return(RCFS_ERROR);

    f->addr       = baseaddr + addr;
    f->data       = (unsigned char *)(f->addr + FLASH_FILE_HEADER_SIZE);
    f->datalength = (size - FLASH_FILE_HEADER_SIZE);

    // Read header
    RCFS_ReadHeader( f );

    // slot should be 0
    return(0);
}

/*-----------------------------------------------------------------------------*/
/** @brief     Find the next file in the file system                           */
/** @param[in] f pointer to a flash file header                                */
/*-----------------------------------------------------------------------------*/
int
RCFS_FindNextFile( flash_file *f )
{
    long *toc = (long *)(baseaddr + VTOC_OFFSET);
    long  addr;
    long  size;
    short slot;

    if( f == NULL )
        return(RCFS_ERROR);

    // more than kMaxNumbofFlashFiles files we have an error
    for(slot=0;slot<kMaxNumbofFlashFiles;slot++)
        {
        // Read file address
        addr = *toc++;
        // read file size
        size = *toc++;

        // Good file ?
        if( addr == (-1) )
            return(RCFS_ERROR);

        // found starting file
        if( (unsigned long)(baseaddr+addr) == f->addr )
            {
            // Get next file
            slot++;
            // Read file address
            addr = *toc++;
            // read file size
            size = *toc++;

            // Good file ?
            if( addr == (-1) )
                return(-1);

            f->addr       = baseaddr + addr;
            f->data       = (unsigned char *)(f->addr + FLASH_FILE_HEADER_SIZE);
            f->datalength = (size - FLASH_FILE_HEADER_SIZE);

            // Read header
            RCFS_ReadHeader( f );

            // return this slot
            return(slot);
            }
        }

    // error
    return(RCFS_ERROR);
}

/*-----------------------------------------------------------------------------*/
/** @brief     Add a file to the file system                                   */
/** @param[in] data pointer to the data to be written                          */
/** @param[in] length plength of data in bytes                                 */
/** @param[in] name name of the file to be written                             */
/*-----------------------------------------------------------------------------*/

int
RCFS_AddFile( unsigned char *data, int length, char *name )
{
    long *toc = (long *)(baseaddr + VTOC_OFFSET);
    long  addr;
    long  size;
    short slot;

    long  maxaddr  = 0;
    long  nextaddr = 0;

    volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;
    flash_file   f;

    // bounds check length
    if( (length <= 0) || (length > MAX_FLASH_FILE_SIZE))
        return(RCFS_ERROR);

    // more than kMaxNumbofFlashFiles files we have an error
    for(slot=0;slot<kMaxNumbofFlashFiles;slot++)
        {
        // Read next file address
        addr = *toc;

        // End of table ?
        if( addr == (-1) )
            {
            // Start on Word boundary
            if(nextaddr & 1)
                nextaddr++;

            // move us into high menory
            // V3.51 had crap at 8040000 so we had to push back
            // to 8030000
            if(nextaddr < 0x18000)
                nextaddr = 0x18000;

            // Check if there is room for the file
            // We reserve 4K for user parameter storage
            if( (nextaddr + length ) > 0x47000 )
                return(RCFS_ERROR);

            // create new file
            RCFS_FileInit( &f );

            // Copy name, max 15 chars
            strncpy( &f.name[0], name, 15 );

            // setup address, data pointer and length for this file
            f.addr       = baseaddr + nextaddr;
            f.data       = data;
            f.datalength = length;

#ifdef  FFDEBUG
            // Debug
            RCFS_DebugFile(&f);
#endif
            // Unlock the Flash Bank1 Program Erase controller
            FLASH_UnlockBank1();

            // Clear All pending flags
            FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

            // write table of contents entry
            FLASHStatus = FLASH_ProgramWord( (uint32_t)toc++, nextaddr );
            FLASHStatus = FLASH_ProgramWord( (uint32_t)toc++, length + FLASH_FILE_HEADER_SIZE );

            // Write file
            RCFS_Write( &f );

            // We are done
            return(RCFS_SUCCESS);
            }
        else
            {
            // Valid file found
            toc++;
            size = *toc++;

            // Last file in memory ?
            if( addr > maxaddr )
                {
                // maximum address found
                maxaddr = addr;
                // Address after this file
                nextaddr = addr + size;
                }
            }
        }

    // No more VTOC space if here
    return(RCFS_ERROR);
}

/*-----------------------------------------------------------------------------*/
/** @brief     Add a file to the file system                                   */
/** @param[in] data pointer to the data to be written                          */
/** @param[in] length length of data in bytes                                  */
/*-----------------------------------------------------------------------------*/
/** @details
 *  Add a file to the file system using the default filename
 */
int
RCFS_AddFile( unsigned char *data, int length )
{
    char name[16];
    int  slot;

    // Get the last file slot number
    slot = RCFS_FindLastSlot();

    // any room left ?
    if( slot < 0 )
        return(RCFS_ERROR);
    // create default filename
    sprintf( &name[0], "%s%03d", RCFS_BASENAME, slot);

    // Add this file
    return( RCFS_AddFile( data, length, name ) );
}

/*-----------------------------------------------------------------------------*/
/** @brief     Get a pointer to data in a file                                 */
/** @param[in] name the name of the file to open                               */
/** @param[in] data handle used to return a pointer to the data                */
/** @param[in] length pointer to returned length of data in bytes              */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This function searches through the file table of contents looking for a file
 *  with a name that matches the requested name.  It returns a pointer to the
 *  files data and it's length in words.
 */
int
RCFS_GetFile( char *name, unsigned char **data, int *length )
{
    flash_file  f;

    if( data == NULL )
        return(RCFS_ERROR);
    if( length == NULL )
        return(RCFS_ERROR);

    // Get the first file
    if( RCFS_FindFirstFile(&f) >= 0 )
        {
        do {
            // Check file for match on name
            if( strcmp( name, &f.name[0] ) == 0 )
                {
                // Match
                *data   = f.data;
                *length = f.datalength;
                return(RCFS_SUCCESS);
                }
            } while( RCFS_FindNextFile(&f) >= 0 );
        }

    // No match
    return( RCFS_ERROR );
}

/*-----------------------------------------------------------------------------*/
/** @brief     Get the name of the last file in the VTOC                       */
/*-----------------------------------------------------------------------------*/
/** @details   Use this if you write a file with the default name to dtermine
 *  what that name was.
 */

int
RCFS_GetLastFilename( char *name, int len )
{
    flash_file  f;

    if( name == NULL )
        return( RCFS_ERROR );

    if(len > 16)
        len = 16;

    // Get the first file
    if( RCFS_FindFirstFile(&f) >= 0 )
        {
        while( RCFS_FindNextFile(&f) >= 0 )
            len = len; // nothing to do, remove warning

        strncpy( name, &f.name[0], len );

        return(RCFS_SUCCESS);
        }

    return( RCFS_ERROR );
}
