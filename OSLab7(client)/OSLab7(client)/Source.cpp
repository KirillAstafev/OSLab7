#include <stdio.h>
#include <Windows.h>
#include <time.h>

#define SHMEMNAME "sh_mem"
#define CS_SEM "cs_sem"
#define CL_WORK_SEM "cl_work_sem"
#define CL_WRITE_SEM "cl_write_sem"
#define SRV_WRITE_SEM "crv_write_sem"
#define COUNTER_SEM "counter_sem"

struct sh_field
{
	char inp_str[512], sub_str[3];
	int len_str;
	int position = -1;
};

void createTask(sh_field* field) {
	field->len_str = rand() % 30 + 10;
	field->inp_str[field->len_str] = '\0';
	for (int i = 0; i != field->len_str; i++) {		
		field->inp_str[i] = (char)(rand() % 26 + 97);
	}
	
	int subStringPosition = rand() % 25;
	for (int i = 0; i < 3; i++) {
		field->sub_str[i] = field->inp_str[subStringPosition];
		subStringPosition++;
	}

	printf("«апрос: %s %s\t", field->inp_str, field->sub_str);
}

int main() {
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	srand(GetCurrentProcessId());
	size_t sizeBuffer = sizeof(sh_field);
	HANDLE cs_sempahore = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, false, CS_SEM); //семафор кртитической секции
	HANDLE clwork_semaphore = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, false, CL_WORK_SEM); //семафор зан€тости клиента с сервером
	HANDLE clwrite_semaphore = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, false, CL_WRITE_SEM); // семафор записи и чтени€ данных клиентом
	HANDLE servwrite_semaphore = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, false, SRV_WRITE_SEM); //семафор записи и чтени€ данных сервером
	HANDLE counter_semaphore = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, false, COUNTER_SEM); // семафор дл€ отслеживани€ очереди

	DWORD isQueueFull = WaitForSingleObject(counter_semaphore, 1);
	if (isQueueFull == WAIT_TIMEOUT) {
		printf("ѕроцесс %d не попал в очередь(зан€то).\n", GetCurrentProcessId());
		return 1;
	}
	/*ожидает создани€ области раздел€емой пам€ти в начале, а затем ожидает готовности сервера отработать запрос*/
	WaitForSingleObject(clwork_semaphore, INFINITE);
	HANDLE hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, false, SHMEMNAME);
	if (hMapFile == NULL) {
		ReleaseSemaphore(clwork_semaphore, 1, NULL);
		return 1;
	}
	sh_field* field = (sh_field*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeBuffer);
	if (field == NULL) {
		ReleaseSemaphore(clwork_semaphore, 1, NULL);
		CloseHandle(hMapFile);
		return 1;
	}
	/*критическа€ область*/
	WaitForSingleObject(cs_sempahore, INFINITE);
	createTask(field);
	Sleep(100);
	ReleaseSemaphore(cs_sempahore, 1, NULL);
	/* сигнал серверу о готовности запроса */
	ReleaseSemaphore(servwrite_semaphore, 1, NULL);
	/*ожидание ответа от сервера*/
	WaitForSingleObject(clwrite_semaphore, INFINITE);
	printf("ќтвет: %d\n", field->position);
	UnmapViewOfFile(field);
	/*дает пон€ть, что сервер свободен*/
	ReleaseSemaphore(clwork_semaphore, 1, NULL);
	/*освобождени€ места в очереди*/
	ReleaseSemaphore(counter_semaphore, 1, NULL);
	return 0;
}
