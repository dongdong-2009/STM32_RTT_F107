/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-02-22 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/
#ifndef __W25Q64FLASH_H__
#define __W25Q64FLASH_H__

/*****************************************************************************/
/*                                                                           */
/*  Definitions                                                              */
/*                                                                           */
/*****************************************************************************/
typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned int U32;

/*****************************************************************************/
/*                                                                           */
/*  Function Declarations                                                    */
/*                                                                           */
/*****************************************************************************/
void SPI_Send_Byte(U8 out);
U8 SPI_Get_Byte(void);
void SPI_Wait_Busy(void);
void SPI_Write_Enable(void);		//дʹ��
void SPI_Read_nBytes(U32 Dst_Addr, U8 nBytes,U8 *header);//��flashоƬ��ȡ����
void SPI_Write_nBytes(U32 Dst_Addr, U8 nBytes, U8 *header);//��flashоƬд����
void SPI_Erase_Block(U32 Dst_Addr);	//������
void SPI_Erase_Half_Block(U32 Dst_Addr);	//�������
void SPI_Erase_Sector(U32 Dst_Addr);		//��������
void SPI_Erase_All(void);			//��������оƬ
void SPI_erase(U32 address,U16 numbers,U8 mode);
void SPI_WriteW25X_Disable(void);
void SPI_init(void);
#endif /*__W25Q64FLASH_H__*/
