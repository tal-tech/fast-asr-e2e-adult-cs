// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/metrics/persistent_memory_allocator.h"

#include "base/files/file.h"
#include "base/files/file_util.h"
#include "base/files/memory_mapped_file.h"
#include "base/files/scoped_temp_dir.h"
#include "base/memory/scoped_ptr.h"
#include "base/metrics/histogram.h"
#include "base/rand_util.h"
#include "base/strings/safe_sprintf.h"
#include "base/threading/simple_thread.h"
#include "testing/gmock/include/gmock/gmock.h"

namespace {

const uint32_t TEST_MEMORY_SIZE = 1 << 20;   // 1 MiB
const uint32_t TEST_MEMORY_PAGE = 64 << 10;  // 64 KiB
const uint32_t TEST_ID = 12345;
const char TEST_NAME[] = "TestAllocator";

}  // namespace

namespace base {

typedef PersistentMemoryAllocator::Reference Reference;

class PersistentMemoryAllocatorTest : public testing::Test {
 public:
  // This can't be statically initialized because it's value isn't defined
  // in the PersistentMemoryAllocator header file. Instead, it's simply set
  // in the constructor.
  uint32_t kAllocAlignment;

  struct TestObject1 {
    int onething;
    char oranother;
  };

  struct TestObject2 {
    int thiis;
    long that;
    float andthe;
    char other;
    double thing;
  };

  PersistentMemoryAllocatorTest() {
    kAllocAlignment = PersistentMemoryAllocator::kAllocAlignment;
    mem_segment_.reset(new char[TEST_MEMORY_SIZE]);
  }

  void SetUp() override {
    allocator_.reset();
    ::memset(mem_segment_.get(), 0, TEST_MEMORY_SIZE);
    allocator_.reset(new PersistentMemoryAllocator(
        mem_segment_.get(), TEST_MEMORY_SIZE, TEST_MEMORY_PAGE,
        TEST_ID, TEST_NAME, false));
    allocator_->CreateTrackingHistograms(allocator_->Name());
  }

  void TearDown() override {
    allocator_.reset();
  }

  unsigned CountIterables() {
    PersistentMemoryAllocator::Iterator iter;
    uint32_t type;
    unsigned count = 0;
    for (allocator_->CreateIterator(&iter);
         allocator_->GetNextIterable(&iter, &type) != 0;) {
      count++;
    }
    return count;
  }

