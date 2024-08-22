# UsMInt
Short for User Minimalist Interface. Or maybe United States Mint. Alas, this project cannot print money.

This is a C++ UI library designed to work with Vulkan, agnostic of the program's implementation of texture handling, command buffer recording, etc. This allows for optimizations for your specific program's structuring and handling of such objects and events.

### Building
A generalized CMake file has yet to be made, but if you'd like to compile it yourself without one just compile `UI.h` and `UI.cpp` and link against the most current versions of Vulkan, Freetype, and BZ2. You can use CMake's `add_library` to compile to a `.a` file. Then simply include `UI.h` in your project and link the `.a` you compiled!
Until a CMake file is added to this repository, feel free to reach out to Danp1140 with any compilation questions.

### Usage 

Like many UI libraries, you're gonna need to make a lot of callback functions. To allow interfacing between your Vulkan implementation and the UI code, you use `UIComponent::setDefaultDrawFunc`, `UIText::setTexLoadFunc`, and `UIText::setTexDestroyFunc`. The setup can become sizeable so I recommend writing your own UI Handler object to contain it all. From there you can instantiate UI components and use their methods. Just make sure to call `draw()` on every top-most `UIComponent` in your draw loop (i.e., all the `UIComponent`s you have that do not have a parent).
