#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <unordered_set>
#include <filesystem>

#if defined(_WIN32)
const std::string DEFAULT_SEARCH_DIR = "C:\\Program Files";
#elif defined(__APPLE__) && defined(__MACH__)
const std::string DEFAULT_SEARCH_DIR = "/Applications";
#elif defined(__linux__)
const std::string DEFAULT_SEARCH_DIR = "/usr/local/share";
#else
const std::string DEFAULT_SEARCH_DIR = "./";
#endif

std::vector<std::string> SearchFilesInWorkingDirectory();
std::string glob_to_regex(const std::string& glob);

//파일 클래스 관리를 위한 observer 패턴
class IDirObserver {
public:
	virtual void OnDirChanged(const std::string& newDir) = 0;
	virtual ~IDirObserver() = default;
};

//디렉토리 처리를 위한 singleton 클래스
class DirSingleton {
private:
	static DirSingleton* instance;
	std::string dir = DEFAULT_SEARCH_DIR;
	std::vector<IDirObserver*> observers;
protected:
	DirSingleton() = default;
public:
	static DirSingleton& GetInstance();
	std::string& Getdir();
	void SetDir(const std::string& usr_dir);
	void AttachObserver(IDirObserver* obs);
	void DetachObserver(IDirObserver* obs);
	void NotifyObserver();
	DirSingleton(const DirSingleton&) = delete;
	DirSingleton& operator=(const DirSingleton&) = delete;

};


class FileManagement : public IDirObserver {
private:
	std::string dir = DirSingleton::GetInstance().Getdir();
	std::unordered_set<std::string> filenames;
public:
	FileManagement();
	~FileManagement();
	void OnDirChanged(const std::string& newDir) override;
	std::vector<std::string> find_last_of(const std::string& last);
	std::vector<std::string> glob(const std::string& pattern);
	void SaveFilename();
	std::unordered_set<std::string> SearchFilenames();
	std::unordered_set<std::string> SearchFilenames(const std::string& directory);
	bool IsExistFile(const std::string& filename);
};