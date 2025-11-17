This repository contains some bare metal stuff to do with the MilkV Duo 256M (Sophgo SG2002) 
This is to booted using bootelf from U-Boot in Supervisor mode
Load the ELF at 0x81000000, bootelf relocates it to 0x80200000 and executes
