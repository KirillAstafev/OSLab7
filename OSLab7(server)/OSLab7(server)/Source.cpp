#include <string.h>
#include <Windows.h>
#include <string>
using namespace std;

#define SHMEMNAME "sh_mem" //��� �������� ����������� ������
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
	/* �������� ����������� �������� ����������� ������ */
	HANDLE hShMem = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sz_buf, SHMEMNAME);
	/* �������� ��������� �� ��������� � ������� ����������� ������ */
	sh_field* field = (sh_field*)MapViewOfFile(hShMem, FILE_MAP_ALL_ACCESS, 0, 0, sz_buf);

	HANDLE cs_sempahore = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, false, CS_SEM); //������� ������������ ������
	HANDLE clwork_semaphore = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, false, CL_WORK_SEM); //������� ��������� ������� � ��������
	HANDLE clwrite_semaphore = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, false, CL_WRITE_SEM);  // ������� ������ � ������ ������ ��������
	HANDLE servwrite_semaphore = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, false, SRV_WRITE_SEM);  //������� ������ � ������ ������ ��������

	/* ���� ������ �� ������� ������� ����������� ������, �������������� � ��� ��������� ���������
	clwork, ����� �������� ������� ����������� ������, �� ��������������� � 1 */
	ReleaseSemaphore(clwork_semaphore, 1, NULL);
	bool answer = true;
	while (answer == true) {
		/* ������ ������� ����������� ��������� �������� servrwrite ��� ������ ������ � ������ � ������� ����������� ������ */
		WaitForSingleObject(servwrite_semaphore, INFINITE);
		/* ����������� ������ */
		WaitForSingleObject(cs_sempahore, INFINITE);
		if (field->inp_str == "")
			answer = false;
		replaceRange(field);
		ReleaseSemaphore(cs_sempahore, 1, NULL);
		/* ������ ������� � ���������� ������, ������������� �������� */
		ReleaseSemaphore(clwrite_semaphore, 1, NULL);
	}
	UnmapViewOfFile(field);
	CloseHandle(hShMem);
}
