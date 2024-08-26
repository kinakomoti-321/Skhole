#pragma once

#include <include.h>
#include <scene/scene.h>


namespace Skhole {

	class File {
	public:
		File() {};
		~File() {};

		static std::string ReadFile(const std::string& path) {}

		static bool CallFileDialog(std::string& filepath) {
			OPENFILENAME ofn;
			TCHAR szPath[MAX_PATH];
			TCHAR szFile[MAX_PATH] = TEXT("");

			GetCurrentDirectory(MAX_PATH, szPath);

			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = NULL;
			ofn.lpstrFile = szFile;
			ofn.lpstrInitialDir = szPath;
			ofn.nMaxFile = MAX_PATH;
			ofn.nFilterIndex = 1;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

			if (GetOpenFileName(&ofn)) {
				std::wstring ws(szFile);
				std::string fullPath(ws.begin(), ws.end());
				filepath = fullPath;
				return true;
			}
			else {
				return false;
			}
		}
	};
}
