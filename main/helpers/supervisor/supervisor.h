#ifndef SUPERVISOR_TASK_H
#define SUPERVISOR_TASK_H

/* 
    * @brief Supervisor task header file.
    *
    * This file declares the supervisor task that monitors the system runtime,
    * merges CSV logs, and initiates a system restart after a defined period.
*/
void supervisor_task(void *pvParameters);

#endif // SUPERVISOR_TASK_H