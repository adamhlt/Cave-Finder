#include <Windows.h>
#include <cstdio>
#include <strsafe.h>

DWORD_PTR dwCaveSize;

/**
 *	Function to retrieve the PE file content.
 *	\param lpFilePath : path of the PE file.
 *	\return : address of the content in the explorer memory.
 */
HANDLE GetFileContent(const LPSTR lpFilePath)
{
	const HANDLE hFile = CreateFileA(lpFilePath, GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("[-] An error occured when trying to open the PE file !\n");
		CloseHandle(hFile);
		return nullptr;
	}

	const DWORD dFileSize = GetFileSize(hFile, nullptr);
	if (dFileSize == INVALID_FILE_SIZE)
	{
		printf("[-] An error occured when trying to get the PE file size !\n");
		CloseHandle(hFile);
		return nullptr;
	}

	const HANDLE hFileContent = HeapAlloc(GetProcessHeap(), 0, dFileSize);
	if (hFileContent == INVALID_HANDLE_VALUE)
	{
		printf("[-] An error occured when trying to allocate memory for the PE file content !\n");
		CloseHandle(hFile);
		CloseHandle(hFileContent);
		return nullptr;
	}

	const BOOL bFileRead = ReadFile(hFile, hFileContent, dFileSize, nullptr, nullptr);
	if (!bFileRead)
	{
		printf("[-] An error occured when trying to read the PE file content !\n");

		CloseHandle(hFile);
		if (hFileContent != nullptr)
			CloseHandle(hFileContent);

		return nullptr;
	}

	CloseHandle(hFile);
	return hFileContent;
}

/**
 * Retrieve and display the protection of the section.
 * \param dCharacteristics : characteristics of the section.
 * \return : the description of the protection.
 */
const char* GetSectionProtection(const DWORD dCharacteristics)
{
	char lpSectionProtection[1024] = {};
	StringCchCatA(lpSectionProtection, 1024, "(");
	bool bExecute = false, bRead = false;

	if (dCharacteristics & IMAGE_SCN_MEM_EXECUTE)
	{
		bExecute = true;
		StringCchCatA(lpSectionProtection, 1024, "EXECUTE");
	}

	if (dCharacteristics & IMAGE_SCN_MEM_READ)
	{
		bRead = true;
		if (bExecute)
			StringCchCatA(lpSectionProtection, 1024, " | ");
		StringCchCatA(lpSectionProtection, 1024, "READ");
	}

	if (dCharacteristics & IMAGE_SCN_MEM_WRITE)
	{
		if (bExecute || bRead)
			StringCchCatA(lpSectionProtection, 1024, " | ");
		StringCchCatA(lpSectionProtection, 1024, "WRITE");
	}

	StringCchCatA(lpSectionProtection, 1024, ")");
	return lpSectionProtection;
}

/**
 * Function to find code cave in a section.
 * \param lpImageSectionHeader : section to search in.
 * \param lpImage : image of the executable.
 */
void FindCave(const PIMAGE_SECTION_HEADER lpImageSectionHeader, const LPVOID lpImage)
{
	const DWORD_PTR dwSectionSize = lpImageSectionHeader->SizeOfRawData;
	const auto lpSectionData = (BYTE*)((DWORD_PTR)lpImage + lpImageSectionHeader->PointerToRawData);

	for (int i = 0; i < dwSectionSize; i++)
	{
		if (lpSectionData[i] == 0x0)
		{
			DWORD_PTR dwOffset = 0;

			while (lpSectionData[dwOffset + i] == 0)
			{
				if ((i + dwOffset) >= dwSectionSize)
					break;

				dwOffset++;
			}

			if (dwOffset >= dwCaveSize)
			{
				printf("\n[+] Code Cave Found !\n");
				printf("    Section %s\n", (LPSTR)lpImageSectionHeader->Name);
				printf("    Cave Size : 0x%X\n", (UINT)dwOffset);
				printf("    Start Offset : 0x%X\n", (UINT)(lpImageSectionHeader->PointerToRawData + i));
				printf("    End Offset : 0x%X\n", (UINT)(lpImageSectionHeader->PointerToRawData + i + dwOffset));
				printf("    Start Virtual Address : 0x%X\n", (UINT)(lpImageSectionHeader->VirtualAddress + i));
				printf("    End Virtual Address : 0x%X\n", (UINT)(lpImageSectionHeader->VirtualAddress + i + dwOffset));
				printf("    Permissions %s\n", GetSectionProtection(lpImageSectionHeader->Characteristics));
			}

			i += (int)dwOffset;
		}
	}
}

