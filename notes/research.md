# Research notes: NTFS, APFS, FAT32 vs. our file system

The Oct project assignment asks: what file system does a Mac use (APFS), what does a PC use (NTFS), what does a flash drive use (FAT32/exFAT), how they compare, and what gets lost when you copy a file between them.
Facts below are from general knowledge. Check them against the textbook (Ch. 14) or official docs before they go into the final report.

## Our file system (Week 14 spec)
- Disk: 100 sectors x 512 bytes, held in an in-memory array (`union block sector[100]`)
- Allocation: **linked**. Each data sector holds 504 bytes plus `backf`/`frwdf` links (8 bytes of overhead per sector, about 1.6%)
- Free space: **linked list of free sector numbers** (`buildList` / `Remove` / `Insert`). The free list sits outside the disk, in malloc'd nodes
- Directories: a tree. 25 entries per directory sector, and a directory can chain more sectors with `back`/`frwd`
- Metadata per file: type, 8-char name, first sector, size (a `short`, so 32767 bytes max)
- Weakness: random access (SEEK) has to walk the chain from the start. One bad link loses the rest of the file
- No permissions, timestamps, journaling, or crash recovery

## FAT32 (flash drives, SD cards)
- Allocation: **linked, but the links live in a table (the FAT)**, not inside the data blocks. Entry *n* holds the next cluster after cluster *n*. An end-of-chain marker ends the file
- This is the same idea as ours. The difference is that the links sit together in one table, which can be cached, so seeking doesn't have to read every data block
- Free space: a FAT entry of 0 means the cluster is free. There's no separate bitmap or list. The FSInfo sector keeps a free-cluster count and a "next free" hint
- Directory entry: 8.3 name (long names stored in extra entries), first cluster, 32-bit size
- Limits: 4 GB max file size. No permissions or ownership. No journaling
- Used on flash drives because almost every OS can read and write it. exFAT is the newer replacement without the 4 GB limit

## NTFS (Windows PCs)
- Metadata: the Master File Table (MFT). Every file has an MFT record (usually 1 KB) holding its attributes. Very small files fit entirely inside the record
- Allocation: **extents ("data runs")**, i.e. (start cluster, length) pairs. Closer to contiguous/indexed allocation than to linked allocation
- Free space: a **bitmap** stored as a file named `$Bitmap`, one bit per cluster. Our original proposal used a bitmap; the Week 14 spec swapped it for a linked list
- Directories: B+ trees indexed by name, so lookup is faster than our linear search
- Has journaling (`$LogFile`), ACL permissions, alternate data streams, compression, and encryption

## APFS (macOS since 10.13, iOS)
- Copy-on-write: changed data goes to new blocks and is never overwritten in place, so a crash can't leave a half-written file
- Allocation: **extents**, tracked in B-trees
- Free space: a space manager that uses **bitmaps** for the container's blocks. Several volumes in one container share the same free space
- Snapshots and clones: copying a file on the same volume shares its blocks until one copy changes
- Built for SSDs. Has native encryption and nanosecond timestamps

## Comparison at a glance
| | Ours | FAT32 | NTFS | APFS |
|---|---|---|---|---|
| Allocation | linked (links in block) | linked (links in FAT table) | extents | extents, copy-on-write |
| Free space | linked list | 0 entries in FAT | bitmap | bitmap (space manager) |
| Directory | linear, 25 per sector | linear | B+ tree | B-tree |
| Crash safety | none | none | journal | copy-on-write |
| Max file | 32767 B | 4 GB | huge | huge |

## What's lost when copying between file systems
- APFS/NTFS to FAT32: files over 4 GB won't copy. Permissions/ownership and ACLs are lost. Timestamps lose precision (FAT stores modified time to 2 seconds). Names with characters FAT doesn't allow are rejected
- NTFS to anything else: alternate data streams are dropped (for example the "downloaded from internet" mark)
- macOS to FAT32/exFAT: extended attributes and resource forks get saved as hidden `._filename` files, which show up as clutter on Windows
- Case: APFS (default) and NTFS ignore case but keep it. Linux ext4 is case-sensitive, so `A.txt` and `a.txt` can collide when copied over
- The file's bytes themselves copy fine. What gets lost is the metadata the destination has no place to store
