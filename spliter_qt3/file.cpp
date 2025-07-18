#include "file.h"

DirSingleton& DirSingleton::GetInstance() {
	static DirSingleton instance;
	return instance;

}

std::string& DirSingleton::Getdir() {
	return dir;
}

void DirSingleton::SetDir(const std::string& usr_dir) {
	dir = usr_dir;
	NotifyObserver();
}

void DirSingleton::AttachObserver(IDirObserver* obs) {
	observers.push_back(obs);
}

void DirSingleton::DetachObserver(IDirObserver* obs) {
	observers.erase(std::remove(observers.begin(), observers.end(), obs), observers.end());
}

void DirSingleton::NotifyObserver() {
	for (auto i : observers) {
		i->OnDirChanged(this->dir);
	}
}

FileManagement::FileManagement() {
	DirSingleton::GetInstance().AttachObserver(this);
}

FileManagement::~FileManagement() {
	DirSingleton::GetInstance().DetachObserver(this);
}

void FileManagement::OnDirChanged(const std::string& newDir) {
	dir = newDir;
	SaveFilename();
}

//.c나 .o 파일들을 찾기 위한 메서드
std::vector<std::string> FileManagement::find_last_of(const std::string& last) {
	std::vector<std::string> names;

	for (const auto& i : filenames) {
		if (i.size() >= last.size()) {
			if (i.substr(i.size() - last.size()) == last) {
				names.push_back(i);
			}
		}
	}
	return names;
}

std::vector<std::string> FileManagement::glob(const std::string& pattern) {
	if (pattern.front() == '*' && pattern.find_first_of("?[]") == std::string::npos && pattern.find('/') == std::string::npos) {
		return find_last_of(pattern.substr(1));
	}


	return std::vector<std::string>();
}

void FileManagement::SaveFilename() {
	filenames = SearchFilenames();
}

std::unordered_set<std::string> FileManagement::SearchFilenames() {
	return SearchFilenames(dir);

}

std::unordered_set<std::string> FileManagement::SearchFilenames(const std::string& directory) {
	std::unordered_set<std::string> filenames;

	std::filesystem::path p = directory;
	if (!std::filesystem::exists(p) || !std::filesystem::is_directory(p)) {
		std::cerr << "유효하지 않은 디렉터리입니다.\n";
		return filenames;
	}

	for (const auto& entry : std::filesystem::directory_iterator(p)) {
		if (std::filesystem::is_regular_file(entry.status())) {
			filenames.emplace(entry.path().filename().string());
		}
	}

	return filenames;
}

bool FileManagement::IsExistFile(const std::string& filename) {
	if (filenames.empty()) {
		SaveFilename();
	}
	if (filenames.find(filename) != filenames.end()) {
		return true;
	}

	return false;
}

std::vector<std::string> SearchFilesInWorkingDirectory() {
	std::filesystem::path p = DirSingleton::GetInstance().Getdir();
	std::vector<std::string> filenames;
	if (!std::filesystem::exists(p) || !std::filesystem::is_directory(p)) {
		std::cerr << "유효하지 않은 디렉터리입니다.\n";
		return std::vector<std::string>();
	}

	for (const auto& entry : std::filesystem::directory_iterator(p)) {
		if (std::filesystem::is_regular_file(entry.status())) {
			filenames.push_back(entry.path().filename().string());
		}
	}
	return filenames;
}

std::string glob_to_regex(const std::string& glob) {
	std::string regex = "^";
	for (size_t i = 0; i < glob.size(); ++i) {
		char c = glob[i];
		switch (c) {
		case '*': regex += ".*"; break;
		case '?': regex += "."; break;
		case '.': regex += "\\."; break;
		case '\\': regex += "\\\\"; break;
		case '[': regex += "["; break;
		case ']': regex += "]"; break;
		default:
			if (std::ispunct(c) && c != '[' && c != ']')
				regex += '\\'; // escape regex special chars
			regex += c;
		}
	}
	regex += "$";
	return regex;
}