#include "my_file_handler.h"
#include <bits/stdc++.h>

namespace server::file_handler{
  bool MyFileHandler::Delete(const std::string &filename) {

    return remove(filename.c_str());
    
  }//Delete
  bool MyFileHandler::Put(const std::string &filename, const std::vector<uint8_t> &contents) {

    std::ofstream stream;
    //convert contents to string format
    std::string str_contents(contents.begin(), contents.end());

    //open file
    stream.open(filename);

    //write contents to file
    stream << str_contents;

    //close file
    stream.close();
    
    return true;
  }//Put
  bool MyFileHandler::MakeDir(const std::string &name) {
    
    return mkdir(name.c_str(),0777)==0;
    
  }//MakeDir
  
  bool MyFileHandler::ChangeDir(const std::string &sub_folder) {
    
    return chdir(sub_folder.c_str());
    
  }//ChangeDir
  std::vector<uint8_t> MyFileHandler::Get(const std::string &filename) const {
    std::ifstream stream;

    stream.open(filename);

    //get contents of file, convert to byte vector
    std::string str_contents( (std::istreambuf_iterator<char>(stream) ),
                         (std::istreambuf_iterator<char>()));

    std::vector<uint8_t> vec_contents(str_contents.begin(),str_contents.end());

    stream.close();
    return vec_contents;

  }//Get

  bool MyFileHandler::UpDir() {
    return chdir("..")!=-1;
  }//UpDir

  std::vector<std::string> MyFileHandler::List() const {
    std::vector<std::string> list;
    for (const auto & file : std::filesystem::directory_iterator(GetCurrentDir())) {
      std::string str(file.path());
      str.erase(0,GetCurrentDir().length()+1);
      list.push_back(str);
    }

    return list;

  }//List

  std::string MyFileHandler::GetCurrentDir() const {
    return std::filesystem::current_path();
  }//GetCurrentDir

}//namespace server
