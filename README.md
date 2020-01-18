# gbatoqr

<img src="https://user-images.githubusercontent.com/823043/72662760-4e9c6380-3a2e-11ea-859a-1cf0862881a1.png"> <img src="https://user-images.githubusercontent.com/823043/72662755-48a68280-3a2e-11ea-8aee-006d0c7217be.png" width="300">

This repository consists of the following two programs.

* A NDS program for dumping GBA ROMs as 2D codes (ndswide folder)
* A program that scans the 2D codes (detector folder)

## How to build the NDS program

devkitPro is required.

First, execute `./copy-submodule-files.sh` in the repository root directory.
Second, execute the `make` command in the ndswide directory.

## How to run the scanning program

In the repository root directory, execute the following command:
```
npm run dev
```

To build, execute the following command:
```
npm run build
```
