# Setting up Visual Studio Code for Kernel Modules Developments
## Short Story
First thing,I'm a windows guy, and i have been coding with visual studio since 7 years, never used any other IDE. since it's hard to code in linux kernel with it i had to find another way (VisualKernel didnt work out well for me) and using linux as my main OS was not an option. I had to dump Visual Studio this time :/, and went with her sister Visual Studio Code (it felt like cheating on her)
VSCode allows you to do remote development through ssh, now i dont have to use linux code on it (smart move Microsoft, no one can beat it with its development tools)
## How To Set it Up
The only thing you need to do for remote development is to install "Remote - SSH" extention.
Check out configiration files to make intillisense work well with you modules.
I have used Ubuntu 18.04 for this, so you need to download the kernel headers of you disto and change the corresponding paths in config files.