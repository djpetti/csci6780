#include "file_handler.h"

namespace server::file_handler {

FileHandler::FileHandler() : current_dir_(std::filesystem::current_path()) {}

bool FileHandler::Delete(const std::string &filename) {
  return remove(filename.c_str()) == 0;

}  // Delete
bool FileHandler::Put(const std::string &filename,
                      const std::vector<uint8_t> &contents) {
  // convert contents to string format
  std::string str_contents(contents.begin(), contents.end());

  // open file
  std::ofstream stream(current_dir_ / filename);

  // write contents to file
  stream << str_contents;

  // close file
  stream.close();

  return true;
}  // Put
bool FileHandler::MakeDir(const std::string &name) {
  return std::filesystem::create_directory(current_dir_ / name);

}  // MakeDir

bool FileHandler::ChangeDir(const std::string &sub_folder) {
  std::filesystem::current_path(current_dir_ / sub_folder);
  return true;

}  // ChangeDir
std::vector<uint8_t> FileHandler::Get(const std::string &filename) const {
  std::ifstream stream(current_dir_ / filename);

  // get contents of file, convert to byte vector
  std::string str_contents((std::istreambuf_iterator<char>(stream)),
                           (std::istreambuf_iterator<char>()));

  std::vector<uint8_t> vec_contents(str_contents.begin(), str_contents.end());

  stream.close();
  return vec_contents;

}  // Get

bool FileHandler::UpDir() {
  current_dir_ = current_dir_.parent_path();
  return true;
}  // UpDir

std::vector<std::string> FileHandler::List() const {
  std::vector<std::string> list;
  for (const auto &file :
       std::filesystem::directory_iterator(current_dir_)) {
    std::string str(file.path());
    str.erase(0, current_dir_.string().length() + 1);
    list.push_back(str);
  }

  return list;

}  // List

std::string FileHandler::GetCurrentDir() const {
  return current_dir_;
}
// GetCurrentDir

}  // namespace server::file_handler
