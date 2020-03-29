# Setting up Visual Studio Code for Kernel Modules Developments
You can easily use remote VSCode to an ARM processor linux but C/C++ intellisense wont work.<br>
The best solution i could think of is setting up remote VSCode to a WSL and build the modules of ARM linux using the cross compiler, then use scp to transfer the output module from WSL to the ARM linux.