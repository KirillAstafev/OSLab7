#include <string.h>
#include <Windows.h>
#include <string>
using namespace std;

#define SHMEMNAME "sh_mem" //имя сегмента разделяемой памяти
#define CS_SEM "cs_sem"
#define CL_WORK_SEM "cl_work_sem" 
#define CL_WRITE_SEM "cl_write_sem"
#define SRV_WRITE_SEM "crv_write_sem" 


struct sh_field
{
	char inp_str[512], sub_str[3];
	int len_str;
	int position = -1;
};

void replaceRange(sh_field* field) {
	string s(field->inp_str);
	field->position = s.rfind(field->sub_str);
}

int main() {
	size_t sz_buf = sizeof(sh_field);
	/* создание обработчика сегмента разделяемой памяти */
	HANDLE hShMem = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sz_buf, SHMEMNAME);
	/* создание указателя на структуру в области разделяемой памяти */
	sh_field* field = (sh_field*)MapViewOfFile(hShMem, FILE_MAP_ALL_ACCESS, 0, 0, sz_buf);

	HANDLE cs_sempahore = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, false, CS_SEM); //семафор кртитической секции
	HANDLE clwork_semaphore = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, false, CL_WORK_SEM); //семафор занятости клиента с сервером
	HANDLE clwrite_semaphore = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, false, CL_WRITE_SEM);  // семафор записи и чтения данных клиентом
	HANDLE servwrite_semaphore = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, false, SRV_WRITE_SEM);  //семафор записи и чтения данных сервером

	/* пока сервер не создаст область разделяемой памяти, взаимодействие с ней запрещено семафором
	clwork, после создания области разделяемой памяти, он устанавливается в 1 */
	ReleaseSemaphore(clwork_semaphore, 1, NULL);
	bool answer = true;
	while (answer == true) {
		/* сервер ожидает сигнального состояния семафора servrwrite для начала чтения и записи в сегмент разделенной памяти */
		WaitForSingleObject(servwrite_semaphore, INFINITE);
		/* критическая секция */
		WaitForSingleObject(cs_sempahore, INFINITE);
		if (field->inp_str == "")
			answer = false;
		replaceRange(field);
		ReleaseSemaphore(cs_sempahore, 1, NULL);
		/* сигнал клиенту о готовности данных, произведенных сервером */
		ReleaseSemaphore(clwrite_semaphore, 1, NULL);
	}
	UnmapViewOfFile(field);
	CloseHandle(hShMem);
}
