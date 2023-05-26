#ifndef _PID_v1_H
#define _PID_v1_H

typedef struct
{
  int SetPoint;     // 设定目标 Desired Value
  float Proportion; // 比例常数 Proportional Const
  float Integral;   // 积分常数 Integral  Const
  float Derivative; // 微分常数 Derivative Const
  int LastError;    // Error[-1]
  int PrevError;    // Error[-2]
} PID;

float IncPIDCalc(PID *sptr, int NextPoint)
{
  register int iError;
  register float iIncpid;
  iError = sptr->SetPoint - NextPoint;
  iIncpid = sptr->Proportion * (iError - sptr->LastError)                          // E[k] 项
            + sptr->Integral * iError                                     // E[k－1]项
            + sptr->Derivative * (iError - 2 * sptr->LastError + sptr->PrevError); // E[k－2]项
  sptr->PrevError = sptr->LastError;
  sptr->LastError = iError;
  return (iIncpid);
}
#endif