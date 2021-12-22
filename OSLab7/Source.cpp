#include <Windows.h>

#define CS_SEM "cs_sem"
#define CL_WORK_SEM "cl_work_sem"
#define CL_WRITE_SEM "cl_write_sem"
#define SRV_WRITE_SEM "crv_write_sem"
#define COUNTER_SEM "counter_sem"

#define COUNT_CLIENT 20
#define MAX_SIZE_QUEUE 12

int main() {
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);

	HANDLE cs_semaphore = CreateSemaphoreA(NULL, 1, 1, CS_SEM);  //семафор кртитической секции
	HANDLE clwork_semaphore = CreateSemaphoreA(NULL, 0, 1, CL_WORK_SEM); //семафор занятости клиента с сервером
	HANDLE clwrite_semaphore = CreateSemaphoreA(NULL, 0, 1, CL_WRITE_SEM);  // семафор записи и чтения данных клиентом
	HANDLE servwrite_semaphore = CreateSemaphoreA(NULL, 0, 1, SRV_WRITE_SEM);  //семафор записи и чтения данных сервером
	HANDLE counter_semaphore = CreateSemaphoreA(NULL, MAX_SIZE_QUEUE, MAX_SIZE_QUEUE, COUNTER_SEM); // семафор для отслеживания очереди

	PROCESS_INFORMATION* pi = new PROCESS_INFORMATION[COUNT_CLIENT + 1];
	STARTUPINFOA si;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	CreateProcessA(NULL, const_cast<LPSTR>("OSLab7(server).exe"), NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi[0]);
	for (int i = 0; i != COUNT_CLIENT; i++) {
		CreateProcessA(NULL, const_cast<LPSTR>("OSLab7(client).exe"), NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi[i + 1]);
		Sleep(10);
	}
	return 0;
}
