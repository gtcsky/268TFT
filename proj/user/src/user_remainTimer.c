/*
 * user_remainTimer.c
 *
 *  Created on: 2021年12月3日
 *      Author: Sky
 */

#include "user_remainTimer.h"
#include "command_center.h"
#include "display.h"
#include "log.h"

#if(REMAIN_TIME_DISPLAY==1)
volatile uint8 fIsUpdateRemainTimer = 0;
uint16 vRemainTimer = 0;
uint16 vRemainCheckTimes = 0;
uint8 vCwMaxTimer = 0;
volatile uint8 vCalcTime1s = 0;
static float battCompVoltArray[BATT_ARRAY_SIZE] = { 0 };
extern displayParamsStruct displayParams;
extern PcaDataStruct pcaDataStruct;
void powerLineAdjust(float *currentPower) {
	switch (displayParams.brightness) {
	case 100:
//		if (displayParams.colorTemperature != MAX_ColorTemp && displayParams.colorTemperature != MIN_ColorTemp)
//			*currentPower *= 0.97;
		break;
	case 95:
		break;
	case 90:
//		*currentPower *= 1.05;
		break;
	case 85:
//		*currentPower *= 1.05;
		break;
	case 80:
//		*currentPower *= 1.05;
		break;
	case 75:
//		*currentPower *= 1.05;
		break;
	case 70:
//		*currentPower *= 1.05;
		break;
	case 65:
//		*currentPower *= 1.05;
		break;
	case 60:
//		*currentPower *= 1.10;
		break;
	case 55:
//		*currentPower *= 1.05;
		break;
	case 50:
//		*currentPower *= 1.05;
		break;
	case 45:
//		*currentPower *= 1.05;
		break;
	case 40:
//		*currentPower *= 1.05;
		break;
	case 35:
//		*currentPower *= 1.05;
		break;
	case 30:
//		*currentPower *= 1.11;
		break;
	case 25:
		break;
	case 20:
		*currentPower *= 0.90;
		break;
	case 15:
		*currentPower *= 0.87;
		break;
	case 10:
		*currentPower *= 0.85;
		break;
	case 5:
		*currentPower *= 0.80;
		break;
	case 1:
		*currentPower *= 0.75;
		break;
	}
}
/***************************************************************************
 *
 * 剩余时间估算
 *
 * ignoreCompare:  TRUE忽略结果与原来值的比较,直接赋值
 * 				 FALSE:比较结果,新生成的值小于原来的值才赋值
 *
 ****************************************************************************/
