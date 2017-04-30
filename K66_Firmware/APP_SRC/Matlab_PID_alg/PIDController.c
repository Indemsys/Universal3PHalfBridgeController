//
// File: PIDController.c
//
// Code generated for Simulink model 'PIDController'.
//
// Model version                  : 1.76
// Simulink Coder version         : 8.11 (R2016b) 25-Aug-2016
// C/C++ source code generated on : Mon Apr 17 21:25:43 2017
//
// Target selection: ert.tlc
// Embedded hardware selection: ARM Compatible->ARM Cortex
// Code generation objectives:
//    1. Debugging
//    2. Traceability
//    3. Execution efficiency
// Validation result: Not run
//
#include "PIDController.h"

// Block signals and states (auto storage)
DW rtDW;

// Real-time model
RT_MODEL rtM_;
RT_MODEL *const rtM = &rtM_;

// Model step function
void PIDController_custom(real32_T arg_PID_Input, real32_T arg_SmplTime,
  PIDParams *arg_Pars, real32_T *arg_PID_Output)
{
  real32_T rtb_Product_Kn;
  real32_T rtb_Integr1Scale;
  real32_T rtb_Sum2;
  real32_T rtb_Product_Ki;
  boolean_T rtb_NotEqual;
  real32_T rtb_Product_Ki_0;

  // Product: '<Root>/Product_Kn' incorporates:
  //   DiscreteTransferFcn: '<Root>/Integr2'
  //   Inport: '<Root>/PID_Input'
  //   Inport: '<Root>/Pars'
  //   Product: '<Root>/Product_Kd'
  //   Sum: '<Root>/Sum1'

  rtb_Product_Kn = ((arg_PID_Input * arg_Pars->Kd) - rtDW.Integr2_states) *
    arg_Pars->Kn;

  // Sum: '<Root>/Sum2' incorporates:
  //   DiscreteTransferFcn: '<Root>/Integr1'
  //   Inport: '<Root>/PID_Input'
  //   Inport: '<Root>/Pars'
  //   Inport: '<Root>/SmplTime'
  //   Product: '<Root>/Integr1Scale'
  //   Product: '<Root>/Product_Kp'

  rtb_Sum2 = ((arg_Pars->Kp * arg_PID_Input) + (rtDW.Integr1_states *
    arg_SmplTime)) + rtb_Product_Kn;

  // UnitDelay: '<Root>/Del2'
  //
  //  Block description for '<Root>/Del2':
  //
  //   Store in Global RAM

  rtb_Integr1Scale = rtDW.Del2_DSTATE;

  // Switch: '<S2>/Switch2' incorporates:
  //   Inport: '<Root>/Pars'
  //   RelationalOperator: '<S2>/LowerRelop1'
  //   RelationalOperator: '<S2>/UpperRelop'
  //   Switch: '<S2>/Switch'

  if (rtb_Sum2 > arg_Pars->OutUpLim)
  {
    rtb_Product_Ki = arg_Pars->OutUpLim;
  }
  else if (rtb_Sum2 < arg_Pars->OutLoLim)
  {
    // Switch: '<S2>/Switch'
    rtb_Product_Ki = arg_Pars->OutLoLim;
  }
  else
  {
    rtb_Product_Ki = rtb_Sum2;
  }

  // End of Switch: '<S2>/Switch2'

  // Sum: '<Root>/Sum3'
  //
  //  Block description for '<Root>/Sum3':
  //
  //   Add in CPU

  rtb_Product_Ki -= rtb_Integr1Scale;

  // Switch: '<S3>/Switch2' incorporates:
  //   Gain: '<Root>/Gain2'
  //   Inport: '<Root>/Pars'
  //   RelationalOperator: '<S3>/LowerRelop1'
  //   RelationalOperator: '<S3>/UpperRelop'
  //   Switch: '<S3>/Switch'

  if (rtb_Product_Ki > arg_Pars->OutRateLim)
  {
    rtb_Product_Ki = arg_Pars->OutRateLim;
  }
  else
  {
    if (rtb_Product_Ki < (-arg_Pars->OutRateLim))
    {
      // Switch: '<S3>/Switch' incorporates:
      //   Gain: '<Root>/Gain2'

      rtb_Product_Ki = -arg_Pars->OutRateLim;
    }
  }

  // End of Switch: '<S3>/Switch2'

  // Sum: '<Root>/Sum4'
  //
  //  Block description for '<Root>/Sum4':
  //
  //   Add in CPU

  rtb_Integr1Scale += rtb_Product_Ki;

  // Outport: '<Root>/PID_Output'
  *arg_PID_Output = rtb_Integr1Scale;

  // Switch: '<S1>/Switch' incorporates:
  //   Inport: '<Root>/Pars'
  //   RelationalOperator: '<S1>/u_GTE_up'
  //   RelationalOperator: '<S1>/u_GT_lo'
  //   Switch: '<S1>/Switch1'

  if (rtb_Sum2 >= arg_Pars->OutUpLim)
  {
    rtb_Product_Ki = arg_Pars->OutUpLim;
  }
  else if (rtb_Sum2 > arg_Pars->OutLoLim)
  {
    // Switch: '<S1>/Switch1'
    rtb_Product_Ki = rtb_Sum2;
  }
  else
  {
    rtb_Product_Ki = arg_Pars->OutLoLim;
  }

  // End of Switch: '<S1>/Switch'

  // Sum: '<S1>/Diff'
  rtb_Product_Ki = rtb_Sum2 - rtb_Product_Ki;

  // RelationalOperator: '<Root>/NotEqual' incorporates:
  //   Constant: '<Root>/Const2'

  rtb_NotEqual = (0.0F != rtb_Product_Ki);

  // Signum: '<Root>/Sign1'
  if (rtb_Product_Ki < 0.0F)
  {
    rtb_Sum2 = -1.0F;
  }
  else if (rtb_Product_Ki > 0.0F)
  {
    rtb_Sum2 = 1.0F;
  }
  else if (rtb_Product_Ki == 0.0F)
  {
    rtb_Sum2 = 0.0F;
  }
  else
  {
    rtb_Sum2 = rtb_Product_Ki;
  }

  // End of Signum: '<Root>/Sign1'

  // Product: '<Root>/Product_Ki' incorporates:
  //   Inport: '<Root>/PID_Input'
  //   Inport: '<Root>/Pars'

  rtb_Product_Ki = arg_Pars->Ki * arg_PID_Input;

  // Product: '<Root>/Integr2Scale' incorporates:
  //   Inport: '<Root>/SmplTime'

  rtb_Product_Kn *= arg_SmplTime;

  // Signum: '<Root>/Sign2'
  if (rtb_Product_Ki < 0.0F)
  {
    rtb_Product_Ki_0 = -1.0F;
  }
  else if (rtb_Product_Ki > 0.0F)
  {
    rtb_Product_Ki_0 = 1.0F;
  }
  else if (rtb_Product_Ki == 0.0F)
  {
    rtb_Product_Ki_0 = 0.0F;
  }
  else
  {
    rtb_Product_Ki_0 = rtb_Product_Ki;
  }

  // End of Signum: '<Root>/Sign2'

  // Switch: '<Root>/Switch' incorporates:
  //   Constant: '<Root>/Const1'
  //   Logic: '<Root>/And1'
  //   RelationalOperator: '<Root>/Equal1'

  if (rtb_NotEqual && (rtb_Sum2 == rtb_Product_Ki_0))
  {
    rtb_Sum2 = 0.0F;
  }
  else
  {
    rtb_Sum2 = rtb_Product_Ki;
  }

  // End of Switch: '<Root>/Switch'

  // Update for DiscreteTransferFcn: '<Root>/Integr1'
  rtDW.Integr1_states = rtb_Sum2 - (-rtDW.Integr1_states);

  // Update for DiscreteTransferFcn: '<Root>/Integr2'
  rtDW.Integr2_states = rtb_Product_Kn - (-rtDW.Integr2_states);

  // Update for UnitDelay: '<Root>/Del2'
  //
  //  Block description for '<Root>/Del2':
  //
  //   Store in Global RAM

  rtDW.Del2_DSTATE = rtb_Integr1Scale;
}

// Model initialize function
void PIDController_initialize(void)
{
  // (no initialization code required)
}

//
// File trailer for generated code.
//
// [EOF]
//
