#ifndef PWM_CONTROL_H
#define PWM_CONTROL_H

int AkDrvPwmOpen(int device_no);
int AkDrvPwmSet(int device_no, long long duty_ns, long long period_ns);
int AkDrvPwmClose(int device_no);

#endif
