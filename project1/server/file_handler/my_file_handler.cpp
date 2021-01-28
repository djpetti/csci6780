//
// Created by jake on 1/25/21.
//

#include "my_file_handler.h"
#include <bits/stdc++.h>

namespace server::file_handler{
  bool file_handler::MyFileHandler::Delete(const std::string &filename) {

    //convert filename to char*
    int n = filename.length();
    char fileName[n+1];
    strcpy(fileName, filename.c_str());

    //delete file
    return remove(fileName);
  }//Delete
  bool file_handler::MyFileHandler::Put(const std::string &filename, const std::vector<uint8_t> &contents) {

    //convert contents to string format
    std::string str_contents(contents.begin(), contents.end());

    //open file
    try {this->os.open(filename);}
    catch (std::exception &e) {
      this->os.close();
      return false;
    }

    //write contents to file
    try{this->os << str_contents;}
    catch (std::exception &e){
      this->os.close();
      return false;
    }

    //close file
    this->os.close();
    return true;
  }//Put
  bool file_handler::MyFileHandler::MakeDir(const std::string &name) {
    //convert directory name to char*
    int n = name.length();
    char dir[n+1];
    strcpy(dir, name.c_str());

    if (mkdir(dir,0777)==-1) return false;
    else return true;
  }//MakeDir
  bool file_handler::MyFileHandler::ChangeDir(const std::string &sub_folder) {

    //convert subdirectory name to char*
    int n = sub_folder.length();
    char dir[n+1];
    strcpy(dir, sub_folder.c_str());

    if(chdir(dir)==-1) return false;
    else return true;
  }//ChangeDir
  std::vector<uint8_t> file_handler::MyFileHandler::Get(const std::string &filename) const {
    std::ifstream stream;

    stream.open(filename);

    //get contents of file, convert to byte vector
    std::string str_contents( (std::istreambuf_iterator<char>(stream) ),
                         (std::istreambuf_iterator<char>()    ) );

    std::vector<uint8_t> vec_contents(str_contents.begin(),str_contents.end());

    stream.close();
    return vec_contents;

  }//Get

  bool file_handler::MyFileHandler::UpDir() {
    if(chdir("..")==1) return false;
    else return true;
  }//UpDir

  std::vector<std::string> file_handler::MyFileHandler::List() const {
    std::vector<std::string> list;
    for (const auto & file : std::filesystem::directory_iterator(GetCurrentDir())) {
      std::string str(file.path());
      str.erase(0,GetCurrentDir().length());
      list.push_back(str);
    }

    return list;

  }//List

  std::string file_handler::MyFileHandler::GetCurrentDir() const {
    return std::filesystem::current_path();
  }//GetCurrentDir

}//namespace server
