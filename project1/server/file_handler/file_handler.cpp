#include "file_handler.h"

#include <filesystem>
namespace server::file_handler {
bool FileHandler::Delete(const std::string &filename) {
  return remove(filename.c_str())==0;

}  // Delete
bool FileHandler::Put(const std::string &filename,
                        const std::vector<uint8_t> &contents) {
  
  // convert contents to string format
  std::string str_contents(contents.begin(), contents.end());

  // open file
  std::ofstream stream(filename);

  // write contents to file
  stream << str_contents;

  // close file
  stream.close();

  return true;
}  // Put
bool FileHandler::MakeDir(const std::string &name) {
  const std::filesystem::path path = name;
  return std::filesystem::create_directory(path);

}  // MakeDir

bool FileHandler::ChangeDir(const std::string &sub_folder) {
  const std::filesystem::path path = sub_folder;
  std::filesystem::current_path(path);
  return true;

}  // ChangeDir
std::vector<uint8_t> FileHandler::Get(const std::string &filename) const {
  std::ifstream stream(filename);

  // get contents of file, convert to byte vector
  std::string str_contents((std::istreambuf_iterator<char>(stream)),
                           (std::istreambuf_iterator<char>()));

  std::vector<uint8_t> vec_contents(str_contents.begin(), str_contents.end());

  stream.close();
  return vec_contents;

}  // Get

bool FileHandler::UpDir() { return chdir("..") != -1; }  // UpDir

std::vector<std::string> FileHandler::List() const {
  std::vector<std::string> list;
  for (const auto &file :
       std::filesystem::directory_iterator(GetCurrentDir())) {
    std::string str(file.path());
    str.erase(0, GetCurrentDir().length() + 1);
    list.push_back(str);
  }

  return list;

}  // List

std::string FileHandler::GetCurrentDir() const {
  return std::filesystem::current_path();
}  // GetCurrentDir

}  // namespace server::file_handler