uint16 calcRemainTime(uint8 ignoreCompare) {
	float vtSpeedCW = 0, vtSpeedMW = 0,vtRedSpeed,vtGreenSpeed,vtBlueSpeed;
	uint16 totalTimer = 0;
	if (getSysStatus() && displayParams.brightness) {					//LED ON
		float vCompensationVolt = getCompensationVolt();
		if (vCompensationVolt >= REMAIN_TIME_STEP5_THRESHOLD) {
			if (vCompensationVolt >= REMAIN_TIME_STEP6_THRESHOLD)
				vCwMaxTimer = REMAIN_TIME_STEP5_START + REMAIN_TIME_STEP5_LAST;
			else
				vCwMaxTimer = REMAIN_TIME_STEP5_START + (REMAIN_TIME_STEP5_LAST * (vCompensationVolt - REMAIN_TIME_STEP5_THRESHOLD) / REMAIN_TIME_STEP5_VOLT_LAST);
		} else if (vCompensationVolt >= REMAIN_TIME_STEP4_THRESHOLD) {
			vCwMaxTimer = REMAIN_TIME_STEP4_START + (REMAIN_TIME_STEP4_LAST * (vCompensationVolt - REMAIN_TIME_STEP4_THRESHOLD) / REMAIN_TIME_STEP4_VOLT_LAST);
		} else if (vCompensationVolt >= REMAIN_TIME_STEP3_THRESHOLD) {
			vCwMaxTimer = REMAIN_TIME_STEP3_START + (REMAIN_TIME_STEP3_LAST * (vCompensationVolt - REMAIN_TIME_STEP3_THRESHOLD) / REMAIN_TIME_STEP3_VOLT_LAST);
		} else if (vCompensationVolt >= REMAIN_TIME_STEP2_THRESHOLD) {
			vCwMaxTimer = REMAIN_TIME_STEP2_START + (REMAIN_TIME_STEP2_LAST * (vCompensationVolt - REMAIN_TIME_STEP2_THRESHOLD) / REMAIN_TIME_STEP2_VOLT_LAST);
		} else if (vCompensationVolt >= REMAIN_TIME_STEP1_THRESHOLD) {
			vCwMaxTimer = REMAIN_TIME_STEP1_START + (REMAIN_TIME_STEP1_LAST * (vCompensationVolt - REMAIN_TIME_STEP1_THRESHOLD) / REMAIN_TIME_STEP1_VOLT_LAST);
		}
		if (displayParams.vModeIndex < PreinstallEffect) {						//普通模式
			if (displayParams.vModeIndex == ColorTempSetting) {
				if (pcaDataStruct.valueOfCw) {
					vtSpeedCW = ((float) pcaDataStruct.valueOfCw / PWM_FRQ_CONST) * CW_MAX_TIMER_CONST;
					vtSpeedCW /= vCwMaxTimer;
				}
				powerLineAdjust(&vtSpeedCW);
				if (pcaDataStruct.valueOfMw) {
					vtSpeedMW = ((float) pcaDataStruct.valueOfMw / PWM_FRQ_CONST) * MW_MAX_TIMER_CONST;
					vtSpeedMW /= vCwMaxTimer;
				}
				powerLineAdjust(&vtSpeedMW);
			}else{
				vtSpeedCW = ((float) PWM_MAX_CW/ PWM_FRQ_CONST) * CW_MAX_TIMER_CONST;		//vtSpeedCW only user for calculate remain time
				vtRedSpeed=(RED_POWER_RATING/CW_POWER_RATING)*vtSpeedCW;
				vtRedSpeed*=(float) pcaDataStruct.valueOfRed / PWM_FRQ_CONST;
				vtGreenSpeed=(GREEN_POWER_RATING/CW_POWER_RATING)*vtSpeedCW;
				vtGreenSpeed*=(float) pcaDataStruct.valueOfGreen / PWM_FRQ_CONST;
				vtBlueSpeed=(BLUE_POWER_RATING/CW_POWER_RATING)*vtSpeedCW;
				vtBlueSpeed*=(float) pcaDataStruct.valueOfBlue / PWM_FRQ_CONST;
				vtSpeedCW=vtRedSpeed+vtGreenSpeed+vtBlueSpeed;									//vtSpeedCW only user for calculate remain time
				vtSpeedCW /= vCwMaxTimer;
				powerLineAdjust(&vtSpeedCW);
//				displayFloat(10,3,vtSpeedCW,3,' ');
//				displayFloat(10,5,vtSpeedMW,3,' ');
			}
		} else {									//灯效模式
			if ((PreinstallEffect == displayParams.vModeIndex)) {
				if (displayParams.style1Value == 0) {
					vtSpeedCW = displayParams.brightness * 0.18 * CW_MAX_TIMER_CONST / (100 * vCwMaxTimer);							//等效0.18  CW duty
				} else if (displayParams.style1Value == 1) {
					vtSpeedCW = displayParams.brightness * 0.22 * CW_MAX_TIMER_CONST / (100 * vCwMaxTimer);							//等效0.22  CW duty
				} else if (displayParams.style1Value == 2) {
					vtSpeedCW = displayParams.brightness * 0.08 * CW_MAX_TIMER_CONST / (100 * vCwMaxTimer);								//等效0.8  CW duty
				}
				vtSpeedMW = 0;
			}
			if (CustomizeEffect == displayParams.vModeIndex) {
				if ((displayParams.style1Value == 0) || (displayParams.style1Value == 2)) {
					vtSpeedCW = displayParams.brightness * 0.55 * CW_MAX_TIMER_CONST / (100 * vCwMaxTimer);							//等效0.55  CW duty
					vtSpeedMW = 0;
				} else if (displayParams.style1Value == 1) {
					vtSpeedMW = displayParams.brightness * 0.55 * MW_MAX_TIMER_CONST / (100 * vCwMaxTimer);							//等效0.55  WW duty
					vtSpeedCW = 0;
				}
			}
		}
		vtSpeedMW += (0.07 * MW_MAX_TIMER_CONST) / vCwMaxTimer;				//加上约130mA的LCD+系统耗电
		totalTimer = (u16) (1.0 / (vtSpeedCW + vtSpeedMW));
		if (displayParams.brightness == 100 && ColorTempSetting == displayParams.vModeIndex) {
			if (totalTimer >= (REMAIN_TIME_STEP5_START + REMAIN_TIME_STEP5_LAST))
				totalTimer = REMAIN_TIME_STEP5_START + REMAIN_TIME_STEP5_LAST;
		}
		if (CCS_GET_ChargeStatus()) {
			ignoreCompare = TRUE;												//充电状态,允许剩余时间增加变化
		}
		if (vRemainTimer != totalTimer) {
			if (ignoreCompare) {
				updateRemainingTimeByValue(totalTimer);
				vRemainTimer = totalTimer;
			} else if (!CCS_GET_ChargeStatus() && vRemainTimer && vRemainTimer > totalTimer) {
				updateRemainingTimeByValue(totalTimer);
				vRemainTimer = totalTimer;
			}
		}
		else{
			if (ignoreCompare)
				updateRemainingTimeByValue(totalTimer);
		}
		return totalTimer;
	}
	return FALSE;
}