/**
 * Function to find code cave in x86 image.
 * \param lpImage : pointer to the image.
 * \return : 0 if success else -1.
 */
int Miner32(const LPVOID lpImage)
{
	const auto lpImageDOSHeader = (PIMAGE_DOS_HEADER)(lpImage);
	const auto lpImageNTHeader32 = (PIMAGE_NT_HEADERS32)((DWORD_PTR)lpImage + lpImageDOSHeader->e_lfanew);
	auto lpImageSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD_PTR)lpImageNTHeader32 + 4 + sizeof(IMAGE_FILE_HEADER) + lpImageNTHeader32->FileHeader.SizeOfOptionalHeader);

	printf("[+] x86 architecture\n");
	printf("[+] Number of sections : %i\n", lpImageNTHeader32->FileHeader.NumberOfSections);

	for (int i = 0; i < lpImageNTHeader32->FileHeader.NumberOfSections; i++)
	{
		FindCave(lpImageSectionHeader, lpImage);
		lpImageSectionHeader++;
	}

	return 0;
}

/**
 * Function to find code cave in x64 image.
 * \param lpImage : pointer to the image.
 * \return : 0 if success else -1.
 */
int Miner64(const LPVOID lpImage)
{
	const auto lpImageDOSHeader = (PIMAGE_DOS_HEADER)(lpImage);
	const auto lpImageNTHeader64 = (PIMAGE_NT_HEADERS64)((DWORD_PTR)lpImage + lpImageDOSHeader->e_lfanew);
	auto lpImageSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD_PTR)lpImageNTHeader64 + 4 + sizeof(IMAGE_FILE_HEADER) + lpImageNTHeader64->FileHeader.SizeOfOptionalHeader);

	printf("[+] x64 architecture\n");
	printf("[+] Number of sections : %i\n", lpImageNTHeader64->FileHeader.NumberOfSections);

	for (int i = 0; i < lpImageNTHeader64->FileHeader.NumberOfSections; i++)
	{
		FindCave(lpImageSectionHeader, lpImage);
		lpImageSectionHeader++;
	}

	return 0;
}

int main(const int argc, char* argv[])
{
    printf("[+] CODE CAVE MINER\n");

	char* lpFilePath;

	if (argc == 3)
	{
		lpFilePath = argv[1];
		dwCaveSize = strtol(argv[2], nullptr, 10);
	}
	else
	{
		printf("[HELP] miner.exe <file> <size>\n");
		return -1;
	}

	const HANDLE hFileContent = GetFileContent(lpFilePath);
	if (hFileContent == INVALID_HANDLE_VALUE)
	{
		if (hFileContent != nullptr)
			CloseHandle(hFileContent);

		return -1;
	}

	printf("[+] Cave Size : %llu\n", dwCaveSize);

	const auto lpImageDOSHeader = (PIMAGE_DOS_HEADER)hFileContent;
	const auto lpImageNTHeader = (PIMAGE_NT_HEADERS32)((DWORD_PTR)lpImageDOSHeader + lpImageDOSHeader->e_lfanew);

	if (lpImageNTHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		printf("[-] Your file is not a valid a PE image.\n");
		return -1;
	}

	int Result = -1;

	if (lpImageNTHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
		Result = Miner32(hFileContent);

	if (lpImageNTHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
		Result = Miner64(hFileContent);

	if (hFileContent != nullptr)
		HeapFree(hFileContent, 0, nullptr);

	printf("\n");
	system("PAUSE");

	return Result;
}