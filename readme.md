# What is this?
This is a very basic Vulkan library for displaying graphics. Vulkan is an industry-standard graphics API that has the potential to be extremely fast at rendering 3d objects on a monitor. In this project, I only was able to render a couple squares, but behind the scenes there are tons of things that have to happen to get that far.

Most libraries don't bother with rendering with multiple windows, especially in Vulkan, but from the beginning I made sure that I could do everything on multiple windows. This makes it significantly more difficult to write.
![test](https://user-images.githubusercontent.com/45665232/118214556-8d310b00-b435-11eb-8e63-4e43fe5dc696.png)

# Installation
On the right side of this github reposoitory, click the "releases" page and download and unzip the "Volcano.zip" file, and double click the "Volcano.exe" file.
# Setup
```
git clone https://github.com/davidmedin/Volcano
cd Volcano
git submodule update --init --recursive
build.bat
```
You might need to move glfw3.dll into bin/debug/ btw
