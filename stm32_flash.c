/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                 2012-2014                                   */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     stm32_flash.c                                                */
/*    Author:     James Pearman                                                */
/*    Created:    21 August 2012                                               */
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
/*                                                                             */
/*        Description:                                                         */
/*                                                                             */
/*        ROBOTC port of stm32 peripheral library                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/

typedef unsigned long  uint32_t;
typedef unsigned short uint16_t;

typedef struct
{
    uint32_t ACR;
    uint32_t KEYR;
    uint32_t OPTKEYR;
    uint32_t SR;
    uint32_t CR;
    uint32_t AR;
    uint32_t RESERVED;
    uint32_t OBR;
    uint32_t WRPR;
} FLASH_TypeDef;

#define FLASH_BASE                  (0x08000000)        /*!< FLASH base address in the alias region */
#define SRAM_BASE                   (0x20000000)        /*!< SRAM base address in the alias region */
#define PERIPH_BASE                 (0x40000000)        /*!< Peripheral base address in the alias region */
#define APB1PERIPH_BASE             PERIPH_BASE
#define APB2PERIPH_BASE             (PERIPH_BASE + 0x10000)
#define AHBPERIPH_BASE              (PERIPH_BASE + 0x20000)
#define FLASH_R_BASE                (AHBPERIPH_BASE + 0x2000) /*!< Flash registers base address */
#define FLASH                       ((FLASH_TypeDef *) FLASH_R_BASE)

#define FLASH_FLAG_BSY              (0x00000001)        /*!< FLASH Busy flag */
#define FLASH_FLAG_EOP              (0x00000020)        /*!< FLASH End of Operation flag */
#define FLASH_FLAG_PGERR            (0x00000004)        /*!< FLASH Program error flag */
#define FLASH_FLAG_WRPRTERR         (0x00000010)        /*!< FLASH Write protected error flag */
#define FLASH_FLAG_OPTERR           (0x00000001)        /*!< FLASH Option Byte error flag */

#define FLASH_FLAG_BANK1_BSY        FLASH_FLAG_BSY      /*!< FLASH BANK1 Busy flag*/
#define FLASH_FLAG_BANK1_EOP        FLASH_FLAG_EOP      /*!< FLASH BANK1 End of Operation flag */
#define FLASH_FLAG_BANK1_PGERR      FLASH_FLAG_PGERR    /*!< FLASH BANK1 Program error flag */
#define FLASH_FLAG_BANK1_WRPRTERR   FLASH_FLAG_WRPRTERR /*!< FLASH BANK1 Write protected error flag */

#define IS_FLASH_CLEAR_FLAG(FLAG)   ((((FLAG) & (uint32_t)0xFFFFFFCA) == 0x00000000) && ((FLAG) != 0x00000000))
#define IS_FLASH_GET_FLAG(FLAG)       ((FLAG) == FLASH_FLAG_BSY) || ((FLAG) == FLASH_FLAG_EOP) || \
                                      ((FLAG) == FLASH_FLAG_PGERR) || ((FLAG) == FLASH_FLAG_WRPRTERR) || \
								      ((FLAG) == FLASH_FLAG_BANK1_BSY) || ((FLAG) == FLASH_FLAG_BANK1_EOP) || \
                                      ((FLAG) == FLASH_FLAG_BANK1_PGERR) || ((FLAG) == FLASH_FLAG_BANK1_WRPRTERR) || \
                                      ((FLAG) == FLASH_FLAG_OPTERR))

/** @defgroup FLASH_Private_Defines
  * @{
  */

/* Flash Access Control Register bits */
#define ACR_LATENCY_Mask            (0x00000038)
#define ACR_HLFCYA_Mask             (0xFFFFFFF7)
#define ACR_PRFTBE_Mask             (0xFFFFFFEF)

/* Flash Access Control Register bits */
#define ACR_PRFTBS_Mask             (0x00000020)

/* Flash Control Register bits */
#define CR_PG_Set                   (0x00000001)
#define CR_PG_Reset                 (0x00001FFE)
#define CR_PER_Set                  (0x00000002)
#define CR_PER_Reset                (0x00001FFD)
#define CR_MER_Set                  (0x00000004)
#define CR_MER_Reset                (0x00001FFB)
#define CR_OPTPG_Set                (0x00000010)
#define CR_OPTPG_Reset              (0x00001FEF)
#define CR_OPTER_Set                (0x00000020)
#define CR_OPTER_Reset              (0x00001FDF)
#define CR_STRT_Set                 (0x00000040)
#define CR_LOCK_Set                 (0x00000080)

