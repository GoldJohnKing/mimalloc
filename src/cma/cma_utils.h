#pragma once

int CmaGetReservedHugePagesCount(void);

void CmaCreateMemoryCollectorThread(void);

void CmaTerminateMemoryCollectorThread(void);