float minOfFloatArray(float *arr,u8 size){
	float vtMin=4.5;
	u8 i=0;
	i=sizeof(arr);
	i/=sizeof(float);
	i=0;
	for(;i<size;i++){
		if(vtMin>arr[i]){
			vtMin=arr[i];
		}
	}
	return vtMin;
}

void freshRemainTimeCheck(void) {
	u8	minWith10sCnt=STANDER_MIN_60S;
	if(fIsUpdateRemainTimer)
		return;
	float vCompensationVolt=getCompensationVolt();
	battCompVoltArray[vCalcTime1s % 10]=vCompensationVolt;
	if(++vCalcTime1s>=10)
		vCalcTime1s=0;
	if (getSysStatus()&&displayParams.brightness && (vCalcTime1s % 10 == 9)) {
//		LOG("here\n");
		u16 oldData = 0, newData = 0;
		vCompensationVolt=minOfFloatArray(battCompVoltArray,BATT_ARRAY_SIZE);
		oldData = vRemainTimer;
		newData = calcRemainTime(FALSE);
		if (vCompensationVolt <= BATT_LV4_THESHOLD&&displayParams.brightness>=35) {
#if(REMAIN_TIME_AUTO_ADJUST==1)
			if (newData > oldData){
				if((newData-oldData)>6)
					minWith10sCnt = SLOW_MIN_80S;
				else if((newData-oldData)>3)
					minWith10sCnt = SLOW_MIN_70S;
				else
					minWith10sCnt=STANDER_MIN_60S;
			}
#endif
		}
		if (newData && (oldData <=newData)) {
			if(++vRemainCheckTimes>=minWith10sCnt){
				vRemainCheckTimes=0;
				if(oldData>0){
					vRemainTimer = oldData - 1;
				}else{
					vRemainTimer=0;
				}
				updateRemainingTimeByValue(vRemainTimer);
			}
		}else if(oldData > newData){
			vRemainCheckTimes=0;
		}
	}
}
#endif