/* FLASH Mask */
#define RDPRT_Mask                  (0x00000002)
#define WRP0_Mask                   (0x000000FF)
#define WRP1_Mask                   (0x0000FF00)
#define WRP2_Mask                   (0x00FF0000)
#define WRP3_Mask                   (0xFF000000)
#define OB_USER_BFB2                ((uint16_t)0x0008)

/* FLASH Keys */
#define RDP_Key                     ((uint16_t)0x00A5)
#define FLASH_KEY1                  (0x45670123)
#define FLASH_KEY2                  (0xCDEF89AB)

/* FLASH BANK address */
#define FLASH_BANK1_END_ADDRESS     (0x807FFFF)

/* Delay definition */
#define EraseTimeout                (0x000B0000)
#define ProgramTimeout              (0x00002000)

// V4 changed the way enums are handled
#if kRobotCVersionNumeric < 400
typedef enum
#else
typedef enum _FLASH_Status
#endif
{
  FLASH_BUSY = 1,
  FLASH_ERROR_PG,
  FLASH_ERROR_WRP,
  FLASH_COMPLETE,
  FLASH_TIMEOUT
}FLASH_Status;


// For user functions, these are not in the stm32 library
#define FLASH_ERROR_WRITE         (-1)
#define FLASH_ERROR_WRITE_LIMIT   (-2)
#define FLASH_ERROR_ERASE         (-3)
#define FLASH_ERROR_ERASE_LIMIT   (-4)



/**
  * @brief  Returns the FLASH Bank1 Status.
  * @note   This function can be used for all STM32F10x devices, it is equivalent
  *         to FLASH_GetStatus function.
  * @param  None
  * @retval FLASH Status: The returned value can be: FLASH_BUSY, FLASH_ERROR_PG,
  *         FLASH_ERROR_WRP or FLASH_COMPLETE
  */

FLASH_Status FLASH_GetBank1Status(void)
{
  FLASH_Status flashstatus = FLASH_COMPLETE;
  FLASH_TypeDef   *f = FLASH;
  long      tmp;

  tmp = f->SR;

  if((tmp & FLASH_FLAG_BANK1_BSY) == FLASH_FLAG_BSY)
  {
    flashstatus = FLASH_BUSY;
  }
  else
  {
    if((tmp & FLASH_FLAG_BANK1_PGERR) != 0)
    {
      flashstatus = FLASH_ERROR_PG;
    }
    else
    {
      if((tmp & FLASH_FLAG_BANK1_WRPRTERR) != 0 )
      {
        flashstatus = FLASH_ERROR_WRP;
      }
      else
      {
        flashstatus = FLASH_COMPLETE;
      }
    }
  }
  /* Return the Flash Status */
  return flashstatus;
}

/**
  * @brief  Waits for a Flash operation to complete or a TIMEOUT to occur.
  * @note   This function can be used for all STM32F10x devices,
  *         it is equivalent to FLASH_WaitForLastBank1Operation.
  *         - For STM32F10X_XL devices this function waits for a Bank1 Flash operation
  *           to complete or a TIMEOUT to occur.
  *         - For all other devices it waits for a Flash operation to complete
  *           or a TIMEOUT to occur.
  * @param  Timeout: FLASH programming Timeout
  * @retval FLASH Status: The returned value can be: FLASH_ERROR_PG,
  *         FLASH_ERROR_WRP, FLASH_COMPLETE or FLASH_TIMEOUT.
  */

FLASH_Status FLASH_WaitForLastOperation(uint32_t Timeout)
{
  FLASH_Status status = FLASH_COMPLETE;

  /* Check for the Flash Status */
  status = FLASH_GetBank1Status();
  /* Wait for a Flash operation to complete or a TIMEOUT to occur */
  while((status == FLASH_BUSY) && (Timeout != 0x00))
  {
    status = FLASH_GetBank1Status();
    Timeout--;
  }
  if(Timeout == 0x00 )
  {
    status = FLASH_TIMEOUT;
  }
  /* Return the operation status */
  return status;
}

/**
  * @brief  Erases a specified FLASH page.
  * @note   This function can be used for all STM32F10x devices.
  * @param  Page_Address: The page address to be erased.
  * @retval FLASH Status: The returned value can be: FLASH_BUSY, FLASH_ERROR_PG,
  *         FLASH_ERROR_WRP, FLASH_COMPLETE or FLASH_TIMEOUT.
  */
FLASH_Status FLASH_ErasePage(uint32_t Page_Address)
{
  FLASH_Status status = FLASH_COMPLETE;
  FLASH_TypeDef   *f = FLASH;

  /* Wait for last operation to be completed */
  status = FLASH_WaitForLastOperation(EraseTimeout);

  if(status == FLASH_COMPLETE)
  {
    /* if the previous operation is completed, proceed to erase the page */
    f->CR |= CR_PER_Set;
    f->AR  = Page_Address;
    f->CR |= CR_STRT_Set;

    /* Wait for last operation to be completed */
    status = FLASH_WaitForLastOperation(EraseTimeout);

    /* Disable the PER Bit */
    f->CR &= CR_PER_Reset;
  }

  /* Return the Erase Status */
  return status;
}

