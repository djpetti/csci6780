/**
 * @file Implementation of common file handling operations
 */

#ifndef PROJECT1_MY_FILE_HANDLER_H
#define PROJECT1_MY_FILE_HANDLER_H

#include "file_handler.h"
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <sys/stat.h>





/**
 * @brief This class represents common file handling operations.
 *
 */
namespace server::file_handler{
class MyFileHandler: public IFileHandler {

 public:

  final std::vector<uint8_t> Get(const std::string& filename) const;

  bool Put(const std::string& filename,
           const std::vector<uint8_t>& contents);

  bool Delete(const std::string& filename);

  std::vector<std::string> List() const;

  bool ChangeDir(const std::string& sub_folder);

  bool UpDir();

  bool MakeDir(const std::string& name);

  std::string GetCurrentDir() const;
 
 private:
  
 std::ofstream os;


};
} //namespace server::file_handler
#endif  // PROJECT1_MY_FILE_HANDLER_H
