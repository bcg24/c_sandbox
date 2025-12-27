# Linux cheatsheet

## Launching WSL:
in cmd CLI: wsl
cd ~/cSandbox
code . to open in VSCode

## Basic File Commands

** ls: list the names of the contents of a directory
* options: -l: long listing
linux is case-sensitive

** more : view the contents of a file
* e.g. more cobras
Called more because at the bottom of the list, there's a "--More--"
Hit the spacebar to see the rest of the list

** mkdir : make a directory

** mv : move file A to directory B 
* e.g. mv A B
Renames a file or directory by moving it to a directory with a new name

** cd : change directory
* cd .. : goes to the parent directory

** pwd : print working directory/where you are
* / : root directory
Relative pathname is relative to current working directory
* e.g. pwd = /animals > mv cats/tigers cats/siberians
If we weren't in animals, we could perform this from directory "dogs" with ../cats/tigers etc.

** cp : copy file to directory
* e.g. cp baseFile targetDir

** rm : remove file

** rmdir : remove directory

## Security & Permissions
 
 Using the long string option "-l" of "ls", we can see:
 * "-" for file, "d" for directory
 
1. Security Modes:
	** r: read permission
	** w: write permission
	** x: execute permission (only for programs)
	There are three sets of these permssions for owners, group, any user (world)
	
2. Owner of the file
3. Group owner of the file
4. Size of the file in bytes
5. Date last modified
6. File name
 
 ** chmod : change mode/modify security permissions
 * args: 
	u/g/o : (user/group/other) indicates which of the three sets to modify
	+ : adding a permisison
* e.g. chmod o+x file
 
### Wildcards
Select multiple files at a time with "*" and "?"
"*ing" would select all files ending in "ing"
Specify "*" as the filename to execute a command on all files in current directory
"?" matches exactly one character
e.g. want to match "sport", but not "spat"
Use "sp??t". The first '?' matches the 'a' in "spat", but the second '?' can't match anything, so "spat" fails.
 
 Can only assign one group per file or director
 Can use Access Control Lists if this functionality needs to be expanded
 
 ** groups : returns the groups you are part of
 

 