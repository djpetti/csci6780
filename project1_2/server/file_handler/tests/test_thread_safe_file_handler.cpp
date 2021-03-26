#include <cstdint>
#include <ctime>
#include <filesystem>
#include <memory>
#include <random>
#include <sstream>
#include <thread>
#include <vector>

#include "../file_access_manager.h"
#include "../thread_safe_file_handler.h"
#include "gtest/gtest.h"

using std::filesystem::path;

namespace server::file_handler::tests {
namespace {

/**
 * @brief Manages a temp directory to use for testing.
 */
class TestDir {
 public:
  TestDir() {
    // Create the test directory.
    const path kTempDir = path("/tmp");

    std::default_random_engine generator;
    std::uniform_int_distribution<uint32_t> distribution(0, 1000);

    std::stringstream dir_name;
    dir_name << "test_" << time(nullptr) << "_" << distribution(generator);
    test_dir_ = kTempDir / dir_name.str();
    std::filesystem::create_directory(test_dir_);
  }

  ~TestDir() {
    // Remove the test directory.
    std::filesystem::remove_all(test_dir_);
  }

  /**
   * @return The path to the testing directory.
   */
  [[nodiscard]] path Get() const { return test_dir_; }

 private:
  /// Directory to use for storing the tests.
  path test_dir_;
};

/**
 * @brief Encapsulates the FileAccessManagers we are using for these tests.
 */
struct AccessManagers {
  std::shared_ptr<FileAccessManager> read_manager;
  std::shared_ptr<FileAccessManager> write_manager;
};

/**
 * @brief Creates FileAccessManagers to use for testing.
 * @return The access managers it created.
 */
AccessManagers CreateAccessManagers() {
  return {std::make_shared<FileAccessManager>(),
          std::make_shared<FileAccessManager>()};
}

}  // namespace

/**
 * @test Tests that we can write a file and then read it back.
 */
TEST(FileHandler, PutGet) {
  // Arrange.
  TestDir test_dir;

  // File to test with.
  const path kTestFile = test_dir.Get() / "test_file.txt";
  // Data to write.
  const std::vector<uint8_t> kTestData = {1, 2, 3, 4, 5};

  AccessManagers managers = CreateAccessManagers();
  ThreadSafeFileHandler file_handler(managers.read_manager,
                                     managers.write_manager);

  // Act.
  ASSERT_TRUE(file_handler.Put(kTestFile, kTestData));
  const auto kGotData = file_handler.Get(kTestFile);

  // Assert.
  // It should have gotten the correct data.
  EXPECT_EQ(kTestData, kGotData);
  // The file should still be there.
  EXPECT_TRUE(std::filesystem::exists(kTestFile));
}

/**
 * @test Tests that we can write a file and then delete it.
 */
TEST(FileHandler, PutDelete) {
  // Arrange.
  TestDir test_dir;

  // File to test with.
  const path kTestFile = test_dir.Get() / "test_file.txt";

  AccessManagers managers = CreateAccessManagers();
  ThreadSafeFileHandler file_handler(managers.read_manager,
                                     managers.write_manager);

  // Act.
  ASSERT_TRUE(file_handler.Put(kTestFile, {}));
  const bool kDeleteSuccess = file_handler.Delete(kTestFile);

  // Assert.
  // Deletion should have succeeded.
  EXPECT_TRUE(kDeleteSuccess);
  // The file should no longer exist.
  EXPECT_FALSE(std::filesystem::exists(kTestFile));
}

/**
 * @test Tests that we can make a new directory.
 */
TEST(FileHandler, MakeDir) {
  // Arrange.
  TestDir test_dir;

  // Test directory.
  const path kTestDir = test_dir.Get() / "test_dir";

  AccessManagers managers = CreateAccessManagers();
  ThreadSafeFileHandler file_handler(managers.read_manager,
                                     managers.write_manager);

  // Act.
  const bool kMakeDirResult = file_handler.MakeDir(kTestDir);

  // Assert.
  // It should have succeeded in creating the directory.
  EXPECT_TRUE(kMakeDirResult);
  // The directory should exist.
  EXPECT_TRUE(std::filesystem::exists(kTestDir));
}

/**
 * @test Tests that we can access two different files concurrently.
 */
TEST(FileHandler, PutGetConcurrentDifferentFiles) {
  // Arrange.
  TestDir test_dir;

  // Files to test with.
  const path kTestFile1 = test_dir.Get() / "test_file_1.txt";
  const path kTestFile2 = test_dir.Get() / "test_file_2.txt";

  // Data to write.
  const std::vector<uint8_t> kTestData = {1, 2, 3, 4, 5};

  AccessManagers managers = CreateAccessManagers();
  ThreadSafeFileHandler file_handler1(managers.read_manager,
                                      managers.write_manager);
  ThreadSafeFileHandler file_handler2(managers.read_manager,
                                      managers.write_manager);

  // Act and assert.
  std::thread thread1([&]() {
    EXPECT_TRUE(file_handler1.Put(kTestFile1, kTestData));
    const auto kGotData = file_handler1.Get(kTestFile1);

    EXPECT_EQ(kTestData, kGotData);
  });
  std::thread thread2([&]() {
    EXPECT_TRUE(file_handler2.Put(kTestFile2, kTestData));
    const auto kGotData = file_handler2.Get(kTestFile2);

    EXPECT_EQ(kTestData, kGotData);
  });

  thread1.join();
  thread2.join();
}

/**
 * @test Tests that we can access the same file concurrently.
 */
TEST(FileHandler, PutGetConcurrentSameFile) {
  // Arrange.
  TestDir test_dir;

  // Files to test with.
  const path kTestFile = test_dir.Get() / "test_file.txt";

  // Data to write.
  const std::vector<uint8_t> kTestData = {1, 2, 3, 4, 5};

  AccessManagers managers = CreateAccessManagers();
  ThreadSafeFileHandler file_handler1(managers.read_manager,
                                      managers.write_manager);
  ThreadSafeFileHandler file_handler2(managers.read_manager,
                                      managers.write_manager);

  // Act.
  std::thread thread1([&]() {
    EXPECT_TRUE(file_handler1.Put(kTestFile, kTestData));
  });
  std::thread thread2([&]() {
    EXPECT_TRUE(file_handler2.Put(kTestFile, kTestData));
  });

  thread1.join();
  thread2.join();

  // Assert.
  // One write should have cleanly overwritten the other, with no data
  // duplication.
  const auto kGotData = file_handler1.Get(kTestFile);
  EXPECT_EQ(kTestData, kGotData);
}

}  // namespace server::file_handler::tests