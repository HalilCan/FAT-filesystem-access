## Access
If you have a password, you can follow the accompanying instructions. If you don't, you can contact me. I cannot make even the modified code publicly available because a lot of the current work is based on PennOS and that would be detrimental to the class.

# FAT-fs
## 1. What
FAT-fs is a deployment-ready filesystem I built for my Operating Systems Architecture class at Penn. It is based on the FAT filesystem standard. We used it to build a rudimentary operating system called PennOS, the kernel and filesystem interface for which were written by my teammates. The version delivered for the class project was somewhat more advanced, as seen in the screenshot, but the core functionality remains the same.

![FAT filesystem in action](/images/fat.png)

### 1.a. Current State
FAT-fs is a complete filesystem with all the basic functionality an operating system could need. This primarily means indexing, creating, tracking, managing, and manipulating files, directories, and `file descriptors`. FAT-fs can also defragment files and has support for copy-on-write. Finally, the node structure allows for multiple directory depth, though the implementation required by the class called for a flat filesystem.

The documentation in `fatOperations.h`, I hear, is to die for.
  
---

## 2. Why
Homework! Also _fun_, but that came later and reports are questionable.

---

## 3. How
### 3.a. Implementation
A lot of system calls.

Since `fatOperations` is the backend that drives the filesystem, I'll discuss its functionality. Given a filesystem block (an empty but appropriately-sized file generated by `mkFlatFat`), `fatOperations` assumes it is the entirety of the partition. The first `FAT_WIDTH` bytes are reserved for the File Allocation Table, which tracks the linked physical locations of the logically contiguous blocks that make up all files.

In other words, `0-2-2-0-4` at the beginning of `FAT` would mean that:
- The first physical block `(0=0)` is occupied by the `root` directory file, and it ends in that block
- The second physical block `1=2` is the start of some file, which ends at the third physical block `2=2`
- The fourth physical block `3=0` is empty
- The fifth physical block `4=4` stores a file that ends in that block.

The `directory file` tracks the properties such as the names and starting physical block addresses of all files in that directory, including other directory files.

When files are being read or written, that file's physical block links are followed through `FAT`. During writing, it is important to keep track of how many bytes to add within a block, which depends on remaining bytes in input and free bytes remaining in that block. Applying this logic recursively allows for us to read/write arbitrarily-sized files in FAT-fs. Finally, when blocks are modified in place or otherwise moved, we check if it is necessary to update `FAT` or the `directory file`.

`File descriptors` are made available at runtime. At its simplest, their structure is similar to the (LinkedList) `node` structure that the `directory files` use, but integer-to-`fileNode` mappings are provided to serve as `file descriptors`.

### 3.b. Usage
While `fatOperations` is where everything important happens, its back-end nature can hide a lot of things. The four other `.c` files thus serve as an interface to run the filesystem by hand.
- Compile with `make remake`. To see the inner workings of the code in action, make sure `DEBUG_MODE` is not commented out at the top of `fatOperations.h`
- mkFlatFat.c:
  - `mkFlatFat [filesystemName]` will create a FAT-based filesystem named `filesystemName` in current directory. You can change its size by changing `BLOCK_SIZE` in `fatOperations.h`.
- catFlatFat.c:
  - `catFlatFat [filesystem] [file name] [one flag]` for all operations. The flags:
    - `-r`: Read `file name` and output to `STD_OUT`
    - `-w`: Write into `file name`. You will be asked for input.
    - `-a`: Append to `file name`. You will be asked for input.
    - `-d`: Delete `file name`.
    - `-t`: Tail `file name`; write the trailing bytes to `STD_OUT`
    - `-f`: Defrag `file name`. This will make sure all logical blocks that make up the `file name` are as physically contiguous as possible.
- lsFlatFat.c:
  - `lsFlatFat [filesystemName]` will run the equivalent of `ls` on the root directory of the filesystem; listing file names, sizes, and block information.
- defragFlatFat.c:
  - `defragFlatFat [filesystemName]` will defragment the entire filesystem in an optimized manner. The optimization is implicit in that "latter" files(starting at a larger physical block) are more likely to be fragmented, thus following that chain back ensures fastest defragmentation.

