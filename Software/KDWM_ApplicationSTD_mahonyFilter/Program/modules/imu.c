/*====================================================================================================*/
/*====================================================================================================*/
#include "drivers\stm32f4_system.h"
#include "drivers\stm32f4_spi.h"
#include "modules\imu.h"

#include <stdio.h>
#include <string.h>
/*====================================================================================================*/
/*====================================================================================================*/
#define IMU_SPIx                    SPI1
#define IMU_SPIx_CLK_ENABLE()       RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE)
#define IMU_SPIx_SPEED_HIGH         SPI_BaudRatePrescaler_2
#define IMU_SPIx_SPEED_LOW          SPI_BaudRatePrescaler_256

#define IMU_SCK_PIN                 GPIO_Pin_5
#define IMU_SCK_GPIO_PORT           GPIOA
#define IMU_SCK_AF                  GPIO_AF_SPI1
#define IMU_SCK_SOURCE              GPIO_PinSource5

#define IMU_SDO_PIN                 GPIO_Pin_6
#define IMU_SDO_GPIO_PORT           GPIOA
#define IMU_SDO_AF                  GPIO_AF_SPI1
#define IMU_SDO_SOURCE              GPIO_PinSource6

#define IMU_SDI_PIN                 GPIO_Pin_7
#define IMU_SDI_GPIO_PORT           GPIOA
#define IMU_SDI_AF                  GPIO_AF_SPI1
#define IMU_SDI_SOURCE              GPIO_PinSource7

#define IMU_CSM_PIN                 GPIO_Pin_4
#define IMU_CSM_GPIO_PORT           GPIOA
#define IMU_CSM_H()                 __GPIO_SET(IMU_CSM_GPIO_PORT, IMU_CSM_PIN)
#define IMU_CSM_L()                 __GPIO_RST(IMU_CSM_GPIO_PORT, IMU_CSM_PIN)

//#if defined(__USE_BAROMETER)
#define IMU_CSB_PIN                 GPIO_Pin_0
#define IMU_CSB_GPIO_PORT           GPIOB
#define IMU_CSB_H()                 __GPIO_SET(IMU_CSB_GPIO_PORT, IMU_CSB_PIN)
#define IMU_CSB_L()                 __GPIO_RST(IMU_CSB_GPIO_PORT, IMU_CSB_PIN)
//#endif

#define IMU_INTM_PIN                GPIO_Pin_1
#define IMU_INTM_GPIO_PORT          GPIOA
#define IMU_INTM_IRQ                EXTI1_IRQn
#define IMU_INTM_LINE               EXTI_Line1
#define IMU_INTM_SOURCE             GPIO_PinSource1

