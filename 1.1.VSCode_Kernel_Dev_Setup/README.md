# Setting up Visual Studio Code for Kernel Modules Developments
## Short Story
First thing, I'm a Windows guy, and i have been coding in Visual Studio since 7 years, never really used any other IDE. <br>Since it's hard to code in linux kernel with VS i had to find another way (VisualKernel didnt work out well for me) and using linux as my main OS was not an option. I had to dump Visual Studio this time :/, and went with her sister Visual Studio Code (it felt like cheating on her)<br>
VSCode allows you to do remote development through ssh, now i dont have to use linux to code on it (smart move Microsoft, no one can beat you with your development tools)
## How To Set it Up
The only thing you need to do for remote development is to install "Remote - SSH" extention.<br>
Check out configuration files to make intillisense work well with your modules.<br>
I have used Ubuntu 18.04 for this, so you need to download the kernel headers of your disto and change the corresponding paths in config files.<br>