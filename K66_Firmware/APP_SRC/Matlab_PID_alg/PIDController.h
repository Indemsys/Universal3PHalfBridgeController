//
// File: PIDController.h
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
#ifndef RTW_HEADER_PIDController_h_
#define RTW_HEADER_PIDController_h_
#include "rtwtypes.h"
#include <stddef.h>
#ifndef PIDController_COMMON_INCLUDES_
# define PIDController_COMMON_INCLUDES_
#include "rtwtypes.h"
#endif                                 // PIDController_COMMON_INCLUDES_

// Macros for accessing real-time model data structure
#ifndef rtmGetErrorStatus
# define rtmGetErrorStatus(rtm)        ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
# define rtmSetErrorStatus(rtm, val)   ((rtm)->errorStatus = (val))
#endif

#define PIDController_M                (rtM)

// Forward declaration for rtModel
typedef struct tag_RTM RT_MODEL;

#ifndef DEFINED_TYPEDEF_FOR_PIDParams_
#define DEFINED_TYPEDEF_FOR_PIDParams_

typedef struct
{
  real32_T Kp;
  real32_T Ki;
  real32_T Kd;
  real32_T Kn;
  real32_T OutUpLim;
  real32_T OutLoLim;
  real32_T OutRateLim;
}
PIDParams;

#endif

// Block signals and states (auto storage) for system '<Root>'
typedef struct
{
  real32_T Integr1_states;             // '<Root>/Integr1'
  real32_T Integr2_states;             // '<Root>/Integr2'
  real32_T Del2_DSTATE;                // '<Root>/Del2'
}
DW;

// Real-time Model Data Structure
struct tag_RTM
{
  const char_T *errorStatus;
};

// Block signals and states (auto storage)
extern DW rtDW;

// Model entry point functions
extern void PIDController_initialize(void);

// Customized model step function
extern void PIDController_custom(real32_T arg_PID_Input, real32_T arg_SmplTime,
  PIDParams *arg_Pars, real32_T *arg_PID_Output);

// Real-time Model object
extern RT_MODEL *const rtM;

//-
//  These blocks were eliminated from the model due to optimizations:
//
//  Block '<S2>/Data Type Duplicate' : Unused code path elimination
//  Block '<S2>/Data Type Propagation' : Unused code path elimination
//  Block '<S3>/Data Type Duplicate' : Unused code path elimination
//  Block '<S3>/Data Type Propagation' : Unused code path elimination


//-
//  The generated code includes comments that allow you to trace directly
//  back to the appropriate location in the model.  The basic format
//  is <system>/block_name, where system is the system number (uniquely
//  assigned by Simulink) and block_name is the name of the block.
//
//  Use the MATLAB hilite_system command to trace the generated code back
//  to the model.  For example,
//
//  hilite_system('<S3>')    - opens system 3
//  hilite_system('<S3>/Kp') - opens and selects block Kp which resides in S3
//
//  Here is the system hierarchy for this model
//
//  '<Root>' : 'PIDController'
//  '<S1>'   : 'PIDController/DeadZone'
//  '<S2>'   : 'PIDController/Saturation1'
//  '<S3>'   : 'PIDController/Saturation2'


//-
//  Requirements for '<Root>': PIDController

#endif                                 // RTW_HEADER_PIDController_h_

//
// File trailer for generated code.
//
// [EOF]
//