#if defined(__USE_BAROMETER)
#define IMU_INTB_PIN                GPIO_Pin_0
#define IMU_INTB_GPIO_PORT          GPIOA
#define IMU_INTB_IRQ                EXTI0_IRQn
#define IMU_INTB_LINE               EXTI_Line0
#define IMU_INTB_SOURCE             GPIO_PinSource0
#endif
/*====================================================================================================*/
/*====================================================================================================*/
IMU_DataTypeDef IMU;
/*====================================================================================================*/
/*====================================================================================================*
**函數 : IMU_SetSpeed
**功能 : Set SPI Speed
**輸入 : SpeedSel
**輸出 : None
**使用 : IMU_SetSpeed(SPIx_SPEED_LOW);
**====================================================================================================*/
/*====================================================================================================*/
static void IMU_SetSpeed( uint8_t speedSel )
{
  SPI_InitTypeDef SPI_InitStruct;

  SPI_I2S_DeInit(IMU_SPIx);

  SPI_InitStruct.SPI_Mode              = SPI_Mode_Master;
  SPI_InitStruct.SPI_Direction         = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStruct.SPI_DataSize          = SPI_DataSize_8b;
  SPI_InitStruct.SPI_CPOL              = SPI_CPOL_High;
  SPI_InitStruct.SPI_CPHA              = SPI_CPHA_2Edge;
  SPI_InitStruct.SPI_NSS               = SPI_NSS_Soft;
  SPI_InitStruct.SPI_BaudRatePrescaler = speedSel;
  SPI_InitStruct.SPI_FirstBit          = SPI_FirstBit_MSB;
  SPI_InitStruct.SPI_CRCPolynomial     = 7;
  SPI_Init(IMU_SPIx, &SPI_InitStruct);

  SPI_Cmd(IMU_SPIx, ENABLE); 
}
/*====================================================================================================*/
/*====================================================================================================*
**函數 : IMU_Config
**功能 : IMU Config
**輸入 : None
**輸出 : None
**使用 : IMU_Config();
**====================================================================================================*/
/*====================================================================================================*/
void IMU_Config( void )
{
  GPIO_InitTypeDef GPIO_InitStruct;
  SPI_InitTypeDef SPI_InitStruct;
//  NVIC_InitTypeDef NVIC_InitStruct;
//  EXTI_InitTypeDef EXTI_InitStruct;

  /* SPI Clk ******************************************************************/
  IMU_SPIx_CLK_ENABLE();

  /* SPI AF *******************************************************************/
  GPIO_PinAFConfig(IMU_SCK_GPIO_PORT, IMU_SCK_SOURCE, IMU_SCK_AF);
  GPIO_PinAFConfig(IMU_SDO_GPIO_PORT, IMU_SDO_SOURCE, IMU_SDO_AF);    
  GPIO_PinAFConfig(IMU_SDI_GPIO_PORT, IMU_SDI_SOURCE, IMU_SDI_AF);

  /* SPI Pin ******************************************************************/
  GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_UP;

  GPIO_InitStruct.GPIO_Pin   = IMU_CSM_PIN;
  GPIO_Init(IMU_CSM_GPIO_PORT, &GPIO_InitStruct);

//#if defined(__USE_BAROMETER)
  GPIO_InitStruct.GPIO_Pin   = IMU_CSB_PIN;
  GPIO_Init(IMU_CSB_GPIO_PORT, &GPIO_InitStruct);
//#endif

  GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_UP;

  GPIO_InitStruct.GPIO_Pin   = IMU_SCK_PIN;
  GPIO_Init(IMU_SCK_GPIO_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.GPIO_Pin   = IMU_SDO_PIN;
  GPIO_Init(IMU_SDO_GPIO_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.GPIO_Pin   = IMU_SDI_PIN;
  GPIO_Init(IMU_SDI_GPIO_PORT, &GPIO_InitStruct);

//  GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IN;
//  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
//  GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
//  GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;

//  GPIO_InitStruct.GPIO_Pin   = IMU_INTM_PIN;
//  GPIO_Init(IMU_INTM_GPIO_PORT, &GPIO_InitStruct);

//  GPIO_InitStruct.GPIO_Pin   = IMU_INTB_PIN;
//  GPIO_Init(IMU_INTB_GPIO_PORT, &GPIO_InitStruct);

  IMU_CSM_H();  // LOW ENABLE
//#if defined(__USE_BAROMETER)
  IMU_CSB_H();  // LOW ENABLE
//#endif

  /* INT NVIC ****************************************************************/
//  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
//  NVIC_InitStruct.NVIC_IRQChannel                   = SPIx_INT_IRQ;
//  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
//  NVIC_InitStruct.NVIC_IRQChannelSubPriority        = 0;
//  NVIC_InitStruct.NVIC_IRQChannelCmd                = ENABLE;
//  NVIC_Init(&NVIC_InitStruct);

  /* INT EXTI ****************************************************************/
//  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, SPIx_INT_SOURCE);
//  EXTI_InitStruct.EXTI_Line    = SPIx_INT_LINE;
//  EXTI_InitStruct.EXTI_Mode    = EXTI_Mode_Interrupt;
//  EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;  
//  EXTI_InitStruct.EXTI_LineCmd = ENABLE;
//  EXTI_Init(&EXTI_InitStruct);

  /* SPI Init ****************************************************************/
  SPI_InitStruct.SPI_Mode              = SPI_Mode_Master;
  SPI_InitStruct.SPI_Direction         = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStruct.SPI_DataSize          = SPI_DataSize_8b;
  SPI_InitStruct.SPI_CPOL              = SPI_CPOL_High;
  SPI_InitStruct.SPI_CPHA              = SPI_CPHA_2Edge;
  SPI_InitStruct.SPI_NSS               = SPI_NSS_Soft;
  SPI_InitStruct.SPI_BaudRatePrescaler = IMU_SPIx_SPEED_LOW;
  SPI_InitStruct.SPI_FirstBit          = SPI_FirstBit_MSB;
  SPI_InitStruct.SPI_CRCPolynomial     = 7;
  SPI_Init(IMU_SPIx, &SPI_InitStruct);

  SPI_Cmd(IMU_SPIx, ENABLE); 
}
/*====================================================================================================*/
/*====================================================================================================*
**函數 : IMU_Init
**功能 : IMU Init
**輸入 : None
**輸出 : None
**使用 : status = IMU_Init(&IMUx_InitStruct);
**====================================================================================================*/
/*====================================================================================================*/
#define MPU6500_InitRegNum 11
#define AK8963_InitRegNum  5
#define LPS25H_InitRegNum  2
int8_t IMU_Init( IMU_InitTypeDef *IMUx )
{
  int8_t status = ERROR;
  uint8_t MPU6500_InitData[MPU6500_InitRegNum][2] = {
    {0x80, MPU6500_PWR_MGMT_1},     /* [0]  Reset Device                  */
    {0x04, MPU6500_PWR_MGMT_1},     /* [1]  Clock Source                  */
    {0x10, MPU6500_INT_PIN_CFG},    /* [2]  Set INT_ANYRD_2CLEAR          */
    {0x01, MPU6500_INT_ENABLE},     /* [3]  Set RAW_RDY_EN                */
    {0x00, MPU6500_PWR_MGMT_2},     /* [4]  Enable Acc & Gyro             */
    {0x00, MPU6500_SMPLRT_DIV},     /* [5]  Sample Rate Divider           */
    {0x00, MPU6500_GYRO_CONFIG},    /* [6]  default : +-250dps            */
    {0x00, MPU6500_ACCEL_CONFIG},   /* [7]  default : +-2G                */
    {0x00, MPU6500_CONFIG},         /* [8]  default : GyrLPS_250Hz        */
    {0x00, MPU6500_ACCEL_CONFIG_2}, /* [9]  default : AccLPS_460Hz        */
    {0x30, MPU6500_USER_CTRL},      /* [10] Set I2C_MST_EN, I2C_IF_DIS    */
  };
  uint8_t AK8963_InitData[AK8963_InitRegNum][2] = {
    {0x01, AK8963_CNTL2},           /* [0]  Reset Device                  */
    {0x00, AK8963_CNTL1},           /* [1]  Power-down mode               */
    {0x0F, AK8963_CNTL1},           /* [2]  Fuse ROM access mode          */
                                    /*      Read sensitivity adjustment   */
    {0x00, AK8963_CNTL1},           /* [3]  Power-down mode               */
    {0x06, AK8963_CNTL1},           /* [4]  Continuous measurement mode 2 */
  };

  IMU_InitData(IMUx->Data);
  IMU_SetSensitivity(IMUx);

  MPU6500_InitData[6][0] = IMUx->Init.MPU_Gyr_FullScale;       /* [6] MPU6500_GYRO_CONFIG */
  MPU6500_InitData[7][0] = IMUx->Init.MPU_Acc_FullScale;       /* [7] MPU6500_ACCEL_CONFIG */
  MPU6500_InitData[8][0] = IMUx->Init.MPU_Gyr_LowPassFilter;   /* [8] MPU6500_CONFIG */
  MPU6500_InitData[9][0] = IMUx->Init.MPU_Acc_LowPassFilter;   /* [9] MPU6500_ACCEL_CONFIG_2 */

  for(uint32_t i = 0; i < MPU6500_InitRegNum; i++) {
    delay_ms(2);
    MPU9250_WriteReg(MPU6500_InitData[i][1], MPU6500_InitData[i][0]);
  }

  delay_ms(2);
  status = IMU_DeviceCheck(IMUx->Data);
  if(status != SUCCESS)
    return ERROR;

#if defined(__USE_MAGNETOMETER)
  uint8_t ASA[3] = {0};
  float32_t sensAdj[3] = {0};

  AK8963_InitData[4][0] |= IMUx->Init.MPU_Mag_FullScale;        /* [4] AK8963_CNTL1 */

  for(uint32_t i = 0; i < AK8963_InitRegNum; i++) {
    delay_ms(2);
    MPU9250_Mag_WriteReg(AK8963_InitData[i][1], AK8963_InitData[i][0]);
    if(i == 2) {  /* Read sensitivity adjustment values */
      delay_ms(2);
      MPU9250_Mag_ReadRegs(AK8963_ASAX, ASA, 3);
    }
  }

  for(uint8_t i = 0; i < 3; i++) {
    sensAdj[i] = (ASA[i] + 128) / 256.0;
    IMUx->Data->magScale[i] *= sensAdj[i];
  }
//  printf("ASA = %i, %i, %i\r\n", ASA[0], ASA[1], ASA[2]);
//  printf("sensAdj = %f, %f, %f\r\n", sensAdj[0], sensAdj[1], sensAdj[2]);

  delay_ms(2);
  MPU9250_WriteReg(MPU6500_I2C_MST_CTRL, 0x5D);
  delay_ms(2);
  MPU9250_WriteReg(MPU6500_I2C_SLV0_ADDR, AK8963_I2C_ADDR | 0x80);
  delay_ms(2);
  MPU9250_WriteReg(MPU6500_I2C_SLV0_REG, AK8963_ST1);
  delay_ms(2);
  MPU9250_WriteReg(MPU6500_I2C_SLV0_CTRL, MPU6500_I2C_SLVx_EN | 8);
  delay_ms(2);
  MPU9250_WriteReg(MPU6500_I2C_SLV4_CTRL, 0x09);
  delay_ms(2);
  MPU9250_WriteReg(MPU6500_I2C_MST_DELAY_CTRL, 0x81);
  delay_ms(100);
#endif

#if defined(__USE_BAROMETER)
//  for(uint8_t i = 0; i < LPS25H_InitRegNum; i++) {
//    Delay_1ms(2);
//    LPS25H_WriteReg(LPS25H_InitData[i][1], LPS25H_InitData[i][0]);
//  }
#endif

  IMU_SetSpeed(IMU_SPIx_SPEED_HIGH);
  delay_ms(10);

  return SUCCESS;
}
/*====================================================================================================*/
/*====================================================================================================*
**函數 : IMU_DeviceCheck
**功能 : Check Device ID
**輸入 : *pIMU
**輸出 : status
**使用 : status = IMU_DeviceCheck(pIMU);
**====================================================================================================*/
/*====================================================================================================*/
int8_t IMU_DeviceCheck( IMU_DataTypeDef *pIMU )
{
  uint8_t deviceID = 0x00;

  deviceID = MPU9250_ReadReg(MPU6500_WHO_AM_I);
  if(deviceID != MPU6500_DeviceID) {
    return ERROR;
  }

#if defined(__USE_MAGNETOMETER)
  deviceID = MPU9250_Mag_ReadReg(AK8963_WIA);
  if(deviceID != AK8963_DeviceID) {
    return ERROR;
  }
#endif

#if defined(__USE_BAROMETER)
  deviceID = LPS25HB_ReadReg(LPS25HB_WHO_AM_I);
  if(deviceID != LPS25HB_DeviceID) {
    return ERROR;
  }
#endif

  return SUCCESS;
}
/*====================================================================================================*/
/*====================================================================================================*
**函數 : IMU_InitData
**功能 : Set to default
**輸入 : *pIMU
**輸出 : None
**使用 : IMU_InitData(IMU);
**====================================================================================================*/
/*====================================================================================================*/
void IMU_InitData( IMU_DataTypeDef *pIMU )
{
  memset(pIMU, 0, sizeof(IMU_DataTypeDef));

  pIMU->gyrCalib[0] = 1.0f;
  pIMU->gyrCalib[1] = 1.0f;
  pIMU->gyrCalib[2] = 1.0f;

  pIMU->accStrength = 1.0f;
  pIMU->accCalib[0] = 1.0f;
  pIMU->accCalib[4] = 1.0f;
  pIMU->accCalib[8] = 1.0f;

  pIMU->magStrength = 1.0f;
  pIMU->magCalib[0] = 1.0f;
  pIMU->magCalib[4] = 1.0f;
  pIMU->magCalib[8] = 1.0f;
}
/*====================================================================================================*/
/*====================================================================================================*
**函數 : IMU_SetSensitivity
**功能 : Set Sensitivity
**輸入 : *IMUx
**輸出 : None
**使用 : IMU_SetSensitivity(IMUx);
**====================================================================================================*/
/*====================================================================================================*/
void IMU_SetSensitivity( IMU_InitTypeDef *IMUx )
{
  float64_t scale = 0.0;

  /* Set gyroscope sensitivity (dps/LSB) */
#if defined(__USE_GYROSCOPE)
  switch(IMUx->Init.MPU_Gyr_FullScale) {
    case MPU_GyrFS_250dps:    scale = 250.0;    break;
    case MPU_GyrFS_500dps:    scale = 500.0;    break;
    case MPU_GyrFS_1000dps:   scale = 1000.0;   break;
    case MPU_GyrFS_2000dps:   scale = 2000.0;   break;
    default:                  scale = 0.0;      break;
  }
  scale = scale / 32768.0;
  IMUx->Data->gyrScale[0] = scale;
  IMUx->Data->gyrScale[1] = scale;
  IMUx->Data->gyrScale[2] = scale;
#else
  IMUx->Data->gyrScale[0] = 0.0f;
  IMUx->Data->gyrScale[1] = 0.0f;
  IMUx->Data->gyrScale[2] = 0.0f;
#endif

  /* Set accelerometer sensitivity (g/LSB) */
#if defined(__USE_ACCELEROMETER)
  switch(IMUx->Init.MPU_Acc_FullScale) {
    case MPU_AccFS_2g:    scale = 2.0;    break;
    case MPU_AccFS_4g:    scale = 4.0;    break;
    case MPU_AccFS_8g:    scale = 8.0;    break;
    case MPU_AccFS_16g:   scale = 16.0;   break;
    default:              scale = 0.0f;   break;
  }
  scale = scale / 32768.0;
  IMUx->Data->accScale[0] = scale;
  IMUx->Data->accScale[1] = scale;
  IMUx->Data->accScale[2] = scale;
#else
  IMUx->Data->accScale[0] = 0.0f;
  IMUx->Data->accScale[1] = 0.0f;
  IMUx->Data->accScale[2] = 0.0f;
#endif

  /* Set magnetometer sensitivity (uT/LSB) */
#if defined(__USE_MAGNETOMETER)
  IMUx->Data->magScale[0] = 0.6;  /* +-4800uT */
  IMUx->Data->magScale[1] = 0.6;
  IMUx->Data->magScale[2] = 0.6;
#else
  IMUx->Data->magScale[0] = 0.0f;
  IMUx->Data->magScale[1] = 0.0f;
  IMUx->Data->magScale[2] = 0.0f;
#endif

  /* Set ictemperature sensitivity (degC/LSB) */
#if defined(__USE_ICTEMPERATURE)
  IMUx->Data->ictempScale  = 1.0 / 333.87;
  IMUx->Data->ictempOffset = 21.0f;
#else
  IMUx->Data->ictempScale  = 0.0f;
  IMUx->Data->ictempOffset = 0.0f;
#endif

  /* Set barometer sensitivity (degC/LSB, hPa/LSB) */
#if defined(__USE_BAROMETER)
  IMUx->Data->baroScale[0] = LPS25HB_T_degC;
  IMUx->Data->baroScale[1] = LPS25HB_P_hPa;
#else
  IMUx->Data->baroScale[0] = 0.0f;
  IMUx->Data->baroScale[1] = 0.0f;
#endif
}
/*====================================================================================================*/
/*====================================================================================================*
**函數 : IMU_GetRawData
**功能 : Get IMU Data
**輸入 : *pIMU
**輸出 : None
**使用 : IMU_GetRawData(pIMU);
**====================================================================================================*/
/*====================================================================================================*/
void IMU_GetRawData( IMU_DataTypeDef *pIMU )
{
  uint8_t readBuf[22] = {0};

#if defined(__USE_MAGNETOMETER)
  MPU9250_ReadRegs(MPU6500_ACCEL_XOUT_H, readBuf, 22);    /* Read Gyr, Acc, Mag */
#else
  MPU9250_ReadRegs(MPU6500_ACCEL_XOUT_H, readBuf, 14);    /* Read Gyr, Acc */
#endif

  pIMU->gyrRaw[0] = (int16_t)(readBuf[8]  << 8) | readBuf[9];     /* Gyr.X */
  pIMU->gyrRaw[1] = (int16_t)(readBuf[10] << 8) | readBuf[11];    /* Gyr.Y */
  pIMU->gyrRaw[2] = (int16_t)(readBuf[12] << 8) | readBuf[13];    /* Gyr.Z */
  pIMU->accRaw[0] = (int16_t)(readBuf[0]  << 8) | readBuf[1];     /* Acc.X */
  pIMU->accRaw[1] = (int16_t)(readBuf[2]  << 8) | readBuf[3];     /* Acc.Y */
  pIMU->accRaw[2] = (int16_t)(readBuf[4]  << 8) | readBuf[5];     /* Acc.Z */
  pIMU->ictempRaw = (int16_t)(readBuf[6]  << 8) | readBuf[7];     /* ICTemp */

#if defined(__USE_MAGNETOMETER)
  if(!(!(readBuf[14] & AK8963_STATUS_DRDY) || (readBuf[14] & AK8963_STATUS_DOR) || (readBuf[21] & AK8963_STATUS_HOFL))) {
    pIMU->magRaw[0] = (int16_t)(readBuf[16] << 8) | readBuf[15];  /* Mag.X */
    pIMU->magRaw[1] = (int16_t)(readBuf[18] << 8) | readBuf[17];  /* Mag.Y */
    pIMU->magRaw[2] = (int16_t)(readBuf[20] << 8) | readBuf[19];  /* Mag.Z */
  }
#endif

#if defined(__USE_BAROMETER)
//  LPS25H_ReadReg(LPS25H_WHO_AM_I, );
#endif
}
/*====================================================================================================*/
/*====================================================================================================*
**函數 : IMU_GetCalibData
**功能 : Get Calibration Data
**輸入 : *pIMU
**輸出 : None
**使用 : IMU_GetCalibData(pIMU);
**====================================================================================================*/
/*====================================================================================================*/
void IMU_GetCalibData( IMU_DataTypeDef *pIMU )
{
  float32_t tmp[3] = {0};

  IMU_GetRawData(pIMU);

  pIMU->gyrData[0] = (pIMU->gyrRaw[0] - pIMU->gyrOffset[0]) * pIMU->gyrCalib[0];   /* Gyr.X */
  pIMU->gyrData[1] = (pIMU->gyrRaw[1] - pIMU->gyrOffset[1]) * pIMU->gyrCalib[1];   /* Gyr.Y */
  pIMU->gyrData[2] = (pIMU->gyrRaw[2] - pIMU->gyrOffset[2]) * pIMU->gyrCalib[2];   /* Gyr.Z */

  pIMU->accData[0] = (pIMU->accCalib[0] * pIMU->accRaw[0] + pIMU->accCalib[1] * pIMU->accRaw[1] + pIMU->accCalib[2] * pIMU->accRaw[2]) + pIMU->accOffset[0];  /* Acc.X */
  pIMU->accData[1] = (pIMU->accCalib[3] * pIMU->accRaw[0] + pIMU->accCalib[4] * pIMU->accRaw[1] + pIMU->accCalib[5] * pIMU->accRaw[2]) + pIMU->accOffset[1];  /* Acc.X */
  pIMU->accData[2] = (pIMU->accCalib[6] * pIMU->accRaw[0] + pIMU->accCalib[7] * pIMU->accRaw[1] + pIMU->accCalib[8] * pIMU->accRaw[2]) + pIMU->accOffset[2];  /* Acc.X */

  pIMU->ictempData = pIMU->ictempRaw * pIMU->ictempScale + pIMU->ictempOffset;

#if defined(__USE_MAGNETOMETER)
  tmp[0] = pIMU->magRaw[0] - pIMU->magOffset[0];   /* Mag.X */
  tmp[1] = pIMU->magRaw[1] - pIMU->magOffset[1];   /* Mag.Y */
  tmp[2] = pIMU->magRaw[2] - pIMU->magOffset[2];   /* Mag.Z */

  pIMU->magData[0] = pIMU->magCalib[0] * tmp[0] + pIMU->magCalib[1] * tmp[1] + pIMU->magCalib[2] * tmp[2];
  pIMU->magData[1] = pIMU->magCalib[3] * tmp[0] + pIMU->magCalib[4] * tmp[1] + pIMU->magCalib[5] * tmp[2];
  pIMU->magData[2] = pIMU->magCalib[6] * tmp[0] + pIMU->magCalib[7] * tmp[1] + pIMU->magCalib[8] * tmp[2];
#endif

#if defined(__USE_BAROMETER)
    pIMU->baroData[0] = pIMU->baroData[0] - pIMU->TempOffset;   /* Baro.T */
    pIMU->baroData[1] = pIMU->baroData[1] - pIMU->PresOffset;   /* Baro.P */
#endif
}
/*====================================================================================================*/
/*====================================================================================================*
**函數 : IMU_MergeScaleCalib
**功能 : Merge scale and calibration
**輸入 : *pIMU
**輸出 : None
**使用 : IMU_MergeScaleCalib(pIMU);
**====================================================================================================*/
/*====================================================================================================*/
void IMU_MergeScaleCalib( IMU_DataTypeDef *pIMU )
{
  /* Merge gyroscope scale and calibration (dps/LSB) */
  pIMU->gyrCalib[0] = pIMU->gyrCalib[0] * pIMU->gyrScale[0];
  pIMU->gyrCalib[1] = pIMU->gyrCalib[1] * pIMU->gyrScale[1];
  pIMU->gyrCalib[2] = pIMU->gyrCalib[2] * pIMU->gyrScale[2];

  /* Merge accelerometer scale and sensitivity (g/LSB) */
  pIMU->accCalib[0] = pIMU->accCalib[0] * pIMU->accScale[0];
  pIMU->accCalib[1] = pIMU->accCalib[1] * pIMU->accScale[1];
  pIMU->accCalib[2] = pIMU->accCalib[2] * pIMU->accScale[2];
  pIMU->accCalib[3] = pIMU->accCalib[3] * pIMU->accScale[0];
  pIMU->accCalib[4] = pIMU->accCalib[4] * pIMU->accScale[1];
  pIMU->accCalib[5] = pIMU->accCalib[5] * pIMU->accScale[2];
  pIMU->accCalib[6] = pIMU->accCalib[6] * pIMU->accScale[0];
  pIMU->accCalib[7] = pIMU->accCalib[7] * pIMU->accScale[1];
  pIMU->accCalib[8] = pIMU->accCalib[8] * pIMU->accScale[2];

  /* Merge magnetometer scale and sensitivity (uT/LSB) */
  pIMU->magCalib[0] = pIMU->magCalib[0] * pIMU->magScale[0];
  pIMU->magCalib[1] = pIMU->magCalib[1] * pIMU->magScale[1];
  pIMU->magCalib[2] = pIMU->magCalib[2] * pIMU->magScale[2];
  pIMU->magCalib[3] = pIMU->magCalib[3] * pIMU->magScale[0];
  pIMU->magCalib[4] = pIMU->magCalib[4] * pIMU->magScale[1];
  pIMU->magCalib[5] = pIMU->magCalib[5] * pIMU->magScale[2];
  pIMU->magCalib[6] = pIMU->magCalib[6] * pIMU->magScale[0];
  pIMU->magCalib[7] = pIMU->magCalib[7] * pIMU->magScale[1];
  pIMU->magCalib[8] = pIMU->magCalib[8] * pIMU->magScale[2];
}
/*====================================================================================================*/
/*====================================================================================================*
**函數 : IMU_PrintData
**功能 : Print Data
**輸入 : *pIMU
**輸出 : None
**使用 : IMU_PrintData(IMU);
**====================================================================================================*/
/*====================================================================================================*/
void IMU_PrintData( IMU_DataTypeDef *pIMU )
{
  printf("\r\n");
  printf("- Print IMU Data -----------------\r\n");
  printf("G_raw : %f, %f, %f\r\n", pIMU->gyrRaw[0], pIMU->gyrRaw[1], pIMU->gyrRaw[2]);
  printf("A_raw : %f, %f, %f\r\n", pIMU->accRaw[0], pIMU->accRaw[1], pIMU->accRaw[2]);
#if defined(__USE_MAGNETOMETER)
  printf("M_raw : %f, %f, %f\r\n", pIMU->magRaw[0], pIMU->magRaw[1], pIMU->magRaw[2]);
#endif
  printf("T_raw : %f\r\n", pIMU->ictempRaw);
#if defined(__USE_BAROMETER)
  printf("B_raw : %f, %f\r\n", pIMU->baroRaw[0], pIMU->baroRaw[1]);
#endif

  printf("\r\n");
  printf("G_offset : %f, %f, %f\r\n", pIMU->gyrOffset[0], pIMU->gyrOffset[1], pIMU->gyrOffset[2]);
  printf("A_offset : %f, %f, %f\r\n", pIMU->accOffset[0], pIMU->accOffset[1], pIMU->accOffset[2]);
#if defined(__USE_MAGNETOMETER)
  printf("M_offset : %f, %f, %f\r\n", pIMU->magOffset[0], pIMU->magOffset[1], pIMU->magOffset[2]);
#endif
  printf("T_offset : %f\r\n", pIMU->ictempOffset);

  printf("\r\n");
  printf("G_data : %f, %f, %f\r\n", pIMU->gyrData[0], pIMU->gyrData[1], pIMU->gyrData[2]);
  printf("A_data : %f, %f, %f\r\n", pIMU->accData[0], pIMU->accData[1], pIMU->accData[2]);
#if defined(__USE_MAGNETOMETER)
  printf("M_data : %f, %f, %f\r\n", pIMU->magData[0], pIMU->magData[1], pIMU->magData[2]);
#endif
  printf("T_data : %f\r\n", pIMU->ictempData);
#if defined(__USE_BAROMETER)
  printf("B_data : %f, %f\r\n", pIMU->baroData[0], pIMU->baroData[1]);
#endif

  printf("\r\n");
  printf("G_calib : %f, %f, %f\r\n", pIMU->gyrCalib[0], pIMU->gyrCalib[1], pIMU->gyrCalib[2]);
  printf("A_calib : %f, %f, %f\r\n", pIMU->accCalib[0], pIMU->accCalib[1], pIMU->accCalib[2]);
  printf("          %f, %f, %f\r\n", pIMU->accCalib[3], pIMU->accCalib[4], pIMU->accCalib[5]);
  printf("          %f, %f, %f\r\n", pIMU->accCalib[6], pIMU->accCalib[7], pIMU->accCalib[8]);
#if defined(__USE_MAGNETOMETER)
  printf("M_calib : %f, %f, %f\r\n", pIMU->magCalib[0], pIMU->magCalib[1], pIMU->magCalib[2]);
  printf("          %f, %f, %f\r\n", pIMU->magCalib[3], pIMU->magCalib[4], pIMU->magCalib[5]);
  printf("          %f, %f, %f\r\n", pIMU->magCalib[6], pIMU->magCalib[7], pIMU->magCalib[8]);
#endif

  printf("\r\n");
  printf("G_scale : %f, %f, %f\r\n", pIMU->gyrScale[0], pIMU->gyrScale[1], pIMU->gyrScale[2]);
  printf("A_scale : %f, %f, %f\r\n", pIMU->accScale[0], pIMU->accScale[1], pIMU->accScale[2]);
#if defined(__USE_MAGNETOMETER)
  printf("M_scale : %f, %f, %f\r\n", pIMU->magScale[0], pIMU->magScale[1], pIMU->magScale[2]);
#endif
  printf("T_scale : %f\r\n", pIMU->ictempScale);
#if defined(__USE_BAROMETER)
  printf("B_scale : %f, %f\r\n", pIMU->baroScale[0], pIMU->baroScale[1]);
#endif

  printf("\r\n");
#if defined(__USE_MAGNETOMETER)
  printf("A_strength : %f\r\n", pIMU->accStrength);
  printf("M_strength : %f\r\n", pIMU->magStrength);
#endif

  printf("----------------------------------\r\n");
  printf("\r\n\r\n");
}
/*==============================================================================================*/
/*==============================================================================================*/