/**
  * @brief  Clears the FLASH's pending flags.
  * @note   This function can be used for all STM32F10x devices.
  *         - For STM32F10X_XL devices, this function clears Bank1 or Bank2ís pending flags
  *         - For other devices, it clears Bank1ís pending flags.
  * @param  FLASH_FLAG: specifies the FLASH flags to clear.
  *   This parameter can be any combination of the following values:
  *     @arg FLASH_FLAG_PGERR: FLASH Program error flag
  *     @arg FLASH_FLAG_WRPRTERR: FLASH Write protected error flag
  *     @arg FLASH_FLAG_EOP: FLASH End of Operation flag
  * @retval None
  */
void FLASH_ClearFlag(uint32_t FLASH_FLAG)
{
    FLASH_TypeDef   *f = FLASH;

    /* Clear the flags */
    f->SR = FLASH_FLAG;
}

/**
  * @brief  Programs a word at a specified address.
  * @note   This function can be used for all STM32F10x devices.
  * @param  Address: specifies the address to be programmed.
  * @param  Data: specifies the data to be programmed.
  * @retval FLASH Status: The returned value can be: FLASH_ERROR_PG,
  *         FLASH_ERROR_WRP, FLASH_COMPLETE or FLASH_TIMEOUT.
  */

FLASH_Status FLASH_ProgramWord(uint32_t Address, uint32_t Data)
{
  FLASH_Status status = FLASH_COMPLETE;
  FLASH_TypeDef   *f = FLASH;
  short *Addr = (short *)Address;

  /* Wait for last operation to be completed */
  status = FLASH_WaitForLastOperation(ProgramTimeout);

  if(status == FLASH_COMPLETE)
  {
    /* if the previous operation is completed, proceed to program the new first
    half word */
    f->CR |= CR_PG_Set;

    *Addr = (uint16_t)(Data & 0xFFFF);

    /* Wait for last operation to be completed */
    status = FLASH_WaitForLastOperation(ProgramTimeout);

    if(status == FLASH_COMPLETE)
    {
      /* if the previous operation is completed, proceed to program the new second
      half word */
      Addr++;

      *Addr = (uint16_t)(Data >> 16);

      /* Wait for last operation to be completed */
      status = FLASH_WaitForLastOperation(ProgramTimeout);

      /* Disable the PG Bit */
      f->CR &= CR_PG_Reset;
    }
    else
    {
        writeDebugStreamLine("status error %d", status );

      /* Disable the PG Bit */
      f->CR &= CR_PG_Reset;
    }
  }

  /* Return the Program Status */
  return status;
}

/**
  * @brief  Programs a half word at a specified address.
  * @note   This function can be used for all STM32F10x devices.
  * @param  Address: specifies the address to be programmed.
  * @param  Data: specifies the data to be programmed.
  * @retval FLASH Status: The returned value can be: FLASH_ERROR_PG,
  *         FLASH_ERROR_WRP, FLASH_COMPLETE or FLASH_TIMEOUT.
  */
FLASH_Status FLASH_ProgramHalfWord(uint32_t Address, uint16_t Data)
{
  FLASH_Status status = FLASH_COMPLETE;
  FLASH_TypeDef   *f = FLASH;
  short *Addr = (short *)Address;

  /* Wait for last operation to be completed */
  status = FLASH_WaitForLastOperation(ProgramTimeout);

  if(status == FLASH_COMPLETE)
  {
    /* if the previous operation is completed, proceed to program the new first
    half word */
    f->CR |= CR_PG_Set;

    *Addr = Data;

    /* Wait for last operation to be completed */
    status = FLASH_WaitForLastOperation(ProgramTimeout);

    /* Disable the PG Bit */
    f->CR &= CR_PG_Reset;
  }

  /* Return the Program Status */
  return status;
}



/**
  * @brief  Unlocks the FLASH Bank1 Program Erase Controller.
  * @note   This function can be used for all STM32F10x devices.
  *         - For STM32F10X_XL devices this function unlocks Bank1.
  *         - For all other devices it unlocks Bank1 and it is
  *           equivalent to FLASH_Unlock function.
  * @param  None
  * @retval None
  */

void FLASH_UnlockBank1(void)
{
  FLASH_TypeDef   *f = FLASH;

  /* Authorize the FPEC of Bank1 Access */
  f->KEYR = FLASH_KEY1;
  f->KEYR = FLASH_KEY2;
}