 protected:
  scoped_ptr<char[]> mem_segment_;
  scoped_ptr<PersistentMemoryAllocator> allocator_;
};

TEST_F(PersistentMemoryAllocatorTest, AllocateAndIterate) {
  std::string base_name(TEST_NAME);
  EXPECT_EQ(TEST_ID, allocator_->Id());
  EXPECT_TRUE(allocator_->used_histogram_);
  EXPECT_EQ(base_name + ".UsedKiB",
            allocator_->used_histogram_->histogram_name());
  EXPECT_TRUE(allocator_->allocs_histogram_);
  EXPECT_EQ(base_name + ".Allocs",
            allocator_->allocs_histogram_->histogram_name());

  // Get base memory info for later comparison.
  PersistentMemoryAllocator::MemoryInfo meminfo0;
  allocator_->GetMemoryInfo(&meminfo0);
  EXPECT_EQ(TEST_MEMORY_SIZE, meminfo0.total);
  EXPECT_GT(meminfo0.total, meminfo0.free);

  // Validate allocation of test object and make sure it can be referenced
  // and all metadata looks correct.
  Reference block1 = allocator_->Allocate(sizeof(TestObject1), 1);
  EXPECT_NE(0U, block1);
  EXPECT_NE(nullptr, allocator_->GetAsObject<TestObject1>(block1, 1));
  EXPECT_EQ(nullptr, allocator_->GetAsObject<TestObject2>(block1, 1));
  EXPECT_LE(sizeof(TestObject1), allocator_->GetAllocSize(block1));
  EXPECT_GT(sizeof(TestObject1) + kAllocAlignment,
            allocator_->GetAllocSize(block1));
  PersistentMemoryAllocator::MemoryInfo meminfo1;
  allocator_->GetMemoryInfo(&meminfo1);
  EXPECT_EQ(meminfo0.total, meminfo1.total);
  EXPECT_GT(meminfo0.free, meminfo1.free);

  // Ensure that the test-object can be made iterable.
  PersistentMemoryAllocator::Iterator iter;
  uint32_t type;
  allocator_->CreateIterator(&iter);
  EXPECT_EQ(0U, allocator_->GetNextIterable(&iter, &type));
  allocator_->MakeIterable(block1);
  EXPECT_EQ(block1, allocator_->GetNextIterable(&iter, &type));
  EXPECT_EQ(1U, type);
  EXPECT_EQ(0U, allocator_->GetNextIterable(&iter, &type));

  // Create second test-object and ensure everything is good and it cannot
  // be confused with test-object of another type.
  Reference block2 = allocator_->Allocate(sizeof(TestObject2), 2);
  EXPECT_NE(0U, block2);
  EXPECT_NE(nullptr, allocator_->GetAsObject<TestObject2>(block2, 2));
  EXPECT_EQ(nullptr, allocator_->GetAsObject<TestObject2>(block2, 1));
  EXPECT_LE(sizeof(TestObject2), allocator_->GetAllocSize(block2));
  EXPECT_GT(sizeof(TestObject2) + kAllocAlignment,
            allocator_->GetAllocSize(block2));
  PersistentMemoryAllocator::MemoryInfo meminfo2;
  allocator_->GetMemoryInfo(&meminfo2);
  EXPECT_EQ(meminfo1.total, meminfo2.total);
  EXPECT_GT(meminfo1.free, meminfo2.free);

  // Ensure that second test-object can also be made iterable.
  allocator_->MakeIterable(block2);
  EXPECT_EQ(block2, allocator_->GetNextIterable(&iter, &type));
  EXPECT_EQ(2U, type);
  EXPECT_EQ(0U, allocator_->GetNextIterable(&iter, &type));

  // Check that iteration can begin after an arbitrary location.
  allocator_->CreateIterator(&iter, block1);
  EXPECT_EQ(block2, allocator_->GetNextIterable(&iter, &type));
  EXPECT_EQ(0U, allocator_->GetNextIterable(&iter, &type));

  // Ensure nothing has gone noticably wrong.
  EXPECT_FALSE(allocator_->IsFull());
  EXPECT_FALSE(allocator_->IsCorrupt());

  // Check the internal histogram record of used memory.
  allocator_->UpdateTrackingHistograms();
  scoped_ptr<HistogramSamples> used_samples(
      allocator_->used_histogram_->SnapshotSamples());
  EXPECT_TRUE(used_samples);
  EXPECT_EQ(1, used_samples->TotalCount());

  // Check the internal histogram record of allocation requests.
  scoped_ptr<HistogramSamples> allocs_samples(
      allocator_->allocs_histogram_->SnapshotSamples());
  EXPECT_TRUE(allocs_samples);
  EXPECT_EQ(2, allocs_samples->TotalCount());
  EXPECT_EQ(0, allocs_samples->GetCount(0));
  EXPECT_EQ(1, allocs_samples->GetCount(sizeof(TestObject1)));
  EXPECT_EQ(1, allocs_samples->GetCount(sizeof(TestObject2)));
#if !DCHECK_IS_ON()  // DCHECK builds will die at a NOTREACHED().
  EXPECT_EQ(0U, allocator_->Allocate(TEST_MEMORY_SIZE + 1, 0));
  allocs_samples = allocator_->allocs_histogram_->SnapshotSamples();
  EXPECT_EQ(3, allocs_samples->TotalCount());
  EXPECT_EQ(1, allocs_samples->GetCount(0));
#endif

  // Check that an objcet's type can be changed.
  EXPECT_EQ(2U, allocator_->GetType(block2));
  allocator_->SetType(block2, 3);
  EXPECT_EQ(3U, allocator_->GetType(block2));
  allocator_->SetType(block2, 2);
  EXPECT_EQ(2U, allocator_->GetType(block2));

  // Create second allocator (read/write) using the same memory segment.
  scoped_ptr<PersistentMemoryAllocator> allocator2(
      new PersistentMemoryAllocator(
          mem_segment_.get(), TEST_MEMORY_SIZE, TEST_MEMORY_PAGE, 0, "",
          false));
  EXPECT_EQ(TEST_ID, allocator2->Id());
  EXPECT_FALSE(allocator2->used_histogram_);
  EXPECT_FALSE(allocator2->allocs_histogram_);
  EXPECT_NE(allocator2->allocs_histogram_, allocator_->allocs_histogram_);

  // Ensure that iteration and access through second allocator works.
  allocator2->CreateIterator(&iter);
  EXPECT_EQ(block1, allocator2->GetNextIterable(&iter, &type));
  EXPECT_EQ(block2, allocator2->GetNextIterable(&iter, &type));
  EXPECT_EQ(0U, allocator2->GetNextIterable(&iter, &type));
  EXPECT_NE(nullptr, allocator2->GetAsObject<TestObject1>(block1, 1));
  EXPECT_NE(nullptr, allocator2->GetAsObject<TestObject2>(block2, 2));

  // Create a third allocator (read-only) using the same memory segment.
  scoped_ptr<const PersistentMemoryAllocator> allocator3(
      new PersistentMemoryAllocator(
          mem_segment_.get(), TEST_MEMORY_SIZE, TEST_MEMORY_PAGE, 0, "", true));
  EXPECT_EQ(TEST_ID, allocator3->Id());
  EXPECT_FALSE(allocator3->used_histogram_);
  EXPECT_FALSE(allocator3->allocs_histogram_);

  // Ensure that iteration and access through third allocator works.
  allocator3->CreateIterator(&iter);
  EXPECT_EQ(block1, allocator3->GetNextIterable(&iter, &type));
  EXPECT_EQ(block2, allocator3->GetNextIterable(&iter, &type));
  EXPECT_EQ(0U, allocator3->GetNextIterable(&iter, &type));
  EXPECT_NE(nullptr, allocator3->GetAsObject<TestObject1>(block1, 1));
  EXPECT_NE(nullptr, allocator3->GetAsObject<TestObject2>(block2, 2));
}

TEST_F(PersistentMemoryAllocatorTest, PageTest) {
  // This allocation will go into the first memory page.
  Reference block1 = allocator_->Allocate(TEST_MEMORY_PAGE / 2, 1);
  EXPECT_LT(0U, block1);
  EXPECT_GT(TEST_MEMORY_PAGE, block1);

  // This allocation won't fit in same page as previous block.
  Reference block2 =
      allocator_->Allocate(TEST_MEMORY_PAGE - 2 * kAllocAlignment, 2);
  EXPECT_EQ(TEST_MEMORY_PAGE, block2);

  // This allocation will also require a new page.
  Reference block3 = allocator_->Allocate(2 * kAllocAlignment + 99, 3);
  EXPECT_EQ(2U * TEST_MEMORY_PAGE, block3);
}

// A simple thread that takes an allocator and repeatedly allocates random-
// sized chunks from it until no more can be done.
class AllocatorThread : public SimpleThread {
 public:
  AllocatorThread(const std::string& name,
                  void* base,
                  uint32_t size,
                  uint32_t page_size)
      : SimpleThread(name, Options()),
        count_(0),
        iterable_(0),
        allocator_(base, size, page_size, 0, std::string(), false) {}

  void Run() override {
    for (;;) {
      uint32_t size = RandInt(1, 99);
      uint32_t type = RandInt(100, 999);
      Reference block = allocator_.Allocate(size, type);
      if (!block)
        break;

      count_++;
      if (RandInt(0, 1)) {
        allocator_.MakeIterable(block);
        iterable_++;
      }
    }
  }

  unsigned iterable() { return iterable_; }
  unsigned count() { return count_; }

 private:
  unsigned count_;
  unsigned iterable_;
  PersistentMemoryAllocator allocator_;
};

// Test parallel allocation/iteration and ensure consistency across all
// instances.
TEST_F(PersistentMemoryAllocatorTest, ParallelismTest) {
  void* memory = mem_segment_.get();
  AllocatorThread t1("t1", memory, TEST_MEMORY_SIZE, TEST_MEMORY_PAGE);
  AllocatorThread t2("t2", memory, TEST_MEMORY_SIZE, TEST_MEMORY_PAGE);
  AllocatorThread t3("t3", memory, TEST_MEMORY_SIZE, TEST_MEMORY_PAGE);
  AllocatorThread t4("t4", memory, TEST_MEMORY_SIZE, TEST_MEMORY_PAGE);
  AllocatorThread t5("t5", memory, TEST_MEMORY_SIZE, TEST_MEMORY_PAGE);

  t1.Start();
  t2.Start();
  t3.Start();
  t4.Start();
  t5.Start();

  unsigned last_count = 0;
  do {
    unsigned count = CountIterables();
    EXPECT_LE(last_count, count);
  } while (!allocator_->IsCorrupt() && !allocator_->IsFull());

  t1.Join();
  t2.Join();
  t3.Join();
  t4.Join();
  t5.Join();

  EXPECT_FALSE(allocator_->IsCorrupt());
  EXPECT_TRUE(allocator_->IsFull());
  EXPECT_EQ(CountIterables(),
            t1.iterable() + t2.iterable() + t3.iterable() + t4.iterable() +
            t5.iterable());
}

// This test doesn't verify anything other than it doesn't crash. Its goal
// is to find coding errors that aren't otherwise tested for, much like a
// "fuzzer" would.
// This test is suppsoed to fail on TSAN bot (crbug.com/579867).
#if defined(THREAD_SANITIZER)
#define MAYBE_CorruptionTest DISABLED_CorruptionTest
#else
#define MAYBE_CorruptionTest CorruptionTest
#endif
TEST_F(PersistentMemoryAllocatorTest, MAYBE_CorruptionTest) {
  char* memory = mem_segment_.get();
  AllocatorThread t1("t1", memory, TEST_MEMORY_SIZE, TEST_MEMORY_PAGE);
  AllocatorThread t2("t2", memory, TEST_MEMORY_SIZE, TEST_MEMORY_PAGE);
  AllocatorThread t3("t3", memory, TEST_MEMORY_SIZE, TEST_MEMORY_PAGE);
  AllocatorThread t4("t4", memory, TEST_MEMORY_SIZE, TEST_MEMORY_PAGE);
  AllocatorThread t5("t5", memory, TEST_MEMORY_SIZE, TEST_MEMORY_PAGE);

  t1.Start();
  t2.Start();
  t3.Start();
  t4.Start();
  t5.Start();

  do {
    size_t offset = RandInt(0, TEST_MEMORY_SIZE - 1);
    char value = RandInt(0, 255);
    memory[offset] = value;
  } while (!allocator_->IsCorrupt() && !allocator_->IsFull());

  t1.Join();
  t2.Join();
  t3.Join();
  t4.Join();
  t5.Join();

  CountIterables();
}

// Attempt to cause crashes or loops by expressly creating dangerous conditions.
TEST_F(PersistentMemoryAllocatorTest, MaliciousTest) {
  Reference block1 = allocator_->Allocate(sizeof(TestObject1), 1);
  Reference block2 = allocator_->Allocate(sizeof(TestObject1), 2);
  Reference block3 = allocator_->Allocate(sizeof(TestObject1), 3);
  Reference block4 = allocator_->Allocate(sizeof(TestObject1), 3);
  Reference block5 = allocator_->Allocate(sizeof(TestObject1), 3);
  allocator_->MakeIterable(block1);
  allocator_->MakeIterable(block2);
  allocator_->MakeIterable(block3);
  allocator_->MakeIterable(block4);
  allocator_->MakeIterable(block5);
  EXPECT_EQ(5U, CountIterables());
  EXPECT_FALSE(allocator_->IsCorrupt());

  // Create loop in iterable list and ensure it doesn't hang. The return value
  // from CountIterables() in these cases is unpredictable. If there is a
  // failure, the call will hang and the test killed for taking too long.
  uint32_t* header4 = (uint32_t*)(mem_segment_.get() + block4);
  EXPECT_EQ(block5, header4[3]);
  header4[3] = block4;
  CountIterables();  // loop: 1-2-3-4-4
  EXPECT_TRUE(allocator_->IsCorrupt());

  // Test where loop goes back to previous block.
  header4[3] = block3;
  CountIterables();  // loop: 1-2-3-4-3

  // Test where loop goes back to the beginning.
  header4[3] = block1;
  CountIterables();  // loop: 1-2-3-4-1
}


//----- LocalPersistentMemoryAllocator -----------------------------------------

TEST(LocalPersistentMemoryAllocatorTest, CreationTest) {
  LocalPersistentMemoryAllocator allocator(TEST_MEMORY_SIZE, 42, "");
  EXPECT_EQ(42U, allocator.Id());
  EXPECT_NE(0U, allocator.Allocate(24, 1));
  EXPECT_FALSE(allocator.IsFull());
  EXPECT_FALSE(allocator.IsCorrupt());
}


//----- FilePersistentMemoryAllocator ------------------------------------------

TEST(FilePersistentMemoryAllocatorTest, CreationTest) {
  ScopedTempDir temp_dir;
  ASSERT_TRUE(temp_dir.CreateUniqueTempDir());
  FilePath file_path = temp_dir.path().AppendASCII("persistent_memory");

  PersistentMemoryAllocator::MemoryInfo meminfo1;
  Reference r123, r456, r789;
  {
    LocalPersistentMemoryAllocator local(TEST_MEMORY_SIZE, TEST_ID, "");
    EXPECT_FALSE(local.IsReadonly());
    r123 = local.Allocate(123, 123);
    r456 = local.Allocate(456, 456);
    r789 = local.Allocate(789, 789);
    local.MakeIterable(r123);
    local.SetType(r456, 654);
    local.MakeIterable(r789);
    local.GetMemoryInfo(&meminfo1);
    EXPECT_FALSE(local.IsFull());
    EXPECT_FALSE(local.IsCorrupt());

    File writer(file_path, File::FLAG_CREATE | File::FLAG_WRITE);
    ASSERT_TRUE(writer.IsValid());
    writer.Write(0, (const char*)local.data(), local.used());
  }

  scoped_ptr<MemoryMappedFile> mmfile(new MemoryMappedFile());
  mmfile->Initialize(file_path);
  EXPECT_TRUE(mmfile->IsValid());
  const size_t mmlength = mmfile->length();
  EXPECT_GE(meminfo1.total, mmlength);

  FilePersistentMemoryAllocator file(mmfile.release(), 0, "");
  EXPECT_TRUE(file.IsReadonly());
  EXPECT_EQ(TEST_ID, file.Id());
  EXPECT_FALSE(file.IsFull());
  EXPECT_FALSE(file.IsCorrupt());

  PersistentMemoryAllocator::Iterator iter;
  uint32_t type;
  file.CreateIterator(&iter);
  EXPECT_EQ(r123, file.GetNextIterable(&iter, &type));
  EXPECT_EQ(r789, file.GetNextIterable(&iter, &type));
  EXPECT_EQ(0U, file.GetNextIterable(&iter, &type));

  EXPECT_EQ(123U, file.GetType(r123));
  EXPECT_EQ(654U, file.GetType(r456));
  EXPECT_EQ(789U, file.GetType(r789));

  PersistentMemoryAllocator::MemoryInfo meminfo2;
  file.GetMemoryInfo(&meminfo2);
  EXPECT_GE(meminfo1.total, meminfo2.total);
  EXPECT_GE(meminfo1.free, meminfo2.free);
  EXPECT_EQ(mmlength, meminfo2.total);
  EXPECT_EQ(0U, meminfo2.free);
}

TEST(FilePersistentMemoryAllocatorTest, AcceptableTest) {
  ScopedTempDir temp_dir;
  ASSERT_TRUE(temp_dir.CreateUniqueTempDir());
  FilePath file_path_base = temp_dir.path().AppendASCII("persistent_memory_");

  LocalPersistentMemoryAllocator local(TEST_MEMORY_SIZE, TEST_ID, "");
  const size_t minsize = local.used();
  scoped_ptr<char[]> garbage(new char[minsize]);
  RandBytes(garbage.get(), minsize);

  scoped_ptr<MemoryMappedFile> mmfile;
  char filename[100];
  for (size_t filesize = minsize; filesize > 0; --filesize) {
    strings::SafeSPrintf(filename, "memory_%d_A", filesize);
    FilePath file_path = temp_dir.path().AppendASCII(filename);
    ASSERT_FALSE(PathExists(file_path));
    {
      File writer(file_path, File::FLAG_CREATE | File::FLAG_WRITE);
      ASSERT_TRUE(writer.IsValid());
      writer.Write(0, (const char*)local.data(), filesize);
    }
    ASSERT_TRUE(PathExists(file_path));

    mmfile.reset(new MemoryMappedFile());
    mmfile->Initialize(file_path);
    EXPECT_EQ(filesize, mmfile->length());
    if (FilePersistentMemoryAllocator::IsFileAcceptable(*mmfile)) {
      // Just need to make sure it doesn't crash.
      FilePersistentMemoryAllocator allocator(mmfile.release(), 0, "");
      (void)allocator;  // Ensure compiler can't optimize-out above variable.
    } else {
      // For filesize >= minsize, the file must be acceptable. This
      // else clause (file-not-acceptable) should be reached only if
      // filesize < minsize.
      EXPECT_LT(filesize, minsize);
    }

#if !DCHECK_IS_ON()  // DCHECK builds will die at a NOTREACHED().
    strings::SafeSPrintf(filename, "memory_%d_B", filesize);
    file_path = temp_dir.path().AppendASCII(filename);
    ASSERT_FALSE(PathExists(file_path));
    {
      File writer(file_path, File::FLAG_CREATE | File::FLAG_WRITE);
      ASSERT_TRUE(writer.IsValid());
      writer.Write(0, (const char*)garbage.get(), filesize);
    }
    ASSERT_TRUE(PathExists(file_path));

    mmfile.reset(new MemoryMappedFile());
    mmfile->Initialize(file_path);
    EXPECT_EQ(filesize, mmfile->length());
    if (FilePersistentMemoryAllocator::IsFileAcceptable(*mmfile)) {
      // Just need to make sure it doesn't crash.
      FilePersistentMemoryAllocator allocator(mmfile.release(), 0, "") ;
      EXPECT_TRUE(allocator.IsCorrupt());  // Garbage data so it should be.
    } else {
      // For filesize >= minsize, the file must be acceptable. This
      // else clause (file-not-acceptable) should be reached only if
      // filesize < minsize.
      EXPECT_GT(minsize, filesize);
    }
#endif
  }
}

}  // namespace base
